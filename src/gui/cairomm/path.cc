/* Copyright (C) 2005 The cairomm Development Team
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
 * License along with this library; if not, see <https://www.gnu.org/licenses/>.
 */

#include <cairomm/path.h>
#include <cairomm/private.h>
#include <iostream>

namespace Cairo
{

Path::Path(cairo_path_t* cobject, bool take_ownership)
: m_cobject(nullptr)
{
  if(take_ownership)
    m_cobject = cobject;
  else
  {
    std::cerr << "cairomm: Path::Path(): copying of the underlying cairo_path_t* is not yet implemented." << std::endl;
    //m_cobject = cairo_path_copy(cobject);
  }
}

Path::~Path()
{
  if(m_cobject)
    cairo_path_destroy(m_cobject);
}

/***************************************************
 * Path::Element
 ***************************************************/

Path::Element::Element(cobject* pData) :
  m_cobject(pData)
{}

unsigned int Path::Element::size() const
{
  // We only want the count of data points so subtract one to ignore the header.
  return (m_cobject->header.length) - 1;
}

Path::Element::Point Path::Element::operator[](unsigned int idx) const
{
  if (idx >= size())
  {
    throw std::out_of_range("Invalid array index");
  }
  /* Since this is zero-based, and the zero-th element of the underlying C array
   * is actually the header element, we need to add one to the idx to access the
   * correct member of the array. */
  cobject* p = m_cobject + (idx + 1);
  Point pt = {p->point.x, p->point.y};
  return pt;
}

/***************************************************
 * Path::const_iterator
 ***************************************************/
Path::const_iterator::const_iterator(cairo_path_data_t* path_data) :
  m_node(path_data)
{}

Path::const_iterator& Path::const_iterator::operator++()  // pre-increment
{
  // Need to add one more than size() in order to skip the header as well.
  m_node.reset(m_node.cobj() + m_node.size() + 1);
  return *this;
}

Path::const_iterator Path::const_iterator::operator++(int)  // post-increment
{
  const_iterator tmp = *this;
  operator++();
  return tmp;
}

} //namespace Cairo
