#ifndef __LIBHARDWARE2_WIFI_H__
#define __LIBHARDWARE2_WIFI_H__

#include <pthread.h>

#ifdef  __cplusplus
extern "C" {
#endif

/* WIFI网络状态信息结构体 */
struct wifi_network_status_info {
    /* WIFI连接的热点的 MAC 地址 */
    char bssid[64];

    /* WIFI 频率 单位MHz*/
    int freq;

    /* WIFI名称 */
    char ssid[256];

    /* 网络的id号 */
    int network_id;

    /* WIFI 模式(station模式、 ap模式) */
    char mode[32];

    /* WIFI 加密套件(AES-CCMP、TKIP、WEP40、WEP104、WEP128) */
    char group_cipher[32];

    /* WIFI 加密模式 (WPA2-PSK 、WPA2-PSK、 WPA2、WPA、不加密) */
    char cipher_mode[32];

    /* WIFI 状态
     * INACTIVE             WIFI管理器已经关闭，无效状态
     * SCANNING             正在扫描网络热点
     * ASSOCIATING          WIFI设备正在连接中
     * ASSOCIATED           WIFI设备连接成功
     * FOUR_WAY_HANDSHAKE   正在处理WPA 4-Way Key握手过程中
     * GROUP_HANDSHAKE      正在处理WPA Group Key握手过程中
     * COMPLETED            所有鉴定工作完成
     * DISCONNECTED         WIFI设备不能连接热点，处于断开状态
     */
    char status[64];

    /* WIFI 被分配的IP地址 */
    char ip_addr[32];

    /* WIFI网卡MAC地址 */
    char mac_addr[64];

    /* 通用唯一识别码 */
    char uuid[64];

    /* WIFI 信号强度(0~100) 单位：dB */
    int signal_strength;


};

/* wifi网络扫描热点信息结构体 */
struct wifi_network_scan_info {
    /* WIFI连接的热点的 MAC 地址 */
    char bssid[64];

    /* WIFI 频率 单位MHz */
    int freq;

    /* WIFI 信号强度(0~100) 单位：dB */
    int signal_strength;

    /* WIFI热点加密模式(WIFI 模式 + WIFI 加密套件 的组合) */
    char cipher_mode[128];

    /* WIFI名称 */
    char ssid[256];
};


/* 网络状态 */
enum wifi_connect_state {
    /* 网络正在连接 */
    WIFI_CONNECTING,
    /* 网络连接成功 */
    WIFI_CONNECTED,
    /* 网络断开连接 */
    WIFI_DISCONNECTED,
};

enum wifi_server_state {
    /* 服务端处于打开状态 */
    SERVER_CONNECT,
    /* 服务端处于关闭状态 */
    SERVER_DISCONNECT,
};

/* wifi 网络事件回调 */
typedef struct wifi_event_callback {
    /* 连接网络不存在回调 */
    void (*network_not_found_cb)();
    /* 连接网络密钥错误回调 */
    void (*network_wrong_key_cb)();
    /* 服务端状态发生改变回调 */
    void (*network_server_changed_cb)(enum wifi_server_state state);
    /* 网络状态发生改变回调 */
    void (*network_state_changed_cb)(enum wifi_connect_state state);
    /* 此callback_state变量为回调状态,用户无需设置，内部实现使用 */
    volatile unsigned int callback_state;
    pthread_t tid;
} wifi_event_callback;

/*
 * @brief 注册监听事件回调
 * @param callback 回调函数句柄
*/
void wifi_register_event_callback(struct wifi_event_callback *callback);

/*
 * @brief 注销监听事件回调
 * @param callback 回调函数句柄
*/
void wifi_unregister_event_callback(struct wifi_event_callback *callback);

/*
 * @brief 打开wifi服务端
 * @param network_config_path wifi服务端配置路径，文件并不存在会被创建
*/
void wifi_start_network_server(const char *network_config_path);

/*
 * @brief 关闭wifi服务端
*/
void wifi_stop_network_server(void);

/*
 * @brief 查看wifi服务端状态
 * @return 返回1服务端被关闭,返回0服务端被打开
*/
enum wifi_server_state wifi_check_network_server_state(void);

/*
 * @brief 配置网络
 * @param network_ssid wifi名称
 * @param network_psk wifi密码,无密码填NULL
 * @param network_bssid 连接热点mac地址，不填可填NULL(当热点ssid和psk都相同时可以由bssid指定连接哪个热点)
 * @return 成功返回非负数,失败返回负数
*/
int wifi_connect_network(const char *network_ssid, const char *network_psk, const char *network_bssid);

/**
 * @brief 获取wifi状态
 * @return 成功返回非负数,失败返回负数
 */
int wifi_disconnect_current_network(void);

/**
 * @brief 扫描周围wifi
 * @param network_info 保存扫描wifi信息
 * @param info_count 保存扫描wifi信息结构体的数量
 * @return 成功返回扫描到wifi数量,失败返回负数
 */
int wifi_get_scan_info(struct wifi_network_scan_info* network_info, int info_count);

/**
 * @brief 获取wifi状态
 * @param neiwork_info 保存获取wifi状态信息
 * @return 成功返回获取的状态,失败返回负数
 */
int wifi_get_status_info(struct wifi_network_status_info* neiwork_info);

/*
 * @brief 保存连接成功的网络配置
 * @param network_config_path wifi配置保存的路径，文件并不存在会被创建
 * @param network_ssid wifi名称
 * @param network_psk wifi密码，不填可填NULL
 * @param network_bssid wifi连接热点的mac地址,不填可填NULL
 * @return 成功返回0,失败返回负数
*/
int wifi_save_network_config(const char *network_config_path, const char *network_ssid, const char *network_psk, const char *network_bssid);

#ifdef  __cplusplus
}
#endif

#endif /* __LIBHARDWARE2_WIFI_H__ */