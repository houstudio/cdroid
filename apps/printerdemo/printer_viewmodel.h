/*********************************************************************************
 * Activity-scoped data model for the MFP demo. Port of androidx.lifecycle.ViewModel
 * (viewModelScope / LiveData are not ported; Fragments re-read on view creation,
 * which is enough for nav-driven updates). Nothing in the layouts is hardcoded —
 * the ViewModel is the single source of the device telemetry.
 *********************************************************************************/
#ifndef __PRINTER_VIEWMODEL_H__
#define __PRINTER_VIEWMODEL_H__
#include <lifecycle/viewmodel.h>
#include <lifecycle/viewmodelprovider.h>
#include <string>
#include <vector>
#include <algorithm>

namespace printerdemo {

// One tank of the CMYK cartridge set.
struct InkCartridge {
    std::string label;   // "K" | "C" | "M" | "Y"
    int level;           // remaining level in tenths of a percent, 0..1000 (850 == 85.0%)
    int color;           // ARGB fill color
    int lowThreshold;    // below this percent the level is flagged "low"
    int percent() const { return (level + 5) / 10; } // rounded to whole percent
};

// A paper tray: sheets loaded vs. capacity.
struct PaperTray {
    std::string name;    // "纸盒 1 · A4"
    int capacity;        // sheets the tray holds
    int remaining;       // sheets currently loaded
    int percent() const { return capacity > 0 ? (remaining * 100 + capacity / 2) / capacity : 0; }
};

// Holds the live device telemetry (ink levels, paper, job counters, status) that
// every Fragment renders or mutates. Scoped to the FragmentActivity, so a single
// instance is shared across the NavHostFragment's sibling destinations.
class PrinterViewModel : public cdroid::lifecycle::ViewModel {
public:
    PrinterViewModel()
        : mInks({ {"K", 850, (int)0xFF2E3440, 20},
                  {"C", 600, (int)0xFF0288D1, 20},
                  {"M", 400, (int)0xFFD81B60, 25},
                  {"Y", 720, (int)0xFFF9A825, 20} })
        , mPaper({"纸盒 1 · A4", 250, 200})
        , mStatus("就绪")
        , mNetwork("Wi-Fi · CDROID-Net · 192.168.1.50") {}

    const std::vector<InkCartridge>& getInks() const { return mInks; }
    const PaperTray& getPaper() const { return mPaper; }
    int getTotalPrints() const { return mTotalPrints; }
    int getTotalScans() const { return mTotalScans; }
    int getTotalCopies() const { return mTotalCopies; }
    int getTotalMaintenance() const { return mTotalMaintenance; }
    const std::string& getStatus() const { return mStatus; }
    const std::string& getNetwork() const { return mNetwork; }

    // Each page/event drains 1% from the relevant tanks (INK_UNIT tenths = 1.0%).
    static constexpr int INK_UNIT = 10;

    // A copy job: bumps the copy/print counters by `copies`, feeds that many sheets,
    // and drains 1% per job (K always; C/M/Y too when color). Clamped at 0.
    void recordCopy(int copies, bool color) {
        if (copies <= 0) return;
        mTotalCopies += copies;
        mTotalPrints += copies;
        mPaper.remaining = std::max(0, mPaper.remaining - copies);
        for (auto& ink : mInks) {
            if (ink.label == "K" || color)
                ink.level = std::max(0, ink.level - INK_UNIT); // 1% per copy job
        }
    }
    // A scan: bumps the scan counter and drains 1% from every tank.
    void recordScan() {
        mTotalScans++;
        for (auto& ink : mInks) ink.level = std::max(0, ink.level - INK_UNIT);
    }
    // A maintenance cycle (clean / nozzle check / alignment / self-test).
    void recordMaintenance() { mTotalMaintenance++; }

private:

    std::vector<InkCartridge> mInks;
    PaperTray mPaper;
    // Lifetime page/event counters as read off a working machine's service menu.
    int mTotalPrints = 12480;
    int mTotalScans = 3206;
    int mTotalCopies = 5012;
    int mTotalMaintenance = 36;
    std::string mStatus;
    std::string mNetwork;
};

// C++ has no reflection, so the caller supplies the class-name -> instance map
// (see ViewModelProvider::Factory). Only PrinterViewModel is needed here.
class PrinterViewModelFactory : public cdroid::lifecycle::ViewModelProvider::Factory {
public:
    cdroid::lifecycle::ViewModel* create(const std::string& modelClass,
                                         cdroid::lifecycle::CreationExtras&) override {
        if (modelClass == "PrinterViewModel") return new PrinterViewModel();
        return nullptr;
    }
};

} // namespace printerdemo
#endif
