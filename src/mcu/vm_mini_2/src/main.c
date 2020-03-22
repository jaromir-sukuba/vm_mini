#include "hw.h"
#include "disp.h"
#include <stdio.h>
#include <string.h>
#include <math.h>


void main_loop_content (void);
double get_voltage (uint8_t * adc_arrray, uint8_t it_adc, float cf);

key_var keys, keys_new,keys_old,keys_older;
struct cal_struct	cal_data;


volatile uint8_t rx_msg[RX_MSG_LEN],rx_msg_ptr,rx_flag,adc_data[6];
char disp_msg[50], meas_disp_arr[20];
uint16_t cnt,on_state,k1_cnt,sr_data_active, sr_data_az;
double adc_in_arr[25],adc_az_arr[25],adc_in,adc_az,adc_raw_value,bat_volt,it_raw_cf,relative_val;
uint8_t tx_msg[50],lcd_pix_buffer[132*4],meas_state,meas_cnt,data_flag,avg_size,meas_count;
uint8_t it_array_ptr,it_update,it,it_raw,it_adc,it_mul,relative_act;
uint8_t meas_type, meas_range, shift, meas_auto,lcd_update,input_res,ohm_4w,bklt;
uint8_t it_array[10]=    {1, 2, 5, 10, 20, 50, 100, 0};
uint8_t it_array_adc[10]={0, 1, 4, 9, 9, 9, 9, 0};
uint8_t it_array_mul[10]={1, 1, 1, 1, 2, 5, 10, 0};
uint8_t it_array_raw[10]={1, 2, 5, 10, 10, 10, 10, 0};
double it_array_rcf[10] = {1.000145,1.000060,1.000015,1.0,1.0,1.0,1.0};	//TODO



int main(void)
	{
    hw_init ();
    disp_init();
    sr_shift16(0x0 | SR_M11 | SR_M24 | SR_M22 | SR_K2 | SR_K1);
    avg_size = 1;
    meas_type = MEAS_VOLT;
    meas_range = MEAS_VOLT_RN-1;
    it_update=1;
    it_array_ptr = 3;
    bklt = 1;

    // load calbration structure with data I manually measured
    // should use external FLASH and add code to perform user
    // calibration from panel
    cal_data.adc_step = CAL_PREDEF_STEP;
    cal_data.adc_z_offset = CAL_PREDEF_NZO;
    cal_data.v_1V_offset = CAL_PREDEF_1VO;
    cal_data.v_1V_gain = CAL_PREDEF_1VG;
    cal_data.v_10V_offset = CAL_PREDEF_10VO;
    cal_data.v_10V_gain = CAL_PREDEF_10VG;
    cal_data.v_100V_offset = CAL_PREDEF_100VO;
    cal_data.v_100V_gain = CAL_PREDEF_100VG;
    cal_data.r_1k_offset = CAL_PREDEF_1KO;
    cal_data.r_1k_gain = CAL_PREDEF_1KG;
    cal_data.r_10k_offset = CAL_PREDEF_10KO;
    cal_data.r_10k_gain = CAL_PREDEF_10KG;
    cal_data.r_100k_offset = CAL_PREDEF_100KO;
    cal_data.r_100k_gain = CAL_PREDEF_100KG;
    cal_data.r_1M_offset = CAL_PREDEF_1MO;
    cal_data.r_1M_gain = CAL_PREDEF_1MG;
    cal_data.r_10M_offset = CAL_PREDEF_10MO;
    cal_data.r_10M_gain = CAL_PREDEF_10MG;

//    on_state = RUN_STATE_SHDN;
//    if (K_K1_R==0)
    	on_state = RUN_STATE_RUN;
    while (1)
	{
    if (on_state==RUN_STATE_SHDN)
    	{
    	turn_off();
    	on_state = RUN_STATE_RUN_ENTRY;
    	k1_cnt=0;
    	D_BK_L;
    	}
    if (on_state==RUN_STATE_RUN_ENTRY)
    	{
    	if (k1_cnt>5) NVIC_SystemReset();
    	if (K_K1_R==1) on_state = RUN_STATE_SHDN;
    	}
    if (on_state == RUN_STATE_RUN)
    	{
    	main_loop_content();
        if (k1_cnt>5)
        	{
        	if (shift==1) on_state = 0;
        	}
    	}
    if (TIM7->SR)
		{
    	TIM7->SR = 0;
    	if (on_state == RUN_STATE_RUN) bat_volt = get_bat_volt();
    	keys_new.CHAR = 0;
        if (K_K1_R==0)
        	{
        	keys_new.k1 = 1;
        	k1_cnt++;
        	}
        else
        	k1_cnt=0;
        if (K_K2_R==0) keys_new.k2 = 1;
        if (K_K3_R==0) keys_new.k3 = 1;
        if (K_K4_R==0) keys_new.k4 = 1;
        if (K_K5_R==0) keys_new.k5 = 1;
        if (K_K6_R==0) keys_new.k6 = 1;
    	keys.CHAR = keys.CHAR | ((keys_new.CHAR^keys_old.CHAR)&keys_new.CHAR);
    	keys_old.CHAR =keys_new.CHAR;
		}


    if (on_state == RUN_STATE_RUN)
    	{
        if (bklt)
        	D_BK_H;
        else
        	D_BK_L;
        //keys handling. due to a lot of alternative functions the code gets hairy quite a bit
		if (keys.k1)
			{
			keys.k1 = 0;
			if (on_state == RUN_STATE_RUN)
			if (shift==0)
				{
				if (bklt==0) bklt = 1;
				else bklt = 0;
				}
			}
		if (keys.k2)
			{
			keys.k2 = 0;
			lcd_update = 1;
			if (shift==1)
				{
				input_res++;
				if (input_res==MEAS_IR_RN) input_res=0;
				}
			else if (shift==2)
				{
				if (relative_act==1) relative_act = 0;
				else relative_act = 1;
				relative_val = adc_in;
				}
			else
				{
				meas_type=MEAS_VOLT;
				meas_range=MEAS_VOLT_RN-1;
				}
			}
		if (keys.k3)
			{
			keys.k3 = 0;
			lcd_update = 1;
			if (shift==1)
				{
				if (ohm_4w==0) ohm_4w = 1;
				else ohm_4w = 0;
				}
			else if (shift==2)
				{
				meas_type=MEAS_DIO;
				meas_range=MEAS_DIO_RN-1;
				}
			else
				{
				meas_type=MEAS_OHM;
				meas_range=MEAS_OHM_RN-1;
				}
			}
		if (keys.k4)
			{
			keys.k4 = 0;
			lcd_update = 2;
			if (shift==0) shift = 1;
			else if (shift==1) shift = 2;
			else shift = 0;
			}
		if (keys.k6)
			{
			keys.k6 = 0;
			lcd_update = 1;
			if (shift==1)
				{
				it_array_ptr++;
				if (it_array[it_array_ptr]==0) it_array_ptr--;
				it_update=1;
				}
			else
				{
				relative_act = 0;
				if (meas_type==MEAS_VOLT)
					{
					meas_range++;
					if (meas_range>=MEAS_VOLT_RN) meas_range--;
					}
				if (meas_type==MEAS_OHM)
					{
					meas_range++;
					if (meas_range>=MEAS_OHM_RN) meas_range--;
					}
				if (meas_type==MEAS_DIO)
					{
					meas_range++;
					if (meas_range>=MEAS_DIO_RN) meas_range--;
					}
				}
			}
		if (keys.k5)
			{
			keys.k5 = 0;
			lcd_update = 1;
			if (shift==1)
				{
				if (it_array_ptr>0) it_array_ptr--;
				if (it_array[it_array_ptr]==0) it_array_ptr--;
				it = it_array[it_array_ptr];
				it_update=1;
				}
			else
				{
				relative_act = 0;
				if (meas_type==MEAS_VOLT)
					{
					if (meas_range>0) meas_range--;
					if (meas_range>=MEAS_VOLT_RN) meas_range--;
					}
				if (meas_type==MEAS_OHM)
					{
					if (meas_range>0) meas_range--;
					if (meas_range>=MEAS_OHM_RN) meas_range--;
					}
				if (meas_type==MEAS_DIO)
					{
					if (meas_range>0) meas_range--;
					if (meas_range>=MEAS_DIO_RN) meas_range--;
					}
				}
			}
		}
	}
}


void main_loop_content (void)
	{
	uint16_t i;
    if (it_update==1)					//integration time update
    	{
    	it_update=0;
    	it_adc = it_array_adc[it_array_ptr];
    	it_mul = it_array_mul[it_array_ptr];
    	it_raw = it_array_raw[it_array_ptr];
    	it_raw_cf = it_array_rcf[it_array_ptr];
    	it = it_array[it_array_ptr];
    	avg_size = it_mul;
    	meas_state=0;
    	A_CT_L;							//keep FPGA state machine in reset
    	refresh_lcd_line(0,0);			//somehow stupid way of doing time delay
    	usart_send_byte(0x50+it_adc);	//now send the actual integration time update
    	refresh_lcd_line(0,0);			//if it's stupid but it works, it isn't stupid
    	A_CT_H;							//let FPGA SM run
    	}
    if (lcd_update==1)
    	{
    	lcd_update = 0;
    	fill_disp_buff(0);
    	if (relative_act==1)
    		strncat(meas_disp_arr," R",2);	//if relative mode is on, display R to notice user
    	else
    		strncat(meas_disp_arr,"  ",2);
    	if (meas_type==MEAS_VOLT)
    		{
			print_str(meas_disp_arr,0,0,BIG,0);
    		if (meas_range==0) strcpy(disp_msg,"Range: 1V");
    		if (meas_range==1) strcpy(disp_msg,"Range: 10V");
    		if (meas_range==2) strcpy(disp_msg,"Range: 100V");
			print_str(disp_msg,2,0,SMALL,0);
			if (meas_range==2) strcpy(disp_msg,"Ri=10M");
			else
				{
				if (input_res==0) strcpy(disp_msg,"Ri=1M");
				if (input_res==1) strcpy(disp_msg,"Ri=10M");
				if (input_res==2) strcpy(disp_msg,"Ri>1G");
				}
			print_str(disp_msg,2,70,SMALL,0);
    		}
    	if (meas_type==MEAS_OHM)
    		{
			print_str(meas_disp_arr,0,0,BIG,0);
    		if (meas_range==0) strcpy(disp_msg,"Range: 1kOhm");
    		if (meas_range==1) strcpy(disp_msg,"Range: 10kOhm");
    		if (meas_range==2) strcpy(disp_msg,"Range: 100kOhm");
    		if (meas_range==3) strcpy(disp_msg,"Range: 1MOhm");
    		if (meas_range==4) strcpy(disp_msg,"Range: 10MOhm");
			print_str(disp_msg,2,0,SMALL,0);
    		}
    	if (meas_type==MEAS_DIO)
    		{
			print_str(meas_disp_arr,0,0,BIG,0);
    		if (meas_range==0) strcpy(disp_msg,"Range: 1mA");
    		if (meas_range==1) strcpy(disp_msg,"Range: 100uA");
    		if (meas_range==2) strcpy(disp_msg,"Range: 10uA");
    		if (meas_range==3) strcpy(disp_msg,"Range: 1uA");
			print_str(disp_msg,2,0,SMALL,0);
    		}
    	if (meas_count&0x01) print_str("_",0,120,BIG,0);	//blinky jumpy thing in ADC rhythm
    	else print_str("-",0,120,BIG,0);
    	if (shift==1) print_str("2nd",3,114,SMALL,1);
    	else if (shift==2) print_str("3rd",3,114,SMALL,1);
    	if (ohm_4w) print_str("4W",2,114,SMALL,1);
    	sprintf(disp_msg,"Bat:%1.1fV I:%dPLC",bat_volt,it);
    	print_str(disp_msg,3,0,SMALL,0);
    	refresh_lcd();
    	}

    if (lcd_update==2)
    	{
    	lcd_update = 0;
    	if (shift==1) print_str("2nd",3,114,SMALL,1);
    	else if (shift==2) print_str("3rd",3,114,SMALL,1);
    	else print_str("     ",3,114,SMALL,0);
    	refresh_lcd_line(3,114);
    	}

	if (rx_flag==1)					//adc result (raw packet from FPGA) is here, decide what to do with it
	    {
	    rx_flag=0;
	    adc_raw_value = get_voltage(adc_data,it_raw,it_raw_cf);
	    if (meas_state==0)
			{
			meas_state=1;
			meas_cnt=0;
			}
	    else if (meas_state==1)
			{
			adc_in_arr[meas_cnt] = adc_raw_value;
			meas_cnt++;
			if (meas_cnt==avg_size)
				{
				meas_state=2;
				meas_cnt=0;
				sr_shift16(sr_data_az);
				}
			}
	    else if (meas_state==2)
			{
			meas_state=3;
			meas_cnt=0;
			}
	    else if (meas_state==3)
			{
			adc_az_arr[meas_cnt] = adc_raw_value;
			meas_cnt++;
			if (meas_cnt==avg_size)
				{
				meas_state=4;
				meas_cnt=0;
				sr_shift16(sr_data_active);
				}
			}
	    if (meas_state==4)
			{
			meas_state=0;
			adc_in=0.0;
			adc_az=0.0;
			for (i=0;i<avg_size;i++)
				{
				adc_in = adc_in + adc_in_arr[i];
				adc_az = adc_az + adc_az_arr[i];
				}
			adc_in = adc_in / avg_size;
			adc_az = adc_az / avg_size;
			data_flag = 1;
			}

	    if (data_flag)		//new numeric result (after AZ and optional averaging) is available
			{				//decide how to display it, depending on range, also setup switches accordingly
			data_flag = 0;
			meas_count++;
			adc_in = adc_in / 1000.0;
			adc_az = adc_az / 1000.0;
			adc_in = adc_in - adc_az;
			lcd_update = 1;
			if (meas_type==MEAS_VOLT)
				{
				if (meas_range==0)
					{
					//1V range
					adc_in = adc_in / 10.0;
					adc_in = adc_in + cal_data.v_1V_offset;
					adc_in = adc_in * cal_data.v_1V_gain;
					if (relative_act) adc_in = adc_in - relative_val;
					sprintf(meas_disp_arr,"%+9.6fV",adc_in);
					sr_data_az = SR_K1 | SR_M14 | SR_M24 | SR_M21;
					sr_data_active = SR_K1 | SR_M11 | SR_M24 | SR_M21;
					if (input_res==0)
						{
						sr_data_active |= SR_K4;
						sr_data_az |= SR_K4;
						}
					if (input_res==1)
						{
						sr_data_active |= SR_K2;
						sr_data_az |= SR_K2;
						}
					}
				if (meas_range==1)
					{
					//10V range
					adc_in = adc_in * 1.0;
					adc_in = adc_in + cal_data.v_10V_offset;
					adc_in = adc_in * cal_data.v_10V_gain;
					if (relative_act) adc_in = adc_in - relative_val;
					sprintf(meas_disp_arr,"%+9.5fV",adc_in);
					sr_data_az = SR_K1 | SR_M14 | SR_M24 | SR_M22;
					sr_data_active = SR_K1 | SR_M11 | SR_M24 | SR_M22;
					if (input_res==0)
						{
						sr_data_active |= SR_K4;
						sr_data_az |= SR_K4;
						}
					if (input_res==1)
						{
						sr_data_active |= SR_K2;
						sr_data_az |= SR_K2;
						}
					}
				if (meas_range==2)
					{
					//100V range
					adc_in = adc_in * 10.0;
					adc_in = adc_in + cal_data.v_100V_offset;
					adc_in = adc_in * cal_data.v_100V_gain;
					if (relative_act) adc_in = adc_in - relative_val;
					sprintf(meas_disp_arr,"%+9.4fV",adc_in);
					sr_data_az = SR_K2 | SR_K4 | SR_M14 | SR_M24 | SR_M22;
					sr_data_active = SR_K2 | SR_K4 | SR_M11 | SR_M24 | SR_M22;
					}
				}
			if (meas_type==MEAS_OHM)
				{
				if (meas_range==0)
					{
					//1k range - 1mA/G10
					adc_in = adc_in * 100.0;
					adc_in = adc_in + cal_data.r_1k_offset;
					adc_in = adc_in * cal_data.r_1k_gain;
					if (relative_act) adc_in = adc_in - relative_val;
					if (adc_in<1020.0)
						sprintf(meas_disp_arr,"%+09.3fOhm",adc_in);
					else
						sprintf(meas_disp_arr,"Overload");
					if (ohm_4w==0)
						{
						sr_data_active = SR_K1 | SR_K3 | SR_M11 | SR_M21 | SR_M3B | SR_M3D;
						sr_data_az = SR_K1 | SR_K3 | SR_M14 | SR_M21 | SR_M3B | SR_M3D | SR_M24;
						}
					else
						{
						sr_data_active = SR_K1 | SR_K3 | SR_M12 | SR_M21 | SR_M3B | SR_M3D;
						sr_data_az = SR_K1 | SR_K3 | SR_M13 | SR_M21 | SR_M3B | SR_M3D | SR_M24;
						}
					}
				if (meas_range==1)
					{
					//10k range - 1mA/G1
					adc_in = adc_in * 1.0;
					adc_in = adc_in + cal_data.r_10k_offset;
					adc_in = adc_in * cal_data.r_10k_gain;
					if (relative_act) adc_in = adc_in - relative_val;
					if (adc_in<10.20)
						sprintf(meas_disp_arr,"%+09.5fkOhm",adc_in);
					else
						sprintf(meas_disp_arr,"Overload");
					if (ohm_4w==0)
						{
						sr_data_active = SR_K1 | SR_K3 | SR_M11 | SR_M22 | SR_M3B | SR_M3D;
						sr_data_az = SR_K1 | SR_K3 | SR_M14 | SR_M22 | SR_M3B | SR_M3D | SR_M24;
						}
					else
						{
						sr_data_active = SR_K1 | SR_K3 | SR_M12 | SR_M22 | SR_M3B | SR_M3D;
						sr_data_az = SR_K1 | SR_K3 | SR_M13 | SR_M22 | SR_M3B | SR_M3D | SR_M24;
						}
					}
				if (meas_range==2)
					{
					//100k range - 100uA/G1
					adc_in = adc_in * 10.0;
					adc_in = adc_in + cal_data.r_100k_offset;
					adc_in = adc_in * cal_data.r_100k_gain;
					if (relative_act) adc_in = adc_in - relative_val;
					if (adc_in<102.0)
						sprintf(meas_disp_arr,"%+09.4fkOhm",adc_in);
					else
						sprintf(meas_disp_arr,"Overload");

					if (ohm_4w==0)
						{
						sr_data_active = SR_K1 | SR_K3 | SR_M11 | SR_M22 | SR_M3B | SR_M3C;
						sr_data_az = SR_K1 | SR_K3 | SR_M14 | SR_M22 | SR_M3B | SR_M3C | SR_M24;
						}
					else
						{
						sr_data_active = SR_K1 | SR_K3 | SR_M12 | SR_M22 | SR_M3B | SR_M3C;
						sr_data_az = SR_K1 | SR_K3 | SR_M13 | SR_M22 | SR_M3B | SR_M3C | SR_M24;
						}
					}
				if (meas_range==3)
					{
					//1M range - 10uA/G1
					adc_in = adc_in * 100.0;
					adc_in = adc_in + cal_data.r_1M_offset;
					adc_in = adc_in * cal_data.r_1M_gain;
					if (relative_act) adc_in = adc_in - relative_val;
					if (adc_in<1020.0)
						sprintf(meas_disp_arr,"%+09.3fkOhm",adc_in);
					else
						sprintf(meas_disp_arr,"Overload");
					if (ohm_4w==0)
						{
						sr_data_active = SR_K1 | SR_K3 | SR_M11 | SR_M22 | SR_M3A | SR_M3D;
						sr_data_az = SR_K1 | SR_K3 | SR_M14 | SR_M22 | SR_M3A | SR_M3D | SR_M24;
						}
					else
						{
						sr_data_active = SR_K1 | SR_K3 | SR_M12 | SR_M22 | SR_M3A | SR_M3D;
						sr_data_az = SR_K1 | SR_K3 | SR_M13 | SR_M22 | SR_M3A | SR_M3D | SR_M24;
						}
					}
				if (meas_range==4)
					{
					//10M range - 1uA/G1
					adc_in = adc_in * 1.0;
					adc_in = adc_in + cal_data.r_10M_offset;
					adc_in = adc_in * cal_data.r_10M_gain;
					if (relative_act) adc_in = adc_in - relative_val;
					if (adc_in<10.20)
						sprintf(meas_disp_arr,"%+09.5fMOhm",adc_in);
					else
						sprintf(meas_disp_arr,"Overload");
					if (ohm_4w==0)
						{
						sr_data_active = SR_K1 | SR_K3 | SR_M11 | SR_M22 | SR_M3A | SR_M3C;
						sr_data_az = SR_K1 | SR_K3 | SR_M14 | SR_M22 | SR_M3A | SR_M3C | SR_M24;
						}
					else
						{
						sr_data_active = SR_K1 | SR_K3 | SR_M12 | SR_M22 | SR_M3A | SR_M3C;
						sr_data_az = SR_K1 | SR_K3 | SR_M13 | SR_M22 | SR_M3A | SR_M3C | SR_M24;
						}
					}
				}
			if (meas_type==MEAS_DIO)
				{
				adc_in = adc_in * 1.0;
				adc_in = adc_in + cal_data.v_10V_offset;
				adc_in = adc_in * cal_data.v_10V_gain;
				if (adc_in<10.20)
					sprintf(meas_disp_arr,"%+09.5fV",adc_in);
				else
					sprintf(meas_disp_arr,"Overload");

				if (meas_range==0)
					{
					//1mA/G1
					if (ohm_4w==0)
						{
						sr_data_active = SR_K1 | SR_K3 | SR_M11 | SR_M22 | SR_M3B | SR_M3D;
						sr_data_az = SR_K1 | SR_K3 | SR_M14 | SR_M22 | SR_M3B | SR_M3D | SR_M24;
						}
					else
						{
						sr_data_active = SR_K1 | SR_K3 | SR_M12 | SR_M22 | SR_M3B | SR_M3D;
						sr_data_az = SR_K1 | SR_K3 | SR_M13 | SR_M22 | SR_M3B | SR_M3D | SR_M24;
						}
					}
				if (meas_range==1)
					{
					//100uA/G1
					if (ohm_4w==0)
						{
						sr_data_active = SR_K1 | SR_K3 | SR_M11 | SR_M22 | SR_M3B | SR_M3C;
						sr_data_az = SR_K1 | SR_K3 | SR_M14 | SR_M22 | SR_M3B | SR_M3C | SR_M24;
						}
					else
						{
						sr_data_active = SR_K1 | SR_K3 | SR_M12 | SR_M22 | SR_M3B | SR_M3C;
						sr_data_az = SR_K1 | SR_K3 | SR_M13 | SR_M22 | SR_M3B | SR_M3C | SR_M24;
						}
					}
				if (meas_range==2)
					{
					//10uA/G1
					if (ohm_4w==0)
						{
						sr_data_active = SR_K1 | SR_K3 | SR_M11 | SR_M22 | SR_M3A | SR_M3D;
						sr_data_az = SR_K1 | SR_K3 | SR_M14 | SR_M22 | SR_M3A | SR_M3D | SR_M24;
						}
					else
						{
						sr_data_active = SR_K1 | SR_K3 | SR_M12 | SR_M22 | SR_M3A | SR_M3D;
						sr_data_az = SR_K1 | SR_K3 | SR_M13 | SR_M22 | SR_M3A | SR_M3D | SR_M24;
						}
					}
				if (meas_range==3)
					{
					//1uA/G1
					if (ohm_4w==0)
						{
						sr_data_active = SR_K1 | SR_K3 | SR_M11 | SR_M22 | SR_M3A | SR_M3C;
						sr_data_az = SR_K1 | SR_K3 | SR_M14 | SR_M22 | SR_M3A | SR_M3C | SR_M24;
						}
					else
						{
						sr_data_active = SR_K1 | SR_K3 | SR_M12 | SR_M22 | SR_M3A | SR_M3C;
						sr_data_az = SR_K1 | SR_K3 | SR_M13 | SR_M22 | SR_M3A | SR_M3C | SR_M24;
						}
					}
				}
			usart2_send_str(meas_disp_arr);
			usart2_send_str("\n");
			}
	    }
	}


double get_voltage (uint8_t * adc_arrray, uint8_t it_adc, float cf)
    {
    int64_t tmp1,tmp2;
    uint16_t adc_up, adc_total, adc_rundown;
    uint8_t adc_x;
    double retval;
    adc_up = (((uint16_t)(adc_arrray[0]))<<8)+(((uint16_t)(adc_arrray[1]))<<0);
    adc_total = (((uint16_t)(adc_arrray[2]&0x0F))<<8)+(((uint16_t)(adc_arrray[3]))<<0);
    adc_rundown = (((uint16_t)(adc_arrray[4]))<<8)+(((uint16_t)(adc_arrray[5]))<<0);
    adc_x = (adc_arrray[2]&0x80)>>7;
    if (adc_x==1)
    	tmp1 = adc_rundown+(cal_data.adc_z_offset);
    else
    	tmp1 = -adc_rundown;
    tmp2 = (adc_total+1)/2;
    tmp2 = tmp2-adc_up;
    tmp2 = tmp2*cal_data.adc_step;
    tmp2 = tmp2 - tmp1;
    retval = tmp2;							//tmp2 now holds ADC counts
    retval = retval/338.10;					//roughly map ADC counts to voltage
    retval = retval/((double)(it_adc));		//correction to integration time
    retval = retval / cf;
    return retval;
    }

//*************************************************************************************
//*************************************************************************************

void TIM6_IRQHandler(void)
    {
    TIM6->SR=0;
    TIM6->CR1=0x0000;
    NVIC_DisableIRQ(TIM6_IRQn);
    if (rx_msg_ptr==6)					//once guard timer expired and there is 6 characters in
		{								//buffer, we have new ADC result
		rx_flag=1;
		adc_data[0]=rx_msg[0];
		adc_data[1]=rx_msg[1];
		adc_data[2]=rx_msg[2];
		adc_data[3]=rx_msg[3];
		adc_data[4]=rx_msg[4];
		adc_data[5]=rx_msg[5];
		}
    rx_msg_ptr=0;
    }

void USART1_IRQHandler(void)
    {
    rx_msg[rx_msg_ptr++] = USART1->DR;
    if (rx_msg_ptr>RX_MSG_LEN) rx_msg_ptr = RX_MSG_LEN;
    TIM6->CR1=0x0000;
    TIM6->SR=0;
    TIM6->CNT=0x0000;
    TIM6->PSC=0x0000;
    TIM6->ARR=6*400;
    TIM6->DIER=0x0001;
    TIM6->CR1=0x0001;
    NVIC_EnableIRQ(TIM6_IRQn);		//after each character, reset guard time
    }


void USART2_IRQHandler(void)
    {
	//not used much, obviously
    uint8_t b;
    b = USART2->DR;
    }

void EXTI15_10_IRQHandler(void)
    {
	EXTI->PR = EXTI->PR | (1<<14);
    }
