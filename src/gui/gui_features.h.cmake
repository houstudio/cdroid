/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *********************************************************************************/
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
#cmakedefine ENABLE_WEAR_WIDGETS 1

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

#define USE(FEATURE) (defined(USE_##FEATURE) && USE_##FEATURE)
#define ENABLE(FEATURE) (defined(ENABLE_##FEATURE) && ENABLE_##FEATURE)

#endif/*__CDROID_FEATURES_H__*/
