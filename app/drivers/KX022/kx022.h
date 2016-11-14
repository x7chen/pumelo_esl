/*
* Copyright (c) 2014 Avnet.  All rights reserved.
*
* THE SOURCE CODE DOCUMENT IS PROVIDED HEREN IS CONFIDENTIAL AND 
* PROPRIETARY INFORMATION PROPERTY OF AVNET AND ALL ITS SUBSIDIARIES. 
* USER MAY NOT COPY OR REDISTRIBUTE THIS DOCUMENT NEITHER IN WHOLE OR IN
* PART IN ANY FORM WHATSOEVER INCLUDING BUT NOT LIMITED TO 
* PRINTED COPIES OR ANY ALTERNATE ENCODED REPRESENTATION OF IT.
*
* AVNET IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A 
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
* ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR 
 * STANDARD, AVNET IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION 
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE 
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION
* AVNET EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO 
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO 
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE 
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY 
 * AND FITNESS FOR A PARTICULAR PURPOSE.
*/
#ifndef __KX022__H__
#define __KX022__H__
#include "stdint.h"
#include "stdbool.h"
#include "app_timer.h"

#define KX022_DEVICE_ADDR               0x1E
#define KX022_I2C_WRITE_BIT             0x00
#define KX022_I2C_READ_BIT              0x01

#define KX_XHPL_ADDR                    0x00
#define KX_XHPH_ADDR                    0x01
#define KX_YHPL_ADDR                    0x02
#define KX_YHPH_ADDR                    0x03
#define KX_ZHPL_ADDR                    0x04
#define KX_ZHPH_ADDR                    0x05
#define KX_XOUTL_ADDR                   0x06
#define KX_XOUTH_ADDR                   0x07
#define KX_YOUTL_ADDR                   0x08
#define KX_YOUTH_ADDR                   0x09
#define KX_ZOUTL_ADDR                   0x0A
#define KX_ZOUTH_ADDR                   0x0B
#define KX_COTR_ADDR                    0x0C
#define KX_WHOAMI_ADDR                  0x0F
#define KX_TSCP_ADDR                    0x10
#define KX_TSPP_ADDR                    0x11
#define KX_INS1_ADDR				    0x12
#define KX_INS2_ADDR				    0x13
#define KX_INS3_ADDR				    0x14
#define KX_STATUSREG_ADDR			    0x15
#define KX_INTREL_ADDR				    0x17
#define KX_CNTL1_ADDR				    0x18
#define KX_CNTL2_ADDR				    0x19
#define KX_CNTL3_ADDR				    0x1A
#define KX_ODCNTL_ADDR				    0x1B
#define KX_INC1_ADDR				    0x1C
#define KX_INC2_ADDR				    0x1D
#define KX_INC3_ADDR				    0x1E
#define KX_INC4_ADDR				    0x1F
#define KX_INC5_ADDR				    0x20
#define KX_INC6_ADDR				    0x21
#define KX_TILT_TIMER_ADDR				0x22
#define KX_WUFC_ADDR				    0x23
#define KX_TDTRC_ADDR				    0x24
#define KX_TDTC_ADDR				    0x25
#define KX_TTH_ADDR				        0x26
#define KX_TTL_ADDR				        0x27
#define KX_FTD_ADDR				        0x28
#define KX_STD_ADDR				        0x29
#define KX_TLT_ADDR				        0x2A
#define KX_TWS_ADDR				        0x2B
#define KX_ATH_ADDR				        0x30
#define KX_TILT_ANGLE_LL_ADDR			0x32
#define KX_TILT_ANGLE_HL_ADDR			0x33
#define KX_HYST_SET_ADDR				0x34
#define KX_LP_CNTL_ADDR				    0x35
#define KX_BUF_CNTL1_ADDR				0x3A
#define KX_BUF_CNTL2_ADDR				0x3B
#define KX_BUF_STATUS1_ADDR				0x3C
#define KX_BUF_STATUS2_ADDR				0x3D
#define KX_BUF_CLEAR_ADDR				0x3E
#define KX_BUF_READ_ADDR				0x3F
#define KX_SELF_TEST_ADDR				0x60


typedef struct
{
    int16_t    X;
    int16_t    Y;
    int16_t    Z;
}AxesRaw_t;

extern AxesRaw_t KX_Value;

extern uint8_t KX_INT_Value;
//调用AccelCounter,输入三轴加速度数据，数据为12Bit 50HZ
void AccelCounter(short Accel_Xchannl,short Accel_Ychannl,short Accel_Zchannl);
//void AccelerationDateSetting(void);
unsigned int ReadStep(void);
char ReadRunState(void);

bool KX_Init(void);
bool KX_Close(void);
bool KX_I2C_Read(uint8_t reg_addr,uint8_t *data,uint8_t length);
bool KX_I2C_Write(uint8_t reg_addr,uint8_t *data,uint8_t length);
uint8_t  KX_I2C_Read_Byte(uint8_t reg_addr);
void KX_I2C_Write_Byte(uint8_t reg_addr,uint8_t data);
bool KX_Get_AccValue(AxesRaw_t *result);
void KX_ReadBuf(uint8_t *val,uint8_t *len);
void Algorithm_accelerate_data_in(void);
uint32_t KX022_INT1_Init();
uint32_t KX022_INT1_Enable(void);
uint32_t KX022_INT1_Disable(void);
void KX_INT1_handle(void);
#endif
