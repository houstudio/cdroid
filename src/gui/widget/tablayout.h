/*********************************************************************************
 * Copyright (C) [2019] [houzh@msn.com]
 *
 * Compatibility shim. TabLayout (an androidx material-components port) has moved
 * to widgetEx/tablayout/ — co-located with its per-component res/ tree. This
 * header forwards so existing `#include <widget/tablayout.h>` sites (apps, the
 * umbrella cdroid.h, TabLayoutMediator) keep working unchanged.
 *********************************************************************************/
#ifndef __TABLAYOUT_SHIM_H__
#define __TABLAYOUT_SHIM_H__
#include <widgetEx/tablayout/tablayout.h>
#endif // __TABLAYOUT_SHIM_H__
