#ifndef __DRAGEVENT_H__
#define __DRAGEVENT_H__

namespace cdroid{
class ClipData;
class ClipDescription;
class View;
class DragEvent{
private:
    static constexpr bool TRACK_RECYCLED_LOCATION = false;
protected:
    int mAction;
    float mX, mY;
    ClipDescription* mClipDescription;
    ClipData* mClipData;
    //IDragAndDropPermissions mDragAndDropPermissions;
    void* mLocalState;
    bool mDragResult;
    bool mEventHandlerWasCalled;
    friend class View;
private:
    DragEvent* mNext;
    //RuntimeException mRecycledLocation;
    bool mRecycled;

    static constexpr int MAX_RECYCLED = 10;
    static int gRecyclerUsed;
    static DragEvent* gRecyclerTop;

    /**
     * Action constant returned by {@link #getAction()}: Signals the start of a
     * drag and drop operation. The View should return {@code true} from its
     * {@link View#onDragEvent(DragEvent) onDragEvent()} handler method or
     * {@link View.OnDragListener#onDrag(View,DragEvent) OnDragListener.onDrag()} listener
     * if it can accept a drop. The onDragEvent() or onDrag() methods usually inspect the metadata
     * from {@link #getClipDescription()} to determine if they can accept the data contained in
     * this drag. For an operation that doesn't represent data transfer, these methods may
     * perform other actions to determine whether or not the View should accept the data.
     * If the View wants to indicate that it is a valid drop target, it can also react by
     * changing its appearance.
     * <p>
     *  Views added or becoming visible for the first time during a drag operation receive this
     *  event when they are added or becoming visible.
     * </p>
     * <p>
     *  A View only receives further drag events for the drag operation if it returns {@code true}
     *  in response to ACTION_DRAG_STARTED.
     * </p>
     * @see #ACTION_DRAG_ENDED
     * @see #getX()
     * @see #getY()
     */
public:
    static constexpr int ACTION_DRAG_STARTED = 1;

    /**
     * Action constant returned by {@link #getAction()}: Sent to a View after
     * {@link #ACTION_DRAG_ENTERED} while the drag shadow is still within the View object's bounding
     * box, but not within a descendant view that can accept the data. The {@link #getX()} and
     * {@link #getY()} methods supply
     * the X and Y position of of the drag point within the View object's bounding box.
     * <p>
     * A View receives an {@link #ACTION_DRAG_ENTERED} event before receiving any
     * ACTION_DRAG_LOCATION events.
     * </p>
     * <p>
     * The system stops sending ACTION_DRAG_LOCATION events to a View once the user moves the
     * drag shadow out of the View object's bounding box or into a descendant view that can accept
     * the data. If the user moves the drag shadow back into the View object's bounding box or out
     * of a descendant view that can accept the data, the View receives an ACTION_DRAG_ENTERED again
     * before receiving any more ACTION_DRAG_LOCATION events.
     * </p>
     * @see #ACTION_DRAG_ENTERED
     * @see #getX()
     * @see #getY()
     */
    static constexpr int ACTION_DRAG_LOCATION = 2;

    /**
     * Action constant returned by {@link #getAction()}: Signals to a View that the user
     * has released the drag shadow, and the drag point is within the bounding box of the View and
     * not within a descendant view that can accept the data.
     * The View should retrieve the data from the DragEvent by calling {@link #getClipData()}.
     * The methods {@link #getX()} and {@link #getY()} return the X and Y position of the drop point
     * within the View object's bounding box.
     * <p>
     * The View should return {@code true} from its {@link View#onDragEvent(DragEvent)}
     * handler or {@link View.OnDragListener#onDrag(View,DragEvent) OnDragListener.onDrag()}
     * listener if it accepted the drop, and {@code false} if it ignored the drop.
     * </p>
     * <p>
     * The View can also react to this action by changing its appearance.
     * </p>
     * @see #getClipData()
     * @see #getX()
     * @see #getY()
     */
    static constexpr int ACTION_DROP = 3;

    /**
     * Action constant returned by {@link #getAction()}:  Signals to a View that the drag and drop
     * operation has concluded.  A View that changed its appearance during the operation should
     * return to its usual drawing state in response to this event.
     * <p>
     *  All views with listeners that returned bool <code>true</code> for the ACTION_DRAG_STARTED
     *  event will receive the ACTION_DRAG_ENDED event even if they are not currently visible when
     *  the drag ends. Views removed during the drag operation won't receive the ACTION_DRAG_ENDED
     *  event.
     * </p>
     * <p>
     *  The View object can call {@link #getResult()} to see the result of the operation.
     *  If a View returned {@code true} in response to {@link #ACTION_DROP}, then
     *  getResult() returns {@code true}, otherwise it returns {@code false}.
     * </p>
     * @see #ACTION_DRAG_STARTED
     * @see #getResult()
     */
    static constexpr int ACTION_DRAG_ENDED = 4;

    /**
     * Action constant returned by {@link #getAction()}: Signals to a View that the drag point has
     * entered the bounding box of the View.
     * <p>
     *  If the View can accept a drop, it can react to ACTION_DRAG_ENTERED
     *  by changing its appearance in a way that tells the user that the View is the current
     *  drop target.
     * </p>
     * The system stops sending ACTION_DRAG_LOCATION events to a View once the user moves the
     * drag shadow out of the View object's bounding box or into a descendant view that can accept
     * the data. If the user moves the drag shadow back into the View object's bounding box or out
     * of a descendant view that can accept the data, the View receives an ACTION_DRAG_ENTERED again
     * before receiving any more ACTION_DRAG_LOCATION events.
     * </p>
     * @see #ACTION_DRAG_ENTERED
     * @see #ACTION_DRAG_LOCATION
     */
    static constexpr int ACTION_DRAG_ENTERED = 5;

    /**
     * Action constant returned by {@link #getAction()}: Signals that the user has moved the
     * drag shadow out of the bounding box of the View or into a descendant view that can accept
     * the data.
     * The View can react by changing its appearance in a way that tells the user that
     * View is no longer the immediate drop target.
     * <p>
     *  After the system sends an ACTION_DRAG_EXITED event to the View, the View receives no more
     *  ACTION_DRAG_LOCATION events until the user drags the drag shadow back over the View.
     * </p>
     *
     */
     static constexpr int ACTION_DRAG_EXITED = 6;
private:
    DragEvent();

    void init(int action, float x, float y, ClipDescription* description, ClipData* data,void* localState, bool result);

protected:
    static DragEvent* obtain();

    /** @hide */
public:
    static DragEvent* obtain(int action, float x, float y, void* localState,
            ClipDescription* description, ClipData* data,bool result);

    /** @hide */
    static DragEvent* obtain(DragEvent& source);

    /**
     * Inspect the action value of this event.
     * @return One of the following action constants, in the order in which they usually occur
     * during a drag and drop operation:
     * <ul>
     *  <li>{@link #ACTION_DRAG_STARTED}</li>
     *  <li>{@link #ACTION_DRAG_ENTERED}</li>
     *  <li>{@link #ACTION_DRAG_LOCATION}</li>
     *  <li>{@link #ACTION_DROP}</li>
     *  <li>{@link #ACTION_DRAG_EXITED}</li>
     *  <li>{@link #ACTION_DRAG_ENDED}</li>
     * </ul>
     */
    int getAction()const;

    /**
     * Gets the X coordinate of the drag point. The value is only valid if the event action is
     * {@link #ACTION_DRAG_STARTED}, {@link #ACTION_DRAG_LOCATION} or {@link #ACTION_DROP}.
     * @return The current drag point's X coordinate
     */
    float getX()const;

    /**
     * Gets the Y coordinate of the drag point. The value is only valid if the event action is
     * {@link #ACTION_DRAG_STARTED}, {@link #ACTION_DRAG_LOCATION} or {@link #ACTION_DROP}.
     * @return The current drag point's Y coordinate
     */
    float getY()const;

    /**
     * Returns the {@link android.content.ClipData} object sent to the system as part of the call
     * to
     * {@link android.view.View#startDragAndDrop(ClipData,View.DragShadowBuilder,Object,int)
     * startDragAndDrop()}.
     * This method only returns valid data if the event action is {@link #ACTION_DROP}.
     * @return The ClipData sent to the system by startDragAndDrop().
     */
    ClipData* getClipData()const;

    /**
     * Returns the {@link android.content.ClipDescription} object contained in the
     * {@link android.content.ClipData} object sent to the system as part of the call to
     * {@link android.view.View#startDragAndDrop(ClipData,View.DragShadowBuilder,Object,int)
     * startDragAndDrop()}.
     * The drag handler or listener for a View can use the metadata in this object to decide if the
     * View can accept the dragged View object's data.
     * <p>
     * This method returns valid data for all event actions except for {@link #ACTION_DRAG_ENDED}.
     * @return The ClipDescription that was part of the ClipData sent to the system by
     *     startDragAndDrop().
     */
    ClipDescription* getClipDescription() const;

    /**
     * Returns the local state object sent to the system as part of the call to
     * {@link android.view.View#startDragAndDrop(ClipData,View.DragShadowBuilder,Object,int)
     * startDragAndDrop()}.
     * The object is intended to provide local information about the drag and drop operation. For
     * example, it can indicate whether the drag and drop operation is a copy or a move.
     * <p>
     * The local state is available only to views in the activity which has started the drag
     * operation. In all other activities this method will return null
     * </p>
     * <p>
     *  This method returns valid data for all event actions.
     * </p>
     * @return The local state object sent to the system by startDragAndDrop().
     */
    void* getLocalState()const;

    /**
     * <p>
     * Returns an indication of the result of the drag and drop operation.
     * This method only returns valid data if the action type is {@link #ACTION_DRAG_ENDED}.
     * The return value depends on what happens after the user releases the drag shadow.
     * </p>
     * <p>
     * If the user releases the drag shadow on a View that can accept a drop, the system sends an
     * {@link #ACTION_DROP} event to the View object's drag event listener. If the listener
     * returns {@code true}, then getResult() will return {@code true}.
     * If the listener returns {@code false}, then getResult() returns {@code false}.
     * </p>
     * <p>
     * Notice that getResult() also returns {@code false} if no {@link #ACTION_DROP} is sent. This
     * happens, for example, when the user releases the drag shadow over an area outside of the
     * application. In this case, the system sends out {@link #ACTION_DRAG_ENDED} for the current
     * operation, but never sends out {@link #ACTION_DROP}.
     * </p>
     * @return {@code true} if a drag event listener returned {@code true} in response to
     * {@link #ACTION_DROP}. If the system did not send {@link #ACTION_DROP} before
     * {@link #ACTION_DRAG_ENDED}, or if the listener returned {@code false} in response to
     * {@link #ACTION_DROP}, then {@code false} is returned.
     */
    bool getResult() const;

    /**
     * Recycle the DragEvent, to be re-used by a later caller.  After calling
     * this function you must never touch the event again.
     *
     * @hide
     */
    void recycle();

    /* Parcelable interface */

    /**
     * Returns information about the {@link android.os.Parcel} representation of this DragEvent
     * object.
     * @return Information about the {@link android.os.Parcel} representation.
     */
    int describeContents();

    /**
     * Creates a {@link android.os.Parcel} object from this DragEvent object.
     * @param dest A {@link android.os.Parcel} object in which to put the DragEvent object.
     * @param flags Flags to store in the Parcel.
     */
    void writeToParcel(Parcel& dest, int flags);
};
}/*endof namspace*/
#endif
