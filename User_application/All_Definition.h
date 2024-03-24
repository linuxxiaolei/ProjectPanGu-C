#ifndef ALL_DEFINITATION_H
#define ALL_DEFINITATION_H

//#include <stdio.h>
//#include <stdlib.h>
//#include <math.h>
#include "math.h"
//#include "IQmathLib.h"

/**/
// CLA
// header files
#include "DCLCLA.h"
#include "CLA_shared.h"
#include "F2837xD_Cla.h"                // Control Law Accelerator Registers
#include "F2837xD_Cla_defines.h"              // Macros used for CLA examples.

// function prototypes
//interrupt void control_Isr(void);

// global  variables
long IdleLoopCount = 0;
long IsrCount = 0;
float Duty;

// shared variables
#pragma DATA_SECTION(rk, "CpuToCla1MsgRAM")
#pragma DATA_SECTION(yk, "CpuToCla1MsgRAM")
#pragma DATA_SECTION(uk, "Cla1ToCpuMsgRAM")
float rk = 0.25f;
float yk;
float uk;

#pragma DATA_SECTION(pi1, "Cla1DataRam1")
DCL_PI_CLA pi1 = PI_CLA_DEFAULTS;

// CLA end



/* DSP system Configuration------------------------------------------------------------------*/
    #include "F2837xD_Cla_typedefs.h"  // F2837xD CLA Type definitions
    #include "F2837xD_device.h"        // F2837xD Headerfile Include File
    #include "F2837xD_Examples.h"      // F2837xD Examples Include File
/* 经常需要修改的宏定义声明文件 */
    #include "ACMConfig.h"
    #include "ACMSim.h"
    #include "Experiment.h"
/* Motor Library file------------------------------------------------------------------------*/
    #define USE_DEATIME_PRECOMP FALSE
    __interrupt void EPWM1ISR(void);
    #define ENABLE_ECAP 0
    #define USE_ECAP_CEVT2_INTERRUPT 0 // measure terminal voltage duty using e-capture
    __interrupt void ecap1_isr(void);
    __interrupt void ecap2_isr(void);
    __interrupt void ecap3_isr(void);
    __interrupt void EQEP_UTO_INT(void);
    __interrupt void scicRxFifoIsr(void);
    __interrupt void scicTxFifoIsr(void);
    void SVGEN_Drive(SVGENDQ* ptrV);
/* Driver -----------------------------------------------------------------------------------*/
    #include "CONSOLE.h"
    #include "DAC_MAX5307.h"
    #include "ECaptureVoltage.h"
    #include "F2837xD_Ipc_drivers.h" // 双核通讯
    #include "F2837xD_struct.h"
    #include "F2837xD_sdfm_drivers.h"
    #include "ShareMemory.h"
    void MemCopy(Uint16 *SourceAddr, Uint16* SourceEndAddr, Uint16* DestAddr);//flash
/* Hardware Peripherals Configuration -------------------------------------------------------*/
    void PWM_1ch_UpDwnCnt_CNF(int16 n, Uint16 period, int16 db);
    void ePWM_initialize(void);
    void eQEP_initialize(int m);
    void Gpio_initialize(void);
    void ADC_initialize(void);
    //PWM CONFIGURATION // see ACMSim.h
    //QEP CONFIGURATION
    // #define SYSTEM_QEP_LINE                  2500     //encoder line
    // #define SYSTEM_QEP_POLE_PAIRS               4     //pairs
    #define SYSTEM_QEP_UNITTIME_ISR          0.001    //1K ,1ms     time_out timer   isr_time
    #define SYSTEM_QEP_CAP_X128           0.00000064  //cap timer
    #define SYSTEM_QEP_CUTOFF_FILTER           5      // CUTOFF FREQUENCY 10HZ
    #define SYSTEM_QEP_SWAP_ENABLE              1     //正方向计数
    #define SYSTEM_QEP_SWAP_DISABLE             0     //反方向计数
    //ADC CONFIGURATION is moved to main.c
    //DAC Configuration
    #define NO_OF_DAC_CHANNELS 8
    //TRIP CONFIGURATION
    //#define MAX_CURRENT_N                      -11.8      //-12A  // Set your negative current trip threshold here in [0, 4095]
    //#define MAX_CURRENT_P                       11.8       //12A   // Set your positive current trip threshold here in [0, 4095]
    //#define MAX_OVERVOLTAGE                    400      //400V  // Set your unipolar trip Over-voltage threshold in [0, 4095]
    //GPIO
//    #define DSP_PWM_DISABLE        GpioDataRegs.GPDSET.bit.GPIO105=1;    // 低有效，置位封波
//    #define DSP_PWM_ENABLE         GpioDataRegs.GPDCLEAR.bit.GPIO105=1;  // 低有效，清零有效
//    #define DSP_2PWM_DISABLE       GpioDataRegs.GPASET.bit.GPIO27=1;
//    #define DSP_2PWM_ENABLE        GpioDataRegs.GPACLEAR.bit.GPIO27=1;
#define DSP_PWM_DISABLE        GpioDataRegs.GPDSET.bit.GPIO108=1;    // 低有效，置位封波
#define DSP_PWM_ENABLE         GpioDataRegs.GPDCLEAR.bit.GPIO108=1;  // 低有效，清零有效
#define DSP_2PWM_DISABLE       GpioDataRegs.GPCSET.bit.GPIO93=1;
#define DSP_2PWM_ENABLE        GpioDataRegs.GPCCLEAR.bit.GPIO93=1;

    #define INVERTER_FLT_FAULT     GpioDataRegs.GPDDAT.bit.GPIO104  //Inverter_error signal

    #define DSP_STOP_LED1  GpioDataRegs.GPDCLEAR.bit.GPIO124=1;
    #define DSP_STOP_LED2  GpioDataRegs.GPBCLEAR.bit.GPIO33=1;
    #define DSP_START_LED1 GpioDataRegs.GPDSET.bit.GPIO124=1;
    #define DSP_START_LED2 GpioDataRegs.GPBSET.bit.GPIO33=1;

/* Logic  -----------------------------------------------------------------------------------*/
    //#include "Logic.h"                //逻辑库 包括故障代码，DI,状态机
    extern int VoltageOVER_FLAG;
    struct Trip_Variables
    {
        BOOL VoltageOVER_FLAG;
        BOOL CurrentOVER_FLAG;
    };
    extern struct Trip_Variables trip;//trip flag
    //#define Motor_mode_START    GpioDataRegs.GPCDAT.bit.GPIO75    //DI Start Button
    //#define Motor_mode_START    digital_virtual_button          //virtual DI Start Button
    int Motor_MODE_START(void);
    int Motor_MODE_STOP(void);
    int Motor_MODE_REVERSE(void);
    void System_Checking(void);
/* Global Variable */
typedef struct{
    int ID;
    struct ControllerForExperiment *pCTRL;
    // Commonly used for prototype motor testing
       //int use_first_set_three_phase;
        int Set_current_loop;
        int Set_x_suspension_current_loop;
        int Set_y_suspension_current_loop;
        REAL Set_manual_rpm;
        REAL Prev_manual_rpm;
        REAL rampRate_rpm;
        REAL Set_manual_current_id;
        REAL Set_manual_current_iq;
        REAL Set_manual_current_ix;
        REAL Set_manual_current_iy;
        int Select_exp_operation;
        //int *pFLAG_INVERTER_NONLINEARITY_COMPENSATION;
        int flag_overwrite_theta_d;
        REAL Overwrite_Current_Frequency;
        REAL Overwrite_Suspension_Current_Frequency;
        REAL used_theta_d_elec;
        REAL angle_shift_for_first_inverter;
        REAL angle_shift_for_second_inverter;
        REAL OverwriteSpeedOutLimitDuringInit;
        int FLAG_ENABLE_PWM_OUTPUT; // 电机模式标志位
    // ADC Offset
        // Automatic Offset Removing
        int AD_offset_flag2;
        REAL offset_counter;
        REAL offset_online[6];
        // Raw
        REAL adc_offset[12]; // ADC offset. U, V, W corresponds to ADCRESULT2, ADCRESULT3, ADCRESULT1.
        REAL adc_scale[12];
//        volatile struct ADC_RESULT_REGS *pAdcaResultRegs;
//        volatile struct ADC_RESULT_REGS *pAdcbResultRegs;
//        volatile struct ADC_RESULT_REGS *pAdccResultRegs;
    // Sensor - Raw measurement
        REAL vdc;
        REAL iabg[6];
        REAL iuvw[6];
        REAL iuvw_offset_online[6];
    // DAC
        int DAC_MAX5307_FLAG; // for single core case
        REAL dac_offset[NO_OF_DAC_CHANNELS];
        REAL dac_time;
        REAL dac_watch[80];
        REAL dac_watch_stator_resistance;
        int channels[NO_OF_DAC_CHANNELS];
        int channels_preset;
} st_axis;
extern st_axis Axis_1;
extern st_axis Axis_2;
extern st_axis *Axis;



#endif
