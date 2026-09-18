#ifndef __DESKCLOCK_ITEMADAPTER_H__
#define __DESKCLOCK_ITEMADAPTER_H__
/*********************************************************************************
 * Port of com.android.deskclock.ItemAdapter — base adapter for a collection of
 * ItemHolders: change notification with payloads, per-view-type factories and
 * click listeners, instance-state transfer across dataset changes.
 *
 * Kotlin's ItemHolder<T>/ItemViewHolder<T> generics map to a type-erased
 * ItemHolder/ItemViewHolder base (listeners are type-erased upstream too:
 * onItemChanged(ItemHolder<*>)) plus a typed subclass per model item.
 *********************************************************************************/
#include <unordered_map>
#include <vector>

#include <core/bundle.h>
#include <core/callbackbase.h>
#include <core/object.h>
#include <widgetEx/recyclerview/recyclerview.h>

namespace cdroid {
namespace deskclock {

class ItemHolder;

/** Callback interface for when an item changes and should be re-bound. */
class OnItemChangedListener : public EventSet {
public:
    // Invoked by ItemHolder.notifyItemChanged().
    CallbackBase<void, ItemHolder&> onItemChanged;
    // Invoked by ItemHolder.notifyItemChanged(payload) (payload variant).
    CallbackBase<void, ItemHolder&, Object*> onItemChangedWithPayload;
};

/** Callback interface for handling when an item is clicked. */
typedef CallbackBase<void, RecyclerView::ViewHolder&, int> OnItemClickedListener;

/**
 * Base class for wrapping an item for compatibility with an ItemAdapter.
 * A bridge between the model and view layer; state that should survive
 * dataset changes goes through onSave/onRestoreInstanceState.
 */
class ItemHolder {
private:
    /** Listeners to be invoked by notifyItemChanged(). */
    std::vector<OnItemChangedListener> mOnItemChangedListeners;

public:
    /** Globally unique id corresponding to the item. */
    const long itemId;

    explicit ItemHolder(long id) : itemId(id) {}
    virtual ~ItemHolder() = default;

    /** @return the unique identifier for the view that should represent the item. */
    virtual int getItemViewType() const = 0;

    /** Adds the listener if it is not already registered. */
    void addOnItemChangedListener(const OnItemChangedListener& listener) {
        for (const OnItemChangedListener& l : mOnItemChangedListeners) {
            if (l == listener) return;
        }
        mOnItemChangedListeners.push_back(listener);
    }

    /** Removes the listener from the current list of registered listeners. */
    void removeOnItemChangedListener(const OnItemChangedListener& listener) {
        for (auto it = mOnItemChangedListeners.begin(); it != mOnItemChangedListeners.end(); ++it) {
            if (*it == listener) {
                mOnItemChangedListeners.erase(it);
                return;
            }
        }
    }

    /** Invokes onItemChanged for all listeners added via addOnItemChangedListener. */
    void notifyItemChanged() {
        // Copy: listeners may mutate the list from the callback.
        std::vector<OnItemChangedListener> listeners = mOnItemChangedListeners;
        for (OnItemChangedListener& listener : listeners) {
            if (listener.onItemChanged) listener.onItemChanged(*this);
        }
    }

    /** Invokes the payload variant of onItemChanged for all listeners. */
    void notifyItemChanged(Object* payload) {
        std::vector<OnItemChangedListener> listeners = mOnItemChangedListeners;
        for (OnItemChangedListener& listener : listeners) {
            if (listener.onItemChangedWithPayload) listener.onItemChangedWithPayload(*this, payload);
        }
    }

    /** Called to retrieve per-instance state; do not retain the bundle. */
    virtual void onSaveInstanceState(Bundle& bundle) {}

    /** Called to restore state previously saved for an item with a matching itemId. */
    virtual void onRestoreInstanceState(const Bundle& bundle) {}
};

/**
 * Typed ItemHolder: wraps one model item (Kotlin's ItemHolder<T> generic).
 */
template <typename T>
class TypedItemHolder : public ItemHolder {
public:
    /** The item held by this holder. */
    T item;

    TypedItemHolder(T item, long id) : ItemHolder(id), item(item) {}
};

/**
 * Base class for a reusable RecyclerView.ViewHolder compatible with an
 * ItemHolder. Binds to an ItemHolder and can later be recycled.
 */
class ItemViewHolder : public RecyclerView::ViewHolder {
private:
    /** The current OnItemClickedListener associated with this holder. */
    OnItemClickedListener mOnItemClickedListener;

public:
    /** The current ItemHolder bound to this holder, or nullptr if unbound. */
    ItemHolder* itemHolder;

    explicit ItemViewHolder(View* itemView)
        : RecyclerView::ViewHolder(itemView)
        , itemHolder(nullptr) {}

    /** Binds the holder's itemView to a particular item. */
    void bindItemView(ItemHolder& itemHolder) {
        this->itemHolder = &itemHolder;
        onBindItemView(itemHolder);
    }

    /** Recycles the current item view, unbinding the current item holder and state. */
    void recycleItemView() {
        itemHolder = nullptr;
        mOnItemClickedListener = OnItemClickedListener();
        onRecycleItemView();
    }

    /** Sets the OnItemClickedListener invoked via notifyItemClicked (nullptr clears). */
    void setOnItemClickedListener(const OnItemClickedListener& listener) {
        mOnItemClickedListener = listener;
    }

    /** Called by subclasses to report a click event so it can be handled at a higher level. */
    void notifyItemClicked(int id) {
        if (mOnItemClickedListener) mOnItemClickedListener(*this, id);
    }

protected:
    /** Called when a new item is bound to the holder; subclassers bind their views here. */
    virtual void onBindItemView(ItemHolder& itemHolder) {}

    /** Called when the current item view is recycled; subclassers release bound state here. */
    virtual void onRecycleItemView() {}
};

/** Factory used by ItemAdapter for creating new ItemViewHolders. */
typedef CallbackBase<ItemViewHolder*, ViewGroup&, int> ItemViewHolderFactory;

/**
 * Base adapter class for displaying a collection of items: changing items,
 * persistent item state, item click events, and reusable item views.
 */
template <typename T /* ItemHolder subclass */>
class ItemAdapter : public RecyclerView::Adapter {
private:
    /** Finds the changed holder and invokes notifyItemChanged (with payload if present). */
    OnItemChangedListener mItemChangedNotifier;

    /** Invokes the OnItemClickedListener corresponding to the holder's view type. */
    OnItemClickedListener mOnItemClickedListener;

    /** Invoked when any item changes. */
    OnItemChangedListener mOnItemChangedListener;

    /** Factories for creating new ItemViewHolder entities, by view type. */
    std::unordered_map<int, ItemViewHolderFactory> mFactoriesByViewType;

    /** Listeners to invoke in mOnItemClickedListener, by view type. */
    std::unordered_map<int, OnItemClickedListener> mListenersByViewType;

public:
    /** List of current item holders represented by this adapter. */
    std::vector<T*>* items = nullptr;

    ItemAdapter() {
        mItemChangedNotifier.onItemChanged = [this](ItemHolder& itemHolder) {
            if (mOnItemChangedListener.onItemChanged) mOnItemChangedListener.onItemChanged(itemHolder);
            int position = indexOfItem(itemHolder);
            if (position != RecyclerView::NO_POSITION) {
                RecyclerView::Adapter::notifyItemChanged(position);
            }
        };
        mItemChangedNotifier.onItemChangedWithPayload =
                [this](ItemHolder& itemHolder, Object* payload) {
            if (mOnItemChangedListener.onItemChangedWithPayload) {
                mOnItemChangedListener.onItemChangedWithPayload(itemHolder, payload);
            }
            int position = indexOfItem(itemHolder);
            if (position != RecyclerView::NO_POSITION) {
                RecyclerView::Adapter::notifyItemChanged(position, payload);
            }
        };
        mOnItemClickedListener = [this](RecyclerView::ViewHolder& viewHolder, int id) {
            auto it = mListenersByViewType.find(viewHolder.getItemViewType());
            if (it != mListenersByViewType.end() && it->second) {
                it->second(viewHolder, id);
            }
        };
    }

    ~ItemAdapter() override {
        // The adapter owns the holders, not just the vector holding them
        // (upstream leans on GC; reloads pass a fresh heap vector each time).
        if (items != nullptr) {
            for (ItemHolder* itemHolder : *items) delete itemHolder;
        }
        delete items;
    }

    /** Convenience for calling setHasStableIds(true); returns *this for chaining. */
    ItemAdapter<T>& setHasStableIds() {
        RecyclerView::Adapter::setHasStableIds(true);
        return *this;
    }

    /** Sets the factory and click listener used for the given view types. */
    ItemAdapter<T>& withViewTypes(const ItemViewHolderFactory& factory,
                                  const OnItemClickedListener& listener,
                                  const std::vector<int>& viewTypes) {
        for (int viewType : viewTypes) {
            mFactoriesByViewType[viewType] = factory;
            mListenersByViewType[viewType] = listener;
        }
        return *this;
    }

    /**
     * Sets the item holders serving as the dataset and invokes notifyDataSetChanged.
     * With stable ids, instance state transfers between holders with equal itemId.
     */
    ItemAdapter<T>& setItems(std::vector<T*>* itemHolders) {
        std::vector<T*>* oldItemHolders = items;
        if (oldItemHolders != itemHolders) {
            if (oldItemHolders != nullptr) {
                for (ItemHolder* oldItemHolder : *oldItemHolders) {
                    oldItemHolder->removeOnItemChangedListener(mItemChangedNotifier);
                }
            }

            if (oldItemHolders != nullptr && itemHolders != nullptr && hasStableIds()) {
                // Transfer instance state from old to new holders by item id (O(N^2)
                // upstream too; item counts are assumed small).
                Bundle bundle;
                for (ItemHolder* newItemHolder : *itemHolders) {
                    for (ItemHolder* oldItemHolder : *oldItemHolders) {
                        if (newItemHolder->itemId == oldItemHolder->itemId
                                && newItemHolder != oldItemHolder) {
                            bundle.clear();
                            oldItemHolder->onSaveInstanceState(bundle);
                            newItemHolder->onRestoreInstanceState(bundle);
                            break;
                        }
                    }
                }
            }

            if (itemHolders != nullptr) {
                for (ItemHolder* newItemHolder : *itemHolders) {
                    newItemHolder->addOnItemChangedListener(mItemChangedNotifier);
                }
            }

            // Ownership passes with each set: free the outgoing holders before
            // the vector (upstream's GC makes this a no-op there).
            if (oldItemHolders != nullptr) {
                for (ItemHolder* oldItemHolder : *oldItemHolders) delete oldItemHolder;
            }
            delete oldItemHolders;
            items = itemHolders;
            notifyDataSetChanged();
        }
        return *this;
    }

    /** Inserts the item holder at position (bounded by size); notifies insert. */
    ItemAdapter<T>& addItem(int position, T* itemHolder) {
        itemHolder->addOnItemChangedListener(mItemChangedNotifier);
        position = std::min(position, (int) items->size());
        items->insert(items->begin() + position, itemHolder);
        notifyItemInserted(position);
        return *this;
    }

    /** Removes the first occurrence of the item holder; notifies remove. */
    ItemAdapter<T>& removeItem(T* itemHolder) {
        int index = indexOfItem(*itemHolder);
        if (index >= 0) {
            T* removed = items->at(index);
            items->erase(items->begin() + index);
            removed->removeOnItemChangedListener(mItemChangedNotifier);
            notifyItemRemoved(index);
        }
        return *this;
    }

    /** Sets the listener to be invoked whenever any item changes. */
    void setOnItemChangedListener(const OnItemChangedListener& listener) {
        mOnItemChangedListener = listener;
    }

    int getItemCount() override { return items ? (int) items->size() : 0; }

    long getItemId(int position) override {
        return hasStableIds() ? items->at(position)->itemId : RecyclerView::NO_ID;
    }

    T* findItemById(long id) const {
        for (T* holder : *items) {
            if (holder->itemId == id) return holder;
        }
        return nullptr;
    }

    int getItemViewType(int position) override {
        return items->at(position)->getItemViewType();
    }

    RecyclerView::ViewHolder* onCreateViewHolder(ViewGroup* parent, int viewType) override {
        auto it = mFactoriesByViewType.find(viewType);
        if (it != mFactoriesByViewType.end() && it->second) {
            return it->second(*parent, viewType);
        }
        throw std::invalid_argument("Unsupported view type: " + std::to_string(viewType));
    }

    void onBindViewHolder(RecyclerView::ViewHolder& viewHolder, int position) override {
        ItemViewHolder& ivh = static_cast<ItemViewHolder&>(viewHolder);
        ivh.bindItemView(*items->at(position));
        ivh.setOnItemClickedListener(mOnItemClickedListener);
    }

    void onViewRecycled(RecyclerView::ViewHolder& viewHolder) override {
        ItemViewHolder& ivh = static_cast<ItemViewHolder&>(viewHolder);
        ivh.setOnItemClickedListener(OnItemClickedListener());
        ivh.recycleItemView();
    }

private:
    int indexOfItem(const ItemHolder& itemHolder) const {
        if (items == nullptr) return RecyclerView::NO_POSITION;
        for (size_t i = 0; i < items->size(); i++) {
            if (items->at(i) == &itemHolder) return (int) i;
        }
        return RecyclerView::NO_POSITION;
    }
};

} // namespace deskclock
} // namespace cdroid

#endif // __DESKCLOCK_ITEMADAPTER_H__
