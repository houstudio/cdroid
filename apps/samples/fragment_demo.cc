/*********************************************************************************
 * Fragment MVP demo: FragmentWindow hosts a Fragment whose onCreateView builds
 * a TextView. Verifies Fragment attach/createView/lifecycle + transaction replace
 * end-to-end. Back-stack pop is exercised via FragmentWindow.onBackPressed (BACK).
 *********************************************************************************/
#include <cdroid.h>
#include <widget/textview.h>
#include <fragment/fragment.h>
#include <fragment/fragmentwindow.h>
#include <fragment/fragmentmanager.h>
#include <fragment/fragmenttransaction.h>

class AFragment : public cdroid::fragment::Fragment {
public:
    cdroid::View* onCreateView(cdroid::LayoutInflater* inflater, cdroid::ViewGroup* container,
                               cdroid::Bundle* savedInstanceState) override {
        cdroid::TextView* tv = new cdroid::TextView("Fragment A\n\nFragment MVP works!", 800, 600);
        tv->setBackgroundColor(0xFF102030);
        return tv;
    }
};

class BFragment : public cdroid::fragment::Fragment {
public:
    cdroid::View* onCreateView(cdroid::LayoutInflater* inflater, cdroid::ViewGroup* container,
                               cdroid::Bundle* savedInstanceState) override {
        cdroid::TextView* tv = new cdroid::TextView("Fragment B (BACK pops to A)", 800, 600);
        tv->setBackgroundColor(0xFF301020);
        return tv;
    }
};

class DemoWindow : public cdroid::fragment::FragmentWindow {
public:
    DemoWindow() : FragmentWindow(0, 0, -1, -1) {
        getSupportFragmentManager()->beginTransaction()
            ->replace(getFragmentContainerId(), new AFragment())
            .commit();
    }
    // Any non-BACK key swaps to B (added to back stack); BACK is handled by
    // FragmentWindow::onBackPressed -> popBackStackImmediate (returns to A).
    bool onKeyDown(int keyCode, cdroid::KeyEvent& evt) override {
        getSupportFragmentManager()->beginTransaction()
            ->replace(getFragmentContainerId(), new BFragment())
            .addToBackStack("B")
            .commit();
        return true;
    }
};

int main(int argc, const char* argv[]) {
    cdroid::App app(argc, argv);
    new DemoWindow();
    return app.exec();
}
