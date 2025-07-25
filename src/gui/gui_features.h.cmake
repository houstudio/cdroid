/*gui_features.h.cmake  Generated from configure.ac by autoheader.*/
#ifndef __CDROID_FEATURES_H__
#define __CDROID_FEATURES_H__

#cmakedefine ENABLE_GESTURE 1
#cmakedefine ENABLE_SPINNER 1
#cmakedefine ENABLE_DIALOGS 1
#cmakedefine ENABLE_RECYCLERVIEW 1
#cmakedefine ENABLE_DAYTIME_WIDGETS 1
#cmakedefine ENABLE_NAVIGATION 1
#cmakedefine ENABLE_BARCODE 1
#cmakedefine ENABLE_QRCODE 1

#cmakedefine ENABLE_I18N   1
#cmakedefine ENABLE_GIF   1     
#cmakedefine ENABLE_JPEG  1
#cmakedefine ENABLE_WEBP  1
#cmakedefine ENABLE_LOTTIE 1
#cmakedefine ENABLE_TURBOJPEG 1
#cmakedefine ENABLE_MBEDTLS   1
#cmakedefine ENABLE_UPNP  1    
#cmakedefine ENABLE_MP3ID3   1
#cmakedefine ENABLE_FRIBIDI  1
#cmakedefine ENABLE_PINYIN2HZ 1
#cmakedefine ENABLE_PLPLOT    1
#cmakedefine ENABLE_MATHGL    1
#cmakedefine ENABLE_MATPLOT   1
#cmakedefine ENABLE_LITEHTML  1
#cmakedefine ENABLE_AUDIO  1
#cmakedefine ENABLE_LCMS   1
#cmakedefine ENABLE_MENU   1
#cmakedefine HAVE_EPOLL 1
#cmakedefine HAVE_POLL  1
#cmakedefine HAVE_EVENTFD 1
#cmakedefine HAVE_MALLINFO 1
#cmakedefine HAVE_MALLINFO2 1
#cmakedefine HAVE_RTAUDIO_H 1

#define CDROID_VERSION "@CDROID_VERSION@"
#define CDROID_VERSION_MAJOR @CDROID_VERSION_MAJOR@
#define CDROID_VERSION_MINOR @CDROID_VERSION_MINOR@
#define CDROID_VERSION_PATCH @CDROID_VERSION_PATCH@
#define CDROID_BUILD_NUMBER  @CDROID_BUILD_NUMBER@
#define CDROID_COMMITID     "@CDROID_COMMITID@"
#define CDROID_PRODUCT      "@CDROID_CHIPSET@"
#define CDROID_BASE_OS      "@CMAKE_SYSTEM_NAME@"

/////////////////////////////////////////////////////////////
#define USE(FEATURE) (defined(USE_##FEATURE) && USE_##FEATURE)
#define ENABLE(FEATURE) (defined(ENABLE_##FEATURE) && ENABLE_##FEATURE)

#if !defined(ENABLE_GESTURE)
#define ENABLE_GESTURE 0
#endif

#if !defined(ENABLE_SPINNER)
#define ENABLE_SPINNER 0
#endif

#if !defined(ENABLE_DIALOGS)
#define ENABLE_DIALOGS 0
#endif

#if !defined(ENABLE_RECYCLERVIEW)
#define ENABLE_RECYCLERVIEW 0
#endif

#if !defined(ENABLE_DAYTIME_WIDGETS)
#define ENABLE_DAYTIME_WIDGETS 0
#endif

#if !defined(ENABLE_NAVIGATION)
#define ENABLE_NAVIGATION 0
#endif

#if !defined(ENABLE_BARCODE)
#define ENABLE_BARCODE 0
#endif

#if !defined(ENABLE_QRCODE)
#define ENABLE_QRCODE 0
#endif

#if !defined(ENABLE_LOTTIE)
#define ENABLE_LOTTIE 0
#endif

#if !defined(ENABLE_GIF)
#define ENABLE_GIF 0
#endif

#if !defined(ENABLE_JPEG)
#define ENABLE_JPEG 0
#endif

#if !defined(ENABLE_WEBP)
#define ENABLE_WEBP 0
#endif

#if !defined(ENABLE_TURBOJPEG)
#define ENABLE_TURBOJPEG 0
#endif

#if !defined(ENABLE_MBEDTLS)
#define ENABLE_MBEDTLS 0
#endif

#if !defined(ENABLE_FRIBIDI)
#define ENABLE_FRIBIDI 0
#endif

#if !defined(ENABLE_PINYIN2HZ)
#define ENABLE_PINYIN2HZ 0
#endif

#if !defined(ENABLE_PLPLOT)
#define ENABLE_PLPLOT 0
#endif

#if !defined(ENABLE_MATHGL)
#define ENABLE_MATHGL 0
#endif

#if !defined(ENABLE_LITEHTML)
#define ENABLE_LITEHTML 0
#endif

#if !defined(ENABLE_AUDIO)
#define ENABLE_AUDIO 0
#endif

#if !defined(ENABLE_LCMS)
#define ENABLE_LCMS 0
#endif

#endif/*__CDROID_FEATURES_H__*/
