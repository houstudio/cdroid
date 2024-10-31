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
