// Port of SupportWearDemos AlertDialogDemo. Upstream builds one androidx
// (appcompat v7) AlertDialog and one framework android.app.AlertDialog; CDROID
// carries a single fused AlertDialog port, so both buttons build through the
// same Builder, keeping the two upstream titles. The upstream
// setPositiveButtonIcon(app_sample_code drawable) is dropped — the CDROID
// Builder has no icon variant.
#include <core/app.h>
#include <cdroid.h>
#include <core/activityfactory.h>
#include <widget/button.h>
#include <app/alertdialog.h>
#include "R.h"

using namespace cdroid;

class AlertDialogDemo : public Window {
private:
    // Upstream keeps both built dialogs alive for repeated show()s (GC holds
    // them); here they are members owned by the window.
    AlertDialog* mV7Dialog;
    AlertDialog* mFrameworkDialog;

public:
    AlertDialogDemo() : Window(&App::getInstance(), 0, 0, -1, -1) {
        ViewGroup* root = (ViewGroup*)LayoutInflater::from(getContext())
                ->inflate(weardemos::R::layout::alert_dialog_demo, this, false);
        addView(root);

        mV7Dialog = createDialog("AppCompatDialog");
        mFrameworkDialog = createDialog("FrameworkDialog");

        Button* v7Trigger = (Button*)root->findViewById(weardemos::R::id::v7_dialog_button);
        v7Trigger->setOnClickListener([this](View&) { mV7Dialog->show(); });

        Button* frameworkTrigger = (Button*)root->findViewById(
                weardemos::R::id::framework_dialog_button);
        frameworkTrigger->setOnClickListener([this](View&) { mFrameworkDialog->show(); });
    }
    ~AlertDialogDemo() override {
        delete mV7Dialog;
        delete mFrameworkDialog;
    }
private:
    AlertDialog* createDialog(const std::string& title) {
        return AlertDialog::Builder(getContext())
                .setTitle(title)
                .setMessage("Lorem ipsum dolor...")
                .setPositiveButton("Ok", nullptr)
                .setNegativeButton("Cancel", nullptr)
                .create();
    }
};
REGISTER_ACTIVITY(AlertDialogDemo);
