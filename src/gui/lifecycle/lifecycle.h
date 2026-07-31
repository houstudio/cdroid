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
#ifndef __LIFECYCLE_H__
#define __LIFECYCLE_H__
/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * Port of androidx.lifecycle.Lifecycle (commonMain). Holds the Event/State
 * enums and the static transition helpers. Coroutine/Flow machinery
 * (coroutineScope/eventFlow/currentStateFlow) is intentionally omitted.
 *********************************************************************************/
#include <stdexcept>

namespace cdroid{
namespace lifecycle{

class LifecycleObserver;
class LifecycleOwner;

class Lifecycle{
public:
    /** Ordered DESTROYED < INITIALIZED < CREATED < STARTED < RESUMED. */
    enum class State:int{
        DESTROYED   = 0,
        INITIALIZED = 1,
        CREATED     = 2,
        STARTED     = 3,
        RESUMED     = 4,
    };

    /** Lifecycle transition events. ON_ANY is a wildcard, never a real transition. */
    enum class Event:int{
        ON_CREATE = 0,
        ON_START,
        ON_RESUME,
        ON_PAUSE,
        ON_STOP,
        ON_DESTROY,
        ON_ANY,
    };

    virtual ~Lifecycle() = default;

    /** Brings the observer up to the current state. */
    virtual void addObserver(LifecycleObserver* observer) = 0;
    /** Removes the observer without dispatching destruction events. */
    virtual void removeObserver(LifecycleObserver* observer) = 0;
    /** The current state of the Lifecycle. */
    virtual State getCurrentState() const = 0;

    /** State reached after dispatching the given event. Throws on ON_ANY. */
    static State getTargetState(Event event);

    /** Nullable transition helpers (Event? in Kotlin). Return false and leave
     *  *out untouched when no transition exists for the given state. */
    static bool downFrom(State state, Event* out);
    static bool downTo(State state, Event* out);
    static bool upFrom(State state, Event* out);
    static bool upTo(State state, Event* out);
};

// Kotlin enums are Comparable by ordinal; provide the relational operators so
// LifecycleRegistry can mirror `state < target` style code verbatim.
inline bool operator<(Lifecycle::State a, Lifecycle::State b){ return (int)a < (int)b; }
inline bool operator>(Lifecycle::State a, Lifecycle::State b){ return (int)a > (int)b; }
inline bool operator<=(Lifecycle::State a, Lifecycle::State b){ return (int)a <= (int)b; }
inline bool operator>=(Lifecycle::State a, Lifecycle::State b){ return (int)a >= (int)b; }

/** Port of State.isAtLeast(state). */
inline bool isAtLeast(Lifecycle::State current, Lifecycle::State state){ return current >= state; }

}//namespace lifecycle
}//namespace cdroid
#endif
