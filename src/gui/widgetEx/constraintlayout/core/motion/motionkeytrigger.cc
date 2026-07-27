#include <widgetEx/constraintlayout/core/motion/motionkeytrigger.h>
#include <widgetEx/constraintlayout/core/motion/typedvalues.h>
namespace cdroid {
bool MotionKeyTrigger::setValue(int type, const std::string& value) {
    using T = TypedValues::TriggerType;
    if (type == T::TYPE_TRIGGER_ID) {
        /* store as string ref */ return true;
    }
    return false;
}
} // namespace cdroid
