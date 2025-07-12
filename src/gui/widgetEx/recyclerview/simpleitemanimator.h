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
#ifndef __SIMPLE_ITEM_ANIMATOR_H__
#define __SIMPLE_ITEM_ANIMATOR_H__
#include <widgetEx/recyclerview/recyclerview.h>

namespace cdroid{

class SimpleItemAnimator:public RecyclerView::ItemAnimator {
private:
    static constexpr bool _Debug = false;
protected:
    bool mSupportsChangeAnimations = true;
public:
    bool getSupportsChangeAnimations();
    void setSupportsChangeAnimations(bool supportsChangeAnimations);
    bool canReuseUpdatedViewHolder(RecyclerView::ViewHolder& viewHolder)override;
    bool animateDisappearance(RecyclerView::ViewHolder& viewHolder,
            ItemHolderInfo& preLayoutInfo, ItemHolderInfo* postLayoutInfo)override;
    bool animateAppearance(RecyclerView::ViewHolder& viewHolder,
            ItemHolderInfo* preLayoutInfo, ItemHolderInfo& postLayoutInfo)override;
    bool animatePersistence(RecyclerView::ViewHolder& viewHolder,
            ItemHolderInfo& preLayoutInfo, ItemHolderInfo& postLayoutInfo)override;
    bool animateChange(RecyclerView::ViewHolder& oldHolder, RecyclerView::ViewHolder& newHolder,
            ItemHolderInfo& preLayoutInfo, ItemHolderInfo& postLayoutInfo)override;
    virtual bool animateRemove(RecyclerView::ViewHolder& holder)=0;

    virtual bool animateAdd(RecyclerView::ViewHolder& holder)=0;

    virtual bool animateMove(RecyclerView::ViewHolder& holder, int fromX, int fromY, int toX, int toY)=0;

    virtual bool animateChange(RecyclerView::ViewHolder& oldHolder,
            RecyclerView::ViewHolder& newHolder, int fromLeft, int fromTop, int toLeft, int toTop)=0;

    void dispatchRemoveFinished(RecyclerView::ViewHolder& item);
    void dispatchMoveFinished(RecyclerView::ViewHolder& item);
    void dispatchAddFinished(RecyclerView::ViewHolder& item);
    void dispatchChangeFinished(RecyclerView::ViewHolder& item, bool oldItem);
    void dispatchRemoveStarting(RecyclerView::ViewHolder& item);
    void dispatchMoveStarting(RecyclerView::ViewHolder& item);
    void dispatchAddStarting(RecyclerView::ViewHolder& item);
    void dispatchChangeStarting(RecyclerView::ViewHolder& item, bool oldItem);
    virtual void onRemoveStarting(RecyclerView::ViewHolder& item);
    virtual void onRemoveFinished(RecyclerView::ViewHolder& item);
    virtual void onAddStarting(RecyclerView::ViewHolder& item);
    virtual void onAddFinished(RecyclerView::ViewHolder& item);
    virtual void onMoveStarting(RecyclerView::ViewHolder& item);
    virtual void onMoveFinished(RecyclerView::ViewHolder& item);
    virtual void onChangeStarting(RecyclerView::ViewHolder& item, bool oldItem);
    virtual void onChangeFinished(RecyclerView::ViewHolder& item, bool oldItem);
};
}/*endof namespace*/
#endif/*__SIMPLE_ITEM_ANIMATOR_H__*/
