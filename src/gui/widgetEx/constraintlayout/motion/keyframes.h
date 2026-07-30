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

/*
 * Ported to C++ for CDROID from androidx.constraintlayout.motion.widget.KeyFrames.
 *
 * Parses the <KeyFrameSet> element of a MotionScene. Each child (<KeyAttribute>, <KeyPosition>,
 * <KeyCycle>, <KeyTimeCycle>, <KeyTrigger>) is read into a core MotionKey subclass (fields are
 * public, mirroring Java package-private access) and filed under the target view id
 * (MotionKey::UNSET applies the key to every view). KeyFrames owns the keys; callers receive
 * borrowed pointers and clone before handing them to a Motion controller (which owns its copies).
 *
 * The XML attribute parsing lives here in the widget layer so core/motion stays free of any
 * Android AttributeSet/Context dependency.
 */
#ifndef CDROID_CONSTRAINTLAYOUT_WIDGET_KEY_FRAMES_H
#define CDROID_CONSTRAINTLAYOUT_WIDGET_KEY_FRAMES_H

#include <memory>
#include <unordered_map>
#include <vector>

#include <widgetEx/constraintlayout/core/motion/motionkey.h>

namespace cdroid {

class Context;
class XmlPullParser;

class KeyFrames {
  public:
    KeyFrames() = default;
    // Parse <KeyFrameSet>; `parser` is at the <KeyFrameSet> START_TAG and is consumed through its
    // matching END_TAG.
    KeyFrames(Context* ctx, XmlPullParser& parser);

    void addKey(std::unique_ptr<MotionKey> key);

    // Borrowed pointers to the keys targeting `viewId` (plus the apply-to-all UNSET keys). Caller
    // must not delete; KeyFrames retains ownership.
    std::vector<MotionKey*> getKeysForView(int viewId) const;
    // Borrowed pointers to every key (all targets, flattened). Used by ViewTransition.addAllFrames
    // to inject the whole KeyFrameSet into a single Motion (Android KeyFrames.addAllFrames).
    std::vector<MotionKey*> getAllKeys() const;
    std::vector<int> getTargets() const;

  private:
    // targetId -> keys. MotionKey::UNSET (-1) means "apply to all views".
    std::unordered_map<int, std::vector<std::unique_ptr<MotionKey>>> mFramesMap;
};

} // namespace cdroid

#endif // CDROID_CONSTRAINTLAYOUT_WIDGET_KEY_FRAMES_H
