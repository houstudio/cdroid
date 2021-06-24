/* Copyright (C) 2010 The cairomm Development Team
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

#ifndef __CAIROMM_DEVICE_H
#define __CAIROMM_DEVICE_H

#include <cairomm/types.h>
#include <cairomm/enums.h>
#include <cairomm/refptr.h>
#include <cairo.h>


namespace Cairo
{

/**
 * Devices are the abstraction Cairo employs for the rendering system used by a
 * cairo_surface_t. You can get the device of a surface using
 * Surface::get_device().
 *
 * Devices are created using custom functions specific to the rendering system
 * you want to use. See the documentation for the surface types for those
 * functions.
 *
 * An important function that devices fulfill is sharing access to the rendering
 * system between Cairo and your application. If you want to access a device
 * directly that you used to draw to with Cairo, you must first call
 * flush() to ensure that Cairo finishes all operations on the
 * device and resets it to a clean state.
 *
 * Cairo also provides the functions acquire() and release() to synchronize
 * access to the rendering system in a multithreaded environment. This is done
 * internally, but can also be used by applications.  There is also a
 * Device::Lock convenience class that allows the device to be acquired and
 * released in an exception-safe manner.
 *
 * This is a reference-counted object that should be used via Cairo::RefPtr.
 *
 * @since 1.10
 */
class CAIROMM_API Device
{
public:
  /** A convenience class for acquiring a Device object in an exception-safe
   * manner.  The device is automatically acquired when a Lock object is created
   * and released when the Lock object is destroyed.  For example:
   *
   * @code
   * void
   * my_device_modifying_function (const RefPtr<Device>& device)
   * {
   *   // Ensure the device is properly reset
   *   device->flush();
   *
   *   Device::Lock lock(device);
   *   // Do the custom operations on the device here.
   *   // But do not call any Cairo functions that might acquire devices.
   *
   * } // device is automatically released at the end of the function scope
   * @endcode
   */
  class Lock
  {
  public:
    /** Create a new Device lock for @a device */
    Lock (const RefPtr<Device>& device);
    Lock (const Lock& other);
    ~Lock();

  private:
    RefPtr<Device> m_device;
  };


  /**
   * @since 1.10
   */
  enum class DeviceType
  {
      /**
       *
       */
      DRM = CAIRO_DEVICE_TYPE_DRM,

      /**
       *
       */
      GL = CAIRO_DEVICE_TYPE_GL,

      /**
       *
       */
      SCRIPT = CAIRO_DEVICE_TYPE_SCRIPT,

      /**
       *
       */
      XCB = CAIRO_DEVICE_TYPE_XCB,

      /**
       *
       */
      XLIB = CAIRO_DEVICE_TYPE_XLIB,

      /**
       *
       */
      XML = CAIRO_DEVICE_TYPE_XML
  };

  /** Create a C++ wrapper for the C instance. This C++ instance should then be given to a RefPtr.
   * @param cobject The C instance.
   * @param has_reference Whether we already have a reference. Otherwise, the constructor will take an extra reference.
   */
  explicit Device(cairo_device_t* cobject, bool has_reference = false);

  virtual ~Device();

  /** This function returns the type of the device */
  DeviceType get_type() const;

  /** Finish any pending operations for the device and also restore any
   * temporary modifications cairo has made to the device's state. This function
   * must be called before switching from using the device with Cairo to
   * operating on it directly with native APIs. If the device doesn't support
   * direct access, then this function does nothing.
   *
   * This function may acquire devices.
   */
  void flush();

  /** This function finishes the device and drops all references to external
   * resources. All surfaces, fonts and other objects created for this device
   * will be finished, too. Further operations on the device will not affect the
   * device but will instead trigger a DEVICE_FINISHED error.
   *
   * When the last reference to the device is dropped, cairo will call
   * finish() if it hasn't been called already, before freeing the resources
   * associated with the device.
   *
   * This function may acquire devices.
   */
  void finish();

  /** Acquires the device for the current thread. This function will block until
   * no other thread has acquired the device.
   *
   * If no exception is thrown, you successfully acquired the device. From now
   * on your thread owns the device and no other thread will be able to acquire
   * it until a matching call to release(). It is allowed to recursively acquire
   * the device multiple times from the same thread.
   *
   * @note It is recommended to use Device::Lock to acquire devices in an
   * exception-safe manner, rather than acquiring and releasing the device
   * manually.
   *
   * @warning You must never acquire two different devices at the same time
   * unless this is explicitly allowed. Otherwise the possibility of deadlocks
   * exist.
   *
   * @warning As various Cairo functions can acquire devices when called, these
   * functions may also cause deadlocks when you call them with an acquired
   * device. So you must not have a device acquired when calling them. These
   * functions are marked in the documentation.
   */
  void acquire();

  /** Releases a device previously acquired using acquire().
   */
  void release();

  typedef cairo_device_t cobject;

  inline cobject* cobj() { return m_cobject; }
  inline const cobject* cobj() const { return m_cobject; }

  #ifndef DOXYGEN_IGNORE_THIS
  ///For use only by the cairomm implementation.
  inline ErrorStatus get_status() const
  { return cairo_device_status(const_cast<cairo_device_t*>(cobj())); }
  #endif //DOXYGEN_IGNORE_THIS

  void reference() const;
  void unreference() const;

protected:

  cobject* m_cobject;
};

} // namespace Cairo

#endif //__CAIROMM_DEVICE_H

// vim: ts=2 sw=2 et
