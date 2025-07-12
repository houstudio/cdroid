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
#ifndef __ZIP_ARCHIVE_H__
#define __ZIP_ARCHIVE_H__
#include <istream>
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace cdroid{

class ZIPArchive{
protected:
  void* zip;
  int method;
public:
  ZIPArchive(const std::string&fname);
  ~ZIPArchive();
  void remove(const std::string&fname)const;
  int getEntries(std::vector<std::string>&entries)const;
  int forEachEntry(std::function<bool(const std::string&)>fun)const;
  bool hasEntry(const std::string&name,bool excludeDirectories=false)const;
  std::istream* getInputStream(const std::string&fname)const;
  void*getZipHandle(const std::string&fname)const;
};

}

#endif
