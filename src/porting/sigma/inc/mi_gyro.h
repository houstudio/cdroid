/* SigmaStar trade secret */
/* Copyright (c) [2019~2020] SigmaStar Technology.
All rights reserved.

Unless otherwise stipulated in writing, any and all information contained
herein regardless in any format shall remain the sole proprietary of
SigmaStar and be kept in strict confidence
(SigmaStar Confidential Information) by the recipient.
Any unauthorized act including without limitation unauthorized disclosure,
copying, use, reproduction, sale, distribution, modification, disassembling,
reverse engineering and compiling of the contents of SigmaStar Confidential
Information is unlawful and strictly prohibited. SigmaStar hereby reserves the
rights to any and all damages, losses, costs and expenses resulting therefrom.
*/
#ifndef _MI_GYRO_H_
#define _MI_GYRO_H_
#ifdef __cplusplus
extern "C" {
#endif


#include "mi_gyro_datatype.h"

int GyroSensor_SetSampleRateDiv(MI_U8 u8DevId, MI_U8 u8Div);

int GyroSensor_SetGyroRange(MI_U8 u8DevId, GyroSensorGyroRange_e eFullScaleRange);
int GyroSensor_GetGyroRange(MI_U8 u8DevId, GyroSensorGyroRange_e *peFullScaleRange);
int GyroSensor_GetGyroSensitivity(MI_U8 u8DevId, MI_U16 *pu16SensitivityDen, MI_U16 *pu16SensitivityMol);
//pu16SensitivityDen+pu16SensitivityMol/1000
int GyroSensor_ReadGyro_XYZ(MI_U8 u8DevId, MI_S16 *ps16X, MI_S16 *ps16Y, MI_S16 *ps16Z);

int GyroSensor_SetAccelRange(MI_U8 u8DevId, GyroSensorAccelRange_e eFullScaleRange);
int GyroSensor_GetAccelRange(MI_U8 u8DevId, GyroSensorAccelRange_e *peFullScaleRange);
int GyroSensor_GetAccelSensitivity(MI_U8 u8DevId,  MI_U16 *pu16SensitivityDen, MI_U16 *pu16SensitivityMol);
//pu16SensitivityDen+pu16SensitivityMol/1000
int GyroSensor_ReadAccel_XYZ(MI_U8 u8DevId, MI_S16 *ps16X, MI_S16 *ps16Y, MI_S16 *ps16Z);

int GyroSensor_ReadTemp(MI_U8 u8DevId, MI_S16 *s16Temp);

int GyroSensor_EnableDev(MI_U8 u8DevId, MI_BOOL bEnFifoMode, MI_U8 u8FifoModeType);
int GyroSensor_ReadFifoData(MI_U8 u8DevId, MI_U16 *pu16FifoCnt, MI_U8 *pu8Data);

#ifdef __cplusplus
}
#endif


#endif
