/*
 * Espresso demo — the CDROID host app that loads the in-process Espresso
 * runner. This stands in for the AOSP flow where `am instrument` injects the
 * test APK into the app process and JUnit drives the tests on the main
 * thread: here REGISTER_ESPRESSO_TEST registers the tests at static init,
 * and the runner is posted on the main looper once the first frame is up.
 */

#include <cdroid.h>
#include <cdlog.h>

#include <core/handler.h>
#include <core/looper.h>
#include <widget/adapter.h>
#include <widget/adapterview.h>
#include <widget/button.h>
#include <widget/checkbox.h>
#include <widget/edittext.h>
#include <widget/linearlayout.h>
#include <widget/listview.h>
#include <widget/textview.h>

#include <app/espresso/espresso.h>
#include <app/espresso/viewinteraction.h>
#include <app/espresso/espressotests.h>
#include <app/espresso/rootmatchers.h>
#include <app/espresso/viewactions.h>
#include <app/espresso/viewassertions.h>
#include <app/espresso/viewmatchers.h>
#include <app/espresso/datainteraction.h>

using namespace cdroid;
using namespace cdroid::espresso;

/* Java static-import ergonomics: onView(...) instead of Espresso::onView(...). */
static ViewInteraction onView(MatcherPtr<View> viewMatcher) {
    return Espresso::onView(std::move(viewMatcher));
}

/* ... and onData(...) instead of Espresso::onData(...). */
static DataInteraction onData(MatcherPtr<void*> dataMatcher) {
    return Espresso::onData(std::move(dataMatcher));
}

/* Test-local view ids (an app would use its R ids). */
static constexpr int ID_TITLE  = 0x00010001;
static constexpr int ID_STATUS = 0x00010002;
static constexpr int ID_BUTTON = 0x00010003;
static constexpr int ID_EDIT   = 0x00010004;
static constexpr int ID_CHECK  = 0x00010005;
static constexpr int ID_LIST   = 0x00010006;

static TextView* gStatus = nullptr;

/* ------------------------------------------------------------------ */
/* The ListView demo page the onData tests exercise.                   */
/* ------------------------------------------------------------------ */

/* A minimal string adapter (an app would use ArrayAdapter<std::string> or
 * its own model adapter): rows are plain TextViews, data items are the
 * std::strings themselves — Adapter::getItem hands them out as void*. */
class StringListAdapter : public Adapter {
public:
    explicit StringListAdapter(Context* context) : mContext(context) {
        for (int i = 0; i < ITEM_COUNT; i++) {
            mItems.push_back("item " + std::to_string(i));
        }
    }

    int getCount() const override { return (int)mItems.size(); }
    void* getItem(int position) const override { return (void*)&mItems[position]; }
    long getItemId(int position) const override { return position; }
    bool hasStableIds() const override { return true; }

    View* getView(int position, View* convertView, ViewGroup* parent) override {
        // Rows are a LinearLayout with an inner TextView, so the demo has a
        // real "subview within the list item" for onChildView().
        LinearLayout* row = dynamic_cast<LinearLayout*>(convertView);
        if (row == nullptr) {
            row = new LinearLayout(mContext);
            row->setOrientation(LinearLayout::VERTICAL);
            row->setLayoutParams(new AbsListView::LayoutParams(
                    ViewGroup::LayoutParams::MATCH_PARENT, 90));
            auto* text = new TextView(mContext);
            text->setTextSize(24);
            text->setPadding(24, 0, 24, 0);
            row->addView(text, new LinearLayout::LayoutParams(
                    LayoutParams::MATCH_PARENT, LayoutParams::WRAP_CONTENT));
        }
        ((TextView*)row->getChildAt(0))->setText(mItems[position]);
        return row;
    }

private:
    static constexpr int ITEM_COUNT = 40;
    Context* mContext;
    std::vector<std::string> mItems;
};

/* The onData-side data matcher: dereferences the adapter's void* item back
 * to the std::string it points at (AOSP samples match the model object). */
class IsItemText : public TypeSafeMatcher<void*> {
public:
    explicit IsItemText(std::string expected) : mExpected(std::move(expected)) {}
    bool matchesSafely(void* const& item) const override {
        const std::string* text = static_cast<const std::string*>(item);
        return text != nullptr && *text == mExpected;
    }
    void describeTo(Description& description) const override {
        description.appendText("item with text ").appendValue(mExpected);
    }
private:
    std::string mExpected;
};

static MatcherPtr<void*> withItemText(const std::string& text) {
    return std::make_shared<IsItemText>(text);
}

/* ------------------------------------------------------------------ */
/* The tests — plain functions using the Espresso API verbatim.        */
/* ------------------------------------------------------------------ */

static void testButtonIsDisplayed() {
    onView(ViewMatchers::withText("Click Me"))
            .check(ViewAssertions::matches(ViewMatchers::isDisplayed()))
            .check(ViewAssertions::matches(ViewMatchers::isEnabled()));
}
REGISTER_ESPRESSO_TEST("button is displayed and enabled", testButtonIsDisplayed);

static void testClickTogglesStatus() {
    onView(ViewMatchers::withText("Click Me")).perform({ViewActions::click()});
    onView(ViewMatchers::withId(ID_STATUS))
            .check(ViewAssertions::matches(ViewMatchers::withText("Clicked!")));
}
REGISTER_ESPRESSO_TEST("click toggles status", testClickTogglesStatus);

static void testTypeTextIntoEdit() {
    onView(ViewMatchers::withId(ID_EDIT)).perform({ViewActions::click(), ViewActions::typeText("hello")});
    onView(ViewMatchers::withId(ID_EDIT))
            .check(ViewAssertions::matches(
                    ViewMatchers::withText(containsString("hello"))));
}
REGISTER_ESPRESSO_TEST("type text into edit", testTypeTextIntoEdit);

static void testCheckBoxToggles() {
    onView(ViewMatchers::withText("Check me")).perform({ViewActions::click()});
    onView(ViewMatchers::withId(ID_CHECK))
            .check(ViewAssertions::matches(ViewMatchers::isChecked()));
}
REGISTER_ESPRESSO_TEST("checkbox toggles", testCheckBoxToggles);

static void testMissingViewDoesNotExist() {
    onView(ViewMatchers::withText("Not part of this demo"))
            .check(ViewAssertions::doesNotExist());
}
REGISTER_ESPRESSO_TEST("missing view does not exist", testMissingViewDoesNotExist);

static void testOnDataScrollsItemIntoView() {
    onData(withItemText("item 38"))
            .inAdapterView(ViewMatchers::withId(ID_LIST))
            .check(ViewAssertions::matches(ViewMatchers::isDisplayed()));
}
REGISTER_ESPRESSO_TEST("onData scrolls item into view", testOnDataScrollsItemIntoView);

static void testOnDataClickItem() {
    onData(withItemText("item 7")).perform({ViewActions::click()});
    onView(ViewMatchers::withId(ID_STATUS))
            .check(ViewAssertions::matches(ViewMatchers::withText("item 7")));
}
REGISTER_ESPRESSO_TEST("onData click item", testOnDataClickItem);

static void testOnDataOnChildView() {
    onData(withItemText("item 39"))
            .onChildView(ViewMatchers::withText(containsString("39")))
            .check(ViewAssertions::matches(ViewMatchers::isDisplayed()));
}
REGISTER_ESPRESSO_TEST("onData on child view", testOnDataOnChildView);

/* ------------------------------------------------------------------ */

int main(int argc, const char* argv[]) {
    App app(argc, argv);

    Window* window = new Window(0, 0, -1, -1);
    // Opaque window background (the samples' pattern): without one the window
    // is composited over whatever the primary surface held before.
    window->setBackgroundColor(0xFFFFFFFF);

    LinearLayout* layout = new LinearLayout(&App::getInstance());
    layout->setOrientation(LinearLayout::VERTICAL);
    window->addView(layout);

    // One LayoutParams per child: addView adopts the params into the view,
    // and each View deletes its own at teardown (a single shared pointer
    // here would be a five-way double-free — Java samples share one object
    // only because the GC tolerates aliasing).
    auto match = [] {
        return new LinearLayout::LayoutParams(
                LayoutParams::MATCH_PARENT, LayoutParams::WRAP_CONTENT);
    };

    auto* title = new TextView(&App::getInstance());
    title->setText("Espresso Demo");
    title->setTextSize(28);
    title->setId(ID_TITLE);
    layout->addView(title, match());

    gStatus = new TextView(&App::getInstance());
    gStatus->setText("Not clicked");
    gStatus->setId(ID_STATUS);
    gStatus->setTextSize(20);
    layout->addView(gStatus, match());

    auto* button = new Button(&App::getInstance());
    button->setText("Click Me");
    button->setId(ID_BUTTON);
    button->setOnClickListener([](View&) { gStatus->setText("Clicked!"); });
    layout->addView(button, match());

    auto* edit = new EditText(&App::getInstance());
    edit->setHint("Type here");
    edit->setId(ID_EDIT);
    layout->addView(edit, match());

    auto* check = new CheckBox(&App::getInstance());
    check->setText("Check me");
    check->setId(ID_CHECK);
    layout->addView(check, match());

    // The AdapterView page the onData tests exercise: 40 rows of 90px each,
    // so "item 38/39" only materialize once Espresso scrolls them in.
    static StringListAdapter listAdapter(&App::getInstance());
    auto* list = new ListView(&App::getInstance());
    list->setId(ID_LIST);
    list->setAdapter(&listAdapter);
    list->setOnItemClickListener([](AdapterView& /*parent*/, View& /*view*/,
                                    int position, long /*id*/) {
        gStatus->setText("item " + std::to_string(position));
    });
    layout->addView(list, new LinearLayout::LayoutParams(
            LayoutParams::MATCH_PARENT, LayoutParams::MATCH_PARENT));

    // The "test runner": once the first frame is up, drive the registered
    // tests on the main thread (AOSP: the instrumentation thread posts the
    // JUnit run into the app's main looper). The Handler must outlive the
    // post — a temporary drops the message before it is dispatched.
    static Handler runner(Looper::getMainLooper());
    runner.postDelayed([] { EspressoTestRegistry::getInstance().runAll(); }, 800);

    return app.exec();
}
