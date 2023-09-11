#ifndef __EXPANDABLE_LISTADAPTER_H__
#define __EXPANDABLE_LISTADAPTER_H__
namespace cdroid{

class HeterogeneousExpandableList {
public:
    /**
     * Get the type of group View that will be created by
     * {@link android.widget.ExpandableListAdapter#getGroupView(int, boolean, View, ViewGroup)}
     * . for the specified group item.
     *
     * @param groupPosition the position of the group for which the type should be returned.
     * @return An integer representing the type of group View. Two group views should share the same
     *         type if one can be converted to the other in
     *         {@link android.widget.ExpandableListAdapter#getGroupView(int, boolean, View, ViewGroup)}
     *         . Note: Integers must be in the range 0 to {@link #getGroupTypeCount} - 1.
     *         {@link android.widget.Adapter#IGNORE_ITEM_VIEW_TYPE} can also be returned.
     * @see android.widget.Adapter#IGNORE_ITEM_VIEW_TYPE
     * @see #getGroupTypeCount()
     */
    virtual int getGroupType(int groupPosition)=0;

    /**
     * Get the type of child View that will be created by
     * {@link android.widget.ExpandableListAdapter#getChildView(int, int, boolean, View, ViewGroup)}
     * for the specified child item.
     *
     * @param groupPosition the position of the group that the child resides in
     * @param childPosition the position of the child with respect to other children in the group
     * @return An integer representing the type of child View. Two child views should share the same
     *         type if one can be converted to the other in
     *         {@link android.widget.ExpandableListAdapter#getChildView(int, int, boolean, View, ViewGroup)}
     *         Note: Integers must be in the range 0 to {@link #getChildTypeCount} - 1.
     *         {@link android.widget.Adapter#IGNORE_ITEM_VIEW_TYPE} can also be returned.
     * @see android.widget.Adapter#IGNORE_ITEM_VIEW_TYPE
     * @see #getChildTypeCount()
     */
    virtual int getChildType(int groupPosition, int childPosition)=0;

    /**
     * <p>
     * Returns the number of types of group Views that will be created by
     * {@link android.widget.ExpandableListAdapter#getGroupView(int, boolean, View, ViewGroup)}
     * . Each type represents a set of views that can be converted in
     * {@link android.widget.ExpandableListAdapter#getGroupView(int, boolean, View, ViewGroup)}
     * . If the adapter always returns the same type of View for all group items, this method should
     * return 1.
     * </p>
     * This method will only be called when the adapter is set on the {@link AdapterView}.
     *
     * @return The number of types of group Views that will be created by this adapter.
     * @see #getChildTypeCount()
     * @see #getGroupType(int)
     */
    virtual int getGroupTypeCount()=0;

    /**
     * <p>
     * Returns the number of types of child Views that will be created by
     * {@link android.widget.ExpandableListAdapter#getChildView(int, int, boolean, View, ViewGroup)}
     * . Each type represents a set of views that can be converted in
     * {@link android.widget.ExpandableListAdapter#getChildView(int, int, boolean, View, ViewGroup)}
     * , for any group. If the adapter always returns the same type of View for
     * all child items, this method should return 1.
     * </p>
     * This method will only be called when the adapter is set on the {@link AdapterView}.
     *
     * @return The total number of types of child Views that will be created by this adapter.
     * @see #getGroupTypeCount()
     * @see #getChildType(int, int)
     */
    virtual int getChildTypeCount()=0;
};

class ExpandableListAdapter {
public:
    /**
     * @see Adapter#registerDataSetObserver(DataSetObserver)
     */
    virtual void registerDataSetObserver(DataSetObserver* observer)=0;

    /**
     * @see Adapter#unregisterDataSetObserver(DataSetObserver)
     */
    virtual void unregisterDataSetObserver(DataSetObserver* observer)=0;

    /**
     * Gets the number of groups.
     *
     * @return the number of groups
     */
    virtual int getGroupCount()=0;

    /**
     * Gets the number of children in a specified group.
     *
     * @param groupPosition the position of the group for which the children
     *            count should be returned
     * @return the children count in the specified group
     */
    virtual int getChildrenCount(int groupPosition)=0;

    /**
     * Gets the data associated with the given group.
     *
     * @param groupPosition the position of the group
     * @return the data child for the specified group
     */
    virtual void* getGroup(int groupPosition)=0;
    /**
     * Get the type of group View that will be created by
     * {@link android.widget.ExpandableListAdapter#getGroupView(int, boolean, View, ViewGroup)}
     * . for the specified group item.
     *
     * @param groupPosition the position of the group for which the type should be returned.
     * @return An integer representing the type of group View. Two group views should share the same
     *         type if one can be converted to the other in
     *         {@link android.widget.ExpandableListAdapter#getGroupView(int, boolean, View, ViewGroup)}
     *         . Note: Integers must be in the range 0 to {@link #getGroupTypeCount} - 1.
     *         {@link android.widget.Adapter#IGNORE_ITEM_VIEW_TYPE} can also be returned.
     * @see android.widget.Adapter#IGNORE_ITEM_VIEW_TYPE
     * @see #getGroupTypeCount()
     */
    virtual int getGroupType(int groupPosition)=0;/*====*/
    /**
     * <p>
     * Returns the number of types of group Views that will be created by
     * {@link android.widget.ExpandableListAdapter#getGroupView(int, boolean, View, ViewGroup)}
     * . Each type represents a set of views that can be converted in
     * {@link android.widget.ExpandableListAdapter#getGroupView(int, boolean, View, ViewGroup)}
     * . If the adapter always returns the same type of View for all group items, this method should
     * return 1.
     * </p>
     * This method will only be called when the adapter is set on the {@link AdapterView}.
     *
     * @return The number of types of group Views that will be created by this adapter.
     * @see #getChildTypeCount()
     * @see #getGroupType(int)
     */

    virtual int getGroupTypeCount()=0;/*====*/
    /**
     * Gets the data associated with the given child within the given group.
     *
     * @param groupPosition the position of the group that the child resides in
     * @param childPosition the position of the child with respect to other
     *            children in the group
     * @return the data of the child
     */
    virtual void* getChild(int groupPosition, int childPosition)=0;
    /**
     * Get the type of child View that will be created by
     * {@link android.widget.ExpandableListAdapter#getChildView(int, int, boolean, View, ViewGroup)}
     * for the specified child item.
     *
     * @param groupPosition the position of the group that the child resides in
     * @param childPosition the position of the child with respect to other children in the group
     * @return An integer representing the type of child View. Two child views should share the same
     *         type if one can be converted to the other in
     *         {@link android.widget.ExpandableListAdapter#getChildView(int, int, boolean, View, ViewGroup)}
     *         Note: Integers must be in the range 0 to {@link #getChildTypeCount} - 1.
     *         {@link android.widget.Adapter#IGNORE_ITEM_VIEW_TYPE} can also be returned.
     * @see android.widget.Adapter#IGNORE_ITEM_VIEW_TYPE
     * @see #getChildTypeCount()
     */
    virtual int getChildType(int groupPosition, int childPosition)=0;/*===*/
    /**
     * <p>
     * Returns the number of types of child Views that will be created by
     * {@link android.widget.ExpandableListAdapter#getChildView(int, int, boolean, View, ViewGroup)}
     * . Each type represents a set of views that can be converted in
     * {@link android.widget.ExpandableListAdapter#getChildView(int, int, boolean, View, ViewGroup)}
     * , for any group. If the adapter always returns the same type of View for
     * all child items, this method should return 1.
     * </p>
     * This method will only be called when the adapter is set on the {@link AdapterView}.
     *
     * @return The total number of types of child Views that will be created by this adapter.
     * @see #getGroupTypeCount()
     * @see #getChildType(int, int)
     */
    
    virtual int getChildTypeCount()=0;/*===*/
    /**
     * Gets the ID for the group at the given position. This group ID must be
     * unique across groups. The combined ID (see
     * {@link #getCombinedGroupId(long)}) must be unique across ALL items
     * (groups and all children).
     *
     * @param groupPosition the position of the group for which the ID is wanted
     * @return the ID associated with the group
     */
    virtual long getGroupId(int groupPosition)=0;

    /**
     * Gets the ID for the given child within the given group. This ID must be
     * unique across all children within the group. The combined ID (see
     * {@link #getCombinedChildId(long, long)}) must be unique across ALL items
     * (groups and all children).
     *
     * @param groupPosition the position of the group that contains the child
     * @param childPosition the position of the child within the group for which
     *            the ID is wanted
     * @return the ID associated with the child
     */
    virtual long getChildId(int groupPosition, int childPosition)=0;

    /**
     * Indicates whether the child and group IDs are stable across changes to the
     * underlying data.
     *
     * @return whether or not the same ID always refers to the same object
     * @see Adapter#hasStableIds()
     */
    virtual bool hasStableIds()=0;

    /**
     * Gets a View that displays the given group. This View is only for the
     * group--the Views for the group's children will be fetched using
     * {@link #getChildView(int, int, boolean, View, ViewGroup)}.
     *
     * @param groupPosition the position of the group for which the View is
     *            returned
     * @param isExpanded whether the group is expanded or collapsed
     * @param convertView the old view to reuse, if possible. You should check
     *            that this view is non-null and of an appropriate type before
     *            using. If it is not possible to convert this view to display
     *            the correct data, this method can create a new view. It is not
     *            guaranteed that the convertView will have been previously
     *            created by
     *            {@link #getGroupView(int, boolean, View, ViewGroup)}.
     * @param parent the parent that this view will eventually be attached to
     * @return the View corresponding to the group at the specified position
     */
    virtual View* getGroupView(int groupPosition, bool isExpanded, View* convertView, ViewGroup* parent)=0;

    /**
     * Gets a View that displays the data for the given child within the given
     * group.
     *
     * @param groupPosition the position of the group that contains the child
     * @param childPosition the position of the child (for which the View is
     *            returned) within the group
     * @param isLastChild Whether the child is the last child within the group
     * @param convertView the old view to reuse, if possible. You should check
     *            that this view is non-null and of an appropriate type before
     *            using. If it is not possible to convert this view to display
     *            the correct data, this method can create a new view. It is not
     *            guaranteed that the convertView will have been previously
     *            created by
     *            {@link #getChildView(int, int, boolean, View, ViewGroup)}.
     * @param parent the parent that this view will eventually be attached to
     * @return the View corresponding to the child at the specified position
     */
    virtual View* getChildView(int groupPosition, int childPosition, bool isLastChild,
            View* convertView, ViewGroup* parent)=0;

    /**
     * Whether the child at the specified position is selectable.
     *
     * @param groupPosition the position of the group that contains the child
     * @param childPosition the position of the child within the group
     * @return whether the child is selectable.
     */
    virtual bool isChildSelectable(int groupPosition, int childPosition)=0;

    /**
     * @see ListAdapter#areAllItemsEnabled()
     */
    virtual bool areAllItemsEnabled()=0;

    /**
     * @see ListAdapter#isEmpty()
     */
    virtual bool isEmpty()=0;

    /**
     * Called when a group is expanded.
     *
     * @param groupPosition The group being expanded.
     */
    virtual void onGroupExpanded(int groupPosition)=0;

    /**
     * Called when a group is collapsed.
     *
     * @param groupPosition The group being collapsed.
     */
    virtual void onGroupCollapsed(int groupPosition)=0;

    /**
     * Gets an ID for a child that is unique across any item (either group or
     * child) that is in this list. Expandable lists require each item (group or
     * child) to have a unique ID among all children and groups in the list.
     * This method is responsible for returning that unique ID given a child's
     * ID and its group's ID. Furthermore, if {@link #hasStableIds()} is true, the
     * returned ID must be stable as well.
     *
     * @param groupId The ID of the group that contains this child.
     * @param childId The ID of the child.
     * @return The unique (and possibly stable) ID of the child across all
     *         groups and children in this list.
     */
    virtual int64_t getCombinedChildId(long groupId, long childId)=0;

    /**
     * Gets an ID for a group that is unique across any item (either group or
     * child) that is in this list. Expandable lists require each item (group or
     * child) to have a unique ID among all children and groups in the list.
     * This method is responsible for returning that unique ID given a group's
     * ID. Furthermore, if {@link #hasStableIds()} is true, the returned ID must be
     * stable as well.
     *
     * @param groupId The ID of the group
     * @return The unique (and possibly stable) ID of the group across all
     *         groups and children in this list.
     */
    virtual int64_t getCombinedGroupId(long groupId)=0;
};
}/*endof namespace*/

#endif/*__EXPANDABLE_LISTADAPTER_H__*/
