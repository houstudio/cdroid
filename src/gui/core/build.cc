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
#include <core/build.h>
#include <gui_features.h>
namespace cdroid{

const std::string Build::VERSION::Release = CDROID_VERSION;
const std::string Build::VERSION::RELEASE = CDROID_VERSION;
const std::string Build::VERSION::CommitID= CDROID_COMMITID;
const std::string Build::VERSION::BASE_OS = CDROID_BASE_OS;
#ifdef DEBUG
const std::string Build::VERSION::CODENAME="Debug";
const std::string Build::VERSION::RELEASE_OR_CODENAME;
#else
const std::string Build::VERSION::CODENAME="Release";
const std::string Build::VERSION::RELEASE_OR_CODENAME;
#endif
const int Build::VERSION::Major = CDROID_VERSION_MAJOR;
const int Build::VERSION::Minor = CDROID_VERSION_MINOR;
const int Build::VERSION::Patch = CDROID_VERSION_PATCH;
const int Build::VERSION::BuildNumber=CDROID_BUILD_NUMBER;

const int Build::VERSION::SDK_INT=Build::VERSION_CODES::P;
}
