#include "ACMSim.h"

/* 逆变器非线性 */

/* 拟合法 */
#ifdef _XCUBE1
    // b-phase @80 V Mini6PhaseIPMInverter
    REAL sig_a2 = 7.705938744542915;
    REAL sig_a3 = 67.0107635483658;

    // 180V (old)
    // REAL sig_a2 = 16.0575341;  // yuefei tuning gives a2' = 9.2*2
    // REAL sig_a3 = 17.59278688; // yuefei tuning gives a3' = a3*5
#else
    // 80 V
    // REAL sig_a2 = 6.67159129;
    // REAL sig_a3 = 8.42010418;

    // 100 V from SlessInv paper
    REAL sig_a2 = 6.7;
    REAL sig_a3 = 5.6;

    // 150 V
    // REAL sig_a2 = 13.25723639;
    // REAL sig_a3 = 5.6420585;

    // 180 V
    // REAL sig_a2 = 15.43046115;
    // REAL sig_a3 = 5.04010863;k
#endif
REAL sigmoid(REAL x){
    // beta
    // REAL a1 = 0.0; // *1.28430164
    // REAL a2 = 8.99347107;
    // REAL a3 = 5.37783655;

    // -------------------------------

    #ifdef _XCUBE1
        /* Si Mini OW IPM Inverter*/

        // b-phase @180 V Mini6PhaseIPMInverter
        // REAL a1 = 0.0; //1.34971502
        // REAL a2 = 16.0575341;  // yuefei tuning gives a2' = 9.2*2
        // REAL a3 = 17.59278688; // yuefei tuning gives a3' = a3*5

        // b-phase @80 V Mini6PhaseIPMInverter
        // REAL a1 = 0.0; //2.10871359 ;
        // REAL a2 = 7.36725304;
        // REAL a3 = 62.98712805;
        REAL a1 = 0.0; //(1.705, 
        REAL a2 = sig_a2;
        REAL a3 = sig_a3;

    #else
        /* SiC Cree 3 Phase Inverter*/

        // b-phase @20 V
        // REAL a1 = 0.0; //1.25991178;
        // REAL a2 = 2.30505904;
        // REAL a3 = 12.93721814;

        // b-phase @80 V
            // REAL a1 = 0.0; //1.28430164;
            // REAL a2 = 7.78857441;
            // REAL a3 = 6.20979077;
        REAL a1 = 0.0; //1.6997103;
        REAL a2 = sig_a2; 6.67159129;
        REAL a3 = sig_a3; 8.42010418;

        // b-phase @180 V
                /*最早的拟合结果*/
                // REAL a1 = 0.0; //1.50469916;
                // REAL a2 = 13.48467604;
                // REAL a3 = 5.22150403;
            /*错误！多乘了一次beta-axis转phase系数！*/
            // REAL a1 = 0.0; //1.83573529
            // REAL a2 = sig_a2; 13.36317172;
            // REAL a3 = sig_a3; 5.81981571;
        /*纠正！100个点，平均分配*/
        // REAL a1 = 0.0; //1.8357354;
        // REAL a2 = sig_a2; 15.43046115;
        // REAL a3 = sig_a3; 5.04010863;

    #endif

    return a1 * x + a2 / (1.0 + exp(-a3 * x)) - a2*0.5;
}
REAL sigmoid_online_v2(REAL x, REAL a2, REAL a3){
    return a2 / (1.0 + exp(-a3 * x)) - a2*0.5;
}
REAL sigmoid_online(REAL x, REAL Vsat, REAL a3){

    REAL a2 = Vsat*2;
    // REAL a3 = 5.22150403;
    return a2 / (1.0 + exp(-a3 * x)) - a2*0.5;
}

/* 真查表法 // C代码参考ui_curve_v4.py */
    #define LUT_N_LC  70
    #define LUT_N_HC  29

    // REAL lut_lc_voltage[70] = {0, -0.0105529, 0.31933, 0.364001, 0.415814, 0.489953, 0.602715, 0.769718, 0.971424, 1.21079, 1.50055, 1.83306, 2.16318, 2.54303, 2.92186, 3.24129, 3.51575, 3.75058, 3.97849, 4.16454, 4.33493, 4.49719, 4.64278, 4.76509, 4.88146, 4.99055, 5.06347, 5.16252, 5.24808, 5.30369, 5.36092, 5.44246, 5.50212, 5.5786, 5.63384, 5.69022, 5.74442, 5.79613, 5.8491, 5.89762, 5.93325, 5.98141, 6.01726, 6.06201, 6.09346, 6.13419, 6.16634, 6.19528, 6.2233, 6.25819, 6.29004, 6.31378, 6.34112, 6.3669, 6.38991, 6.4147, 6.4381, 6.46156, 6.48171, 6.49962, 6.51565, 6.53689, 6.5566, 6.57761, 6.59515, 6.60624, 6.62549, 6.64589, 6.65606, 6.67132};
    // REAL lut_hc_voltage[29] = {6.69023, 6.80461, 6.89879, 6.96976, 7.02613, 7.08644, 7.12535, 7.17312, 7.20858, 7.2444, 7.27558, 7.30321, 7.32961, 7.35726, 7.38272, 7.39944, 7.42055, 7.43142, 7.4416, 7.43598, 7.44959, 7.45352, 7.45434, 7.45356, 7.45172, 7.45522, 7.45602, 7.44348, 7.43926};

    // #define LUT_STEPSIZE_BIG 0.11641244037931034
    // #define LUT_STEPSIZE_SMALL 0.01237159786376811
    // #define LUT_STEPSIZE_BIG_INVERSE 8.59014721057018
    // #define LUT_STEPSIZE_SMALL_INVERSE 80.83030268294078
    // #define LUT_I_TURNING_LC 0.8660118504637677
    // #define LUT_I_TURNING_HC 4.241972621463768
    // #define V_PLATEAU 7.43925517763064

    REAL lut_lc_voltage[70] = {0.0,0.998199284,1.20954859,1.36845672,1.52856529,1.63873553,1.75739849,1.86049855,1.96500587,2.03725982,2.09248209,2.13737798,2.16144872,2.17589974,2.19009066,2.19929051,2.21263218,2.22282434,2.23175001,2.24243355,2.24980879,2.25796008,2.26734352,2.27478409,2.28258562,2.28810096,2.2944901,2.30045795,2.30883789,2.31305194,2.32067561,2.32698846,2.33021784,2.33724499,2.34168768,2.34715605,2.35252976,2.35633898,2.36176372,2.36799812,2.37308788,2.37675405,2.38145471,2.3869226,2.39018703,2.3954072,2.39925385,2.40437961,2.40812206,2.41243339,2.41567254,2.41950607,2.42328215,2.4283421,2.43134332,2.43476081,2.4378922,2.442415,2.44689822,2.44919944,2.45439005,2.4579823,2.46004272,2.46468091,2.46781993,2.4708817,2.47555852,2.47685099,2.482539807,2.48137823802};
    REAL lut_hc_voltage[29] = {2.5152143126,2.5388624802,2.5556035748,2.5687275655999997,2.5805857714,2.5875546051000002,2.5940691616000002,2.6000795477,2.6011016497,2.6039889307000004,2.6059528894,2.6089043281,2.6116798367,2.6111121056,2.6114816701,2.6124932837,2.6135516142,2.6127213802,2.6132182461,2.6145235375999993,2.6144125256,2.6151765069999997,2.6135842187000007,2.6130441795999992,2.6118966960000005,2.6094434740000003,2.6111450680000003,2.936406402,3.23673827};
    
    #define LUT_STEPSIZE_BIG 0.3972883199999995
    #define LUT_STEPSIZE_SMALL 0.06998252999999988
    #define LUT_STEPSIZE_BIG_INVERSE 2.5170636780864872
    #define LUT_STEPSIZE_SMALL_INVERSE 14.289280481857427
    #define LUT_I_TURNING_LC 0.0229527075
    #define LUT_I_TURNING_HC 11.878686
    #define V_PLATEAU 3.000



REAL lookup_compensation_voltage_indexed(REAL current_value){
    REAL abs_current_value = fabs(current_value);

    if(abs_current_value < LUT_I_TURNING_LC){
        REAL float_index = abs_current_value * LUT_STEPSIZE_SMALL_INVERSE;
        int index = (int)float_index;
        REAL slope;
        if(index+1 >= LUT_N_LC)
            slope = (lut_hc_voltage[0] - lut_lc_voltage[index]) * LUT_STEPSIZE_SMALL_INVERSE;
        else
            slope = (lut_lc_voltage[index+1] - lut_lc_voltage[index]) * LUT_STEPSIZE_SMALL_INVERSE;
        return sign(current_value) * (lut_lc_voltage[index] + slope * (abs_current_value - index*LUT_STEPSIZE_SMALL));
    }else{
        REAL float_index = (abs_current_value - LUT_I_TURNING_LC) * LUT_STEPSIZE_BIG_INVERSE;
        int index = (int)float_index; // THIS IS A RELATIVE INDEX!
        REAL slope;
        if(index+1 >= LUT_N_HC)
            return sign(current_value) * V_PLATEAU;
        else
            slope = (lut_hc_voltage[index+1] - lut_hc_voltage[index]) * LUT_STEPSIZE_BIG_INVERSE;
        return sign(current_value) * (lut_hc_voltage[index] + slope * (abs_current_value - LUT_I_TURNING_LC - index*LUT_STEPSIZE_BIG));
    }
}
void get_distorted_voltage_via_LUT_indexed(REAL ial, REAL ibe, REAL *ualbe_dist){

    /* 查表法 */
    if(TRUE){
        REAL ia,ib,ic;
        ia = 1 * (       ial                              );
        ib = 1 * (-0.5 * ial - SIN_DASH_2PI_SLASH_3 * ibe );
        ic = 1 * (-0.5 * ial - SIN_2PI_SLASH_3      * ibe );

        REAL dist_ua = lookup_compensation_voltage_indexed(ia);
        REAL dist_ub = lookup_compensation_voltage_indexed(ib);
        REAL dist_uc = lookup_compensation_voltage_indexed(ic);

        G.ia = ia;
        G.dist_ua = dist_ua;

        // Clarke transformation（三分之二，0.5倍根号三）
        ualbe_dist[0] = 0.66666667 * (dist_ua - 0.5*dist_ub - 0.5*dist_uc);
        ualbe_dist[1] = 0.66666667 * 0.8660254 * ( dist_ub -     dist_uc); // 0.5773502695534
    }else{
        /* AB2U_AI 这宏假设了零序分量为零，即ia+ib+ic=0，但是电压并不一定满足吧，所以还是得用上面的？ */
        REAL ia,ib;//,ic;
        ia = AB2U_AI(ial, ibe); // ia = 1 * (       ial                              );
        ib = AB2V_AI(ial, ibe); // ib = 1 * (-0.5 * ial - SIN_DASH_2PI_SLASH_3 * ibe );

        REAL dist_ua = lookup_compensation_voltage_indexed(ia);
        REAL dist_ub = lookup_compensation_voltage_indexed(ib);

        ualbe_dist[0] = UV2A_AI(dist_ua, dist_ub); // 0.66666667 * (dist_ua - 0.5*dist_ub - 0.5*dist_uc);
        ualbe_dist[1] = UV2B_AI(dist_ua, dist_ub); // 0.66666667 * 0.8660254 * ( dist_ub -     dist_uc);        
    }
}

/* 伪查表法（需要循环比大小，运算量不固定） */
REAL lookup_phase_current(REAL current, REAL *lut_voltage, REAL *lut_current, int length_of_lut){
    /* assume lut_voltage[0] is negative and lut_voltage[-1] is positive */
    int j;
    if( current<lut_current[0] ){
        return lut_voltage[0];
    }else if( current>lut_current[length_of_lut-1] ){
        return lut_voltage[length_of_lut-1];
    }else{
        for(j=0;j<length_of_lut-1;++j){
            if( current>lut_current[j] && current<lut_current[j+1] ){
                REAL slope = (lut_voltage[j+1] - lut_voltage[j]) / (lut_current[j+1] - lut_current[j]);
                return (current - lut_current[j]) * slope + lut_voltage[j];
                // return 0.5*(lut_voltage[j]+lut_voltage[j+1]); // this is just wrong! stupid!
            }
        }
    }
    #if PC_SIMULATION
        printf("DEBUG inverter.c. %g, %d, %d\n", current, j, length_of_lut);
    #endif

    // if a sudden case happens, return the zero to keep safe
    return 0.0;
}
REAL sigmoid_a3_tune = 1.0;
void get_distorted_voltage_via_CurveFitting(REAL ual, REAL ube, REAL ial, REAL ibe, REAL *ualbe_dist){
    // The data are measured in stator resistance identification with amplitude invariant d-axis current and d-axis voltage.

    /* 拟合法 */
    REAL ia,ib,ic;
    ia = 1 * (       ial                              );
    ib = 1 * (-0.5 * ial - SIN_DASH_2PI_SLASH_3 * ibe );
    ic = 1 * (-0.5 * ial - SIN_2PI_SLASH_3      * ibe );

    // offline sigmoid
    REAL dist_ua = sigmoid(ia);
    REAL dist_ub = sigmoid(ib);
    REAL dist_uc = sigmoid(ic);

    // online tunable sigmoid (experimental)
    // REAL dist_ua = sigmoid_online(ia, INV.Vsat, 17.59278688 * sigmoid_a3_tune); //INV.theta_trapezoidal);
    // REAL dist_ub = sigmoid_online(ib, INV.Vsat, 17.59278688 * sigmoid_a3_tune); //INV.theta_trapezoidal);
    // REAL dist_uc = sigmoid_online(ic, INV.Vsat, 17.59278688 * sigmoid_a3_tune); //INV.theta_trapezoidal);


    // Clarke transformation（三分之二，0.5倍根号三）
    ualbe_dist[0] = 0.66666667 * (dist_ua - 0.5*dist_ub - 0.5*dist_uc);
    ualbe_dist[1] = 0.66666667 * 0.8660254  * ( dist_ub -     dist_uc); // 0.5773502695534

    // for plot
    INV.ia = ia;
    INV.ib = ib;
    INV.ic = ic;
    INV.dist_ua = dist_ua;
    INV.dist_ub = dist_ub;
    INV.dist_uc = dist_uc;
}


REAL wubo_debug_iabc[3] = {0.0, 0.0, 0.0};

void get_distorted_voltage_via_LUT(REAL ual, REAL ube, REAL ial, REAL ibe, REAL *ualbe_dist, REAL *lut_voltage, REAL *lut_current, int length_of_lut){
    // The data are measured in stator resistance identification with amplitude invariant d-axis current and d-axis voltage.

    /* 查表法 */
    if(TRUE){
        REAL ia,ib,ic;
        // 恒幅值Clarke反变换
        ia = 1 * (       ial                              );
        ib = 1 * (-0.5 * ial - SIN_DASH_2PI_SLASH_3 * ibe );
        ic = 1 * (-0.5 * ial - SIN_2PI_SLASH_3      * ibe );

        wubo_debug_iabc[0] = ia;
        wubo_debug_iabc[1] = ib;
        wubo_debug_iabc[2] = ic;

        REAL dist_ua = lookup_phase_current(ia, lut_voltage, lut_current, length_of_lut);
        REAL dist_ub = lookup_phase_current(ib, lut_voltage, lut_current, length_of_lut);
        REAL dist_uc = lookup_phase_current(ic, lut_voltage, lut_current, length_of_lut);

        // Clarke transformation（三分之二，0.5倍根号三）
        ualbe_dist[0] = 0.66666667 * (dist_ua - 0.5*dist_ub - 0.5*dist_uc);
        ualbe_dist[1] = 0.66666667 * 0.8660254 * ( dist_ub -     dist_uc); // 0.5773502695534
    }else{
        /* AB2U_AI 这宏假设了零序分量为零，即ia+ib+ic=0，但是电压并不一定满足吧，所以还是得用上面的？ */
        REAL ia,ib;//,ic;
        ia = AB2U_AI(ial, ibe); // ia = 1 * (       ial                              );
        ib = AB2V_AI(ial, ibe); // ib = 1 * (-0.5 * ial - SIN_DASH_2PI_SLASH_3 * ibe );
        // ic = AB2W_AI(ial, ibe); // ic = 1 * (-0.5 * ial - SIN_2PI_SLASH_3      * ibe );

        REAL dist_ua = lookup_phase_current(ia, lut_voltage, lut_current, length_of_lut);
        REAL dist_ub = lookup_phase_current(ib, lut_voltage, lut_current, length_of_lut);
        // REAL dist_uc = lookup_phase_current(ic, lut_voltage, lut_current, length_of_lut);

        // Clarke transformation（三分之二，0.5倍根号三）
        // ualbe_dist[0] = 0.66666667 * (dist_ua - 0.5*dist_ub - 0.5*dist_uc);
        // ualbe_dist[1] = 0.66666667 * 0.8660254 * ( dist_ub -     dist_uc); // 0.5773502695534

        ualbe_dist[0] = UV2A_AI(dist_ua, dist_ub); // 0.66666667 * (dist_ua - 0.5*dist_ub - 0.5*dist_uc);
        ualbe_dist[1] = UV2B_AI(dist_ua, dist_ub); // 0.66666667 * 0.8660254 * ( dist_ub -     dist_uc);        
    }
}

/* Chen 2021 Linear Approximation of U-i Curve */
REAL trapezoidal_voltage_by_phase_current(REAL current, REAL V_plateau, REAL I_plateau, REAL oneOnver_I_plateau){
    REAL abs_current = fabs(current);
    if(abs_current < I_plateau){
        return current * V_plateau * oneOnver_I_plateau;
    }
    else{
        return sign(current) * V_plateau;
    }
}

/* ParkSul2012 梯形波 */
void inverterNonlinearity_Initialization(){
    INV.gamma_theta_trapezoidal = GAIN_THETA_TRAPEZOIDAL;
    #ifdef _XCUBE1
    INV.Vsat = 16.0575341/2; //6.67054; // 180 V SiC
    #else
    // INV.Vsat = sig_a2*0.5; //6.74233802;
    INV.Vsat = 7.84; // 150 V
    #endif

    INV.gain_Vsat = 0 * 10;

    INV.thetaA=0;
    INV.cos_thetaA=1;
    INV.sin_thetaA=0;

    // --
    INV.u_comp[0]=0;
    INV.u_comp[1]=0;
    INV.u_comp[2]=0;
    INV.ual_comp=0;
    INV.ube_comp=0;
    INV.uDcomp_atA=0;
    INV.uQcomp_atA=0;

    INV.iD_atA=0;
    INV.iQ_atA=0;
    INV.I5_plus_I7=0;
    INV.I5_plus_I7_LPF=0.0;
    INV.theta_trapezoidal=11.0 * M_PI_OVER_180; // in rad

    #ifdef _XCUBE1
        INV.I_plateau_Max = 2.0;
        INV.I_plateau_Min = 0.2;
    #else
        INV.I_plateau_Max = 1.0;
        INV.I_plateau_Min = 0.1;
    #endif
    INV.I_plateau = 0.7;
    INV.V_plateau = 1.5 * sig_a2*0.5;
    INV.gamma_I_plateau = 10.0;
    INV.gamma_V_plateau = 0.0; // this is updated by the estimated disturbance to the sinusoidal flux model.

    #if PC_SIMULATION
        INV.sig_a2 = 1.0*sig_a2;
        INV.sig_a3 = 1.5*sig_a3; // = shape parameter
    #else
        INV.sig_a2 = sig_a2; // = Plateau * 2
        INV.sig_a3 = sig_a3; // = shape parameter
    #endif

    INV.w6  = 1;
    INV.w12 = 0;
    INV.w18 = 0;
    INV.gamma_a2 = 400;
    INV.gamma_a3 = 125;
}
#define trapezoidal_voltage_by_current_vector_angle u_comp_per_phase
REAL u_comp_per_phase(REAL Vsat, REAL thetaA, REAL theta_trapezoidal, REAL oneOver_theta_trapezoidal){
    REAL compensation;

    // if(thetaA>0){
    //         compensation = Vsat;
    // }else{ // thetaA < 0
    //         compensation = -Vsat;
    // }

    if(thetaA>0){
        if(thetaA<theta_trapezoidal){
            compensation = thetaA * Vsat * oneOver_theta_trapezoidal;
        }else if(thetaA>M_PI - theta_trapezoidal){
            compensation = (M_PI-thetaA) * Vsat * oneOver_theta_trapezoidal;
        }else{
            compensation = Vsat;
        }
    }else{ // thetaA < 0
        if(-thetaA<theta_trapezoidal){
            compensation = -thetaA * -Vsat * oneOver_theta_trapezoidal;
        }else if(-thetaA>M_PI - theta_trapezoidal){
            compensation = (M_PI+thetaA) * -Vsat * oneOver_theta_trapezoidal;
        }else{
            compensation = -Vsat;
        }
    }

    return compensation;
}
REAL lpf1_inverter(REAL x, REAL y_tminus1){
    // #define LPF1_RC 0.6 // 0.7, 0.8, 1.2, 3太大滤得过分了 /* 观察I5+I7_LPF的动态情况进行确定 */
    // #define ALPHA_RC (TS/(LPF1_RC + TS)) // TODO：优化
    // return y_tminus1 + ALPHA_RC * (x - y_tminus1); // 0.00020828993959591752
    return y_tminus1 + 0.00020828993959591752 * (x - y_tminus1);
}
REAL shift2pi(REAL thetaA){
    if(thetaA>M_PI){
        return thetaA - 2*M_PI;
    }else if(thetaA<-M_PI){
        return thetaA + 2*M_PI;
    }else{
        return thetaA;
    }
}
// REAL watch_theta_trapezoidal = 0.0;
void Modified_ParkSul_Compensation(void){

    /* Park12/14
     * */
    // Phase A current's fundamental component transformation    
    INV.thetaA = -M_PI*1.5 + (*CTRL).i->theta_d_elec + atan2((*CTRL).i->idq_cmd[1], (*CTRL).i->idq_cmd[0]); /* Q: why -pi*(1.5)? */ /* ParkSul2014 suggests to use PLL to extract thetaA from current command */
    INV.thetaA = shift2pi(INV.thetaA); /* Q: how to handle it when INV.thetaA jumps between pi and -pi? */ // 这句话绝对不能省去，否则C相的梯形波会出错。

    INV.cos_thetaA = cos(INV.thetaA);
    INV.sin_thetaA = sin(INV.thetaA);
    INV.iD_atA = AB2M(IS_C(0),IS_C(1), INV.cos_thetaA, INV.sin_thetaA);
    INV.iQ_atA = AB2T(IS_C(0),IS_C(1), INV.cos_thetaA, INV.sin_thetaA);

    INV.I5_plus_I7 = INV.iD_atA * sin(6*INV.thetaA); /* Q: Why sin? Why not cos? 和上面的-1.5*pi有关系吗？ */
    INV.I5_plus_I7_LPF = lpf1_inverter(INV.I5_plus_I7, INV.I5_plus_I7_LPF); /* lpf1 for inverter */

    INV.I11_plus_I13 = INV.iD_atA * sin(12*INV.thetaA);
    INV.I11_plus_I13_LPF = lpf1_inverter(INV.I11_plus_I13, INV.I11_plus_I13_LPF);

    INV.I17_plus_I19 = INV.iD_atA * sin(18*INV.thetaA);
    INV.I17_plus_I19_LPF = lpf1_inverter(INV.I17_plus_I19, INV.I17_plus_I19_LPF);

    // The adjusting of theta_t via 6th harmonic magnitude
    INV.theta_trapezoidal += CL_TS * INV.gamma_theta_trapezoidal \
                            // *fabs((*CTRL).i->cmd_speed_rpm)
                            *(      INV.I5_plus_I7_LPF 
                                + 0*INV.I11_plus_I13_LPF
                                + 0*INV.I17_plus_I19_LPF
                             );
    // 两种方法选其一：第一种可用于sensorless系统。
    if(FALSE){
        /* 利用theta_t的饱和特性辨识Vsat（次优解） */
    }else{
        // 由于实际逆变器有一个调制比低引入5、7次谐波的问题，所以最佳theta_t不在11度附近了。
        if(INV.theta_trapezoidal >= 25*M_PI_OVER_180){ // 17
            INV.theta_trapezoidal = 25*M_PI_OVER_180; // 17
        }
        if(INV.theta_trapezoidal <= 0.223*M_PI_OVER_180){
            INV.theta_trapezoidal = 0.223*M_PI_OVER_180;
        }
    }

    #if PC_SIMULATION == TRUE
        // 用偏小的Vsat，观察theta_t的收敛是否直达下界？
        // if((*CTRL).timebase>23){
        //     INV.Vsat = 9.0/6.0 * 16.0575341/2;
        // }else if((*CTRL).timebase>20){
        //     INV.Vsat = 7.0/6.0 * 16.0575341/2;
        // }else if((*CTRL).timebase>15){
        //     INV.Vsat = 6.0/6.0 * 16.0575341/2;
        // }else if((*CTRL).timebase>10){
        //     INV.Vsat = 5.0/6.0 * 16.0575341/2;
        // }else if((*CTRL).timebase>5){
        //     INV.Vsat = 4.0/6.0 * 16.0575341/2;
        // }

        // /* Adaptive Vsat based on position error */
        // INV.Vsat += CL_TS * INV.gain_Vsat * sin(ENC.theta_d_elec - ELECTRICAL_POSITION_FEEDBACK) * sign(ENC.omg_elec);
        // if (INV.Vsat>15){
        //     INV.Vsat = 15;
        // }else if(INV.Vsat<0){
        //     INV.Vsat = 0;
        // }
    #else
        /* Adaptive Vsat based on position error */
        /* TO use this, you muast have a large enough stator current */
        /* TO use this, you muast have a large enough stator current */
        /* TO use this, you muast have a large enough stator current */
        INV.Vsat += CL_TS * INV.gain_Vsat * sin(ENC.theta_d_elec - ELECTRICAL_POSITION_FEEDBACK) * sign(ENC.omg_elec);
        if (INV.Vsat>15){
            INV.Vsat = 15;
        }else if(INV.Vsat<0){
            INV.Vsat = 0;
        }
    #endif


    #if INVERTER_NONLINEARITY == 3 // [ModelLUT]
        // INV.Vsat = 6.67054;
    #elif INVERTER_NONLINEARITY == 2 // [ModelSigmoid]
        // INV.Vsat = sigmoid(100)*1.0;
            // INV.Vsat = sigmoid(100)*0.95;
            // INV.Vsat = sigmoid(100)*1.05;
    #elif INVERTER_NONLINEARITY == 1 // [ModelSul96]
        REAL TM    = _Toff - _Ton - _Tdead + _Tcomp; // Sul1996
        REAL Udist = (_Udc*TM*CL_TS_INVERSE - _Vce0 - _Vd0) / 6.0;
        INV.Vsat = 3*fabs( Udist ); // 4 = 2*sign(ia) - sign(ib) - sign(ic) when ia is positive and ib/ic is negative
                                    // but Vsat is phase voltage distortion maximum, so we need to add a zero sequence voltage of Udist*(sign(ia) + sign(ib) + sign(ic)),
                                    // so, it is 3 * |Udist| as Vsat.
        static int bool_printed = FALSE;
        if(bool_printed==FALSE){
            printf("\tVsat = %g V; Udist = %g.\n", INV.Vsat, Udist);
            bool_printed=TRUE;
        }
    #endif

    REAL oneOver_theta_trapezoidal = 1.0/INV.theta_trapezoidal;
    // watch_theta_trapezoidal = INV.theta_trapezoidal / M_PI_OVER_180;
    INV.u_comp[0] = u_comp_per_phase(INV.Vsat,          INV.thetaA                 , INV.theta_trapezoidal, oneOver_theta_trapezoidal);
    INV.u_comp[1] = u_comp_per_phase(INV.Vsat, shift2pi(INV.thetaA-  TWO_PI_OVER_3), INV.theta_trapezoidal, oneOver_theta_trapezoidal);
    INV.u_comp[2] = u_comp_per_phase(INV.Vsat, shift2pi(INV.thetaA-2*TWO_PI_OVER_3), INV.theta_trapezoidal, oneOver_theta_trapezoidal);

    // REAL ia,ib,ic;
    // ia = 1 * (       IS_C(0)                                  );
    // ib = 1 * (-0.5 * IS_C(0) - SIN_DASH_2PI_SLASH_3 * IS_C(1) );
    // ic = 1 * (-0.5 * IS_C(0) - SIN_2PI_SLASH_3      * IS_C(1) );
    // #define MISMATCH_A3 1.1
    // INV.u_comp[0] = sigmoid_online(ia, INV.Vsat, 5.22150403 * MISMATCH_A3); //INV.theta_trapezoidal);
    // INV.u_comp[1] = sigmoid_online(ib, INV.Vsat, 5.22150403 * MISMATCH_A3); //INV.theta_trapezoidal);
    // INV.u_comp[2] = sigmoid_online(ic, INV.Vsat, 5.22150403 * MISMATCH_A3); //INV.theta_trapezoidal);

    /* 相补偿电压Clarke为静止正交坐标系电压。 */
    // 改成恒幅值变换
    INV.ual_comp = 0.66666666667 * (INV.u_comp[0] - 0.5*INV.u_comp[1] - 0.5*INV.u_comp[2]);
    INV.ube_comp = 0.66666666667 * 0.86602540378 * (    INV.u_comp[1] -     INV.u_comp[2]);
        // INV.ual_comp = SQRT_2_SLASH_3      * (INV.u_comp[0] - 0.5*INV.u_comp[1] - 0.5*INV.u_comp[2]); // sqrt(2/3.)
        // INV.ube_comp = 0.70710678118654746 * (                    INV.u_comp[1] -     INV.u_comp[2]); // sqrt(2/3.)*sin(2*pi/3) = sqrt(2/3.)*(sqrt(3)/2)

    // 区分补偿前的电压和补偿后的电压：
    // (*CTRL).ual, (*CTRL).ube 是补偿前的电压！
    // (*CTRL).ual + INV.ual_comp, (*CTRL).ube + INV.ube_comp 是补偿后的电压！

    // for plotting
    INV.uDcomp_atA = AB2M(INV.ual_comp, INV.ube_comp, INV.cos_thetaA, INV.sin_thetaA);
    INV.uQcomp_atA = AB2T(INV.ual_comp, INV.ube_comp, INV.cos_thetaA, INV.sin_thetaA);
}

void Online_PAA_Based_Compensation(void){

    // /* 角度反馈: (*CTRL).i->theta_d_elec or ELECTRICAL_POSITION_FEEDBACK? */
    // INV.thetaA = ELECTRICAL_POSITION_FEEDBACK;
    // // i (current vector amplitude)
    // INV.iD_atA = sqrt(IS_C(0)*IS_C(0) + IS_C(1)*IS_C(1));

    /* 在Park2012中，a相电流被建模成了sin函数，一个sin函数的自变量角度放在正交坐标系下看就是在交轴上的，所以要-1.5*pi */

    // Phase A current's fundamental component transformation
    /* 这里使用哪个角度的关键不在于是有感的角度还是无感的角度，而是你FOC电流控制器（Park变换）用的角度是哪个？ */
    if((*CTRL).s->go_sensorless){
        INV.thetaA = -M_PI*1.5 + ELECTRICAL_POSITION_FEEDBACK + atan2((*CTRL).i->idq_cmd[1], (*CTRL).i->idq_cmd[0]); /* Q: why -pi*(1.5)? */ /* ParkSul2014 suggests to use PLL to extract thetaA from current command */
    }else{
        INV.thetaA = -M_PI*1.5 +         (*CTRL).i->theta_d_elec + atan2((*CTRL).i->idq_cmd[1], (*CTRL).i->idq_cmd[0]); /* Q: why -pi*(1.5)? */ /* ParkSul2014 suggests to use PLL to extract thetaA from current command */
    }
    INV.thetaA = shift2pi(INV.thetaA); /* Q: how to handle it when INV.thetaA jumps between pi and -pi? */ // 这句话绝对不能省去，否则C相的梯形波会出错。

    INV.cos_thetaA = cos(INV.thetaA);
    INV.sin_thetaA = sin(INV.thetaA);
    INV.iD_atA = AB2M(IS_C(0),IS_C(1), INV.cos_thetaA, INV.sin_thetaA);
    INV.iQ_atA = AB2T(IS_C(0),IS_C(1), INV.cos_thetaA, INV.sin_thetaA);

    if(FALSE){
        /* Use q-axis current in phase A angle (Tentative and Failed) */
        INV.I5_plus_I7     = INV.iQ_atA * cos(6*INV.thetaA);
        INV.I5_plus_I7_LPF = lpf1_inverter(INV.I5_plus_I7, INV.I5_plus_I7_LPF);

        INV.I11_plus_I13     = INV.iQ_atA * cos(12*INV.thetaA);
        INV.I11_plus_I13_LPF = lpf1_inverter(INV.I11_plus_I13, INV.I11_plus_I13_LPF);

        INV.I17_plus_I19     = INV.iQ_atA * cos(18*INV.thetaA);
        INV.I17_plus_I19_LPF = lpf1_inverter(INV.I17_plus_I19, INV.I17_plus_I19_LPF);        
    }else{
        /* Use d-axis current in phase A angle */
        INV.I5_plus_I7     = INV.iD_atA * sin(6*INV.thetaA); /* Q: Why sin? Why not cos? 和上面的-1.5*pi有关系吗？ */
        INV.I5_plus_I7_LPF = lpf1_inverter(INV.I5_plus_I7, INV.I5_plus_I7_LPF); /* lpf1 for inverter */

        INV.I11_plus_I13     = INV.iD_atA * sin(12*INV.thetaA);
        INV.I11_plus_I13_LPF = lpf1_inverter(INV.I11_plus_I13, INV.I11_plus_I13_LPF);

        INV.I17_plus_I19     = INV.iD_atA * sin(18*INV.thetaA);
        INV.I17_plus_I19_LPF = lpf1_inverter(INV.I17_plus_I19, INV.I17_plus_I19_LPF);        
    }

    #if PC_SIMULATION
        // INV.gamma_a2 = 0.0;
        // INV.gamma_a3 = 0.0;
    #endif

    /* Online Update Sigmoid a3 */
    // if((*CTRL).timebase>35){
    //     INV.gamma_I_plateau = 0.0;
    // }
    INV.sig_a3 -= CL_TS * INV.gamma_a3 \
                            // *fabs((*CTRL).i->cmd_speed_rpm)
                            *(    INV.w6 *INV.I5_plus_I7_LPF 
                                + INV.w12*INV.I11_plus_I13_LPF
                                + INV.w18*INV.I17_plus_I19_LPF
                             );
    #ifdef _XCUBE2
        if(INV.sig_a3 > 50){ INV.sig_a3 = 50; }else if(INV.sig_a3 < 0.5){ INV.sig_a3 = 0.5; }
    #endif
    #ifdef _XCUBE1
        if(INV.sig_a3 > 100){ INV.sig_a3 = 100; }else if(INV.sig_a3 < 5){ INV.sig_a3 = 5; }
    #endif

    // (*CTRL).s->Motor_or_Generator = sign((*CTRL).i->omg_elec * (*CTRL).i->idq_cmd[1]);
    // (*CTRL).s->Motor_or_Generator = sign((*CTRL).i->cmd_omg_elec);

    /* Online Update Sigmoid a2 */
    if((*CTRL).timebase>2){
        /* Sensorless: Adaptive a2 based on flux amplitude error */
        // use linear FE
        // INV.sig_a2 += CL_TS * -100 * AFEOE.output_error_dq[0];

        // use nonlinear (saturation) FE
        INV.sig_a2 += CL_TS * -INV.gamma_a2 * (*CTRL).s->Motor_or_Generator * (MOTOR.KActive - FE.htz.psi_2_ampl_lpf);

        /* Sensored: Adaptive a2 based on position error */
        /* To use this, you must have a large enough stator current */
        /* 这个好像只对梯形波（电流值的函数）有效 ……*/
        // INV.sig_a2 += CL_TS * INV.gain_Vsat * sin(ENC.theta_d_elec - ELECTRICAL_POSITION_FEEDBACK) * (*CTRL).s->Motor_or_Generator;
    }
    if(INV.sig_a2 > 40){ INV.sig_a2 = 40; }else if(INV.sig_a2 < 2){ INV.sig_a2 = 2; }

    /* Chen2021: linear approximation of u-i curve */
    // if((*CTRL).timebase>35){
    //     INV.gamma_I_plateau = 0.0;
    // }
    // INV.I_plateau += CL_TS * 0 * INV.gamma_I_plateau \
    //                         // *fabs((*CTRL).i->cmd_speed_rpm)
    //                         *(    1*INV.I5_plus_I7_LPF 
    //                             + 0*INV.I11_plus_I13_LPF
    //                             + 0*INV.I17_plus_I19_LPF
    //                          );
    // if(INV.I_plateau > INV.I_plateau_Max){
    //     INV.I_plateau = INV.I_plateau_Max;
    // }else if(INV.I_plateau < INV.I_plateau_Min){
    //     INV.I_plateau = INV.I_plateau_Min;
    // }

    /* Chen2021SlessInv 覆盖 */
    if(INV.gamma_I_plateau!=0){
        REAL ia_cmd = (       (*CTRL).o->iab_cmd[0]                                             );
        REAL ib_cmd = (-0.5 * (*CTRL).o->iab_cmd[0] - SIN_DASH_2PI_SLASH_3 * (*CTRL).o->iab_cmd[1] );
        REAL ic_cmd = (-0.5 * (*CTRL).o->iab_cmd[0] - SIN_2PI_SLASH_3      * (*CTRL).o->iab_cmd[1] );
        REAL oneOver_I_plateau = 1.0 / INV.I_plateau;
        INV.u_comp[0] = trapezoidal_voltage_by_phase_current(ia_cmd, INV.V_plateau, INV.I_plateau, oneOver_I_plateau);
        INV.u_comp[1] = trapezoidal_voltage_by_phase_current(ib_cmd, INV.V_plateau, INV.I_plateau, oneOver_I_plateau);
        INV.u_comp[2] = trapezoidal_voltage_by_phase_current(ic_cmd, INV.V_plateau, INV.I_plateau, oneOver_I_plateau);

        /* Online Sigmoid a3 覆盖 覆盖 */
        INV.u_comp[0] = sigmoid_online_v2(ia_cmd, INV.sig_a2, INV.sig_a3);
        INV.u_comp[1] = sigmoid_online_v2(ib_cmd, INV.sig_a2, INV.sig_a3);
        INV.u_comp[2] = sigmoid_online_v2(ic_cmd, INV.sig_a2, INV.sig_a3);
    }



    /* 相补偿电压Clarke为静止正交坐标系电压。 */
    // 改成恒幅值变换
    INV.ual_comp = 0.66666666667 * (INV.u_comp[0] - 0.5*INV.u_comp[1] - 0.5*INV.u_comp[2]);
    INV.ube_comp = 0.66666666667 * 0.86602540378 * (    INV.u_comp[1] -     INV.u_comp[2]);
        // INV.ual_comp = SQRT_2_SLASH_3      * (INV.u_comp[0] - 0.5*INV.u_comp[1] - 0.5*INV.u_comp[2]); // sqrt(2/3.)
        // INV.ube_comp = 0.70710678118654746 * (                    INV.u_comp[1] -     INV.u_comp[2]); // sqrt(2/3.)*sin(2*pi/3) = sqrt(2/3.)*(sqrt(3)/2)

    // 区分补偿前的电压和补偿后的电压：
    // (*CTRL).ual, (*CTRL).ube 是补偿前的电压！
    // (*CTRL).ual + INV.ual_comp, (*CTRL).ube + INV.ube_comp 是补偿后的电压！
}

REAL wubo_debug_inverter_conpensation = 0.0;
REAL wubo_debug_inverter_conpensation_decrease = 0.0;

/* MAIN */
void main_inverter_voltage_command(int bool_use_iab_cmd){
    REAL Ia, Ib;

    if(bool_use_iab_cmd){
        Ia = (*CTRL).o->iab_cmd[0];
        Ib = (*CTRL).o->iab_cmd[1];
    }else{
        Ia = (*CTRL).i->iab[0];
        Ib = (*CTRL).i->iab[1];
    }

    /* We use iab_cmd instead of iab to look-up */

    if(G.FLAG_INVERTER_NONLINEARITY_COMPENSATION == 0){

        (*CTRL).o->uab_cmd_to_inverter[0] = (*CTRL).o->uab_cmd[0];
        (*CTRL).o->uab_cmd_to_inverter[1] = (*CTRL).o->uab_cmd[1];

        (*CTRL).o->uab_cmd_to_inverter[0+2] = (*CTRL).o->uab_cmd[0+2];
        (*CTRL).o->uab_cmd_to_inverter[1+2] = (*CTRL).o->uab_cmd[1+2];

        wubo_debug_inverter_conpensation = G.FLAG_INVERTER_NONLINEARITY_COMPENSATION + 1;

        /* For scope only */
        #if PC_SIMULATION
            REAL ualbe_dist[2];
            get_distorted_voltage_via_CurveFitting( (*CTRL).o->uab_cmd[0], (*CTRL).o->uab_cmd[1], Ia, Ib, ualbe_dist);
            INV.ual_comp = ualbe_dist[0];
            INV.ube_comp = ualbe_dist[1];
        #endif

    }else if(G.FLAG_INVERTER_NONLINEARITY_COMPENSATION == 4){

        REAL ualbe_dist[2];
        get_distorted_voltage_via_LUT_indexed(Ia, Ib, ualbe_dist);
        (*CTRL).o->uab_cmd_to_inverter[0] = (*CTRL).o->uab_cmd[0] + ualbe_dist[0] * wubo_debug_inverter_conpensation_decrease;
        (*CTRL).o->uab_cmd_to_inverter[1] = (*CTRL).o->uab_cmd[1] + ualbe_dist[1] * wubo_debug_inverter_conpensation_decrease;

        /* For scope only */
        INV.ual_comp = ualbe_dist[0];
        INV.ube_comp = ualbe_dist[1];

    }else if(G.FLAG_INVERTER_NONLINEARITY_COMPENSATION == 3){
        /* 查表法-补偿 */
        // // Measured in Simulation when the inverter is modelled according to the experimental measurements
        // #define LENGTH_OF_LUT  19
        // REAL lut_current_ctrl[LENGTH_OF_LUT] = {-3.78, -3.36, -2.94, -2.52, -2.1, -1.68, -1.26, -0.839998, -0.419998, -0, 0.420002, 0.840002, 1.26, 1.68, 2.1, 2.52, 2.94, 3.36, 3.78};
        // // REAL lut_voltage_ctrl[LENGTH_OF_LUT] = {-6.41808, -6.41942, -6.39433, -6.36032, -6.25784, -6.12639, -5.79563, -5.35301, -3.61951, 0, 3.5038, 5.24969, 5.73176, 6.11153, 6.24738, 6.35941, 6.43225, 6.39274, 6.39482};
        // #define MANUAL_CORRECTION 1.1 // [V]
        // REAL lut_voltage_ctrl[LENGTH_OF_LUT] = {
        //     -6.41808+MANUAL_CORRECTION, 
        //     -6.41942+MANUAL_CORRECTION, 
        //     -6.39433+MANUAL_CORRECTION, 
        //     -6.36032+MANUAL_CORRECTION, 
        //     -6.25784+MANUAL_CORRECTION, 
        //     -6.12639+MANUAL_CORRECTION, 
        //     -5.79563+MANUAL_CORRECTION, 
        //     -5.35301+MANUAL_CORRECTION, 
        //     -3.61951, 0, 3.5038,
        //      5.24969-MANUAL_CORRECTION,
        //      5.73176-MANUAL_CORRECTION,
        //      6.11153-MANUAL_CORRECTION,
        //      6.24738-MANUAL_CORRECTION,
        //      6.35941-MANUAL_CORRECTION,
        //      6.43225-MANUAL_CORRECTION,
        //      6.39274-MANUAL_CORRECTION,
        //      6.39482-MANUAL_CORRECTION};

        // #define LENGTH_OF_LUT  100
        // REAL lut_current_ctrl[LENGTH_OF_LUT] = {-4.116, -4.032, -3.948, -3.864, -3.78, -3.696, -3.612, -3.528, -3.444, -3.36, -3.276, -3.192, -3.108, -3.024, -2.94, -2.856, -2.772, -2.688, -2.604, -2.52, -2.436, -2.352, -2.268, -2.184, -2.1, -2.016, -1.932, -1.848, -1.764, -1.68, -1.596, -1.512, -1.428, -1.344, -1.26, -1.176, -1.092, -1.008, -0.923998, -0.839998, -0.755998, -0.671998, -0.587998, -0.503998, -0.419998, -0.335999, -0.251999, -0.168, -0.084, -0, 0.084, 0.168, 0.252001, 0.336001, 0.420002, 0.504002, 0.588002, 0.672002, 0.756002, 0.840002, 0.924002, 1.008, 1.092, 1.176, 1.26, 1.344, 1.428, 1.512, 1.596, 1.68, 1.764, 1.848, 1.932, 2.016, 2.1, 2.184, 2.268, 2.352, 2.436, 2.52, 2.604, 2.688, 2.772, 2.856, 2.94, 3.024, 3.108, 3.192, 3.276, 3.36, 3.444, 3.528, 3.612, 3.696, 3.78, 3.864, 3.948, 4.032, 4.116, 4.2};
        // REAL lut_voltage_ctrl[LENGTH_OF_LUT] = {-6.48905, -6.49021, -6.49137, -6.49253, -6.49368, -6.49227, -6.49086, -6.48945, -6.48803, -6.48662, -6.47993, -6.47323, -6.46653, -6.45983, -6.45313, -6.44465, -6.43617, -6.42769, -6.41921, -6.41072, -6.38854, -6.36637, -6.34419, -6.32202, -6.29984, -6.27187, -6.2439, -6.21593, -6.18796, -6.15999, -6.09216, -6.02433, -5.9565, -5.88867, -5.82083, -5.73067, -5.6405, -5.55033, -5.46016, -5.36981, -5.02143, -4.67305, -4.32467, -3.97629, -3.62791, -2.90251, -2.17689, -1.45126, -0.725632, -1e-06, 0.702441, 1.40488, 2.10732, 2.80976, 3.5122, 3.86321, 4.21409, 4.56497, 4.91585, 5.26649, 5.36459, 5.46268, 5.56078, 5.65887, 5.75696, 5.8346, 5.91224, 5.98987, 6.0675, 6.14513, 6.17402, 6.20286, 6.2317, 6.26054, 6.28938, 6.31347, 6.33755, 6.36164, 6.38572, 6.40981, 6.4303, 6.4508, 6.47129, 6.49178, 6.49105, 6.48483, 6.4786, 6.47238, 6.46616, 6.45994, 6.46204, 6.46413, 6.46623, 6.46832, 6.47042, 6.47202, 6.47363, 6.47524, 6.47684, 6.47843};

        // Experimental measurements
        // #define LENGTH_OF_LUT 21
        // REAL lut_current_ctrl[LENGTH_OF_LUT] = {-4.19999, -3.77999, -3.36001, -2.94002, -2.51999, -2.10004, -1.68004, -1.26002, -0.840052, -0.419948, 5.88754e-06, 0.420032, 0.839998, 1.26003, 1.67998, 2.10009, 2.51996, 2.87326, 3.36001, 3.78002, 4.2};
        // REAL lut_voltage_ctrl[LENGTH_OF_LUT] = {-5.20719, -5.2079, -5.18934, -5.15954, -5.11637, -5.04723, -4.93463, -4.76367, -4.42522, -3.46825, 0.317444, 3.75588, 4.55737, 4.87773, 5.04459, 5.15468, 5.22904, 5.33942, 5.25929, 5.28171, 5.30045};

        // Experimental measurements 03-18-2021
        // #define LENGTH_OF_LUT  41
        // REAL lut_voltage_ctrl[LENGTH_OF_LUT] = {-5.75434, -5.74721, -5.72803, -5.70736, -5.68605, -5.66224, -5.63274, -5.59982, -5.56391, -5.52287, -5.47247, -5.40911, -5.33464, -5.25019, -5.14551, -5.00196, -4.80021, -4.48369, -3.90965, -2.47845, -0.382101, 2.02274, 3.7011, 4.35633, 4.71427, 4.94376, 5.10356, 5.22256, 5.31722, 5.39868, 5.46753, 5.5286, 5.57507, 5.62385, 5.66235, 5.70198, 5.73617, 5.76636, 5.79075, 5.81737, 5.83632};

        // Experimental measurements 09-08-2024 under dc bus voltage 30v
        #define LENGTH_OF_LUT  200

        REAL lut_current_ctrl[LENGTH_OF_LUT] = {-11.8776779,-11.4970999,-11.1185436,-10.6474895,-10.3572016,-9.97967339,-9.59940052,-9.21838284,-8.84146309,-8.45851707,-8.07926941,-7.70210457,-7.3201766,-6.94176292,-6.56116295,-6.18242455,-5.80070448,-5.42353058,-5.04246712,-4.66124821,-4.28115511,-3.90180373,-3.51978588,-3.14739537,-2.76055455,-2.38478518,-2.00395393,-1.62294149,-1.24411798,-0.864782751,-0.852897346,-0.839986682,-0.827657998,-0.817189634,-0.804149926,-0.791425109,-0.777046144,-0.769061804,-0.7663396,-0.738772988,-0.729883254,-0.716701627,-0.70259577,-0.691727519,-0.679201066,-0.667932153,-0.652289569,-0.642280698,-0.629698038,-0.619861841,-0.598099232,-0.584517956,-0.582329273,-0.568974316,-0.556745946,-0.538693666,-0.531982005,-0.519891679,-0.506741107,-0.494050056,-0.481414884,-0.470872998,-0.457402825,-0.445215195,-0.432662994,-0.422413617,-0.407728374,-0.395762384,-0.383426517,-0.370804071,-0.358803779,-0.346271962,-0.334479779,-0.321667403,-0.309202373,-0.296646148,-0.284787089,-0.272635549,-0.260316372,-0.24867785,-0.23487936,-0.222468928,-0.209773988,-0.198154747,-0.185585231,-0.172849521,-0.161111414,-0.149662122,-0.136184499,-0.124069326,-0.11144615,-0.100127995,-0.0867423713,-0.0753339007,-0.064514637,-0.0526625887,-0.042088978,-0.0282326955,-0.018909093,0.0,0.0,0.0229527075,0.0320426635,0.0444958396,0.0526987724,0.0645057634,0.0769482702,0.0886555463,0.100649461,0.112727791,0.124460422,0.134906143,0.149320439,0.161667407,0.173398718,0.185959682,0.19734025,0.210464373,0.222451329,0.234747127,0.247389272,0.25959298,0.272228807,0.284861356,0.29690814,0.309453368,0.321903378,0.334397823,0.345709652,0.358895659,0.370920569,0.38305375,0.395625889,0.40785411,0.42085886,0.432268471,0.445510268,0.457848102,0.470408797,0.482290804,0.494613022,0.506820083,0.520010948,0.531044722,0.544490397,0.556701839,0.56927222,0.580917716,0.594336092,0.605698526,0.618099332,0.631471097,0.643096864,0.655367494,0.667800903,0.680245996,0.693202198,0.704810202,0.71733427,0.729529738,0.742241144,0.754837751,0.766754329,0.779603124,0.791106641,0.804524422,0.816357672,0.829134822,0.8409217,0.853817642,0.866759181,1.24582446,1.62573242,2.00434208,2.38476276,2.76386094,3.14391971,3.52466536,3.90304017,4.28352737,4.66305447,5.04260874,5.42156601,5.80167007,6.18104076,6.56304121,6.94168377,7.32083082,7.70139742,8.08096981,8.45969296,8.83988476,9.2195797,9.59952927,9.98017216,10.3582926,10.7395334,11.1177368,11.4968262,11.878686};
        REAL lut_voltage_ctrl[LENGTH_OF_LUT] = {-5.38730097,-5.02007103,-4.87221098,-4.6202035,-4.53324795,-4.44854498,-4.3732028,-4.29975939,-4.23559904,-4.16993046,-4.12249708,-4.03252506,-3.95337582,-3.87713647,-3.81045818,-3.74736428,-3.66991949,-3.59599805,-3.52867842,-3.45353723,-3.38939047,-3.32130456,-3.24308777,-3.17370367,-3.08607507,-3.01022315,-2.91186547,-2.83919501,-2.71364141,-2.62597418,-2.62412024,-2.6177671,-2.61543202,-2.61532426,-2.61382699,-2.61089659,-2.60890865,-2.60546517,-2.60302424,-2.59868336,-2.59785914,-2.59632635,-2.59474063,-2.59468055,-2.59268713,-2.58563113,-2.58523607,-2.5840013,-2.57777619,-2.55525374,-2.55275393,-2.54705667,-2.54614902,-2.54298687,-2.54003334,-2.53673291,-2.53452826,-2.52030921,-2.50073791,-2.496104,-2.48917699,-2.47862411,-2.47006631,-2.46020293,-2.44905758,-2.43852711,-2.43241525,-2.42216134,-2.40960979,-2.40267992,-2.39286017,-2.38239622,-2.3709662,-2.36114168,-2.3499248,-2.3389442,-2.32855582,-2.31582022,-2.30563259,-2.29364181,-2.27984953,-2.26843977,-2.25706625,-2.24375892,-2.22741103,-2.21287704,-2.1969707,-2.18146443,-2.16188359,-2.13189983,-2.09270763,-2.04049397,-1.97806799,-1.90368187,-1.81561983,-1.72044873,-1.61879766,-1.49855673,-1.35436261,0.0,0.0,0.998199284,1.20954859,1.36845672,1.52856529,1.63873553,1.75739849,1.86049855,1.96500587,2.03725982,2.09248209,2.13737798,2.16144872,2.17589974,2.19009066,2.19929051,2.21263218,2.22282434,2.23175001,2.24243355,2.24980879,2.25796008,2.26734352,2.27478409,2.28258562,2.28810096,2.2944901,2.30045795,2.30883789,2.31305194,2.32067561,2.32698846,2.33021784,2.33724499,2.34168768,2.34715605,2.35252976,2.35633898,2.36176372,2.36799812,2.37308788,2.37675405,2.38145471,2.3869226,2.39018703,2.3954072,2.39925385,2.40437961,2.40812206,2.41243339,2.41567254,2.41950607,2.42328215,2.4283421,2.43134332,2.43476081,2.4378922,2.442415,2.44689822,2.44919944,2.45439005,2.4579823,2.46004272,2.46468091,2.46781993,2.4708817,2.47555852,2.47685099,2.48081493,2.48210359,2.48409557,2.59042096,2.68625164,2.77492857,2.86033249,2.94421935,3.02339935,3.10225558,3.18015718,3.25347185,3.32846928,3.40254855,3.47750187,3.55249715,3.62400985,3.6969595,3.7699132,3.84300947,3.91448689,3.98710251,4.0603652,4.13249063,4.20539665,4.27599478,4.34777689,4.41847229,4.48845482,4.56201506,4.95930338,5.33218861};


        REAL ualbe_dist[2];
        get_distorted_voltage_via_LUT( (*CTRL).o->uab_cmd[0], (*CTRL).o->uab_cmd[1], Ia, Ib, ualbe_dist, lut_voltage_ctrl, lut_current_ctrl, LENGTH_OF_LUT);
        (*CTRL).o->uab_cmd_to_inverter[0] = (*CTRL).o->uab_cmd[0] + ualbe_dist[0] * wubo_debug_inverter_conpensation_decrease;
        (*CTRL).o->uab_cmd_to_inverter[1] = (*CTRL).o->uab_cmd[1] + ualbe_dist[1] * wubo_debug_inverter_conpensation_decrease;

        wubo_debug_inverter_conpensation = G.FLAG_INVERTER_NONLINEARITY_COMPENSATION + 1;

        /* For scope only */
        INV.ual_comp = ualbe_dist[0];
        INV.ube_comp = ualbe_dist[1];

    }else if(G.FLAG_INVERTER_NONLINEARITY_COMPENSATION == 2){
        /* 拟合法-补偿 */
        REAL ualbe_dist[2] = {0.0, 0.0};
        get_distorted_voltage_via_CurveFitting( (*CTRL).o->uab_cmd[0], (*CTRL).o->uab_cmd[1], Ia, Ib, ualbe_dist);
        (*CTRL).o->uab_cmd_to_inverter[0] = (*CTRL).o->uab_cmd[0] + ualbe_dist[0];
        (*CTRL).o->uab_cmd_to_inverter[1] = (*CTRL).o->uab_cmd[1] + ualbe_dist[1];

        /* For scope only */
        INV.ual_comp = ualbe_dist[0];
        INV.ube_comp = ualbe_dist[1];

    }else if(G.FLAG_INVERTER_NONLINEARITY_COMPENSATION == 1){
        /* 梯形波自适应 */
        Modified_ParkSul_Compensation();
        (*CTRL).o->uab_cmd_to_inverter[0] = (*CTRL).o->uab_cmd[0] + INV.ual_comp;
        (*CTRL).o->uab_cmd_to_inverter[1] = (*CTRL).o->uab_cmd[1] + INV.ube_comp;

    }else if(G.FLAG_INVERTER_NONLINEARITY_COMPENSATION == 5){
        /* Sigmoid自适应 */
        Online_PAA_Based_Compensation();
        (*CTRL).o->uab_cmd_to_inverter[0] = (*CTRL).o->uab_cmd[0] + INV.ual_comp;
        (*CTRL).o->uab_cmd_to_inverter[1] = (*CTRL).o->uab_cmd[1] + INV.ube_comp;
    }
}



#if MACHINE_TYPE % 10 == 2

void init_experiment(){

    #if ENABLE_COMMISSIONING
        // init_COMM(); // 参数自整定初始化，不要在这里做！
    #endif
    init_CTRL(); // 控制器结构体初始化

    #if MACHINE_TYPE == 2
        init_pmsm_observers(); // 永磁电机观测器初始化
        init_FE();
    #else
        rk4_init();     // 龙格库塔法结构体初始化
        observer_init();
    #endif
}

// 初始化函数
void init_CTRL(){

/* Basic quantities */
    (*CTRL).ID = 10+axisCnt;
    (*CTRL).timebase = 0.0;
    (*CTRL).timebase_counter = 0;

/* Machine parameters */
    // elec
    (*CTRL).motor->R  = PMSM_RESISTANCE;
    (*CTRL).motor->KE = PMSM_PERMANENT_MAGNET_FLUX_LINKAGE; // * (0.1/0.1342); // 【实验编号：】
    (*CTRL).motor->Ld = PMSM_D_AXIS_INDUCTANCE;
    (*CTRL).motor->Lq = PMSM_Q_AXIS_INDUCTANCE;
    (*CTRL).motor->Lq_inv = 1.0/(*CTRL).motor->Lq;
    (*CTRL).motor->DeltaL = (*CTRL).motor->Ld - (*CTRL).motor->Lq; // for IPMSM
    (*CTRL).motor->KActive = (*CTRL).motor->KE; //TODO:

    #if MACHINE_TYPE % 10 == 1
        (*CTRL).motor->Rreq = U_MOTOR_RREQ;
    #endif
    // mech
    (*CTRL).motor->npp     = MOTOR_NUMBER_OF_POLE_PAIRS;
    (*CTRL).motor->npp_inv = 1.0 / (*CTRL).motor->npp;
    (*CTRL).motor->Js      = MOTOR_SHAFT_INERTIA * (1.0+LOAD_INERTIA);
    (*CTRL).motor->Js_inv  = 1.0 / (*CTRL).motor->Js;

// /* Peripheral configurations */

/* Inverter */
    (*CTRL).inv->filter_pole = 3000*2*M_PI;
    inverterNonlinearity_Initialization();
    G.FLAG_INVERTER_NONLINEARITY_COMPENSATION = INVERTER_NONLINEARITY_COMPENSATION_INIT;
    G.FLAG_TUNING_CURRENT_SCALE_FACTOR = TUNING_CURRENT_SCALE_FACTOR_INIT;
    // 程序最开始确保Ta Tb Tc都是0.5
    (*CTRL).svgen1.Ta = 0.5;
    (*CTRL).svgen1.Tb = 0.5;
    (*CTRL).svgen1.Tc = 0.5;
    (*CTRL).svgen2.Ta = 0.5;
    (*CTRL).svgen2.Tb = 0.5;
    (*CTRL).svgen2.Tc = 0.5;



/* Capture */
    (*CTRL).cap->flag_nonlinear_filtering = FALSE;
    (*CTRL).cap->flag_bad_U_capture = FALSE;
    (*CTRL).cap->flag_bad_V_capture = FALSE;
    (*CTRL).cap->flag_bad_W_capture = FALSE;
    (*CTRL).cap->good_capture_U[0] = SYSTEM_HALF_PWM_MAX_COUNT;
    (*CTRL).cap->good_capture_U[1] = SYSTEM_HALF_PWM_MAX_COUNT;
    (*CTRL).cap->good_capture_U[2] = SYSTEM_HALF_PWM_MAX_COUNT;
    (*CTRL).cap->good_capture_U[3] = SYSTEM_HALF_PWM_MAX_COUNT;
    (*CTRL).cap->good_capture_V[0] = SYSTEM_HALF_PWM_MAX_COUNT;
    (*CTRL).cap->good_capture_V[1] = SYSTEM_HALF_PWM_MAX_COUNT;
    (*CTRL).cap->good_capture_V[2] = SYSTEM_HALF_PWM_MAX_COUNT;
    (*CTRL).cap->good_capture_V[3] = SYSTEM_HALF_PWM_MAX_COUNT;
    (*CTRL).cap->good_capture_W[0] = SYSTEM_HALF_PWM_MAX_COUNT;
    (*CTRL).cap->good_capture_W[1] = SYSTEM_HALF_PWM_MAX_COUNT;
    (*CTRL).cap->good_capture_W[2] = SYSTEM_HALF_PWM_MAX_COUNT;
    (*CTRL).cap->good_capture_W[3] = SYSTEM_HALF_PWM_MAX_COUNT;
    (*CTRL).cap->ECapIntCount[0] = 0;
    (*CTRL).cap->ECapIntCount[1] = 0;
    (*CTRL).cap->ECapIntCount[2] = 0;
    (*CTRL).cap->ECapPassCount[0]= 0;
    (*CTRL).cap->ECapPassCount[1]= 0;
    (*CTRL).cap->ECapPassCount[2]= 0;

/* Peripheral configurations */
    //(*CTRL).enc->OffsetCountBetweenIndexAndUPhaseAxis = 0;
    //(*CTRL).enc->theta_d_offset = (*CTRL).enc->OffsetCountBetweenIndexAndUPhaseAxis * CNT_2_ELEC_RAD;

/* Console */
    // See init_experiment_overwrite() in CJHMainISR.c
    G.flag_do_inverter_characteristics = SELF_COMM_INVERTER;
    G.overwrite_vdc = 20;

/* Black Box Model | Controller quantities */

    // PID调谐
    /*  HERE IS IMPORTANT FOR WUBO */
    ACMSIMC_PIDTuner();
    /*  HERE IS IMPORTANT FOR WUBO */

    // 有一个傻逼在这里重新分配了PID_iD，

    // commands
    (*CTRL).i->cmd_psi = U_MOTOR_KE;
    // (*CTRL).i->idq_cmd[0] = 0.0;
    // (*CTRL).i->idq_cmd[1] = 0.0;
    // // error
    // (*CTRL).omg_ctrl_err = 0.0;
    // (*CTRL).speed_ctrl_err = 0.0;
    // // feedback
    // (*CTRL).i->omg_elec = 0.0;
    // (*CTRL).i->theta_d_elec = 0.0;
    // (*CTRL).i->iab[0] = 0.0;
    // (*CTRL).i->iab[1] = 0.0;
    // (*CTRL).i->idq[0] = 0.0;
    // (*CTRL).i->idq[1] = 0.0;
    // (*CTRL).psi_mu_al__fb = 0.0;
    // (*CTRL).psi_mu_be__fb = 0.0;
    // (*CTRL).Tem = 0.0;
    // // indirect field oriented control
    (*CTRL).s->cosT = 1.0;
    (*CTRL).s->sinT = 0.0;
    (*CTRL).s->cosT_compensated_1p5omegaTs = 1.0;
    (*CTRL).s->sinT_compensated_1p5omegaTs = 0.0;
    (*CTRL).s->cosT2 = 1.0;
    (*CTRL).s->sinT2 = 0.0;
    // (*CTRL).s->omega_syn = 0.0;
    (*CTRL).s->the_vc_count = 1; // starts from 1

    (*CTRL).s->ctrl_strategy = CONTROL_STRATEGY;
    (*CTRL).s->go_sensorless = SENSORLESS_CONTROL;

}

/* id=0控制 */
void null_d_control(int set_current_loop, REAL set_iq_cmd, REAL set_id_cmd){
    // 定义局部变量，减少对CTRL的直接调用
    #define MOTOR  (*(*CTRL).motor)

    /// 5. 转速环（使用反馈转速）
    if((*CTRL).s->the_vc_count++ >= SPEED_LOOP_CEILING){
        (*CTRL).s->the_vc_count = 1;

        PID_spd->Ref = (*CTRL).i->cmd_speed_rpm*RPM_2_ELEC_RAD_PER_SEC;
        PID_spd->Fbk = (*CTRL).i->omg_elec;
        PID_spd->calc(PID_spd);
        (*CTRL).i->idq_cmd[1] = PID_spd->Out;
        // (*CTRL).i->idq_cmd[1+2] = PID_spd->Out;
    }
    // 磁链环
    #if CONTROL_STRATEGY == NULL_D_AXIS_CURRENT_CONTROL
        // (*CTRL).i->cmd_rotor_flux_Wb = 0.0;
        // (*CTRL).i->idq_cmd[0] = (*CTRL).i->cmd_rotor_flux_Wb / MOTOR.Ld;

        #if PC_SIMULATION == TRUE
            REAL IMIN = 2;
            // if((*CTRL).timebase>23){
            //     IMIN = 8;
            // }else if((*CTRL).timebase>20){
            //     IMIN = 6;
            // }else if((*CTRL).timebase>15){
            //     IMIN = 5;
            // }else if((*CTRL).timebase>10){
            //     IMIN = 3;
            // }else if((*CTRL).timebase>5){
            //     IMIN = 2;
            // }
        #else
            REAL IMIN = 2; //2;
        #endif
        /* 逆变器非线性在线校正更新theta_t需要空载时电流不为零 */
        // if(fabs((*CTRL).i->idq_cmd[1])<IMIN){
        //     (*CTRL).i->idq_cmd[0] = IMIN;
        // }else{
        //     (*CTRL).i->idq_cmd[0] = 0;
        // }

        /* Overwrite if set_id_cmd is set [Both are POSITIVE] */
        // if(set_id_cmd!=0){
            (*CTRL).i->idq_cmd[0] = set_id_cmd;
            // (*CTRL).i->idq_cmd[0+2] = set_id_cmd;
        // }
    #else
        (*CTRL).i->cmd_rotor_flux_Wb = MOTOR.Ld * set_id_cmd;
        (*CTRL).i->idq_cmd[0] = set_id_cmd;
        #if PC_SIMULATION
            printf("CONTROL_STRATEGY Not Implemented: %s", CONTROL_STRATEGY);getch();
        #endif
    #endif
    // 转矩 For luenberger position observer for HFSI
    (*CTRL).i->Tem     = MOTOR.npp * (MOTOR.KE*(*CTRL).i->idq[1]     + (MOTOR.Ld-MOTOR.Lq)*(*CTRL).i->idq[0]    *(*CTRL).i->idq[1]);
    (*CTRL).i->Tem_cmd = MOTOR.npp * (MOTOR.KE*(*CTRL).i->idq_cmd[1] + (MOTOR.Ld-MOTOR.Lq)*(*CTRL).i->idq_cmd[0]*(*CTRL).i->idq_cmd[1]);

    /// 5.Sweep 扫频将覆盖上面产生的励磁、转矩电流指令
    #if EXCITATION_TYPE == EXCITATION_SWEEP_FREQUENCY
        #if SWEEP_FREQ_C2V == TRUE
            PID_iQ->Ref = set_iq_cmd; 
        #endif
        #if SWEEP_FREQ_C2C == TRUE
            PID_iQ->Ref = 0.0;
            PID_iD->Ref = set_iq_cmd; // 故意反的
        #endif
    #endif

    // debug
    // set_current_loop = 1;
    // set_iq_cmd = 1;

    /// 6. 电流环
    // d-axis
    PID_iD->Fbk = (*CTRL).i->idq[0];
    PID_iD->Ref = (*CTRL).i->idq_cmd[0];
    PID_iD->calc(PID_iD);
    // pid2_id.Fbk = (*CTRL).i->idq[0+2];
    // pid2_id.Ref = (*CTRL).i->idq_cmd[0+2];
    // pid2_id.calc(pid2_id);
    // q-axis
    PID_iQ->Fbk = (*CTRL).i->idq[1];
    PID_iQ->Ref = (*CTRL).i->idq_cmd[1]; 
    if(set_current_loop==1){
        PID_iQ->Ref = set_iq_cmd; (*CTRL).i->idq_cmd[1] = set_iq_cmd;
        }
    PID_iQ->calc(PID_iQ);
    // pid2_iq.Fbk = (*CTRL).i->idq[1+2];
    // pid2_iq.Ref = (*CTRL).i->idq_cmd[1+2]; if(set_current_loop==1){pid2_iq.Ref = set_iq_cmd;}
    // pid2_iq.calc(pid2_iq);
    // 电压电流解耦
    #if VOLTAGE_CURRENT_DECOUPLING_CIRCUIT == TRUE
        REAL decoupled_d_axis_voltage = PID_iD->Out -             PID_iQ->Fbk*MOTOR.Lq *(*CTRL).i->omg_elec;
        REAL decoupled_q_axis_voltage = PID_iQ->Out + ( MOTOR.KE + PID_iD->Fbk*MOTOR.Ld)*(*CTRL).i->omg_elec;
    #else
        REAL decoupled_d_axis_voltage = PID_iD->Out;
        REAL decoupled_q_axis_voltage = PID_iQ->Out;
    #endif
    (*CTRL).o->udq_cmd[0] = decoupled_d_axis_voltage;
    (*CTRL).o->udq_cmd[1] = decoupled_q_axis_voltage;
    // (*CTRL).o->udq_cmd[0+2] = pid2_id.Out;
    // (*CTRL).o->udq_cmd[1+2] = pid2_iq.Out;
}

REAL controller(REAL set_rpm_speed_command, 
    int set_current_loop, REAL set_iq_cmd, REAL set_id_cmd,
    int flag_overwrite_theta_d, REAL Overwrite_Current_Frequency,
    REAL used_theta_d_elec,
    REAL angle_shift_for_first_inverter,
    REAL angle_shift_for_second_inverter)
{
    /// 0. 参数时变
    // if (fabs((*CTRL).timebase-2.0)<CL_TS){
    //     printf("[Runtime] Rotor resistance of the simulated IM has changed!\n");
    //     ACM.alpha = 0.5*IM_ROTOR_RESISTANCE / IM_MAGNETIZING_INDUCTANCE;
    //     ACM.rreq = ACM.alpha*ACM.Lmu;
    //     ACM.rr   = ACM.alpha*(ACM.Lm+ACM.Llr);
    // }

    /// 1. 生成转速指令
    (*CTRL).i->cmd_position_rad    = 0.0;  // mechanical
    (*CTRL).i->cmd_speed_rpm       = set_rpm_speed_command;     // mechanical
    (*CTRL).i->cmd_omg_elec        = (*CTRL).i->cmd_speed_rpm * RPM_2_ELEC_RAD_PER_SEC; // electrical

    /// 2. 生成磁链指令
    if(set_id_cmd==0.0){
        // if(set_iq_cmd<1){
        //     set_id_cmd = 1; // 2;
        // }
    }

    /// 3. 调用观测器：估计的电气转子位置和电气转子转速反馈
    // pmsm_observers();
    // if((*CTRL).s->go_sensorless == TRUE){
    //     //（无感）
    //     (*CTRL).i->omg_elec     = ELECTRICAL_SPEED_FEEDBACK;    //harnefors.omg_elec;
    //     (*CTRL).i->theta_d_elec = ELECTRICAL_POSITION_FEEDBACK; //harnefors.theta_d;
    // }

    /// 4. 帕克变换（使用反馈位置）
    if(flag_overwrite_theta_d)
    {
        if(fabs(Overwrite_Current_Frequency)>0)
        {
            used_theta_d_elec += CL_TS * Overwrite_Current_Frequency * 2 * M_PI ;
            while(used_theta_d_elec> M_PI) used_theta_d_elec -= 2*M_PI;
            while(used_theta_d_elec<-M_PI) used_theta_d_elec += 2*M_PI;
        }
        else
        {
            used_theta_d_elec = 0.0;
        }
    }
    else // 
    {
        used_theta_d_elec = (*CTRL).i->theta_d_elec;
    }
    (*CTRL).s->cosT = cos(used_theta_d_elec + angle_shift_for_first_inverter); // 此处的angle_shift都是0
    (*CTRL).s->sinT = sin(used_theta_d_elec + angle_shift_for_first_inverter);
    (*CTRL).i->idq[0] = AB2M((*CTRL).i->iab[0], (*CTRL).i->iab[1], (*CTRL).s->cosT, (*CTRL).s->sinT);
    (*CTRL).i->idq[1] = AB2T((*CTRL).i->iab[0], (*CTRL).i->iab[1], (*CTRL).s->cosT, (*CTRL).s->sinT);

    // (*CTRL).s->cosT2 = cos(used_theta_d_elec + angle_shift_for_second_inverter);
    // (*CTRL).s->sinT2 = sin(used_theta_d_elec + angle_shift_for_second_inverter);
    // (*CTRL).i->idq[0+2] = AB2M((*CTRL).i->iab[0+2], (*CTRL).i->iab[1+2], (*CTRL).s->cosT2, (*CTRL).s->sinT2);
    // (*CTRL).i->idq[1+2] = AB2T((*CTRL).i->iab[0+2], (*CTRL).i->iab[1+2], (*CTRL).s->cosT2, (*CTRL).s->sinT2);

    /* 更新依赖于dq轴电流的物理量 */
    // MOTOR.KActive = MOTOR.KE + (MOTOR.Ld - MOTOR.Lq) * (*CTRL).i->idq_cmd[0];
    MOTOR.KActive = MOTOR.KE + (MOTOR.Ld - MOTOR.Lq) * (*CTRL).i->idq[0];

    /// 调用具体的控制器
    #if CONTROL_STRATEGY == NULL_D_AXIS_CURRENT_CONTROL
        null_d_control(set_current_loop, set_iq_cmd, set_id_cmd);
        (*CTRL).s->Motor_or_Generator = sign((*CTRL).i->omg_elec * (*CTRL).i->idq_cmd[1]); // sign((*CTRL).i->cmd_omg_elec);
    #endif


    /// 7. 反帕克变换
    // See D:\Users\horyc\Downloads\Documents\2003 TIA Bae SK Sul A compensation method for time delay of.pdf
    // (*CTRL).s->cosT_compensated_1p5omegaTs = cos(used_theta_d_elec + 1.5*(*CTRL).i->omg_elec*CL_TS);
    // (*CTRL).s->sinT_compensated_1p5omegaTs = sin(used_theta_d_elec + 1.5*(*CTRL).i->omg_elec*CL_TS);
    (*CTRL).s->cosT_compensated_1p5omegaTs = (*CTRL).s->cosT;
    (*CTRL).s->sinT_compensated_1p5omegaTs = (*CTRL).s->sinT;
    (*CTRL).o->uab_cmd[0] = MT2A((*CTRL).o->udq_cmd[0], (*CTRL).o->udq_cmd[1], (*CTRL).s->cosT_compensated_1p5omegaTs, (*CTRL).s->sinT_compensated_1p5omegaTs);
    (*CTRL).o->uab_cmd[1] = MT2B((*CTRL).o->udq_cmd[0], (*CTRL).o->udq_cmd[1], (*CTRL).s->cosT_compensated_1p5omegaTs, (*CTRL).s->sinT_compensated_1p5omegaTs);
    (*CTRL).o->iab_cmd[0] = MT2A((*CTRL).i->idq_cmd[0], (*CTRL).i->idq_cmd[1], (*CTRL).s->cosT_compensated_1p5omegaTs, (*CTRL).s->sinT_compensated_1p5omegaTs);
    (*CTRL).o->iab_cmd[1] = MT2B((*CTRL).i->idq_cmd[0], (*CTRL).i->idq_cmd[1], (*CTRL).s->cosT_compensated_1p5omegaTs, (*CTRL).s->sinT_compensated_1p5omegaTs);

    // (*CTRL).o->uab_cmd[0+2] = MT2A((*CTRL).o->udq_cmd[0+2], (*CTRL).o->udq_cmd[1+2], (*CTRL).s->cosT2, (*CTRL).s->sinT2);
    // (*CTRL).o->uab_cmd[1+2] = MT2B((*CTRL).o->udq_cmd[0+2], (*CTRL).o->udq_cmd[1+2], (*CTRL).s->cosT2, (*CTRL).s->sinT2);

    /// 8. 补偿逆变器非线性
    REAL wubo_debug_use_bool_idq_cmd = 0;

    main_inverter_voltage_command(wubo_debug_use_bool_idq_cmd);

    /// 9. 结束
    #if PC_SIMULATION
        // for plot
        ACM.rpm_cmd = set_rpm_speed_command;
        // (*CTRL).speed_ctrl_err = set_rpm_speed_command*RPM_2_ELEC_RAD_PER_SEC - (*CTRL).i->omg_elec;
    #endif

    return used_theta_d_elec;
}





#endif
