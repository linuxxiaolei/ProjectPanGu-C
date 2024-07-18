#include <All_Definition.h>
#include <AppCury.h>

// st_axis *list_pointer_to_Axes[NUMBER_OF_AXES];
st_axis Axis_1;
st_axis *Axis;
int axisCnt = 0;
double target_tick = 0.0;

// ====为了同时运行两台电机，增加的另一份控制结构体
#if NUMBER_OF_AXES == 2
st_axis Axis_2;
#define get_Axis_CTRL_pointers \
    if (axisCnt == 0)          \
    {                          \
        Axis = &Axis_1;        \
        CTRL = &CTRL_1;        \
    }                          \
    if (axisCnt == 1)          \
    {                          \
        Axis = &Axis_2;        \
        CTRL = &CTRL_2;        \
    }
#else
// do nothing here (use same codes as simulation)
void get_Axis_CTRL_pointers(int axisCnt, st_axis *pAxis, struct ControllerForExperiment *pCTRL)
{
    // if(axisCnt == 0)
    {
        pAxis = &Axis_1;
        pCTRL = &CTRL_1;
    }
}
#endif


// this offset is moved to ACMconfig.h
// #define OFFSET_COUNT_BETWEEN_ENCODER_INDEX_AND_U_PHASE_AXIS 2333 // cjh tuned with id_cmd = 3A 2024-01-19


#define NO_POSITION_CONTROL 0
#define TWOMOTOR_POSITION_CONTROL 1
#define SINGLE_POSITION_CONTROL 2
#define SHANK_LOOP_RUN 3
#define HIP_LOOP_RUN 4
#define BOTH_LOOP_RUN 5
#define IMPEDANCE_CONTROL 6
int positionLoopType = 0;           // TWOMOTOR_POSITION_CONTROL; //SHANK_LOOP_RUN; // SHANK_LOOP_RUN; //BOTH_LOOP_RUN;
int use_first_set_three_phase = -1; //-1 for both motors





#define VL_CL_DOWNSAMPLE 40
Uint16 downSampleCounter = 0;

#define SHANK_POS_CMD_FLAG 0
#define HIP_POS_CMD_FLAG 1

// Uint64 mainWhileLoopCounter1 = 0;
// int64 mainWhileLoopCounter2 = 0;

// int  FLAG_ENABLE_PWM_OUTPUT = FALSE;

//// CLA
//// header files
////#include "DCLCLA.h"
////#include "CLA_shared.h"
////#include "F2837xD_Cla.h"                // Control Law Accelerator Registers
////#include "F2837xD_Cla_defines.h"              // Macros used for CLA examples.
//
//// function prototypes
////interrupt void control_Isr(void);
//
//// global  variables
// long IdleLoopCount = 0;
// long IsrCount = 0;
// float Duty;
//
//// shared variables
// #pragma DATA_SECTION(rk, "CpuToCla1MsgRAM")
// #pragma DATA_SECTION(yk, "CpuToCla1MsgRAM")
// #pragma DATA_SECTION(uk, "Cla1ToCpuMsgRAM")
// float rk = 0.25f;
// float yk;
// float uk;
//
// #pragma DATA_SECTION(pi1, "Cla1DataRam1")
// DCL_PI_CLA pi1 = PI_CLA_DEFAULTS;

// CLA end
void main(void)
{

    // CYM Codes
    //    A.x = 0.0;
    //    A.y = 0.0;
    //    D.x = 51.662e-3;
    //    D.y = 0.0;
    //    o.x = 110.76e-3;
    //    o.y = 192.03e-3;
    //    a_length = 80e-3;
    //    b_length = 139.98e-3;
    //    c_length = 153.069e-3;
    //    offset_length = 97.96e-3;
    //    lead = 16e-3;
    //    joint_offset =(147.6585661123+175.45166) / 180.0 *M_PI;

    InitSysCtrl();      // 1. Initialize System Control: PLL, WatchDog, enable Peripheral Clocks.
    Gpio_initialize();  // 2. Initialize GPIO and assign GPIO to peripherals.
    DINT;               // 3.1 Clear all interrupts and initialize PIE vector table.
    InitPieCtrl();      // 3.2 Initialize the PIE control registers to their default state. The default state is all PIE interrupts disabled and flags are cleared.
    IER = 0x0000;       // 3.3 Disable CPU __interrupts,
    IFR = 0x0000;       // 3.4 and clear all CPU __interrupt flags.
    InitPieVectTable(); // 3.5 Initialize the PIE vector table with pointers to the shell Interrupt Service Routines (ISR). At end, ENPIE = 1.

    // for Slessinv TIE.R1 for measuring the execution time
    InitCpuTimers();
    ConfigCpuTimer(&CpuTimer1, 200, 1000000); // 200MHz, INTERRUPT_period = 1e6 us

// 4.1 IPC
#if NUMBER_OF_DSP_CORES == 2
    IPCBootCPU2(C1C2_BROM_BOOTMODE_BOOT_FROM_FLASH);
#endif
#ifdef _STANDALONE
#ifdef _FLASH
    // 当你需要离线断电再上电运行时用这个：
    //  Send boot command to allow the CPU02 application to begin execution
    IPCBootCPU2(C1C2_BROM_BOOTMODE_BOOT_FROM_FLASH);
#else
    //  Send boot command to allow the CPU02 application to begin execution
    // 这句话我不知道什么意义，可能还是不要比较好。
    // IPCBootCPU2(C1C2_BROM_BOOTMODE_BOOT_FROM_RAM);
#endif
#endif


// 4.2 Initialize peripherals
ePWM_initialize();
ADC_initialize();
eQEP_initialize(0);
InitECaptureContinuousMode();


// 4.3 Assign peripherals to CPU02
/* SPI and SCI */
#if NUMBER_OF_DSP_CORES == 1
    // 同步！！！！！
    InitHighSpeedSpiGpio();
    // InitSpiaGpio();
    // InitSpicGpio();
    InitSpi();

    InitSciGpio();
    InitSci();
#elif NUMBER_OF_DSP_CORES == 2
    /* 双核配置*/
    init_spi();

    EUREKA_GPIO_SETUP();
    //        // =========TEST BOARD PIN============
    //        // =========NOT FOR EUREKA===========

    // 异步！！！！！
    // InitSci(); // this is moved to CPU02

    // 在此之前，已经把GPIO和外设的权限转给CPU2了。
    // 这里再把部分共享内存权限给CPU2，同时告诉CPU2，你可以继续运行代码了。
    while (!(MemCfgRegs.GSxMSEL.bit.MSEL_GS0))
    {
        EALLOW;
        // Give Memory Access to GS0/ GS14 SARAM to CPU02
        MemCfgRegs.GSxMSEL.bit.MSEL_GS0 = 1;
        EDIS;
    }
#endif

    // 4.4 Initialize algorithms
    get_bezier_points();

    //axis_basic_setup(axisCnt);
    for (axisCnt = 0; axisCnt < NUMBER_OF_AXES; axisCnt++)
    {
        get_Axis_CTRL_pointers //(axisCnt, Axis, CTRL);
        axis_basic_setup(axisCnt);
    }
    //axisCnt = 1;

    // 5. Handle Interrupts
    handle_interrupts();

    // 6. Pre main loop
    DSP_STOP_LED1
    DSP_STOP_LED2
    DSP_PWM_DISABLE
    DSP_2PWM_DISABLE
/* Test IPC to CPU02 */
#if NUMBER_OF_DSP_CORES == 2
    test_ipc_tocpu02();
#endif

    // 7. CLA
    /* initialise PI controller */
    pi1.Kp = 5.5f;
    pi1.Ki = 0.015f;
    pi1.i10 = 0.0f;
    pi1.i6 = 1.0f;
    pi1.Umax = 10.2f;
    pi1.Umin = -10.2f;

    /* compute CLA task vectors */
    EALLOW;
    compute_CLA_task_vectors();

    /* CLA task triggers */
    Cla1Regs.MIER.all = 0x00FF;

    /* Switch the CLA program space to the CLA and enable software forcing
     * Also switch over CLA data ram 0,1 and 2
     * CAUTION: The RAMxCPUE bits can only be enabled by writing to the register
     * and not the individual bit field. Furthermore, the status of these bitfields
     * is not reflected in either the watch or register views - they always read as
     * zeros. This is a known bug and the user is advised to test CPU accessibilty
     * first before proceeding
     */
    Cla1Regs.MCTL.bit.IACKE = 1;
    EDIS;


    // 8. Main loop
    main_loop();
}

/* Below is moved from PanGuMainISR.c */
#if SYSTEM_PROGRAM_MODE == 223


// extern long long sci_pos;



void measurement()
{

    CTRL->enc->encoder_abs_cnt_previous = CTRL->enc->encoder_abs_cnt;

    if (axisCnt == 0) measurement_position_count_axisCnt0();
    if (axisCnt == 1) measurement_position_count_axisCnt1();
    
    measurement_enc_and_i();

    if (axisCnt == 0) measurement_current_axisCnt0();
    if (axisCnt == 1) measurement_current_axisCnt1();

    // 只用第一套三相
    IS_C(0) = Axis->iabg[0];
    IS_C(1) = Axis->iabg[1];
    (*CTRL).i->iab[0] = Axis->iabg[0];
    (*CTRL).i->iab[1] = Axis->iabg[1];

    US_C(0) = (*CTRL).o->uab_cmd[0]; // 后缀_P表示上一步的电压，P = Previous
    US_C(1) = (*CTRL).o->uab_cmd[1]; // 后缀_C表示当前步的电压，C = Current

    US_P(0) = US_C(0);
    US_P(1) = US_C(1);

    // 线电压测量（基于占空比和母线电压）
    // voltage_measurement_based_on_eCAP();
    // int axisCnt = 0;

    {
        // pAxis = list_pointer_to_Axes[axisCnt];

        // Vdc用于实时更新电流环限幅
        PID_iD->OutLimit = Axis->vdc * 0.5773672;
        PID_iQ->OutLimit = Axis->vdc * 0.5773672;

        PID_iX->outLimit = Axis->vdc * 0.5773672;
        PID_iY->outLimit = Axis->vdc * 0.5773672;
    }

    //    这样不能形成保护，必须设置故障状态才行。
    //    if(fabs(G.Current_W)>8 || fabs(G.Current_V)>8){
    //        DSP_PWM_DISABLE
    //        DSP_2PWM_DISABLE
    //    }

    // 电流采样ADC温飘校准 // TODO 改成用ADC Raw Results校准。
    //    if(Axis->AD_offset_flag2==FALSE)
    //    {
    //        Axis->offset_counter += 1;
    //        Axis->iuvw_offset_online[0] += (REAL)(AdcaResultRegs.ADCRESULT1 ) ;
    //        Axis->iuvw_offset_online[1] += (REAL)(AdcaResultRegs.ADCRESULT2 ) ;
    //        Axis->iuvw_offset_online[2] += (REAL)(AdcaResultRegs.ADCRESULT3 ) ;
    //        Axis->iuvw_offset_online[3] += (REAL)(AdcaResultRegs.ADCRESULT11 ) ;
    //        Axis->iuvw_offset_online[4] += (REAL)(AdcaResultRegs.ADCRESULT9 ) ;
    //        Axis->iuvw_offset_online[5] += (REAL)(AdcaResultRegs.ADCRESULT8 ) ;
    //        if(Axis->offset_counter>=5000){
    //            Axis->iuvw_offset_online[0] = Axis->iuvw_offset_online[0] / 5000;
    //            Axis->iuvw_offset_online[1] = Axis->iuvw_offset_online[1] / 5000;
    //            Axis->iuvw_offset_online[2] = Axis->iuvw_offset_online[2] / 5000;
    //            Axis->iuvw_offset_online[3] = Axis->iuvw_offset_online[3] / 5000;
    //            Axis->iuvw_offset_online[4] = Axis->iuvw_offset_online[4] / 5000;
    //            Axis->iuvw_offset_online[5] = Axis->iuvw_offset_online[5] / 5000;
    //            Axis->AD_offset_flag2 = TRUE;
    //            Axis->offset_counter = 0;
    //        }
    //
    //        // 来不及完成偏置检测（比如刚上电数字开关就是开的），采用默认值
    //        /* 427-1401：添加开关信号滤波。今天发现在刚上电的时候，XCUBE-II的前两个中断里，数字开关是打开的，然后才变成关闭。*/
    //        if(Axis->FLAG_ENABLE_PWM_OUTPUT && Axis->offset_counter>100){
    //            Axis->iuvw_offset_online[0] = 0.0;
    //            Axis->iuvw_offset_online[1] = 0.0;
    //            Axis->iuvw_offset_online[2] = 0.0;
    //            Axis->iuvw_offset_online[3] = 0.0;
    //            Axis->iuvw_offset_online[4] = 0.0;
    //            Axis->iuvw_offset_online[5] = 0.0;
    //            Axis->AD_offset_flag2 = TRUE;
    //        }
    //
    //        // 上电的时候，电机可能在转，此时根据电流判断是否还要额外进行偏置补偿。
    //        if( fabs(Axis->iuvw[0])>0.05 || fabs(Axis->iuvw[1])>0.05 || fabs(Axis->iuvw[2])>0.05 || \
    //            fabs(Axis->iuvw[3])>0.05 || fabs(Axis->iuvw[4])>0.05 || fabs(Axis->iuvw[5])>0.05){
    //            Axis->iuvw_offset_online[0] = 0.0;
    //            Axis->iuvw_offset_online[1] = 0.0;
    //            Axis->iuvw_offset_online[2] = 0.0;
    //            Axis->iuvw_offset_online[3] = 0.0;
    //            Axis->iuvw_offset_online[4] = 0.0;
    //            Axis->iuvw_offset_online[5] = 0.0;
    //            Axis->AD_offset_flag2 = TRUE;
    //        }
    //    }
}

REAL call_position_loop_controller(int positionLoopType)
{
    CTRL_1.s->spd->Kp = TEST_SHANK_SPD_KP;
    CTRL_1.s->spd->Ki = TEST_SHANK_SPD_KI;

    CTRL_2.s->spd->Kp = TEST_HIP_SPD_KP;
    CTRL_2.s->spd->Ki = TEST_HIP_SPD_KI;


    CTRL_1.s->pos->Kp = TEST_SHANK_KP;
    CTRL_2.s->pos->Kp = TEST_HIP_KP;

    CTRL_1.s->pos->OutLimit = TEST_SHANK_POS_OUTLIMIT;
    CTRL_2.s->pos->OutLimit = TEST_HIP_POS_OUTLIMIT;

    run_iecon_main((*CTRL).timebase_counter);

    if (positionLoopType == TWOMOTOR_POSITION_CONTROL) control_two_motor_position();

    else if (positionLoopType == SINGLE_POSITION_CONTROL) control_single_motor_position();

    else if (positionLoopType == SHANK_LOOP_RUN) run_shank_loop();

    else if (positionLoopType == HIP_LOOP_RUN) run_hip_loop();

    else if (positionLoopType == BOTH_LOOP_RUN) run_both_loop();

    else if (positionLoopType == IMPEDANCE_CONTROL) run_impedance_control();


    return Axis->Set_manual_rpm;
}

// int down_freq_ecap_counter = 1;
// Uint64 timebase_counter = 0;
extern REAL imife_realtime_gain_off;

void PanGuMainISR(void)
{

    // 采样，包括DSP中的ADC采样等
    // DELAY_US(2); // wait for adc conversion TODO: check adc eoc flag?
    measurement();

    if (!Axis_1.FLAG_ENABLE_PWM_OUTPUT) DISABLE_PWM_OUTPUT();
    // TODO:需要增加让另外一项axis的Ta Tb Tc在不使用或者

    else ENABLE_PWM_OUTPUT(positionLoopType);

}

Uint64 EPWM1IntCount = 0;
__interrupt void EPWM1ISR(void)
{
    EPWM1IntCount += 1;

    //    yk = Axis_1.iuvw[0];
    //
    //    // trigger PI controller on CLA
    //    EALLOW;
    //    Cla1ForceTask3andWait();

#if USE_ECAP_CEVT2_INTERRUPT == 1
    CAP.password_isr_nesting = 178; // only if you can stop EPWM ISR, or else you won't know the value of password_isr_nesting.
    /* Step 1. [eCAP] Set the global priority */
    // Set global priority by adjusting IER, so as to allow PIE group 4 to send interrupt flag to CPU stage.
    IER |= M_INT4; // Modify IER to allow CPU interrupts from PIE group 4 to be serviced. Part 1
    IER &= M_INT4; // Modify IER to allow CPU interrupts from PIE group 4 to be serviced. Part 2

    /* Step 2. [eCAP] Set the group priority */
    uint16_t TempPIEIER4;
    TempPIEIER4 = PieCtrlRegs.PIEIER4.all; // Save PIEIER register for later
    PieCtrlRegs.PIEIER4.all &= 0x7;        // Set group priority by adjusting PIEIER4 to allow INT4.1, 4.2, 4.3 to interrupt current ISR

    /* Step 3. [eCAP] Enable interrupts */
    PieCtrlRegs.PIEACK.all = 0xFFFF; // Enable PIE interrupts by writing all 1’s to the PIEACK register
    asm("       NOP");               // Wait at least one cycle
    EINT;                            // Enable global interrupts by clearing INTM
#endif

    if (GpioDataRegs.GPCDAT.bit.GPIO75 == 1)
    {
        Axis_1.FLAG_ENABLE_PWM_OUTPUT = 1;
    }
    else if (GpioDataRegs.GPCDAT.bit.GPIO75 == 0)
    {
        Axis_1.FLAG_ENABLE_PWM_OUTPUT = 0;
    }

/* Step 4. [ePWM] Execute EPWM ISR */
// 只需要做一次的
#if NUMBER_OF_DSP_CORES == 2
    write_DAC_buffer();
#endif

    // 出厂底板，点灯代码
    //    static long int ii = 0;
    //    if(++ii%5000 == 0){
    //        //        EPwm1Regs.CMPA.bit.CMPA = (*CTRL).svgen1.Ta*50000000*CL_TS;
    //        //        EPwm2Regs.CMPA.bit.CMPA = (*CTRL).svgen1.Tb*50000000*CL_TS;
    //        //        EPwm3Regs.CMPA.bit.CMPA = (*CTRL).svgen1.Tc*50000000*CL_TS;
    //        //        EPwm4Regs.CMPA.bit.CMPA = (*CTRL).svgen2.Ta*50000000*CL_TS;
    //        //        EPwm5Regs.CMPA.bit.CMPA = (*CTRL).svgen2.Tb*50000000*CL_TS;
    //
    //        if(EPwm1Regs.CMPA.bit.CMPA==5000){
    //            EPwm1Regs.CMPA.bit.CMPA = 0;
    //        }else{
    //            EPwm1Regs.CMPA.bit.CMPA = 5000;
    //        }
    //    }
    //
    //    static long int jj = 0;
    //    if(++jj%10000 == 0){
    //        if(EPwm2Regs.CMPA.bit.CMPA==5000){
    //            EPwm2Regs.CMPA.bit.CMPA = 0;
    //        }else{
    //            EPwm2Regs.CMPA.bit.CMPA = 5000;
    //        }
    //    }
    //    return;

#if ENABLE_ECAP
    do_enhanced_capture();
#endif

// read from CPU02
    read_count_from_cpu02_dsp_cores_2();

    // 对每一个CTRL都需要做一次的代码
    if (use_first_set_three_phase == -1)
    {
        for (axisCnt = 0; axisCnt < NUMBER_OF_AXES; axisCnt++)
        {
            get_Axis_CTRL_pointers //(axisCnt, Axis, CTRL);
            PanGuMainISR();
        }
        axisCnt = 1;
    }
    else if (use_first_set_three_phase == 1)
    {
        axisCnt = 0;
        get_Axis_CTRL_pointers //(axisCnt, Axis, CTRL);
        PanGuMainISR();
    }
    else if (use_first_set_three_phase == 2)
    {
        axisCnt = 1;
        get_Axis_CTRL_pointers //(axisCnt, Axis, CTRL);
        PanGuMainISR();
    }

#if USE_ECAP_CEVT2_INTERRUPT == 1
    /* Step 5. [eCAP] Disable interrupts */
    DINT;

    /* Step 6. [eCAP] Restore the PIEIERx register */
    PieCtrlRegs.PIEIER4.all = TempPIEIER4;
    CAP.password_isr_nesting = 0;
#endif

    /* Step 7. [ePWM] Exit EPWM1 ISR */
    EPwm1Regs.ETCLR.bit.INT = 1;
    PieCtrlRegs.PIEACK.all |= PIEACK_GROUP3;
}

#endif

/*
//声明全局变量
#if PC_SIMULATION==FALSE
REAL CpuTimer_Delta = 0;
Uint32 CpuTimer_Before = 0;
Uint32 CpuTimer_After = 0;
#endif


//这段放需要测时间的代码前面
#if PC_SIMULATION==FALSE
EALLOW;
CpuTimer1.RegsAddr->TCR.bit.TRB = 1; // reset cpu timer to period value
CpuTimer1.RegsAddr->TCR.bit.TSS = 0; // start/restart
CpuTimer_Before = CpuTimer1.RegsAddr->TIM.all; // get count
EDIS;
#endif

//这段放需要测时间的代码后面前面，观察CpuTimer_Delta的取值，代表经过了多少个 1/200e6 秒。
#if PC_SIMULATION==FALSE
CpuTimer_After = CpuTimer1.RegsAddr->TIM.all; // get count
CpuTimer_Delta = (REAL)CpuTimer_Before - (REAL)CpuTimer_After;
// EALLOW;
// CpuTimer1.RegsAddr->TCR.bit.TSS = 1; // stop (not needed because of the line TRB=1)
// EDIS;
#endif
*/
