/* Copyright (C) 2005 The cairomm Development Team
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <string>
#include <cairomm/exception.h>

namespace Cairo
{

inline static const char* string_or_empty(const char* text)
{
  return (text ? text : "");
}

//TODO: Is it wise to assume that the string is ASCII, as expected by std::logic_error?
logic_error::logic_error(ErrorStatus status)
: std::logic_error( string_or_empty(cairo_status_to_string((cairo_status_t)status)) ),
  m_status(status)
{
}

logic_error::~logic_error() noexcept
{}

ErrorStatus logic_error::get_status_code() const
{
  return m_status;
}

/*
const char* logic_error::what() const noexcept
{
  //Hopefully this is a const char* to a static string.
  return cairo_status_to_string((cairo_status_t)m_status);
}
*/

} //namespace Cairo

// vim: ts=2 sw=2 et
