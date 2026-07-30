#ifndef __FRAGMENTCONTAINER_H__
#define __FRAGMENTCONTAINER_H__
/*********************************************************************************
 * Port of androidx.fragment.app.FragmentContainer. Callbacks to a Fragment's
 * container (locate views). The deprecated instantiate(Context,String,Bundle) is
 * omitted; CDROID instantiates Fragments via FragmentFactory's registry.
 *********************************************************************************/
#include <view/view.h>
namespace cdroid{
namespace fragment{

class FragmentContainer{
public:
    virtual ~FragmentContainer() = default;
    // Returns the view with the given id, or null if not a child of this container.
    virtual cdroid::View* onFindViewById(int id) = 0;
    // Returns true if the container holds any view.
    virtual bool onHasView() = 0;
};

}//namespace fragment
}//namespace cdroid
#endif
