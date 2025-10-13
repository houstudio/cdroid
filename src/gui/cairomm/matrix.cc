/* Copyright (C) 2008 Jonathon Jongsma
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
#include <cairomm/matrix.h>
#include <cairomm/private.h>
#include <cmath>
#include <climits>
namespace Cairo
{

Matrix::Matrix()
{
}

Matrix::Matrix(double xx, double yx, double xy, double yy, double x0, double y0)
{
  cairo_matrix_init(this, xx, yx, xy, yy, x0, y0);
}

Matrix identity_matrix()
{
  Matrix m;
  cairo_matrix_init_identity(&m);
  return m;
}

Matrix translation_matrix(double tx, double ty)
{
  Matrix m;
  cairo_matrix_init_translate(&m, tx, ty);
  return m;
}

Matrix scaling_matrix(double sx, double sy)
{
  Matrix m;
  cairo_matrix_init_scale(&m, sx, sy);
  return m;
}

Matrix rotation_matrix(double radians)
{
  Matrix m;
  cairo_matrix_init_rotate(&m, radians);
  return m;
}

void Matrix::translate(double tx, double ty)
{
  cairo_matrix_translate(this, tx, ty);
}

void Matrix::scale(double sx, double sy)
{
  cairo_matrix_scale(this, sx, sy);
}

void Matrix::rotate(double radians)
{
  cairo_matrix_rotate(this, radians);
}

void Matrix::invert()
{
  auto status = cairo_matrix_invert(this);
  check_status_and_throw_exception(status);
}

// throws exception
void Matrix::multiply(Matrix& a, Matrix& b)
{
  cairo_matrix_multiply(this, &a, &b);
}

void Matrix::transform_distance(double& dx, double& dy) const
{
  cairo_matrix_transform_distance(this, &dx, &dy);
}

void Matrix::transform_point(double& x, double& y) const
{
  cairo_matrix_transform_point(this, &x, &y);
}

Matrix operator*(const Matrix& a, const Matrix& b)
{
  Matrix m;
  cairo_matrix_multiply(&m, &a, &b);
  return m;
}

void Matrix::transform_rectangle(Rectangle& io)const{
    double x1,y1,x2,y2,x3,y3,x4,y4,min_x,min_y,max_x,max_y;
    x1 = io.x;
    y1 = io.y;
    
    x2 = io.x+io.width;
    y2 = io.y;
    
    x3 = io.x+io.width;
    y3 = io.y+io.height;
    
    x4 = io.x;
    y4 = io.y+io.height;
    
    transform_point(x1,y1);
    transform_point(x2,y2);
    transform_point(x3,y3);
    transform_point(x4,y4);

    min_x = fmin(fmin(x1, x2), fmin(x3, x4));
    max_x = fmax(fmax(x1, x2), fmax(x3, x4));
    min_y = fmin(fmin(y1, y2), fmin(y3, y4));
    max_y = fmax(fmax(y1, y2), fmax(y3, y4));

    io.x = min_x;
    io.y = min_y;
    io.width = max_x - min_x;
    io.height= max_y - min_y;
}

void Matrix::transform_rectangle(RectangleInt& io)const{
    Rectangle tmp = {(double)io.x,(double)io.y,(double)io.width,(double)io.height};
    transform_rectangle(tmp);
    io.x = std::floor(tmp.x);
    io.y = std::floor(tmp.y);
    io.width = std::ceil(tmp.width);
    io.height= std::ceil(tmp.height);
}

} // namespace Cairo
