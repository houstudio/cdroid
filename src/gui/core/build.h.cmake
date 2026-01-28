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
#ifndef __CDROID_BUILD_H__
#define __CDROID_BUILD_H__
namespace cdroid{

class Build{
public:
    class VERSION_CODES {
    public:
        static constexpr int CUR_DEVELOPMENT = 10000;
        static constexpr int BASE = 1;
        static constexpr int BASE_1_1 = 2;
        static constexpr int CUPCAKE = 3;
        static constexpr int DONUT = 4;
        static constexpr int ECLAIR = 5;
        static constexpr int ECLAIR_0_1 = 6;
        static constexpr int ECLAIR_MR1 = 7;
        static constexpr int FROYO = 8;
        static constexpr int GINGERBREAD = 9;
        static constexpr int GINGERBREAD_MR1 = 10;
        static constexpr int HONEYCOMB = 11;
        static constexpr int HONEYCOMB_MR2 = 13;
        static constexpr int ICE_CREAM_SANDWICH = 14;
        static constexpr int ICE_CREAM_SANDWICH_MR1 = 15;
        static constexpr int JELLY_BEAN = 16;
        static constexpr int JELLY_BEAN_MR1 = 17;
        static constexpr int JELLY_BEAN_MR2 = 18;
        static constexpr int KITKAT = 19;
        static constexpr int KITKAT_WATCH = 20;
        static constexpr int L = 21;
        static constexpr int LOLLIPOP = 21;
        static constexpr int LOLLIPOP_MR1 = 22;
        static constexpr int M = 23;
        static constexpr int N = 24;
        static constexpr int N_MR1 = 25;
        static constexpr int O = 26;
        static constexpr int O_MR1 = 27;
        static constexpr int P = 28;
        static constexpr int Q = 29;
        static constexpr int R = 30;
        static constexpr int S = 31;
        static constexpr int S_V2 = 32;
        static constexpr int TIRAMISU = 33;
        static constexpr int UPSIDE_DOWN_CAKE = 34;
    };
    class VERSION{
    public:
        static constexpr const char* Release = "@CDROID_VERSION@";
        static constexpr const char* CommitID= "@CDROID_COMMITID@";
        static constexpr int Major = @CDROID_VERSION_MAJOR@;
        static constexpr int Minor = @CDROID_VERSION_MINOR@;
        static constexpr int Patch = @CDROID_VERSION_PATCH@;
        static constexpr int BuildNumber = @CDROID_BUILD_NUMBER@;
        static constexpr int SDK_INT = Build::VERSION_CODES::P;
        /**
         * The user-visible version string.  E.g., "1.0" or "3.4b5" or "bananas".
         *
         * This field is an opaque string. Do not assume that its value
         * has any particular structure or that values of RELEASE from
         * different releases can be somehow ordered.
         */
        static constexpr const char* RELEASE = "@CDROID_VERSION@";
        /**
         * The current development codename, or the string "REL" if this is
         * a release build.
         */
        static constexpr const char* CODENAME = "@CMAKE_BUILD_TYPE@";
        /**
         * The version string.  May be {@link #RELEASE} or {@link #CODENAME} if
         * not a final release build.
         */
        static constexpr const char* RELEASE_OR_CODENAME = "@CMAKE_BUILD_TYPE@";
        /**
         * The base OS build the product is based on.
         */
        static constexpr const char* BASE_OS = "@CMAKE_SYSTEM_NAME@";
    };
};
using BUILD = Build;

}
#endif/*__CDROID_BUILD_H__*/
