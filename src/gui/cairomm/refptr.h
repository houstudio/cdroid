#ifndef __CAIROMM_REFPTR_H
#define __CAIROMM_REFPTR_H
/* Copyright 2005 The cairomm Development Team
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

#include <memory>

namespace Cairo
{

/** %RefPtr<> is a reference-counting shared smartpointer.
 *
 * Reference counting means that a shared reference count is incremented each
 * time a %RefPtr is copied, and decremented each time a %RefPtr is destroyed,
 * for instance when it leaves its scope. When the reference count reaches
 * zero, the contained object is deleted
 *
 * cairomm uses %RefPtr so that you don't need to remember
 * to delete the object explicitly, or know when a method expects you to delete 
 * the object that it returns, and to prevent any need to manually  reference 
 * and unreference cairo objects.
 *
 * @see Cairo::make_refptr_for_instance()
 */
template <typename T_CppObject>
using RefPtr = std::shared_ptr<T_CppObject>;

// Cairo::make_refptr_for_instance() is not strictly necessary. It's used because
// 1. Similar to glibmm. Compare Glib::make_refptr_for_instance().
// 2. When the RefPtr constructor is called only from make_refptr_for_instance(),
//    future changes, if necessary, are easier to make.
/** Create a %RefPtr<> to an instance of any reference-counted class whose
 * destructor is noexcept (the default for destructors).
 *
 * Normal application code should not need to use this. However, this is necessary
 * when implementing create() methods for derived classes, such as a derived
 * Cairo::UserFontFace.
 */
template <typename T_CppObject>
RefPtr<T_CppObject>
make_refptr_for_instance(T_CppObject* object)
{
  return RefPtr<T_CppObject>(object);
}

} // namespace Cairo

#endif /* __CAIROMM_REFPTR_H */
