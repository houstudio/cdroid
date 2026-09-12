cmake_minimum_required(VERSION 3.8)
project(wpa_ctrl C CXX)
set(EXECUTABLE_BUILD_TYPE Release)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
add_definitions(
    -DCONFIG_AP=y
    -DCONFIG_AUTOSCAN_EXPONENTIAL=y
    -DCONFIG_AUTOSCAN_PERIODIC=y
    -DCONFIG_BACKEND=file
    -DCONFIG_BGSCAN_SIMPLE=y
    -DCONFIG_CTRL_IFACE=y
    -DCONFIG_CTRL_IFACE_UNIX=y
    -DCONFIG_DEBUG_FILE=y
    -DCONFIG_DRIVER_NL80211=y
    -DCONFIG_DRIVER_WEXT=y
    -DCONFIG_DRIVER_WIRED=y
    -DCONFIG_EAP_FAST=y
    -DCONFIG_EAP_GTC=y
    -DCONFIG_EAP_LEAP=y
    -DCONFIG_EAP_MD5=y
    -DCONFIG_EAP_MSCHAPV2=y
    -DCONFIG_EAP_OTP=y
    -DCONFIG_EAP_PEAP=y
    -DCONFIG_EAP_PWD=y
    -DCONFIG_EAP_TLS=y
    -DCONFIG_EAP_TTLS=y
    -DCONFIG_HS20=y
    -DCONFIG_HT_OVERRIDES=y
    -DCONFIG_IBSS_RSN=y
    -DCONFIG_IEEE80211AC=y
    -DCONFIG_IEEE80211N=y
    -DCONFIG_IEEE80211R=y
    -DCONFIG_IEEE80211W=y
    -DCONFIG_IEEE8021X_EAPOL=y
    -DCONFIG_INTERWORKING=y
    -DCONFIG_IPV6=y
    -DCONFIG_LIBNL32=y
    -DCONFIG_NO_RANDOM_POOL=y
    -DCONFIG_P2P=y
    -DCONFIG_PEERKEY=y
    -DCONFIG_PKCS12=y
    -DCONFIG_READLINE=y
    -DCONFIG_SMARTCARD=y
    -DCONFIG_TDLS=y
    -DCONFIG_VHT_OVERRIDES=y
    -DCONFIG_WIFI_DISPLAY=y
    -DCONFIG_WPS=y
    -DCONFIG_WPS_NFC=y
    -DCONFIG_TLS_DEFAULT_CIPHERS="DEFAULT:!EXP:!LOW:3DES")
if(ANDROID)
    add_definitions(
        -DAID_SYSTEM=1000  #/* system server */
        -DAID_WIFI=1010  #/* wifi subsystem */
        -DANDROID_SOCKET_NAMESPACE_RESERVED=1
        )
endif()
# Vendor wpa_ctrl library (unmodified wpa_supplicant sources).
add_library(wpa_ctrl wifi/wpa_ctrl.c wifi/utils/os_unix.c wifi/utils/wpabuf.c)
include_directories(wpa_ctrl PUBLIC
   "wifi/utils"   "wifi"
   ${CDROID_INCLUDE_DIRS}
   ${CDROID_DEPINCLUDES}
   ${CMAKE_BINARY_DIR}/include
   ${CMAKE_BINARY_DIR}/include/gui
   ${CMAKE_BINARY_DIR}/include/porting)
target_compile_options(wpa_ctrl PUBLIC -Wall -Wextra -Wno-variadic-macros)

# cdnet — android.net + android.net.wifi port on wpa_ctrl, laid out by the
# android package tree:
#               shared android.net (NetworkInfo, IpConfiguration, ...)
#   ethernet/   ethernet side (EthernetManager)
#   wifi/       android.net.wifi (includes the wpa_ctrl backend)
# Self-contained: does not link cdroid, so it can be consumed (and later
# graduated into src/gui/net) without touching other modules. The
# superseded wpasockclient/wifiscanner prototypes stay on disk but out of
# the build.
add_library(cdnet
    wifi/supplicantclient.cc
    linkaddress.cc
    dhcpinfo.cc
    networkinfo.cc
    networkcapabilities.cc
    staticipconfiguration.cc
    ipconfiguration.cc
    ethernet/ethernetmanager.cc
    dhcpclient.cc
    ipapplicator.cc
    netlinkmonitor.cc
    connectivitymanager.cc
    wifi/wifissid.cc
    wifi/supplicantstate.cc
    wifi/scanresult.cc
    wifi/wifiinfo.cc
    wifi/wificonfiguration.cc
    wifi/nl80211radio.cc
    wifi/wparesponseparser.cc
    wifi/wifimanager.cc)
target_include_directories(cdnet PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(cdnet PUBLIC wpa_ctrl pthread)

add_executable(wpatest main.cc)
target_link_libraries(wpatest cdnet)

add_executable(ethtest ethtest.cc)
target_link_libraries(ethtest cdnet)

# Pure-logic test suites (no daemon / privileges required).
add_executable(wifitests wifitests.cc)
target_link_libraries(wifitests cdnet)

add_executable(nettests nettests.cc)
target_link_libraries(nettests cdnet)
