#ifndef CDROID_ESPRESSO_ADAPTERVIEWPROTOCOL_H
#define CDROID_ESPRESSO_ADAPTERVIEWPROTOCOL_H

/*
 * android.support.test.espresso.action.AdapterViewProtocol — a sadly
 * necessary layer of indirection to interact with AdapterViews.
 *
 * Android breaks the Liskov substitution principle with ExpandableListView —
 * you can't use getAdapter(), getItemAtPosition() and other methods common
 * to AdapterViews on an ExpandableListView because an ExpandableListView
 * isn't an AdapterView - they just share a lot of code. This interface
 * exists to work around this wart and lets the implementor translate
 * Espresso's needs and manipulations of the AdapterView into calls that
 * make sense for the given subtype and context.
 *
 * CDROID adaptations (forced by C++ / the port's scope):
 *  - Java's Object data model maps to void* (CDROID Adapter::getItem already
 *    returns void*), so AdaptedData.opaqueToken is a void*; protocols that
 *    need integer tokens encode them via intptr_t (the standard protocol
 *    stores the adapter position that way, so token equality still means
 *    "same data object", as AOSP's Integer.equals does);
 *  - android.database.Cursor is not ported: AdaptedData.toString() prints
 *    the pointers, and StandardDataFunction drops the Cursor
 *    moveToPosition() branch;
 *  - the deprecated EspressoOptional-returning getDataRenderedByView() is
 *    not ported — only getDataRenderedByView2() is (AOSP's own core only
 *    calls the latter; the default method that bridges them exists purely
 *    for the deprecation).
 */

#include <memory>
#include <string>
#include <vector>

namespace cdroid {
class AdapterView;
class View;

namespace espresso {

/**
 * A custom function that is applied when AdaptedData::getData() is executed.
 */
class DataFunction {
public:
    virtual ~DataFunction() = default;
    virtual void* getData() = 0;
};

/**
 * A holder that associates a data object from an AdapterView with a token
 * the AdapterViewProtocol can use to force that data object to be rendered
 * as a child or deeper descendant of the adapter view.
 */
class AdaptedData {
public:
    /**
     * A token the implementor of AdapterViewProtocol can use to force the
     * adapterView to display this data object as a child or deeper
     * descendant in it. Equal opaqueTokens point to the same data object on
     * the AdapterView.
     */
    void* opaqueToken = nullptr;

    /** One of the objects the AdapterView is exposing to the user. */
    void* getData() const { return mDataFunction->getData(); }

    /** Object.toString() — a debug representation (see file header). */
    std::string toString() const;

    class Builder {
    public:
        Builder& withDataFunction(std::shared_ptr<DataFunction> dataFunction) {
            mDataFunction = std::move(dataFunction);
            return *this;
        }

        Builder& withData(void* data) {
            mData = data;
            return *this;
        }

        Builder& withOpaqueToken(void* opaqueToken) {
            mOpaqueToken = opaqueToken;
            return *this;
        }

        AdaptedData build();

    private:
        void* mData = nullptr;
        void* mOpaqueToken = nullptr;
        std::shared_ptr<DataFunction> mDataFunction;
    };

private:
    AdaptedData(void* data, void* opaqueToken, std::shared_ptr<DataFunction> dataFunction);

    std::shared_ptr<DataFunction> mDataFunction;
};

/**
 * AdapterViewProtocol — see the file header. getDataRenderedByView2()
 * returns a nullable owning pointer (Java's @Nullable AdaptedData on the GC
 * heap): null means "this descendant renders no data".
 */
class AdapterViewProtocol {
public:
    virtual ~AdapterViewProtocol() = default;

    /**
     * Returns all data this AdapterViewProtocol can find within the given
     * AdapterView. Any AdaptedData returned by this method can be passed to
     * makeDataRenderedWithinAdapterView and the implementation should make
     * the AdapterView bring that data item onto the screen.
     */
    virtual std::vector<AdaptedData> getDataInAdapterView(AdapterView& adapterView) = 0;

    /**
     * Returns the data object this particular view is rendering if possible.
     *
     * Implementations are expected to create a relationship between the data
     * in the AdapterView and the descendant views of the AdapterView that
     * obeys the following conditions:
     *  - For each descendant view there exists either 0 or 1 data objects it
     *    is rendering.
     *  - For each data object in the AdapterView there exists either 0 or 1
     *    descendant views which claim to be rendering it.
     */
    virtual std::shared_ptr<AdaptedData> getDataRenderedByView2(
            AdapterView& adapterView, View& descendantView) = 0;

    /**
     * Requests that a particular piece of data held in this AdapterView is
     * actually rendered by it. This need not happen immediately (EG: a
     * ListView implementor may smoothScrollToPosition). The only guarantee
     * is that eventually — with no further interaction necessary — this data
     * item will be rendered as a child or deeper descendant of this
     * AdapterView.
     */
    virtual void makeDataRenderedWithinAdapterView(
            AdapterView& adapterView, const AdaptedData& data) = 0;

    /**
     * Indicates whether or not there now exists a descendant view within
     * adapterView that is rendering this data.
     */
    virtual bool isDataRenderedWithinAdapterView(
            AdapterView& adapterView, const AdaptedData& adaptedData) = 0;
};

} /*endof namespace espresso*/
} /*endof namespace cdroid*/

#endif /*CDROID_ESPRESSO_ADAPTERVIEWPROTOCOL_H*/
