/*********************************************************************************
 * Port of AOSP coretests android.util.ListScenario (+ ListItemFactory bits used
 * by the arrowscroll family), de-Activity-ified: one scenario == one private
 * full-screen Window. The Activity's onCreate becomes the two-phase
 * ctor + launch() (so the virtual init(Params&) dispatches to the derived
 * scenario — C++ ctors don't call derived overrides).
 *
 * Configurable by the number of items, how tall each item should be (in
 * relation to the screen height), and what item should start with selection.
 * Source of truth:
 * frameworks/base/core/tests/coretests/src/android/util/ListScenario.java
 * frameworks/base/core/tests/coretests/src/android/util/ListItemFactory.java
 *********************************************************************************/
#ifndef KEYNAV_LISTSCENARIO_H
#define KEYNAV_LISTSCENARIO_H
#include <cdroid.h>
#include <guienvironment.h>
#include <set>
#include <map>

namespace keynav {

/* Reusable methods for creating more complex list items
   (the subset used by the arrowscroll scenarios). */
struct ListItemFactory {
    static cdroid::View* twoButtonsSeparatedByFiller(int position, cdroid::Context* context, int desiredHeight);
    static cdroid::View* button(int position, cdroid::Context* context, const std::string& text, int desiredHeight);
    static cdroid::View* convertButton(cdroid::View* convertView, const std::string& text, int position);
    static cdroid::View* text(int position, cdroid::Context* context, const std::string& text, int desiredHeight);
    static cdroid::View* convertText(cdroid::View* convertView, const std::string& text, int position);
};

class ListScenario {
public:
    /* Better way to pass in optional params than a honkin' parameter list :) */
    class Params {
    public:
        int mNumItems = 4;
        bool mItemsFocusable = false;
        int mStartingSelectionPosition = 0;
        double mItemScreenSizeFactor = 1.0 / 5;
        double mFadingEdgeScreenSizeFactor = -1.0; /* null in AOSP */

        std::map<int, double> mOverrideItemScreenSizeFactors;
        std::vector<int> mUnselectableItems;
        bool mIncludeHeader = false;
        bool mStackFromBottom = false;
        bool mMustFillScreen = true;
        int mHeaderViewCount = 0;
        bool mHeaderFocusable = false;
        int mFooterViewCount = 0;
        bool mConnectAdapter = true;

        Params& setNumItems(int numItems) { mNumItems = numItems; return *this; }
        Params& setItemsFocusable(bool itemsFocusable) { mItemsFocusable = itemsFocusable; return *this; }
        Params& setStartingSelectionPosition(int p) { mStartingSelectionPosition = p; return *this; }
        Params& setItemScreenSizeFactor(double f) { mItemScreenSizeFactor = f; return *this; }
        Params& setPositionScreenSizeFactorOverride(int position, double f) {
            mOverrideItemScreenSizeFactors[position] = f; return *this;
        }
        Params& setPositionUnselectable(int position) { mUnselectableItems.push_back(position); return *this; }
        Params& setPositionsUnselectable(std::initializer_list<int> positions) {
            for (int pos : positions) setPositionUnselectable(pos);
            return *this;
        }
        Params& includeHeaderAboveList(bool includeHeader) { mIncludeHeader = includeHeader; return *this; }
        Params& setStackFromBottom(bool stackFromBottom) { mStackFromBottom = stackFromBottom; return *this; }
        Params& setMustFillScreen(bool fillScreen) { mMustFillScreen = fillScreen; return *this; }
        Params& setFadingEdgeScreenSizeFactor(double f) { mFadingEdgeScreenSizeFactor = f; return *this; }
        Params& setHeaderViewCount(int headerViewCount) { mHeaderViewCount = headerViewCount; return *this; }
        Params& setHeaderFocusable(bool headerFocusable) { mHeaderFocusable = headerFocusable; return *this; }
        Params& setFooterViewCount(int footerViewCount) { mFooterViewCount = footerViewCount; return *this; }
        Params& setConnectAdapter(bool connectAdapter) { mConnectAdapter = connectAdapter; return *this; }
    };

    ListScenario();
    virtual ~ListScenario();

    /* Activity.onCreate equivalent: createParams + init(params) + build the
       window tree. Call after construction (the derived init() then
       dispatches correctly). */
    void launch();

    cdroid::ListView* getListView() const { return mListView; }
    cdroid::LinearLayout* getListViewContainer() const { return mLinearLayout; }
    cdroid::Window* getWindow() const { return mWindow; }
    int getScreenHeight() const { return mScreenHeight; }

    /* how each scenario customizes its behavior */
    virtual void init(Params& params) = 0;

    /* selection / click hooks */
    virtual void positionSelected(int position);
    virtual void nothingSelected();
    virtual void positionClicked(int position);
    virtual void positionLongClicked(int position);

    const std::string getValueAtPosition(int position) const;
    int getHeightForPosition(int position) const;

    void setClickedPosition(int clickedPosition) { mClickedPosition = clickedPosition; }
    int getClickedPosition() const { return mClickedPosition; }
    void setLongClickedPosition(int longClickedPosition) { mLongClickedPosition = longClickedPosition; }
    int getLongClickedPosition() const { return mLongClickedPosition; }
    int getConvertMisses() const { return mConvertMisses; }

    void enableLongPress();

    /* create a view for a list item; override for a custom view beyond the
       simple focusable / unfocusable text view */
    virtual cdroid::View* createView(int position, cdroid::ViewGroup* parent, int desiredHeight);
    virtual cdroid::View* convertView(int position, cdroid::View* convertView, cdroid::ViewGroup* parent);
    virtual int getItemViewType(int position);
    virtual int getViewTypeCount();

protected:
    virtual cdroid::ListView* createListView();
    virtual void setAdapter(cdroid::ListView* listView);
    virtual Params createParams() { return Params(); }
    void readAndValidateParams(Params& params);
    bool isItemAtPositionSelectable(int position) const;

    cdroid::Window* mWindow = nullptr;
    cdroid::ListView* mListView = nullptr;
    cdroid::TextView* mHeaderTextView = nullptr;
    cdroid::LinearLayout* mLinearLayout = nullptr;

    int mNumItems = 0;
    bool mItemsFocusable = false;
    int mStartingSelectionPosition = 0;
    double mItemScreenSizeFactor = 0;
    std::map<int, double> mOverrideItemScreenSizeFactors;
    int mScreenHeight = 0;
    bool mIncludeHeader = false;
    std::set<int> mUnselectableItems;
    bool mStackFromBottom = false;
    int mClickedPosition = -1;
    int mLongClickedPosition = -1;
    int mConvertMisses = 0;
    int mHeaderViewCount = 0;
    bool mHeadersFocusable = false;
    int mFooterViewCount = 0;

private:
    class MyAdapter;
    friend class MyAdapter;
};

} // namespace keynav
#endif
