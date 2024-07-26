#ifndef __GESTURE_LIBRARIES_H__
#define __GESTURE_LIBRARIES_H__
#include <string>
#include <gesture/instance.h>
#include <gesture/gesturelibrary.h>
namespace cdroid{
class Context;
class GestureLibrary;
class GestureLibraries {
private:
    class StreamGestureLibrary;
    GestureLibraries();
public:
    static GestureLibrary* fromFile(const std::string& path);
    static GestureLibrary* fromRawResource(Context* context, const std::string&resourceId);
};
}/*endof namespace*/
#endif/*__GESTURE_LIBRARIES_H__*/
