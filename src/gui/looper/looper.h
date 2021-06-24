#ifndef __LOOPER_HEADER__
#define __LOOPER_HEADER__

#include <looper/EventLoop.h>
#include <looper/EventSource.h>
#include <looper/FileEvents.h>
#include <looper/FileSource.h>
#include <looper/GenericSignalSource.h>
#include <looper/IdleSource.h>
#include <looper/Platform.h>
#include <looper/SignalSource.h>
#include <looper/TimeoutSource.h>
#include <looper/Utility.h>

#ifdef LOOPER_LINUX
# include <looper/SignalFD.h>
# include <looper/TimerFD.h>
#endif

#endif // LOOPER_HEADER__
