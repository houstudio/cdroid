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

#include <cairomm/region.h>
#include <cairomm/private.h>
#include <algorithm>

namespace Cairo
{

Region::Region()
: m_cobject(cairo_region_create())
{
  check_object_status_and_throw_exception (*this);
}

Region::Region(const RectangleInt& rectangle)
: m_cobject(cairo_region_create_rectangle(&rectangle))
{
  check_object_status_and_throw_exception (*this);
}

// less efficient but convenient
Region::Region(const std::vector<RectangleInt>& rects) :
  m_cobject(nullptr)
{
  auto *carray = new RectangleInt[rects.size()];
  std::copy(rects.begin(), rects.end(), carray);
  m_cobject = cairo_region_create_rectangles (carray, rects.size());

  delete[] carray;

  check_object_status_and_throw_exception (*this);
}

// less convenient but more efficient
Region::Region(const RectangleInt *rects, int count) :
  m_cobject(cairo_region_create_rectangles (rects, count))
{
  check_object_status_and_throw_exception (*this);
}

Region::Region(cairo_region_t* cobject, bool has_reference)
: m_cobject(nullptr)
{
  if(has_reference)
    m_cobject = cobject;
  else
    m_cobject = cairo_region_reference(cobject);

  check_object_status_and_throw_exception (*this);
}

RefPtr<Region> Region::create()
{
  return make_refptr_for_instance<Region>(new Region());
}

RefPtr<Region> Region::create(const RectangleInt& rectangle)
{
  return make_refptr_for_instance<Region>(new Region(rectangle));
}

RefPtr<Region> Region::create(const std::vector<RectangleInt>& rects)
{
  return make_refptr_for_instance<Region>(new Region(rects));
}

RefPtr<Region> Region::create(const RectangleInt *rects, int count)
{
  return make_refptr_for_instance<Region>(new Region(rects, count));
}

RefPtr<Region> Region::copy() const
{
  return make_refptr_for_instance<Region>(new Region (cairo_region_copy (m_cobject), true));
}

Region::~Region()
{
  if(m_cobject)
    cairo_region_destroy(m_cobject);
}

void Region::reference() const
{
 cairo_region_reference(m_cobject);
}

void Region::unreference() const
{
  cairo_region_destroy(m_cobject);
}

RectangleInt Region::get_extents() const
{
  RectangleInt result;
  cairo_region_get_extents(m_cobject, &result);
  return result;
}

int Region::get_num_rectangles() const
{
  return cairo_region_num_rectangles(m_cobject);
}

RectangleInt Region::get_rectangle(int nth_rectangle) const
{
  RectangleInt result;
  cairo_region_get_rectangle(m_cobject, nth_rectangle, &result);
  return result;
}

bool Region::empty() const
{
  return cairo_region_is_empty(m_cobject);
}

Region::Overlap Region::contains_rectangle(const RectangleInt& rectangle) const
{
  return (Overlap)cairo_region_contains_rectangle(m_cobject, &rectangle);
}

bool Region::contains_point(int x, int y) const
{
  return cairo_region_contains_point(m_cobject, x, y);
}

void Region::translate(int dx, int dy)
{
  cairo_region_translate(m_cobject, dx, dy);
}

void Region::subtract(const RefPtr<Region>& other)
{
  auto status = cairo_region_subtract(m_cobject, (other ? other->cobj() : nullptr));
  check_status_and_throw_exception (status);
}

void Region::subtract(const RectangleInt& rectangle)
{
  auto status = cairo_region_subtract_rectangle(m_cobject, &rectangle);
  check_status_and_throw_exception (status);
}

void Region::intersect(const RefPtr<Region>& other)
{
  auto status = cairo_region_intersect(m_cobject, (other ? other->cobj() : nullptr));
  check_status_and_throw_exception (status);
}

void Region::intersect(const RectangleInt& rectangle)
{
  auto status = cairo_region_intersect_rectangle(m_cobject, &rectangle);
  check_status_and_throw_exception (status);
}

void Region::do_union(const RefPtr<Region>& other)
{
  auto status = cairo_region_union(m_cobject, (other ? other->cobj() : nullptr));
  check_status_and_throw_exception (status);
}

void Region::do_union(const RectangleInt& rectangle)
{
  auto status = cairo_region_union_rectangle(m_cobject, &rectangle);
  check_status_and_throw_exception (status);
}

void Region::do_xor(const RefPtr<Region>& other)
{
  auto status = cairo_region_xor(m_cobject, (other ? other->cobj() : nullptr));
  check_status_and_throw_exception (status);
}

void Region::do_xor(const RectangleInt& rectangle)
{
  auto status = cairo_region_xor_rectangle(m_cobject, &rectangle);
  check_status_and_throw_exception (status);
}


} //namespace Cairo

// vim: ts=2 sw=2 et
