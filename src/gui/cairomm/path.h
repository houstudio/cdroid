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

#ifndef __CAIROMM_PATH_H
#define __CAIROMM_PATH_H

#include <cairommconfig.h>

#include <cairomm/enums.h>
#include <iterator>
#include <string>
#include <cairo.h>

namespace Cairo
{
/** @example path-iter.cc
 * An example of iterating through a Cairo::Path
 */

/** A data structure for holding a path.
 *
 * Use Context::copy_path2() or Context::copy_path_flat2()
 * to instantiate a new %Path.
 *
 * If you use the deprecated Context::copy_path() or Context::copy_path_flat()
 * to instantiate a new %Path, the application is responsible for freeing the
 * %Path object when it is no longer needed.
 *
 * To access the path data, use begin() and end() or cbegin() and cend() to iterate
 * through the %Path Elements
 * @see Path::Element, Path::const_iterator
 */
class CAIROMM_API Path
{
public:
  class const_iterator;

  explicit Path(cairo_path_t* cobject, bool take_ownership = false);

  Path(const Path&) = delete;
  Path& operator=(const Path&) = delete;

  virtual ~Path();

  /** Returns a const_iterator pointing to the first Element of the %Path.
   * @newin{1,20}
   */
  inline const_iterator begin() const { return const_iterator(m_cobject->data); }

  /** Returns a const_iterator pointing to the first Element of the %Path.
   * @newin{1,20}
   */
  inline const_iterator cbegin() const { return begin(); }

  /** Returns a const_iterator that is one past the last Element of the %Path.
   * @newin{1,20}
   */
  inline const_iterator end() const { return const_iterator(m_cobject->data + m_cobject->num_data); }

  /** Returns a const_iterator that is one past the last Element of the %Path.
   * @newin{1,20}
   */
  inline const_iterator cend() const { return end(); }

  using cobject = cairo_path_t;
  inline cobject* cobj() { return m_cobject; }
  inline const cobject* cobj() const { return m_cobject; }

#ifndef DOXYGEN_IGNORE_THIS
  /// For use only by the cairomm implementation.
  inline ErrorStatus get_status() const
  { return m_cobject->status; }
#endif //DOXYGEN_IGNORE_THIS

protected:
  cobject* m_cobject;

public:
  /** Describes the type of one portion of a path when represented as a Path::Element.
   *
   * @see Path::Element
   * @newin{1,20}
   */
  enum class ElementType
  {
    /// A move-to operation.
    MOVE_TO = CAIRO_PATH_MOVE_TO,
    /// A line-to operation.
    LINE_TO = CAIRO_PATH_LINE_TO,
    /// A curve-to operation.
    CURVE_TO = CAIRO_PATH_CURVE_TO,
    /// A close-path operation.
    CLOSE_PATH = CAIRO_PATH_CLOSE_PATH
  };

  /** A single element of a path.  Each element has a 'type', which determines
   * how many Points are contained in this element. Use the subscript
   * operator[] to access the sub-points.
   *
   * Most people will rarely need access to the underlying path data,
   * so this will not be needed very often.
   *
   * @newin{1,20}
   */
  class Element
  {
    public:
      /** The base C cairo type */
      using cobject = cairo_path_data_t;

      Element(cobject* pData);

#ifndef DOXYGEN_IGNORE_THIS
      /* Reset this Element to use the base C cairo type specified by pData.
       * The reason it is implemented this way is so that the iterator can
       * contain a single Element instance and just shift what it's pointing at
       * instead of creating new Element objects whenever the iterator is
       * incremented.
       * Just storing a pointer to the base C type in the iterator is problematic,
       * because operator*() needs to return a reference type, so you need to
       * have an Element object to return a reference to.
       */
      inline void reset(cobject* pData) { m_cobject = pData; }
#endif //DOXYGEN_IGNORE_THIS

      /** Get a pointer to the base cairo type. */
      inline cobject* cobj() { return m_cobject; }
      /** Get a const pointer to the base cairo type. */
      inline const cobject* cobj() const { return m_cobject; }

      /** A simple structure for holding an X and Y coordinate pair. */
      struct Point
      {
        double x, y;
      };

      /** You can access the datapoints that make up a path Element by using
       * array notation. The index is zero-based, so element[0] gives you the
       * first point.
       * @throws std::out_of_range on invalid idx
       */
      Point operator[](unsigned int idx) const;

      /** Get the number of points in this path element. This is tightly
       * coupled with the type of %Path %Element that it is. MOVE_TO and
       * LINE_TO both have a single data point, CURVE_TO has three
       * data points, and CLOSE_PATH has none.
       */
      unsigned int size() const;

      /** Gets the type of element for this path element.
       */
      ElementType type() const
      { return static_cast<ElementType>(m_cobject->header.type); }

    protected:
      // The Element does not own the object that m_cobject points to.
      // That cairo_path_data_t object is usually an element in a cairo_path_t,
      // which is wrapped in a Path.
      cobject* m_cobject;
  };

  /** A custom %const_iterator for iterating over a %Path. This is made slightly
   * complicated because each element can have different numbers of
   * sub-elements, so advancing the %const_iterator must advance the base pointer a
   * different amount based on the current element.
   *
   * Also, because of the way the base cairo path structure is set up, you can
   * only traverse the list forward, because the only guarantee about the
   * structure is that the first element is a header type which tells the
   * location of the next header.
   *
   * @newin{1,20}
   */
  class const_iterator
  {
    friend class Path;
    public:
      using iterator_category = std::forward_iterator_tag;
      using value_type = const Element;
      using difference_type = std::ptrdiff_t;
      using pointer = const Element*;
      using reference = const Element&;

      inline bool operator==(const const_iterator& iter) const
      { return m_node.cobj() == iter.m_node.cobj(); }
      inline bool operator!=(const const_iterator& iter) const
      { return ! (*this == iter); }
      const_iterator& operator++();  // pre-increment

      const_iterator operator++(int);  // post-increment

      inline reference operator*() { return m_node; }
      inline pointer operator->() { return &m_node; }

    protected:
      const_iterator(cairo_path_data_t* path_data);

      Element m_node;
  };
};

} // namespace Cairo

#endif //__CAIROMM_PATH_H
