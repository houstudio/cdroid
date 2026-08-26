/*********************************************************************************
 * Port of AOSP coretests android.util.ListScenario — see header. The onCreate
 * body lives in launch(); assertions replace IllegalArgumentException throws.
 *********************************************************************************/
#include <gtest/gtest.h>
#include "listscenario.h"
#include "keysender.h"

using namespace cdroid;

namespace keynav {

/* string-tag sentinels (AOSP uses view tags "text"/"button"/"twoButtons") */
static const char* const TAG_TEXT = "text";
static const char* const TAG_BUTTON = "button";
static const char* const TAG_TWO_BUTTONS = "twoButtons";

static bool hasTag(View* view, const char* tag) {
    return view->getTag() && strcmp((const char*)view->getTag(), tag) == 0;
}

/*********************************************************************************
 * ListItemFactory (subset)
 *********************************************************************************/
View* ListItemFactory::twoButtonsSeparatedByFiller(int position, Context* context, int desiredHeight) {
    EXPECT_GT(desiredHeight, 90) << "need at least 90 pixels of height to create "
            "the two buttons and leave 10 pixels for the filler";

    LinearLayout* ll = new LinearLayout(context);
    ll->setOrientation(LinearLayout::VERTICAL);

    LinearLayout::LayoutParams* buttonLp = new LinearLayout::LayoutParams(
            ViewGroup::LayoutParams::MATCH_PARENT, 50);

    Button* topButton = new Button(context);
    topButton->setLayoutParams(buttonLp);
    topButton->setText("top (position " + std::to_string(position) + ")");
    ll->addView(topButton);

    TextView* middleFiller = new TextView(context);
    middleFiller->setLayoutParams(new LinearLayout::LayoutParams(
            ViewGroup::LayoutParams::MATCH_PARENT, desiredHeight - 100));
    middleFiller->setText("filler");
    ll->addView(middleFiller);

    Button* bottomButton = new Button(context);
    bottomButton->setLayoutParams(new LinearLayout::LayoutParams(
            ViewGroup::LayoutParams::MATCH_PARENT, 50));
    bottomButton->setText("bottom (position " + std::to_string(position) + ")");
    ll->addView(bottomButton);
    ll->setTag((void*)TAG_TWO_BUTTONS);
    return ll;
}

View* ListItemFactory::button(int position, Context* context, const std::string& text, int desiredHeight) {
    TextView* result = new Button(context);
    result->setHeight(desiredHeight);
    result->setText(text);
    result->setLayoutParams(new AbsListView::LayoutParams(
            ViewGroup::LayoutParams::MATCH_PARENT,
            ViewGroup::LayoutParams::WRAP_CONTENT));
    result->setId(position);
    result->setTag((void*)TAG_BUTTON);
    return result;
}

View* ListItemFactory::convertButton(View* convertView, const std::string& text, int position) {
    if (hasTag(convertView, TAG_BUTTON)) {
        ((TextView*)convertView)->setText(text);
        convertView->setId(position);
        return convertView;
    }
    return nullptr;
}

View* ListItemFactory::text(int position, Context* context, const std::string& text, int desiredHeight) {
    TextView* result = new TextView(context);
    result->setHeight(desiredHeight);
    result->setText(text);
    result->setLayoutParams(new AbsListView::LayoutParams(
            ViewGroup::LayoutParams::MATCH_PARENT,
            ViewGroup::LayoutParams::WRAP_CONTENT));
    result->setId(position);
    result->setTag((void*)TAG_TEXT);
    return result;
}

View* ListItemFactory::convertText(View* convertView, const std::string& text, int position) {
    if (hasTag(convertView, TAG_TEXT)) {
        ((TextView*)convertView)->setText(text);
        convertView->setId(position);
        return convertView;
    }
    return nullptr;
}

/*********************************************************************************
 * MyAdapter
 *********************************************************************************/
class ListScenario::MyAdapter : public Adapter {
private:
    ListScenario* mScenario;
public:
    MyAdapter(ListScenario* scenario) : mScenario(scenario) {}

    int getCount() const override { return mScenario->mNumItems; }
    /* AOSP returns the label Object; nobody consumes it here — no allocation
       (nothing owns it to free). */
    void* getItem(int position) const override { return nullptr; }
    long getItemId(int position) const override { return position; }

    bool areAllItemsEnabled() const override {
        return mScenario->mUnselectableItems.empty();
    }
    bool isEnabled(int position) const override {
        return mScenario->isItemAtPositionSelectable(position);
    }

    View* getView(int position, View* convertView, ViewGroup* parent) override {
        View* result = nullptr;
        EXPECT_GE(position, 0);
        EXPECT_LT(position, mScenario->mNumItems);

        if (convertView != nullptr) {
            result = mScenario->convertView(position, convertView, parent);
            if (result == nullptr) {
                mScenario->mConvertMisses++;
            }
        }

        if (result == nullptr) {
            const int desiredHeight = mScenario->getHeightForPosition(position);
            result = mScenario->createView(position, parent, desiredHeight);
        }
        return result;
    }

    int getItemViewType(int position) const override {
        return mScenario->getItemViewType(position);
    }
    int getViewTypeCount() const override {
        return mScenario->getViewTypeCount();
    }
};

/*********************************************************************************
 * ListScenario
 *********************************************************************************/
ListScenario::ListScenario() {
}

ListScenario::~ListScenario() {
}

void ListScenario::launch() {
    App& app = App::getInstance();

    /* the activity: a private full-screen window (harness removes strays) */
    mWindow = new Window(&app, 0, 0, -1, -1);

    /* WindowMetrics height of the fresh activity window */
    mScreenHeight = GUIEnvironment::stage()->getHeight();
    ASSERT_GT(mScreenHeight, 0);

    Params params = createParams();
    init(params);

    readAndValidateParams(params);

    mListView = createListView();
    mListView->setLayoutParams(new ViewGroup::LayoutParams(
            ViewGroup::LayoutParams::MATCH_PARENT,
            ViewGroup::LayoutParams::MATCH_PARENT));
    mListView->setDrawSelectorOnTop(false);

    for (int i = 0; i < mHeaderViewCount; i++) {
        TextView* header = mHeadersFocusable ?
                (TextView*)new EditText(&app) :
                (TextView*)new TextView(&app);
        header->setText("Header: " + std::to_string(i));
        mListView->addHeaderView(header);
    }

    for (int i = 0; i < mFooterViewCount; i++) {
        TextView* footer = new TextView(&app);
        footer->setText("Footer: " + std::to_string(i));
        mListView->addFooterView(footer);
    }

    if (params.mConnectAdapter) {
        setAdapter(mListView);
    }

    mListView->setItemsCanFocus(mItemsFocusable);
    if (mStartingSelectionPosition >= 0) {
        mListView->setSelection(mStartingSelectionPosition);
    }
    mListView->setPadding(0, 0, 0, 0);
    mListView->setStackFromBottom(mStackFromBottom);
    mListView->setDivider(nullptr);

    AdapterView::OnItemSelectedListener selectedListener;
    selectedListener.onItemSelected = [this](AdapterView&, View&, int position, long) {
        positionSelected(position);
    };
    selectedListener.onNothingSelected = [this](AdapterView&) {
        nothingSelected();
    };
    mListView->setOnItemSelectedListener(selectedListener);

    mListView->setOnItemClickListener([this](AdapterView&, View&, int position, long) {
        positionClicked(position);
    });

    /* set the fading edge length proportionally to the screen height for test
       stability */
    if (params.mFadingEdgeScreenSizeFactor >= 0.0) {
        mListView->setFadingEdgeLength((int)(params.mFadingEdgeScreenSizeFactor * mScreenHeight));
    } else {
        mListView->setFadingEdgeLength((int)((64.0 / 480) * mScreenHeight));
    }

    if (mIncludeHeader) {
        mLinearLayout = new LinearLayout(&app);

        mHeaderTextView = new TextView(&app);
        mHeaderTextView->setText("hi");
        mHeaderTextView->setLayoutParams(new LinearLayout::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT,
                ViewGroup::LayoutParams::WRAP_CONTENT));
        mLinearLayout->addView(mHeaderTextView);

        mLinearLayout->setOrientation(LinearLayout::VERTICAL);
        mLinearLayout->setLayoutParams(new ViewGroup::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT,
                ViewGroup::LayoutParams::MATCH_PARENT));
        mListView->setLayoutParams(new LinearLayout::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT, 0, 1.0f));

        mLinearLayout->addView(mListView);
        mWindow->addView(mLinearLayout);
    } else {
        mLinearLayout = new LinearLayout(&app);
        mLinearLayout->setOrientation(LinearLayout::VERTICAL);
        mLinearLayout->setLayoutParams(new ViewGroup::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT,
                ViewGroup::LayoutParams::MATCH_PARENT));
        mListView->setLayoutParams(new LinearLayout::LayoutParams(
                ViewGroup::LayoutParams::MATCH_PARENT, 0, 1.0f));
        mLinearLayout->addView(mListView);
        mWindow->addView(mLinearLayout);
    }

    /* setContentView: explicit first traversal (View::layout takes l,t,w,h) */
    const int w = mWindow->getWidth() > 0 ? mWindow->getWidth() : 1080;
    mLinearLayout->measure(MeasureSpec::makeMeasureSpec(w, MeasureSpec::EXACTLY),
                           MeasureSpec::makeMeasureSpec(mScreenHeight, MeasureSpec::EXACTLY));
    mLinearLayout->layout(0, 0, w, mScreenHeight);
    pumpUntilIdle();

    mLinearLayout->restoreDefaultFocus();
}

void ListScenario::positionSelected(int position) {
}

void ListScenario::nothingSelected() {
}

void ListScenario::positionClicked(int position) {
    setClickedPosition(position);
}

void ListScenario::positionLongClicked(int position) {
    setLongClickedPosition(position);
}

ListView* ListScenario::createListView() {
    return new ListView(&App::getInstance());
}

void ListScenario::setAdapter(ListView* listView) {
    listView->setAdapter(new MyAdapter(this));
}

void ListScenario::enableLongPress() {
    mListView->setOnItemLongClickListener([this](AdapterView&, View&, int position, long) {
        positionLongClicked(position);
        return true;
    });
}

bool ListScenario::isItemAtPositionSelectable(int position) const {
    return mUnselectableItems.find(position) == mUnselectableItems.end();
}

void ListScenario::readAndValidateParams(Params& params) {
    if (params.mMustFillScreen) {
        double totalFactor = 0.0;
        for (int i = 0; i < params.mNumItems; i++) {
            auto it = params.mOverrideItemScreenSizeFactors.find(i);
            if (it != params.mOverrideItemScreenSizeFactors.end()) {
                totalFactor += it->second;
            } else {
                totalFactor += params.mItemScreenSizeFactor;
            }
        }
        ASSERT_GE(totalFactor, 1.0) << "list items must combine to be at least "
                "the height of the screen";
    }

    mNumItems = params.mNumItems;
    mItemsFocusable = params.mItemsFocusable;
    mStartingSelectionPosition = params.mStartingSelectionPosition;
    mItemScreenSizeFactor = params.mItemScreenSizeFactor;
    for (auto& it : params.mOverrideItemScreenSizeFactors) {
        mOverrideItemScreenSizeFactors[it.first] = it.second;
    }
    for (int pos : params.mUnselectableItems) mUnselectableItems.insert(pos);
    mIncludeHeader = params.mIncludeHeader;
    mStackFromBottom = params.mStackFromBottom;
    mHeaderViewCount = params.mHeaderViewCount;
    mHeadersFocusable = params.mHeaderFocusable;
    mFooterViewCount = params.mFooterViewCount;
}

const std::string ListScenario::getValueAtPosition(int position) const {
    return isItemAtPositionSelectable(position) ?
            "position " + std::to_string(position) :
            "------- " + std::to_string(position);
}

int ListScenario::getHeightForPosition(int position) const {
    int desiredHeight = (int)(mScreenHeight * mItemScreenSizeFactor);
    auto it = mOverrideItemScreenSizeFactors.find(position);
    if (it != mOverrideItemScreenSizeFactors.end()) {
        desiredHeight = (int)(mScreenHeight * it->second);
    }
    return desiredHeight;
}

View* ListScenario::createView(int position, ViewGroup* parent, int desiredHeight) {
    return ListItemFactory::text(position, parent->getContext(), getValueAtPosition(position), desiredHeight);
}

View* ListScenario::convertView(int position, View* convertView, ViewGroup*) {
    return ListItemFactory::convertText(convertView, getValueAtPosition(position), position);
}

int ListScenario::getItemViewType(int position) {
    return 0;
}

int ListScenario::getViewTypeCount() {
    return 1;
}

} // namespace keynav
