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
#include <widgetEx/viewgrouputils.h>
#include <cairomm/matrix.h>
namespace cdroid{

ViewGroupUtils::ViewGroupUtils() {
}
void ViewGroupUtils::offsetDescendantRect(ViewGroup* parent, View* descendant, Rect& rect) {
    Cairo::Matrix m=Cairo::identity_matrix();
    
    offsetDescendantMatrix(parent, descendant, m);

    Cairo::RectangleInt&recti= (Cairo::RectangleInt&)rect;
    //rectF.set(rect.left,rect.top,rect.width,rect.height);
    m.transform_rectangle(recti);
}

void ViewGroupUtils::getDescendantRect(ViewGroup* parent, View* descendant, Rect& out) {
    out.set(0, 0, descendant->getWidth(), descendant->getHeight());
    offsetDescendantRect(parent, descendant, out);
}

void ViewGroupUtils::offsetDescendantMatrix(ViewGroup* target, View* view, Cairo::Matrix& m) {
    ViewGroup* parent = view->getParent();
    if (/*parent instanceof View &&*/ parent != target) {
        View* vp = (View*) parent;
        offsetDescendantMatrix(target, vp, m);
        m.translate(-vp->getScrollX(), -vp->getScrollY());
    }

    m.translate(view->getLeft(), view->getTop());

    if (!view->hasIdentityMatrix()){// getMatrix().isIdentity()) {
        m.multiply(m,view->getMatrix());
    }
}

}/*endof namespace*/
