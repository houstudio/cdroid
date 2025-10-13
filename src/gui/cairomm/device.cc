/* Copyright (C) 2010s The cairomm Development Team
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

#include <cairomm/device.h>
#include <cairomm/private.h>

namespace Cairo
{

Device::Device(cairo_device_t* cobject, bool has_reference)
: m_cobject(nullptr)
{
  if(has_reference)
    m_cobject = cobject;
  else
    m_cobject = cairo_device_reference(cobject);
}

Device::~Device()
{
  if(m_cobject)
    cairo_device_destroy(m_cobject);
}

void Device::reference() const
{
 cairo_device_reference(m_cobject);
}

void Device::unreference() const
{
  cairo_device_destroy(m_cobject);
}

Device::DeviceType Device::get_type() const
{
  auto surface_type =
    cairo_device_get_type(const_cast<cobject*>(cobj()));
  check_object_status_and_throw_exception(*this);
  return static_cast<DeviceType>(surface_type);
}

void Device::flush()
{
  cairo_device_flush(m_cobject);
  check_object_status_and_throw_exception(*this);
}

void Device::finish()
{
  cairo_device_flush(m_cobject);
  check_object_status_and_throw_exception(*this);
}

void Device::acquire()
{
  auto status = cairo_device_acquire(m_cobject);
  check_status_and_throw_exception(status);
}

void Device::release()
{
  cairo_device_release(m_cobject);
  check_object_status_and_throw_exception(*this);
}

Device::Lock::Lock(const RefPtr<Device>& device) :
  m_device(device)
{
  m_device->acquire();
}

Device::Lock::Lock (const Lock& other) :
  m_device(other.m_device)
{
  m_device->acquire();
}

Device::Lock::~Lock()
{
  m_device->release();
}


} //namespace Cairo

// vim: ts=2 sw=2 et
