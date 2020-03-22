#include "stm32l1xx.h"


#define	RX_MSG_LEN	10

#define		CAL_PREDEF_1VO		- 0.000070
#define		CAL_PREDEF_1VG		0.9838070
#define		CAL_PREDEF_10VO		- 0.00004
#define		CAL_PREDEF_10VG		0.9838573
#define		CAL_PREDEF_100VO	- 0.0009
#define		CAL_PREDEF_100VG	0.9839094
#define		CAL_PREDEF_1KO		- 0.001
#define		CAL_PREDEF_1KG		0.9379245
#define		CAL_PREDEF_10KO		- 0.0000
#define		CAL_PREDEF_10KG		0.9380175
#define		CAL_PREDEF_100KO	- 0.0000
#define		CAL_PREDEF_100KG	0.9381589
#define		CAL_PREDEF_1MO		- 0.0006
#define		CAL_PREDEF_1MG		0.9381589
#define		CAL_PREDEF_10MO		- 0.000
#define		CAL_PREDEF_10MG		0.9381589
#define		CAL_PREDEF_NZO		-170.0
#define		CAL_PREDEF_STEP		38500

#define	MEAS_VOLT			0
#define	MEAS_OHM			1
#define	MEAS_DIO			2
#define	MEAS_VOLT_RN		3
#define	MEAS_OHM_RN			5
#define	MEAS_DIO_RN			4
#define	MEAS_IR_RN			3
#define	RUN_STATE_SHDN		0
#define	RUN_STATE_RUN_ENTRY	1
#define	RUN_STATE_RUN		2


#define	SR_CLK_H	X_D1_H
#define	SR_CLK_L	X_D1_L
#define	SR_DAT_H	X_D2_H
#define	SR_DAT_L	X_D2_L
#define	SR_LAT_H	X_D3_H
#define	SR_LAT_L	X_D3_L

#define	SR_M14		0x8000
#define	SR_M13		0x4000
#define	SR_M12		0x2000
#define	SR_M11		0x1000
#define	SR_M21		0x0800
#define	SR_M22		0x0400
#define	SR_M23		0x0200
#define	SR_M24		0x0100
#define	SR_K4		0x0080
#define	SR_K3		0x0040
#define	SR_K1		0x0020
#define	SR_K2		0x0010
#define	SR_M3D		0x0008
#define	SR_M3C		0x0004
#define	SR_M3B		0x0002
#define	SR_M3A		0x0001
#define	SR_INVERT_MASK	0xFFFF


#define	PE_A_NUM	12
#define	PE_A_MASK	(1<<PE_A_NUM)
#define	PE_A_H		GPIOA->BSRRL = GPIOA->BSRRL | (PE_A_MASK<<0)
#define	PE_A_L		GPIOA->BSRRH = GPIOA->BSRRH | (PE_A_MASK<<0)
#define	PE_D_NUM	11
#define	PE_D_MASK	(1<<PE_D_NUM)
#define	PE_D_H		GPIOA->BSRRL = GPIOA->BSRRL | (PE_D_MASK<<0)
#define	PE_D_L		GPIOA->BSRRH = GPIOA->BSRRH | (PE_D_MASK<<0)
#define	D_BK_NUM	11
#define	D_BK_MASK	(1<<D_BK_NUM)
#define	D_BK_H		GPIOB->BSRRL |= (D_BK_MASK<<0)
#define	D_BK_L		GPIOB->BSRRH |= (D_BK_MASK<<0)
#define	D_RST_NUM	2
#define	D_RST_MASK	(1<<D_RST_NUM)
#define	D_RST_H		GPIOB->BSRRL |= (D_RST_MASK<<0)
#define	D_RST_L		GPIOB->BSRRH |= (D_RST_MASK<<0)
#define	D_A0_NUM	1
#define	D_A0_MASK	(1<<D_A0_NUM)
#define	D_A0_H		GPIOB->BSRRL |= (D_A0_MASK<<0)
#define	D_A0_L		GPIOB->BSRRH |= (D_A0_MASK<<0)
#define	D_CLK_NUM	0
#define	D_CLK_MASK	(1<<D_CLK_NUM)
#define	D_CLK_H		GPIOB->BSRRL |= (D_CLK_MASK<<0)
#define	D_CLK_L		GPIOB->BSRRH |= (D_CLK_MASK<<0)
#define	D_CS_NUM	10
#define	D_CS_MASK	(1<<D_CS_NUM)
#define	D_CS_H		GPIOB->BSRRL |= (D_CS_MASK<<0)
#define	D_CS_L		GPIOB->BSRRH |= (D_CS_MASK<<0)
#define	D_SI_NUM	7
#define	D_SI_MASK	(1<<D_SI_NUM)
#define	D_SI_H		GPIOA->BSRRL |= (D_SI_MASK<<0)
#define	D_SI_L		GPIOA->BSRRH |= (D_SI_MASK<<0)
#define	X_D1_NUM	4
#define	X_D1_MASK	(1<<X_D1_NUM)
#define	X_D1_H		GPIOA->BSRRL |= (X_D1_MASK<<0)
#define	X_D1_L		GPIOA->BSRRH |= (X_D1_MASK<<0)
#define	X_D2_NUM	5
#define	X_D2_MASK	(1<<X_D2_NUM)
#define	X_D2_H		GPIOA->BSRRL |= (X_D2_MASK<<0)
#define	X_D2_L		GPIOA->BSRRH |= (X_D2_MASK<<0)
#define	X_D3_NUM	6
#define	X_D3_MASK	(1<<X_D3_NUM)
#define	X_D3_H		GPIOA->BSRRL |= (X_D3_MASK<<0)
#define	X_D3_L		GPIOA->BSRRH |= (X_D3_MASK<<0)
#define	A_CT_NUM	8
#define	A_CT_MASK	(1<<A_CT_NUM)
#define	A_CT_H		GPIOA->BSRRL |= (A_CT_MASK<<0)
#define	A_CT_L		GPIOA->BSRRH |= (A_CT_MASK<<0)
#define	K_K1_NUM	14
#define	K_K1_MASK	(1<<K_K1_NUM)
#define	K_K1_R		(GPIOB->IDR&K_K1_MASK)
#define	K_K2_NUM	13
#define	K_K2_MASK	(1<<K_K2_NUM)
#define	K_K2_R		(GPIOB->IDR&K_K2_MASK)
#define	K_K3_NUM	1
#define	K_K3_MASK	(1<<K_K3_NUM)
#define	K_K3_R		(GPIOA->IDR&K_K3_MASK)
#define	K_K4_NUM	0
#define	K_K4_MASK	(1<<K_K4_NUM)
#define	K_K4_R		(GPIOA->IDR&K_K4_MASK)
#define	K_K5_NUM	15
#define	K_K5_MASK	(1<<K_K5_NUM)
#define	K_K5_R		(GPIOB->IDR&K_K5_MASK)
#define	K_K6_NUM	15
#define	K_K6_MASK	(1<<K_K6_NUM)
#define	K_K6_R		(GPIOA->IDR&K_K6_MASK)


typedef union
{
struct
 {
    unsigned k1:1;
    unsigned k2:1;
    unsigned k3:1;
    unsigned k4:1;
    unsigned k5:1;
    unsigned k6:1;
    unsigned k7:1;
 	unsigned k8:1;
 };
unsigned char CHAR;
}key_var;

struct cal_struct
	{
	int32_t	adc_z_offset;
	int32_t	adc_step;
	double	v_1V_offset;
	double	v_1V_gain;
	double	v_10V_offset;
	double	v_10V_gain;
	double	v_100V_offset;
	double	v_100V_gain;
	double	r_1k_offset;
	double	r_1k_gain;
	double	r_10k_offset;
	double	r_10k_gain;
	double	r_100k_offset;
	double	r_100k_gain;
	double	r_1M_offset;
	double	r_1M_gain;
	double	r_10M_offset;
	double	r_10M_gain;
	};


void adc_init (void);
void adc_uninit (void);
uint16_t adc_get (uint8_t channel);
double get_bat_volt (void);
void turn_off (void);
void usart_init (void);
void usart_uninit (void);
void usart_send_byte(int8_t data);
void usart_send_str (char * data);
void usart2_init (void);
void usart2_uninit (void);
void usart2_send_byte(int8_t data);
void usart2_send_str (char * data);
void sr_shift16 (uint16_t data);
void hw_init (void);
