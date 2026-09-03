/*********************************************************************************
 * Cross-page plumbing shared by the printerdemo Fragments and the host activity:
 *   navControllerOf   a Fragment -> its hosting NavHostFragment's NavController
 *   sharedPrinterVM   the single activity-scoped PrinterViewModel
 *   sLocaleTag        persisted locale choice — single source of truth, written by
 *                     the settings language picker and the toolbar quick toggle
 *   applyLocale       AOSP-style locale switch (Configuration change + recreate)
 *********************************************************************************/
#ifndef __PRINTER_COMMON_H__
#define __PRINTER_COMMON_H__
#include <string>
#include "printer_viewmodel.h"

namespace cdroid {
class NavController;
class Fragment;
} // namespace cdroid

cdroid::NavController* navControllerOf(cdroid::Fragment* f);

printerdemo::PrinterViewModel* sharedPrinterVM(cdroid::Fragment* f);

// Persisted locale choice (zh-CN default, matching the pre-locale-switch UI).
extern std::string sLocaleTag;

// AOSP locale switch: route a locale Configuration change through the "system"
// — the arsc language/region gets repacked (values-<locale> variants reselect)
// and the activity recreates under the new locale (Window::recreate posts the
// teardown, so this is safe from inside any menu/click dispatch).
void applyLocale(const std::string& tag);

#endif
