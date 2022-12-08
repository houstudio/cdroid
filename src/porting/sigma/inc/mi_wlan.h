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

#ifndef _MI_WLAN_H_
#define _MI_WLAN_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "mi_wlan_datatype.h"

#define WLAN_MAJOR_VERSION 2
#define WLAN_SUB_VERSION 1
#define MACRO_TO_STR(macro) #macro
#define WLAN_VERSION_STR(major_version,sub_version) ({char *tmp = sub_version/100 ? \
                                    "mi_wlan_version_" MACRO_TO_STR(major_version)"." MACRO_TO_STR(sub_version) : sub_version/10 ? \
                                    "mi_wlan_version_" MACRO_TO_STR(major_version)".0" MACRO_TO_STR(sub_version) : \
                                    "mi_wlan_version_" MACRO_TO_STR(major_version)".00" MACRO_TO_STR(sub_version);tmp;})
#define MI_WLAN_API_VERSION WLAN_VERSION_STR(WLAN_MAJOR_VERSION,WLAN_SUB_VERSION)

extern WLAN_HANDLE AP_HANDLE;

//------------------------------------------------------------------------------
/// @brief setup wlan init parameter , parse wifi config file and internel component
/// @param[in] *pstInitParams
/// @return MI_SUCCESS:  success.
/// @return MI_WLAN_ERR_FAIL: init fail.
/// @return MI_WLAN_ERR_MOD_INITED: already inited.
//------------------------------------------------------------------------------
MI_RESULT MI_WLAN_Init(MI_WLAN_InitParams_t *pstInitParams);

//------------------------------------------------------------------------------
/// @brief do wlan deinit , deinit config file parser and internel component
/// @param none
/// @return MI_SUCCESS: success.
/// @return MI_WLAN_ERR_MOD_NOT_INIT: call MI_WLAN_Init first.
//------------------------------------------------------------------------------
MI_RESULT MI_WLAN_DeInit(void);

//------------------------------------------------------------------------------
/// @brief open wifi device
/// @param[in] *pstParam :which workmode wifi works on
/// @return MI_SUCCESS: success.
/// @return MI_WLAN_ERR_FAIL: failed
/// @return MI_WLAN_ERR_MOD_NOT_INIT: not inited
/// @return MI_WLAN_ERR_CONFIGED: already opened
//------------------------------------------------------------------------------
MI_RESULT MI_WLAN_Open(MI_WLAN_OpenParams_t *pstParam);

//------------------------------------------------------------------------------
/// @brief close wifi device
/// @param[in] *pstParam :which workmode wifi work on
/// @return MI_SUCCESS: success
/// @return MI_WLAN_ERR_NOT_CONFIG: already closed.
//------------------------------------------------------------------------------
MI_RESULT MI_WLAN_Close(MI_WLAN_OpenParams_t *pstParam);

//------------------------------------------------------------------------------
/// @brief connect wifi service
///
/// infra mode : connect to certain wifi network
/// ap mode: start dns hdcp service and wait hosts to connect
///          use corresponding default setting in conf files if ssid or password == NULL
///          set key mgmt to NONE if password == "NONE"(not case sensitive)  
/// @param[in/out] hWLan: wlan handle.
/// (==WLAN_HANDLE_AP) indicates the AP service
/// (<0)indicates a new sta connection and will be assigned a sensible ID if connecton set up
/// (>0)indicates a set up connection
/// @param[in] *pstConnectParam :
/// nessesarry information to establish the connection
/// @return MI_SUCCESS: connect success.
/// @return MI_WLAN_ERR_NOT_CONFIG: open wlan device first
/// @return MI_WLAN_ERR_NULL_PTR: open configuration file failed
/// @return MI_WLAN_ERR_ILLEGAL_PARAM: ssid or password invalid
/// @note a set up connection just specify the setting itself,does not indicate the connection
///  has been build successfully
//------------------------------------------------------------------------------
MI_RESULT MI_WLAN_Connect(WLAN_HANDLE *hWLan, MI_WLAN_ConnectParam_t *pstConnectParam);

//------------------------------------------------------------------------------
/// @brief disconnect wifi service
/// @param[in] hWLan: wlan handle.
/// (==WLAN_HANDLE_AP) AP connection
/// (>0) STA connection
/// @return MI_SUCCESS: Process success.
/// @return MI_WLAN_ERR_NOT_CONFIG: connect wlan first
//------------------------------------------------------------------------------
MI_RESULT MI_WLAN_Disconnect(WLAN_HANDLE hWLan);

//------------------------------------------------------------------------------
/// @brief scan ap info
/// @param[in] *pstParam info param. //reserved
/// @param[out] *pstResult result.
/// @return MI_SUCCESS:  success.
/// @return MI_WLAN_ERR_NOT_CONFIG: open wlan first
/// @note STA connection only
//------------------------------------------------------------------------------
MI_RESULT MI_WLAN_Scan(MI_WLAN_ScanParam_t *pstParam, MI_WLAN_ScanResult_t *pstResult);

//------------------------------------------------------------------------------
/// @brief get current wlan status
/// @param[in] hWLan: wlan handle.
/// (==WLAN_HANDLE_AP) AP connection
/// (>0) STA connection
/// @param[in] the struct to store wlan status
/// @return MI_SUCCESS: Process success.
/// @return MI_WLAN_ERR_NOT_CONFIG: connect wlan first
//------------------------------------------------------------------------------
MI_RESULT MI_WLAN_GetStatus(WLAN_HANDLE hWLan,MI_WLAN_Status_t *status);

//------------------------------------------------------------------------------
/// @brief get wlan chip version
/// @return MI_OK: Process success
/// @return MI_ERR_FAILED: Process failed
//------------------------------------------------------------------------------
MI_RESULT MI_WLAN_GetWlanChipVersion(MI_U8 *ChipVersion);


#ifdef __cplusplus
}
#endif

#endif
