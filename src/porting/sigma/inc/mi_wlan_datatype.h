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

#ifndef __MI_WLAN_DATATYPE_H__
#define __MI_WLAN_DATATYPE_H__
#ifdef __cplusplus
extern "C" {
#endif

#define MI_WLAN_ERR_FAIL (MI_DEF_ERR( E_MI_MODULE_ID_WLAN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_FAILED))
#define MI_WLAN_ERR_INVALID_DEVID (MI_DEF_ERR( E_MI_MODULE_ID_WLAN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_INVALID_DEVID))
#define MI_WLAN_ERR_ILLEGAL_PARAM (MI_DEF_ERR( E_MI_MODULE_ID_WLAN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_ILLEGAL_PARAM))
#define MI_WLAN_ERR_NOT_SUPPORT (MI_DEF_ERR( E_MI_MODULE_ID_WLAN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_SUPPORT))
#define MI_WLAN_ERR_MOD_INITED (MI_DEF_ERR( E_MI_MODULE_ID_WLAN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_INITED))
#define MI_WLAN_ERR_MOD_NOT_INIT (MI_DEF_ERR( E_MI_MODULE_ID_WLAN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_INIT))
#define MI_WLAN_ERR_NOT_CONFIG (MI_DEF_ERR( E_MI_MODULE_ID_WLAN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NOT_CONFIG))
#define MI_WLAN_ERR_CONFIGED (MI_DEF_ERR( E_MI_MODULE_ID_WLAN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_EXIST))
#define MI_WLAN_ERR_INVAL_HANDLE (MI_DEF_ERR( E_MI_MODULE_ID_WLAN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_BADADDR))
#define MI_WLAN_ERR_NULL_PTR (MI_DEF_ERR( E_MI_MODULE_ID_WLAN, E_MI_ERR_LEVEL_ERROR, E_MI_ERR_NULL_PTR))

/// WLAN max MAC len
#define MI_WLAN_MAX_MAC_LEN 32
/// WLAN max SSID len
#define MI_WLAN_MAX_SSID_LEN 128
/// WLAN max password len
#define MI_WLAN_MAX_PASSWD_LEN 40
/// WLAN max AP number
#define MI_WLAN_MAX_APINFO_NUM 64
/// WLAN max path len
#define MI_WLAN_MAX_FOLDERPATH_LEN 256
/// WLAN interface len
#define MI_MAX_INTERFACE_NAME 64
// WLAN BSSID len
#define MI_WLAN_BSSID_LEN 48
// WLAN MAX hosts AP connected
#define MI_WLAN_MAX_HOSTS 64
// WLAN MAX host name length
#define MI_WLAN_MAX_HOST_NAME_LEN 256

#define MI_RESULT MI_S32

/// static AP handle
#define WLAN_HANDLE_AP AP_HANDLE

// HANDLE indicates certain wifi connection
typedef  MI_S32 WLAN_HANDLE;



typedef enum
{
    /// WLAN module security key off
    E_MI_WLAN_SECURITY_NONE       = 1 << 0,
    /// WLAN module security key unknow
    E_MI_WLAN_SECURITY_UNKNOWTYPE = 1 << 1,
    /// WLAN module security key WEP
    E_MI_WLAN_SECURITY_WEP        = 1 << 2,
    /// WLAN module security key WPA
    E_MI_WLAN_SECURITY_WPA        = 1 << 3,
    /// WLAN module security key WPA2
    E_MI_WLAN_SECURITY_WPA2       = 1 << 4,
    /// WLAN module max
    E_MI_WLAN_SECURITY_MAX        = 0xff,
} MI_WLAN_Security_e;


typedef enum
{
    /// WLAN module encrypt type none
    E_MI_WLAN_ENCRYPT_NONE        = 1 << 0,
    /// WLAN module encrypt type unknown
    E_MI_WLAN_ENCRYPT_UNKNOWN     = 1 << 1,
    /// WLAN module encrypt type WEP
    E_MI_WLAN_ENCRYPT_WEP         = 1 << 2,
    /// WLAN module encrypt type TKIP
    E_MI_WLAN_ENCRYPT_TKIP        = 1 << 3,
    /// WLAN module encrypt type AES
    E_MI_WLAN_ENCRYPT_AES         = 1 << 4,
    /// WLAN module max
    E_MI_WLAN_ENCRYPT_MAX        = 0xff,
} MI_WLAN_Encrypt_e;


typedef enum
{
    /// WLAN network infrastructure type
    E_MI_WLAN_NETWORKTYPE_INFRA,
    /// WLAN network AP type
    E_MI_WLAN_NETWORKTYPE_AP,
    /// WLAN network AdHoc type
    E_MI_WLAN_NETWORKTYPE_ADHOC,
    /// WLAN network Monitor type
    E_MI_WLAN_NETWORKTYPE_MONITOR,
    /// WLAN network mode master
    E_MI_WLAN_NETWORKTYPE_MASTER,
    /// WLAN network mode slave
    E_MI_WLAN_NETWORKTYPE_SLAVE,
    /// WLAN param max
    E_MI_WLAN_NETWORKTYPE_MAX
} MI_WLAN_NetworkType_e;


typedef enum
{
    /// Authentication Suite PSK
    E_MI_WLAN_AUTH_SUITE_PSK,
    /// Authentication Suite unknown
    E_MI_WLAN_AUTH_SUITE_UNKNOWN,
    E_MI_WLAN_AUTH_SUITE_MAX

} MI_WLAN_Authentication_Suite_e;




typedef enum
{
    /**
     * WPA_DISCONNECTED - Disconnected state
     */
    WPA_DISCONNECTED,

    /**
     * WPA_INTERFACE_DISABLED - Interface disabled
     */
    WPA_INTERFACE_DISABLED,

    /**
     * WPA_INACTIVE - Inactive state (wpa_supplicant disabled)
     */
    WPA_INACTIVE,

    /**
     * WPA_SCANNING - Scanning for a network
     */
    WPA_SCANNING,

    /**
     * WPA_AUTHENTICATING - Trying to authenticate with a BSS/SSID
     */
    WPA_AUTHENTICATING,

    /**
     * WPA_ASSOCIATING - Trying to associate with a BSS/SSID
     */
    WPA_ASSOCIATING,

    /**
     * WPA_ASSOCIATED - Association completed
     */
    WPA_ASSOCIATED,

    /**
     * WPA_4WAY_HANDSHAKE - WPA 4-Way Key Handshake in progress
     */
    WPA_4WAY_HANDSHAKE,

    /**
     * WPA_GROUP_HANDSHAKE - WPA Group Key Handshake in progress
     */
    WPA_GROUP_HANDSHAKE,

    /**
     * WPA_COMPLETED - All authentication completed
     */
    WPA_COMPLETED
} MI_WLAN_WPAStatus_e;


/// WLAN init parameter
typedef struct MI_WLAN_InitParams_s
{
    /// json description file of wifi dongle
    MI_U8 au8JsonConfFilePath[MI_WLAN_MAX_FOLDERPATH_LEN];
    /// reserved
    MI_U64 u64Reserved;
} MI_WLAN_InitParams_t;


/// WLAN open parameter
typedef struct MI_WLAN_OpenParam_s
{
    // WLAN network type
    MI_WLAN_NetworkType_e eNetworkType;
    // reserved
    MI_BOOL bReserved;
} MI_WLAN_OpenParams_t;


/// WLAN connect info
typedef struct
{
    // WLan security mode
    MI_WLAN_Security_e eSecurity;
    // WLan SSID
    MI_U8 au8SSId[MI_WLAN_MAX_SSID_LEN];
    // WLan password
    MI_U8 au8Password[MI_WLAN_MAX_PASSWD_LEN];
    // WLAN connect overtime
    MI_U32 OverTimeMs;
} MI_WLAN_ConnectParam_t;


typedef struct
{
    MI_U8 curLVL;
    MI_U8 maxLVL;
    MI_S8 signalSTR;
} MI_WLAN_Quality_t;


typedef struct
{
    // WLAN Security mode
    MI_WLAN_Security_e eSecurity;
    // WLAN Encryption type
    MI_WLAN_Encrypt_e eGroupCipher;
    // WLAN Encryption type
    MI_WLAN_Encrypt_e ePairCipher;
    // WLAN authenticationb suite
    MI_WLAN_Authentication_Suite_e eAuthSuite;
} MI_WLAN_Cipher_t;

/// WLAN ap info
typedef struct MI_WLAN_APInfo_s
{
    // WLAN CELL ID
    MI_U16 u16CellId;
    // WLAN Frequency GHz
    MI_FLOAT fFrequency;
    // WLAN Bitrate Mb/s
    MI_FLOAT fBitRate;
    // WLAN Quality
    MI_WLAN_Quality_t stQuality;
    // WLAN Encryption key on/off
    MI_BOOL bEncryptKey;
    // WLAN SSID
    MI_U8 au8SSId[MI_WLAN_MAX_SSID_LEN];
    // WLAN Channel
    MI_U8 u8Channel;
    // WLAN MAC
    MI_U8 au8Mac[MI_WLAN_MAX_MAC_LEN];
    // WLAN Encryption type
    MI_WLAN_Encrypt_e eEncrypt;
    // WLAN AP type (Infrastructure / Ad-Hoc)
    MI_WLAN_NetworkType_e eMode;
    // WLAN cipher kit
    MI_WLAN_Cipher_t stCipher[2];
} MI_WLAN_APInfo_t;

/// WLAN ap info
typedef struct MI_WLAN_ScanParam_s
{
    // WLan set block mode
    MI_BOOL bBlock; //reserved
} MI_WLAN_ScanParam_t;

/// WLAN ap info
typedef struct MI_WLAN_ScanResult_s
{
    // WLan AP number
    MI_WLAN_APInfo_t stAPInfo[MI_WLAN_MAX_APINFO_NUM];
    MI_U8 u8APNumber;
} MI_WLAN_ScanResult_t;


typedef struct MI_WLAN_Status_sta_s
{
    MI_U8 bssid[MI_WLAN_BSSID_LEN];
    MI_U32 freq;
    MI_U8 ssid[MI_WLAN_MAX_SSID_LEN];
    MI_U16 id;
    MI_WLAN_NetworkType_e mode;
    MI_WLAN_Cipher_t stCipher;
    MI_U8 key_mgmt[12];
    MI_WLAN_WPAStatus_e state;
    MI_U8 address[MI_WLAN_BSSID_LEN];
    MI_U8 ip_address[16];
    MI_U32 channel;
    MI_U32 RSSI;
    MI_U8 Bandwidth[8];
} MI_WLAN_Status_sta_t;

typedef struct MI_WLAN_Status_host_s
{
    MI_U8 hostname[MI_WLAN_MAX_HOST_NAME_LEN];
    MI_U8 ipaddr[16];
    MI_U8 macaddr[18];
    MI_U64 connectedtime;

} MI_WLAN_Status_host_t;

typedef struct MI_WLAN_Status_ap_s
{
    MI_WLAN_Status_host_t astHosts[MI_WLAN_MAX_HOSTS];
    MI_U16 u16HostNum;
} MI_WLAN_Status_ap_t;

typedef union MI_WLAN_Status_s
{
    MI_WLAN_Status_sta_t stStaStatus;
    MI_WLAN_Status_ap_t stApStatus;
} MI_WLAN_Status_t;

#ifdef __cplusplus
}
#endif

#endif
