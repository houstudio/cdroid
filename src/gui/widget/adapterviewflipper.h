/*********************************************************************************
+ * Copyright (C) [2019] [houzh@msn.com]
+ *
+ * This library is free software; you can redistribute it and/or
+ * modify it under the terms of the GNU Lesser General Public
+ * License as published by the Free Software Foundation; either
+ * version 2.1 of the License, or (at your option) any later version.
+ *
+ * This library is distributed in the hope that it will be useful,
+ * but WITHOUT ANY WARRANTY; without even the implied warranty of
+ * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
+ * Lesser General Public License for more details.
+ *
+ * You should have received a copy of the GNU Lesser General Public
+ * License along with this library; if not, write to the Free Software
+ * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
+ *********************************************************************************/
#ifndef __ADAPTER_VIEW_FLIPPER_H__
#define __ADAPTER_VIEW_FLIPPER_H__
#include <widget/adapterviewanimator.h>
namespace cdroid{
class AdapterViewFlipper:public AdapterViewAnimator {
private:
    static constexpr int DEFAULT_INTERVAL = 10000;
    int mFlipInterval = DEFAULT_INTERVAL;
    bool mAutoStart = false;
    bool mRunning = false;
    bool mStarted = false;
    bool mVisible = false;
    bool mAdvancedByHost = false;
    Runnable mFlipRunnable;
private:
    void updateRunning(bool flipNow);
protected:
    void onAttachedToWindow() override;
    void onDetachedFromWindow() override;
    void onWindowVisibilityChanged(int visibility) override;
public:
    AdapterViewFlipper(Context* context,const AttributeSet& attrs);
    
    void setAdapter(Adapter* adapter) override;

    int getFlipInterval() const;

    void setFlipInterval(int flipInterval);

    void startFlipping();

    void stopFlipping();

    void showNext() override;

    void showPrevious() override;

    bool isFlipping() const;

    void setAutoStart(bool autoStart);
    bool isAutoStart() const;

    void fyiWillBeAdvancedByHostKThx() ;//override;
    std::string getAccessibilityClassName() const override;
};
}/*endof namespace*/
#endif/*__ADAPTER_VIEW_FLIPPER_H__*/
