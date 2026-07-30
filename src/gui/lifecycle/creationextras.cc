#include <lifecycle/creationextras.h>
namespace cdroid{
namespace lifecycle{
const CreationExtras& EmptyCreationExtras(){
    static CreationExtras empty;
    return empty;
}
}//namespace lifecycle
}//namespace cdroid
