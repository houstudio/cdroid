/*********************************************************************************
 * Definitions for the cross-page helpers declared in printer_common.h.
 *********************************************************************************/
#include <core/app.h>
#include "printer_common.h"
#include <cdroid.h>
#include <navigation/navhostfragment.h>
#include <navigation/navcontroller.h>
#include <fragment/fragment.h>
#include <fragment/fragmentactivity.h>
#include <lifecycle/viewmodelprovider.h>
#include "printer_viewmodel.h"

std::string sLocaleTag = "zh-CN";

cdroid::NavController* navControllerOf(cdroid::fragment::Fragment* f){
    cdroid::NavHostFragment* host = dynamic_cast<cdroid::NavHostFragment*>(f->getParentFragment());
    return host ? host->getNavController() : nullptr;
}

// ---------------------------------------------------------------------------
// Device data model (androidx.lifecycle.ViewModel).
//   sharedPrinterVM() returns the single activity-scoped PrinterViewModel: HomeFragment
//   renders it, Copy/Scan/Maintain mutate it. Scoped to the FragmentActivity that
//   hosts the NavHostFragment, so the instance survives Fragment view destruction
//   (Home -> Copy -> back rebuilds Home's view and re-reads the updated counters/ink).
//   Mirrors ViewModelProvider(requireActivity()).get(PrinterViewModel::class.java).
// ---------------------------------------------------------------------------
printerdemo::PrinterViewModel* sharedPrinterVM(cdroid::fragment::Fragment* f){
    static printerdemo::PrinterViewModelFactory sFactory;
    auto* act = dynamic_cast<cdroid::fragment::FragmentActivity*>(f->getActivity());
    if(!act) return nullptr;
    cdroid::lifecycle::ViewModelProvider provider(&act->getViewModelStore(), &sFactory, nullptr);
    return provider.get<printerdemo::PrinterViewModel>("PrinterViewModel");
}

void applyLocale(const std::string& tag){
    if(tag == sLocaleTag) return;
    sLocaleTag = tag;
    cdroid::Configuration c = cdroid::App::getInstance().getResources().getConfiguration();
    c.setLocales(cdroid::LocaleList(std::vector<cdroid::Locale>{
            cdroid::Locale::forLanguageTag(tag)}));
    cdroid::App::getInstance().handleConfigurationChanged(c);
}
