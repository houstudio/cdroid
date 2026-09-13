/*********************************************************************************
 * Android-Settings-style demo for the androidx.preference port.
 *
 * One PreferenceFragment (SettingsFragment) serves every screen: the root
 * hierarchy comes from assets/xml/settings_root.xml, each entry navigates to
 * a nested screen via OnPreferenceStartScreenCallback (FragmentTransaction
 * replace + back stack, window title follows the screen). Covers switch/
 * checkbox groups, dependency disabling, dialog preferences (EditText/List/
 * MultiSelect), the expand button, and a confirm dialog on "Factory reset".
 *
 * Run with DISPLAY pointing at an X server (export DISPLAY="localhost:13.0").
 *********************************************************************************/
#include <cdroid.h>
#include <R.h>
#include <memory>
#include <preference/preferencefragment.h>
#include <preference/preferencescreen.h>
#include <preference/preferencecategory.h>
#include <preference/preference.h>
#include <preference/checkboxpreference.h>
#include <preference/switchpreference.h>
#include <preference/edittextpreference.h>
#include <preference/listpreference.h>
#include <preference/multiselectlistpreference.h>
#include <fragment/fragmentactivity.h>
#include <fragment/fragmentmanager.h>
#include <fragment/fragmenttransaction.h>
#include <transition/slide.h>
#include <app/alertdialog.h>
#include <app/dialoginterface.h>
#include <widget/toast.h>
#include <widget/scrollview.h>
#include <connectivitymanager.h>          // cdnet: aggregation + DhcpInfo views
#include <ethernet/ethernetmanager.h>
#include <dhcpinfo.h>
#include <ipconfiguration.h>
#include <wifi/wifimanager.h>
#include <wifi/scanresult.h>
#include <wifi/wifiinfo.h>
#include <wifi/wificonfiguration.h>
#include <bluetoothadapter.h>            // cdblue: connected devices screen
#include <bluetoothdevice.h>
#include <bluetoothpairing.h>
#include "bluetooth/bluetoothdevicepreference.h"
#include "bluetooth/bluetoothprogresscategory.h"
#include "bluetooth/devicelistpreferencefragment.h"
#include "bluetooth/localbluetoothmanager.h"
#include <map>
#include <cstdlib>
#include <widget/linearlayout.h>
#include <widget/textview.h>
#include <porting/cdlog.h>
#include <widget/internal_R.h>
#include <content/typedvalue.h>

using cdroid::PreferenceFragment;
using cdroid::PreferenceScreen;
using cdroid::FragmentActivity;

namespace {

/** Resolve a theme color (fallback = the hardcoded Material Light value).
 *  Attrs like textColorPrimary resolve to a ColorStateList reference — the
 *  resourceId must be loaded (->getDefaultColor), not used as a raw ARGB. */
int themeColor(cdroid::Context& ctx, int attr, int fallback) {
    cdroid::TypedValue v;
    if (!ctx.getTheme().resolveAttribute(attr, &v, true)) {
        return fallback;
    }
    if (v.resourceId != 0) {
        auto csl = ctx.getColorStateList((int)v.resourceId);
        if (csl != nullptr) return csl->getDefaultColor();
    }
    return v.data;
}

/** Nested-screen key -> preference XML resource (both under assets/xml/). */
int screenXmlFor(const std::string& key) {
    if (key == "screen_network")          return (int)preferencedemo::R::xml::settings_network;
    if (key == "screen_connected")        return (int)preferencedemo::R::xml::settings_connected;
    if (key == "screen_bluetooth_pairing") return (int)preferencedemo::R::xml::settings_bluetooth_pairing;
    if (key == "screen_apps")             return (int)preferencedemo::R::xml::settings_apps;
    if (key == "screen_notifications")    return (int)preferencedemo::R::xml::settings_notifications;
    if (key == "screen_battery")          return (int)preferencedemo::R::xml::settings_battery;
    if (key == "screen_storage")          return (int)preferencedemo::R::xml::settings_storage;
    if (key == "screen_sound")            return (int)preferencedemo::R::xml::settings_sound;
    if (key == "screen_display")          return (int)preferencedemo::R::xml::settings_display;
    if (key == "screen_accessibility")    return (int)preferencedemo::R::xml::settings_accessibility;
    if (key == "screen_security")         return (int)preferencedemo::R::xml::settings_security;
    if (key == "screen_location")         return (int)preferencedemo::R::xml::settings_location;
    if (key == "screen_privacy")          return (int)preferencedemo::R::xml::settings_privacy;
    if (key == "screen_system")           return (int)preferencedemo::R::xml::settings_system;
    if (key == "screen_about")            return (int)preferencedemo::R::xml::settings_about;
    return 0;
}

} // namespace

class SettingsActivity;

class SettingsFragment : public PreferenceFragment,
                          public cdroid::WifiManager::NetworkStateListener,
                          public preferencedemo::BluetoothCallback,
                          public preferencedemo::DeviceListPreferenceFragment::Host,
                          public cdroid::BluetoothPairingListener {
public:
    void onCreatePreferences(cdroid::Bundle* /*savedInstanceState*/,
            const std::string& rootKey) override {
        mRootKey = rootKey;
        // Pushed screens drive their motion with fragment Transitions (SEC
        // Priority 1): one scene change on the container, so the enter/exit
        // pair stays in lock-step. The legacy custom-Animation path remains
        // selectable (PREFDEMO_ANIM=1 in openScreen) but runs the two ops as
        // independent Animations whose handoff shows a seam. Slide edges mirror
        // the old R.anim slides: enter from right / exit to left; pop returns
        // to right, reenter comes back from left. The root keeps no transition,
        // so the initial show is static and covering it fades (SEC default).
        if (!rootKey.empty()) {
            setEnterTransition(new cdroid::Slide(cdroid::Gravity::RIGHT));
            setExitTransition(new cdroid::Slide(cdroid::Gravity::LEFT));
            setReenterTransition(new cdroid::Slide(cdroid::Gravity::LEFT));
            setReturnTransition(new cdroid::Slide(cdroid::Gravity::RIGHT));
        }
        // Nested screens load their own XML by key; the root loads everything.
        const int xml = rootKey.empty() ? (int)preferencedemo::R::xml::settings_root
                                        : screenXmlFor(rootKey);
        setPreferencesFromResource(xml, std::string());

        applySimpleSummaryProviders(getPreferenceScreen());
        watchTwoStateToggles(getPreferenceScreen());

        // Accessibility screen: collapse "Interaction controls" behind the
        // expand button (initialExpandedChildrenCount is a library-private
        // attr, so it is set from code rather than XML).
        if (getPreferenceScreen() != nullptr) {
            LOGD("[settings] loaded root=%s items=%d", rootKey.empty() ? "<root>" : rootKey.c_str(),
                 getPreferenceScreen()->getPreferenceCount());
        }
        if (rootKey == "screen_network") setupNetworkScreen();
        if (rootKey == "screen_connected") setupConnectedScreen();
        if (rootKey == "screen_bluetooth_pairing") setupBluetoothPairingScreen();

        if (rootKey == "screen_accessibility") {
            auto* cat = dynamic_cast<cdroid::PreferenceGroup*>(
                    findPreference("category_accessibility_interaction"));
            if (cat != nullptr) {
                cat->setInitialExpandedChildrenCount(3);
            }
        }
        // Valgrind/CI driver: PREFDEMO_AUTOCYCLE clicks through every root
        // preference (sub-screens get dwell + Back via the virtual
        // Window::onBackPressed), then exits cleanly so the leak report lands.
        // Only the ROOT screen drives — pushed sub-screen fragments would
        // otherwise start a nested cycle of their own.
        if (rootKey.empty() && getenv("PREFDEMO_AUTOCYCLE") != nullptr) {
            // File-local handler (mHandler is private); generous delays —
            // this runs under valgrind.
            static cdroid::Handler sCycleHandler(cdroid::Looper::getMainLooper());
            sCycleHandler.postDelayed([this]() { cycleStep(0); }, 4000);
        }
    }

    void onDestroy() override {
        *mNetAlive = false;
        cdroid::WifiManager::getInstance().removeNetworkStateListener(this);
        preferencedemo::LocalBluetoothManager::getInstance()
                ->getEventManager()->unregisterCallback(this);
        *mBtAlive = false;
        if (mDeviceList != nullptr) mDeviceList->onStop();
        cdroid::BluetoothAdapter::getDefaultAdapter().removePairingListener(this);
        PreferenceFragment::onDestroy();
    }

    // --- Network & internet screen (cdnet: WifiManager + EthernetManager) --
    // Everything below runs on the main thread except the WifiManager
    // callback, which marshals through a file-local main-looper Handler.
    void setupNetworkScreen();
    void refreshWifiStatus();
    void refreshIpSummary();
    void showWifiPicker();
    void showNetworkDetails();
    void buildEthernetSection();
    static std::string ipToString(uint32_t ip);
    static std::string networkDetailsText();
    /* Heap-stable lifetime flag: the main-looper lambda captures the
     * shared_ptr by value, so a post queued on the monitor thread that runs
     * after the fragment is destroyed reads *mNetAlive==false and returns
     * without touching the freed fragment. */
    std::shared_ptr<bool> mNetAlive = std::make_shared<bool>(false);

    // --- Bluetooth screens (AOSP Settings bluetooth, see bluetooth/) ------
    void setupConnectedScreen();          // BluetoothSettings-shaped
    void setupBluetoothPairingScreen();   // BluetoothPairingDetail-shaped
    void refreshBluetoothStatus();        // device-name row + footer MAC
    void showDeviceRenameDialog();
    std::shared_ptr<bool> mBtAlive = std::make_shared<bool>(false);
    std::unique_ptr<preferencedemo::DeviceListPreferenceFragment> mDeviceList;
    bool mBtPairingAgentRegistered = false;

    // WifiManager::NetworkStateListener (monitor thread).
    void onNetworkStateChanged(const cdroid::WifiInfo&) override;

    // preferencedemo::BluetoothCallback (main thread; BluetoothEventManager).
    void onBluetoothStateChanged(int bluetoothState) override;
    void onScanningStateChanged(bool started) override;
    // preferencedemo::DeviceListPreferenceFragment::Host.
    cdroid::Preference* findPreference(const std::string& key) override {
        return PreferenceFragment::findPreference(key);
    }
    cdroid::Context* prefContext() override { return requireContext(); }
    // BluetoothPairingListener (agent thread).
    void onPairingRequest(const cdroid::BluetoothDevice& device,
                          int pairingVariant, uint32_t passkey) override;
    void onDisplayPasskey(const cdroid::BluetoothDevice& device,
                          uint32_t passkey, int pairedDuration) override;
    void onPairingCancelled(const cdroid::BluetoothDevice& device) override;

    /** AUTOCYCLE walker (see onCreatePreferences). Each step is a fresh
     *  lambda capturing values only — no self-referencing runnable. */
    void cycleStep(int index);

    /** PreferenceFragment layout wrapped with the settings chrome (the AOSP
     *  Settings screen title — CDROID windows have no title API). The chrome
     *  inflates from R::layout::prefdemo_settings: layout/ is header-above-
     *  list, the layout-land/ twin is a title column beside the list — same
     *  ids and view types, so this one wiring path serves both. */
    cdroid::View* onCreateView(cdroid::LayoutInflater* inflater, cdroid::ViewGroup* container,
            cdroid::Bundle* savedInstanceState) override {
        cdroid::View* content = PreferenceFragment::onCreateView(inflater, container,
                savedInstanceState);
        auto* root = dynamic_cast<cdroid::ViewGroup*>(inflater->inflate(
                (int)preferencedemo::R::layout::prefdemo_settings, container, false));
        // The window surface is transparent — the chrome must carry fully
        // opaque backgrounds or the preference list floats over the desktop.
        // Both colors follow the live theme (?android:attr/colorBackground /
        // textColorPrimary), so switching Material <-> Material.Light restyles
        // the chrome with no per-theme constants.
        const int bg = themeColor(*requireContext(),
                (int)internal::R::attr::colorBackground, 0xFFF8F9FA);
        const int fg = themeColor(*requireContext(),
                (int)internal::R::attr::textColorPrimary, 0xFF1B1B1F);
        if (root != nullptr) root->setBackgroundColor(bg);
        if (auto* header = root ? root->findViewById((int)preferencedemo::R::id::prefdemo_header)
                                : nullptr)
            header->setBackgroundColor(bg);
        const std::string title = getPreferenceScreen() && !getPreferenceScreen()->getTitle().empty()
                ? getPreferenceScreen()->getTitle() : std::string("Settings");
        auto* titleView = dynamic_cast<cdroid::TextView*>(
                root ? root->findViewById((int)preferencedemo::R::id::prefdemo_title) : nullptr);
        if (titleView != nullptr) {
            titleView->setText(title);
            titleView->setTextColor(fg);
        }
        // Up affordance (the AOSP Settings app bar arrow): present in the XML,
        // visible only on portrait nested screens (push navigation). Landscape
        // runs two-pane — selecting on the left replaces the right pane, there
        // is no parent to pop; independent of keyboard BACK either way.
        auto* up = dynamic_cast<cdroid::TextView*>(
                root ? root->findViewById((int)preferencedemo::R::id::prefdemo_back) : nullptr);
        const bool portrait = requireContext()->getResources().getConfiguration().orientation
                != cdroid::Configuration::ORIENTATION_LANDSCAPE;
        if (up != nullptr) {
            up->setText(u8"←");
            up->setTextColor(fg);
            up->setClickable(true);
            up->setVisibility((!mRootKey.empty() && portrait)
                    ? cdroid::View::VISIBLE : cdroid::View::GONE);
            up->setOnClickListener([this](cdroid::View&) { requestGoBack(); });
        }
        auto* host = dynamic_cast<cdroid::ViewGroup*>(
                root ? root->findViewById((int)preferencedemo::R::id::prefdemo_content) : nullptr);
        if (host != nullptr) {
            host->setBackgroundColor(bg);
            if (content != nullptr) {
                host->addView(content, new cdroid::LinearLayout::LayoutParams(
                        cdroid::LayoutParams::MATCH_PARENT,
                        cdroid::LayoutParams::MATCH_PARENT));
            }
        } else {
            LOGW("prefdemo_settings layout has no prefdemo_content host");
        }
        return root;
    }

    bool onPreferenceTreeClick(Preference& preference) override;

    // Top-level entries (plain Preferences, the androidx app:fragment
    // equivalent) navigate via the host; defined after SettingsActivity
    // (the host type must be complete for the cast). The second-level
    // screen is pushed onto the back stack so Back returns here.

private:
    std::string mRootKey;

    /** Pop via the host (defined after SettingsActivity — complete type). */
    void requestGoBack();

    bool handleTreeClick(Preference& preference) {
        if (preference.getKey() == "factory_reset") {
            confirmFactoryReset();
            return true;
        }
        if (preference.getKey() == "check_for_updates") {
            cdroid::Toast::makeText(requireContext(), "Already up to date",
                    cdroid::Toast::LENGTH_SHORT)->show();
            return true;
        }
        LOGD("[settings] tree click: %s", preference.getKey().c_str());
        return PreferenceFragment::onPreferenceTreeClick(preference);
    }

public:
    /** android:useSimpleSummaryProvider is a library-private attr — apply the
     *  AOSP SimpleSummaryProviders from code for every List/EditText screen. */
    void applySimpleSummaryProviders(cdroid::PreferenceGroup* group) {
        if (group == nullptr) return;
        for (int i = 0; i < group->getPreferenceCount(); i++) {
            Preference* p = group->getPreference(i);
            if (auto* lp = dynamic_cast<cdroid::ListPreference*>(p)) {
                lp->setSummaryProvider(cdroid::ListPreference::SimpleSummaryProvider());
            } else if (auto* ep = dynamic_cast<cdroid::EditTextPreference*>(p)) {
                ep->setSummaryProvider(cdroid::EditTextPreference::SimpleSummaryProvider());
            } else if (auto* nested = dynamic_cast<cdroid::PreferenceGroup*>(p)) {
                applySimpleSummaryProviders(nested);
            }
        }
    }

    /** Toast feedback on every two-state toggle (like the AOSP Settings QSTile
     *  hint), so value changes are visible without a summary re-read. */
    void watchTwoStateToggles(cdroid::PreferenceGroup* group) {
        if (group == nullptr) return;
        for (int i = 0; i < group->getPreferenceCount(); i++) {
            Preference* p = group->getPreference(i);
            if (auto* nested = dynamic_cast<cdroid::PreferenceGroup*>(p)) {
                watchTwoStateToggles(nested);
                continue;
            }
            auto* tsp = dynamic_cast<cdroid::TwoStatePreference*>(p);
            if (tsp == nullptr) continue;
            const std::string title = tsp->getTitle();
            tsp->setOnPreferenceChangeListener(
                [this, title](Preference&, const nonstd::any& newValue) {
                    const bool on = nonstd::any_cast<bool>(newValue);
                    cdroid::Toast::makeText(requireContext(),
                            title + (on ? ": on" : ": off"),
                            cdroid::Toast::LENGTH_SHORT)->show();
                    return true;
                });
        }
    }

    void confirmFactoryReset() {
        auto* b = new cdroid::AlertDialog::Builder(requireContext());
        b->setTitle("Erase all data?")
         .setMessage("This will reset all settings on this device. This action cannot be undone.")
         .setPositiveButton("Erase all data",
            [this](cdroid::Dialog&, int) {
                cdroid::Toast::makeText(requireContext(), "Reset (demo)",
                        cdroid::Toast::LENGTH_SHORT)->show();
            })
         .setNegativeButton("Cancel", nullptr);
        b->create()->show();
    }
};

// --- Network & internet screen (cdnet) --------------------------------------

std::string SettingsFragment::ipToString(uint32_t ip) {
    if (!ip) return std::string();
    // WifiInfo's int is little-endian packed (AOSP): first octet = low byte.
    return std::to_string(ip & 0xff) + "." + std::to_string((ip >> 8) & 0xff)
         + "." + std::to_string((ip >> 16) & 0xff) + "." + std::to_string((ip >> 24) & 0xff);
}

std::string SettingsFragment::networkDetailsText() {
    std::string text;
    cdroid::NetworkInfo ni =
        cdroid::ConnectivityManager::getInstance().getActiveNetworkInfo();
    text += "活动网络: " + (ni.isConnected() ? ni.getTypeName() + " 已连接"
                                             : ni.getTypeName() + " 未连接");
    if (!ni.getExtraInfo().empty()) text += "  (" + ni.getExtraInfo() + ")";
    text += "\n";

    cdroid::WifiManager& wifi = cdroid::WifiManager::getInstance();
    text += std::string("\nWi-Fi  ") + (wifi.isWifiEnabled() ? "开" : "关");
    if (wifi.isWifiEnabled()) {
        cdroid::WifiInfo info = wifi.getConnectionInfo();
        const std::string ip = ipToString((uint32_t)info.getIpAddress());
        const std::string ssid = info.getSSID();
        text += std::string("\n  IP ") + (ip.empty() ? "未获取" : ip)
              + "   信号 " + std::to_string(info.getRssi()) + " dBm"
              + "\n  BSSID " + info.getBSSID()
              + "   MAC " + info.getMacAddress();
        if (!ssid.empty()) text += "\n  SSID " + ssid;
    }

    cdroid::EthernetManager& em = cdroid::EthernetManager::getInstance();
    const std::vector<std::string> ifaces = em.getAvailableInterfaces();
    text += "\n以太网";
    if (ifaces.empty()) {
        text += "\n  (无可用接口)";
    } else {
        for (const std::string& iface : ifaces) {
            const cdroid::DhcpInfo d = em.getDhcpInfo(iface);
            const std::string ip = cdroid::DhcpInfo::intToStr(d.ipAddress);
            text += "\n  " + iface + (em.isAvailable(iface) ? "  up" : "  down")
                  + "\n    IP " + (ip.empty() || ip == "0.0.0.0" ? "未获取" : ip)
                  + "  掩码 " + cdroid::DhcpInfo::intToStr(d.netmask)
                  + "  网关 " + cdroid::DhcpInfo::intToStr(d.gateway)
                  + "\n    DNS " + cdroid::DhcpInfo::intToStr(d.dns1)
                  + (d.dns2 ? (" / " + cdroid::DhcpInfo::intToStr(d.dns2)) : std::string())
                  + "  服务器 " + cdroid::DhcpInfo::intToStr(d.serverAddress)
                  + "  租约 " + std::to_string(d.leaseDuration) + "s";
        }
    }
    return text;
}

void SettingsFragment::onNetworkStateChanged(const cdroid::WifiInfo&) {
    // Supplicant monitor thread -> main looper (the preference UI lives there).
    if (!*mNetAlive) return;
    static cdroid::Handler sNetHandler(cdroid::Looper::getMainLooper());
    // Capture the lifetime token, not just this: a lambda already queued
    // when the fragment is destroyed must not dereference it.
    sNetHandler.post([this, alive = mNetAlive]{
        if (!*alive) return;
        refreshWifiStatus();
        refreshIpSummary();
    });
}

void SettingsFragment::setupNetworkScreen() {
    *mNetAlive = true;
    // Bluetooth tethering row (AOSP BluetoothTetherPreferenceController):
    // available only while the Bluetooth radio is on; the PAN profile is
    // not ported in cdblue yet, so the toggle answers with a notice.
    if (cdroid::Preference* tether = findPreference("bluetooth_tethering")) {
        const bool btOn = cdroid::BluetoothAdapter::getDefaultAdapter().isEnabled();
        tether->setEnabled(btOn);
        tether->setOnPreferenceChangeListener(
                [this](cdroid::Preference&, const nonstd::any&) -> bool {
            cdroid::Context* c = requireContext();
            if (c != nullptr) {
                cdroid::Toast::makeText(c, "蓝牙网络共享需要 PAN profile(尚未移植)",
                                        cdroid::Toast::LENGTH_SHORT)->show();
            }
            return false;   // reject until the profile layer lands
        });
    }
    // The transport binds the library default (SupplicantClient::
    // defaultCtrlPath: WPA_CTRL_PATH if set, else the system socket) and
    // starts the event pump; idempotent.
    cdroid::WifiManager& wifi = cdroid::WifiManager::getInstance();
    wifi.initialize();
    wifi.addNetworkStateListener(this);
    preferencedemo::LocalBluetoothManager::getInstance()
            ->getEventManager()->registerCallback(this);
    if (wifi.isWifiEnabled()) wifi.startScan();   // warm the scan cache

    if (auto* sw = dynamic_cast<cdroid::SwitchPreference*>(findPreference("wifi_enabled"))) {
        sw->setChecked(wifi.isWifiEnabled());
        sw->setOnPreferenceChangeListener(
                [this](cdroid::Preference&, const nonstd::any& newValue) {
            cdroid::WifiManager& wifi = cdroid::WifiManager::getInstance();
            const bool on = nonstd::any_cast<bool>(newValue);
            wifi.setWifiEnabled(on);
            /* The module-phase setWifiEnabled(true) only re-attaches the
             * supplicant client; an explicit DISCONNECT leaves the daemon
             * idle (wpa does not auto-reconnect), so drive the network
             * selection ourselves — the AOSP framework does this in
             * ClientModeImpl when the radio comes back up. */
            if (on) wifi.reconnect();
            refreshWifiStatus();
            refreshIpSummary();
            return true;
        });
    }
    if (cdroid::Preference* pick = findPreference("wifi_pick")) {
        pick->setOnPreferenceClickListener([this](cdroid::Preference&) {
            showWifiPicker();
            return true;
        });
    }
    if (cdroid::Preference* ip = findPreference("ip_address")) {
        ip->setOnPreferenceClickListener([this](cdroid::Preference&) {
            showNetworkDetails();
            return true;
        });
    }
    refreshWifiStatus();
    refreshIpSummary();
    buildEthernetSection();
}

void SettingsFragment::refreshWifiStatus() {
    cdroid::Preference* status = findPreference("wifi_status");
    if (status == nullptr) return;
    cdroid::WifiManager& wifi = cdroid::WifiManager::getInstance();
    if (!wifi.isWifiEnabled()) { status->setSummary("Off"); return; }
    cdroid::WifiInfo info = wifi.getConnectionInfo();
    const std::string ssid = info.getSSID();
    if (ssid.empty() || ssid == cdroid::WifiManager::UNKNOWN_SSID) {
        status->setSummary("未连接");
        return;
    }
    const std::string ip = ipToString((uint32_t)info.getIpAddress());
    status->setSummary(ssid + "  " + std::to_string(info.getRssi()) + " dBm  "
                       + (ip.empty() ? "IP 未获取" : ip));
}

void SettingsFragment::refreshIpSummary() {
    cdroid::Preference* p = findPreference("ip_address");
    if (p == nullptr) return;
    cdroid::NetworkInfo ni =
        cdroid::ConnectivityManager::getInstance().getActiveNetworkInfo();
    if (!ni.isConnected()) { p->setSummary("未连接"); return; }
    if (ni.getType() == cdroid::ConnectivityManager::TYPE_WIFI) {
        const std::string ip = ipToString((uint32_t)cdroid::WifiManager::getInstance()
                                              .getConnectionInfo().getIpAddress());
        p->setSummary("Wi-Fi · " + (ip.empty() ? "IP 未获取" : ip));
    } else if (ni.getType() == cdroid::ConnectivityManager::TYPE_ETHERNET) {
        std::string s = "以太网";
        cdroid::EthernetManager& em = cdroid::EthernetManager::getInstance();
        for (const std::string& iface : em.getAvailableInterfaces()) {
            const std::string ip =
                cdroid::DhcpInfo::intToStr(em.getDhcpInfo(iface).ipAddress);
            if (!ip.empty() && ip != "0.0.0.0") { s += " · " + ip; break; }
        }
        p->setSummary(s);
    } else {
        p->setSummary(ni.getTypeName());
    }
}

void SettingsFragment::showWifiPicker() {
    cdroid::Context* ctx = requireContext();
    if (ctx == nullptr) return;
    std::vector<cdroid::ScanResult> results =
        cdroid::WifiManager::getInstance().getScanResults();
    std::sort(results.begin(), results.end(),
              [](const cdroid::ScanResult& a, const cdroid::ScanResult& b) {
                  return a.level > b.level;
              });
    // Hidden APs (empty SSID) are out of scope for the trial.
    std::vector<cdroid::ScanResult> kept;
    for (const cdroid::ScanResult& r : results) if (!r.SSID.empty()) kept.push_back(r);
    if (kept.empty()) {
        // The supplicant's cache can be empty (fresh daemon, or the ctrl
        // connection just recovered) — fire a scan so the retry has data.
        cdroid::WifiManager::getInstance().startScan();
        cdroid::Toast::makeText(ctx, "无扫描结果,已触发扫描,请稍后重试",
                                cdroid::Toast::LENGTH_SHORT)->show();
        return;
    }

    // Rows built in code (the same shape the printerdemo Wi-Fi page uses):
    // one TextView per ScanResult, wired for the connect flow below.
    auto* list = new cdroid::LinearLayout(ctx);
    list->setOrientation(cdroid::LinearLayout::VERTICAL);
    cdroid::AlertDialog* dialog = cdroid::AlertDialog::Builder(ctx)
        .setTitle("选择网络")
        .setView(list)
        .setNegativeButton("取消", [](cdroid::DialogInterface&, int) {})
        .create();
    for (const cdroid::ScanResult& r : kept) {
        const bool secured = r.capabilities.find("WPA") != std::string::npos
                          || r.capabilities.find("WEP") != std::string::npos
                          || r.capabilities.find("SAE") != std::string::npos;
        auto* row = new cdroid::TextView(ctx);
        row->setText(r.SSID + (secured ? "  🔒" : "") + "   "
                     + std::to_string(r.level) + " dBm");
        row->setTextSize(15);
        row->setPadding(48, 28, 48, 28);
        row->setClickable(true);
        row->setOnClickListener([this, r, secured, dialog](cdroid::View&) {
            dialog->dismiss();
            cdroid::WifiConfiguration cfg;
            cfg.SSID = "\"" + r.SSID + "\"";
            if (!secured) {
                cdroid::WifiManager::getInstance().connect(cfg, nullptr);
                cdroid::Toast::makeText(requireContext(), "连接 " + r.SSID + " …",
                                        cdroid::Toast::LENGTH_SHORT)->show();
                return;
            }
            cdroid::Context* c = requireContext();
            if (c == nullptr) return;
            auto* input = new cdroid::EditText(c);
            input->setHint("密码");
            cdroid::AlertDialog::Builder(c)
                .setTitle("连接 " + r.SSID)
                .setView(input)
                .setPositiveButton("连接",
                    [this, input, cfg](cdroid::DialogInterface&, int) {
                    cdroid::WifiConfiguration withPsk = cfg;
                    withPsk.preSharedKey = "\"" + std::string(input->getText()) + "\"";
                    cdroid::WifiManager::getInstance().connect(withPsk, nullptr);
                    cdroid::Toast::makeText(requireContext(), "连接 …",
                                            cdroid::Toast::LENGTH_SHORT)->show();
                })
                .setNegativeButton("取消", [](cdroid::DialogInterface&, int) {})
                .show();
        });
        list->addView(row);
    }
    dialog->show();
}

void SettingsFragment::showNetworkDetails() {
    cdroid::Context* ctx = requireContext();
    if (ctx == nullptr) return;
    auto* body = new cdroid::TextView(ctx);
    body->setText(networkDetailsText());
    body->setTextSize(13);
    body->setPadding(48, 36, 48, 8);
    auto* scroll = new cdroid::ScrollView(ctx);
    scroll->addView(body);
    cdroid::AlertDialog::Builder(ctx)
        .setTitle("网络详情")
        .setView(scroll)
        .setPositiveButton("确定", [](cdroid::DialogInterface&, int) {})
        .show();
}

void SettingsFragment::buildEthernetSection() {
    cdroid::PreferenceScreen* screen = getPreferenceScreen();
    if (screen == nullptr) return;
    cdroid::EthernetManager& em = cdroid::EthernetManager::getInstance();
    const std::vector<std::string> ifaces = em.getAvailableInterfaces();
    if (ifaces.empty()) return;   // no ethernet section without interfaces
    cdroid::Context& ctx = *requireContext();

    for (const std::string& iface : ifaces) {
        auto* cat = new cdroid::PreferenceCategory(ctx);
        cat->setKey("eth_cat_" + iface);
        cat->setTitle("以太网 " + iface);
        screen->addPreference(cat);

        auto* status = new cdroid::Preference(ctx);
        status->setKey("eth_status_" + iface);
        status->setTitle("状态");
        status->setSelectable(false);
        const cdroid::DhcpInfo d = em.getDhcpInfo(iface);
        status->setSummary(std::string(em.isAvailable(iface) ? "up" : "down")
                + "  IP " + cdroid::DhcpInfo::intToStr(d.ipAddress)
                + "  掩码 " + cdroid::DhcpInfo::intToStr(d.netmask)
                + "  网关 " + cdroid::DhcpInfo::intToStr(d.gateway));
        cat->addPreference(status);

        auto* mode = new cdroid::ListPreference(ctx);
        mode->setKey("eth_mode_" + iface);
        mode->setTitle("IP 模式");
        mode->setEntries({"DHCP", "静态"});
        mode->setEntryValues({"dhcp", "static"});
        mode->setValue(em.getConfiguration(iface).getIpAssignment()
                       == cdroid::IpConfiguration::IpAssignment::STATIC ? "static" : "dhcp");
        mode->setSummary(mode->getValue() == "static" ? "静态" : "DHCP");
        cat->addPreference(mode);

        static const char* kTitles[5] = {"IP 地址(静态)", "前缀长度", "网关", "DNS 1", "DNS 2"};
        static const char* kHints[5]   = {"192.168.1.10", "24", "192.168.1.1", "192.168.1.1", "8.8.8.8"};
        cdroid::EditTextPreference* fields[5];
        for (int i = 0; i < 5; i++) {
            fields[i] = new cdroid::EditTextPreference(ctx);
            fields[i]->setKey("eth_" + iface + "_f" + std::to_string(i));
            fields[i]->setTitle(kTitles[i]);
            fields[i]->setText("");
            fields[i]->setSummary(std::string("如 ") + kHints[i]);
            cat->addPreference(fields[i]);
        }

        auto* apply = new cdroid::Preference(ctx);
        apply->setKey("eth_apply_" + iface);
        apply->setTitle("应用配置");
        cdroid::ListPreference* m = mode;
        cdroid::EditTextPreference* fip = fields[0];
        cdroid::EditTextPreference* fpx = fields[1];
        cdroid::EditTextPreference* fgw = fields[2];
        cdroid::EditTextPreference* fd1 = fields[3];
        cdroid::EditTextPreference* fd2 = fields[4];
        apply->setOnPreferenceClickListener(
                [this, iface, m, fip, fpx, fgw, fd1, fd2](cdroid::Preference&) {
            cdroid::IpConfiguration cfg;
            if (m->getValue() == "static") {
                cdroid::StaticIpConfiguration sc;
                sc.setIpAddress(cdroid::LinkAddress(fip->getText() + "/" + fpx->getText()));
                sc.setGateway(fgw->getText());
                std::vector<std::string> dns;
                if (!fd1->getText().empty()) dns.push_back(fd1->getText());
                if (!fd2->getText().empty()) dns.push_back(fd2->getText());
                sc.setDnsServers(dns);
                cfg.setStaticIpConfiguration(sc);
                cfg.setIpAssignment(cdroid::IpConfiguration::IpAssignment::STATIC);
            } else {
                cfg.setIpAssignment(cdroid::IpConfiguration::IpAssignment::DHCP);
            }
            cfg.setProxySettings(cdroid::IpConfiguration::ProxySettings::NONE);
            cdroid::EthernetManager::getInstance().setConfiguration(iface, cfg);
            cdroid::Toast::makeText(requireContext(),
                    "已下发 " + iface + " 配置(接口写操作需要相应权限)",
                    cdroid::Toast::LENGTH_SHORT)->show();
            return true;
        });
        cat->addPreference(apply);
    }
}

// --- Bluetooth screens (AOSP Settings bluetooth; classes in bluetooth/) ------

void SettingsFragment::setupConnectedScreen() {
    *mBtAlive = true;
    preferencedemo::LocalBluetoothManager* manager =
            preferencedemo::LocalBluetoothManager::getInstance();
    cdroid::BluetoothAdapter& bt = cdroid::BluetoothAdapter::getDefaultAdapter();

    // BluetoothEnabler: the switch drives the radio; adapter flips come back
    // via onBluetoothStateChanged (the event manager delivers on main).
    if (!mBtPairingAgentRegistered) {
        // Interactive pairing: the dialogs below answer the agent's requests
        // (PIN / passkey / confirmation). Just-works peers never ask.
        bt.registerPairingAgent("DisplayYesNo");
        bt.addPairingListener(this);
        mBtPairingAgentRegistered = true;
    }
    manager->getEventManager()->registerCallback(this);

    if (auto* sw = dynamic_cast<cdroid::SwitchPreference*>(findPreference("bluetooth_enabled"))) {
        sw->setChecked(bt.isEnabled());
        sw->setOnPreferenceChangeListener(
                [this](cdroid::Preference&, const nonstd::any& newValue) {
            preferencedemo::LocalBluetoothAdapter* adapter =
                    preferencedemo::LocalBluetoothManager::getInstance()->getBluetoothAdapter();
            if (nonstd::any_cast<bool>(newValue)) adapter->enable();
            else adapter->disable();
            refreshBluetoothStatus();
            return true;
        });
    }
    if (cdroid::Preference* name = findPreference("device_name")) {
        name->setOnPreferenceClickListener([this](cdroid::Preference&) {
            showDeviceRenameDialog();
            return true;
        });
    }
    // The pair-new-device row navigates through the screen_ router
    // (onPreferenceTreeClick -> screen_bluetooth_pairing); no listener here.

    // BluetoothSettings' paired-devices category: BONDED filter over the
    // cache (AOSP addDeviceCategory(paired, BONDED_DEVICE_FILTER, true)).
    mDeviceList = std::make_unique<preferencedemo::DeviceListPreferenceFragment>(this);
    mDeviceList->onStart();
    if (auto* paired = dynamic_cast<cdroid::PreferenceGroup*>(findPreference("paired_devices"))) {
        mDeviceList->addDeviceCategory(paired, "已配对的设备",
                preferencedemo::DeviceListPreferenceFragment::FILTER_BONDED, true);
    }
    refreshBluetoothStatus();
}

void SettingsFragment::setupBluetoothPairingScreen() {
    *mBtAlive = true;
    preferencedemo::LocalBluetoothManager* manager =
            preferencedemo::LocalBluetoothManager::getInstance();
    cdroid::BluetoothAdapter& bt = cdroid::BluetoothAdapter::getDefaultAdapter();
    if (!mBtPairingAgentRegistered) {
        bt.registerPairingAgent("DisplayYesNo");
        bt.addPairingListener(this);
        mBtPairingAgentRegistered = true;
    }
    manager->getEventManager()->registerCallback(this);

    // DevicePickerFragment shape: the available-devices ProgressCategory
    // (spinner while scanning, empty text when a sweep ends empty) replaces
    // the XML stub under the same key.
    auto* available = new preferencedemo::BluetoothProgressCategory(requireContext());
    available->setKey("bt_device_list");
    available->setEmptyTextRes("未在附近找到蓝牙设备。");
    if (cdroid::PreferenceScreen* screen = getPreferenceScreen()) {
        if (cdroid::Preference* stub = findPreference("bt_device_list")) {
            screen->removePreference(stub);
            delete stub;
        }
        screen->addPreference(available);
    }

    mDeviceList = std::make_unique<preferencedemo::DeviceListPreferenceFragment>(this);
    mDeviceList->onStart();
    // AOSP updateContent(STATE_ON): ALL filter over the cache, then scan.
    mDeviceList->addDeviceCategory(available, "可用设备",
            preferencedemo::DeviceListPreferenceFragment::FILTER_ALL, true);
    mDeviceList->enableScanning();
    available->setProgress(true);
}

void SettingsFragment::refreshBluetoothStatus() {
    cdroid::Preference* name = findPreference("device_name");
    if (name != nullptr) {
        preferencedemo::LocalBluetoothAdapter* adapter =
                preferencedemo::LocalBluetoothManager::getInstance()->getBluetoothAdapter();
        if (!adapter->isEnabled()) name->setSummary("Bluetooth 关");
        else {
            const std::string addr = adapter->getAddress();
            name->setSummary(adapter->getName()
                    + (addr.empty() ? std::string() : "  ·  " + addr));
        }
    }
    // AOSP updateFooterPreference: the MAC footer row.
    cdroid::Preference* footer = findPreference("bluetooth_footer");
    if (footer != nullptr) {
        footer->setTitle("设备的蓝牙地址："
                + cdroid::BluetoothAdapter::getDefaultAdapter().getAddress());
    }
}

void SettingsFragment::showDeviceRenameDialog() {
    // LocalDeviceNameDialogFragment shape: rename the local adapter alias.
    cdroid::Context* ctx = requireContext();
    if (ctx == nullptr) return;
    preferencedemo::LocalBluetoothAdapter* adapter =
            preferencedemo::LocalBluetoothManager::getInstance()->getBluetoothAdapter();
    auto* input = new cdroid::EditText(ctx);
    input->setText(adapter->getName());
    cdroid::AlertDialog::Builder(ctx)
        .setTitle("设备名称")
        .setView(input)
        .setPositiveButton("保存", [this, input](cdroid::DialogInterface&, int) {
            preferencedemo::LocalBluetoothManager::getInstance()
                    ->getBluetoothAdapter()->setName(std::string(input->getText()));
            refreshBluetoothStatus();
        })
        .setNegativeButton("取消", [](cdroid::DialogInterface&, int) {})
        .show();
}

// preferencedemo::BluetoothCallback (main thread).

void SettingsFragment::onBluetoothStateChanged(int bluetoothState) {
    using preferencedemo::DeviceListPreferenceFragment;
    if (auto* sw = dynamic_cast<cdroid::SwitchPreference*>(
            findPreference("bluetooth_enabled"))) {
        sw->setChecked(bluetoothState == cdroid::BluetoothAdapter::STATE_ON);
    }
    if (cdroid::Preference* tether = findPreference("bluetooth_tethering")) {
        tether->setEnabled(bluetoothState == cdroid::BluetoothAdapter::STATE_ON);
    }
    refreshBluetoothStatus();
    if (mDeviceList == nullptr) return;
    if (bluetoothState == cdroid::BluetoothAdapter::STATE_ON) {
        // AOSP BluetoothSettings/PairingDetail updateContent(STATE_ON).
        if (auto* progress = dynamic_cast<preferencedemo::BluetoothProgressCategory*>(
                findPreference("bt_device_list"))) {
            progress->setProgress(true);
            mDeviceList->addDeviceCategory(
                    progress, "可用设备",
                    DeviceListPreferenceFragment::FILTER_ALL, false);
            mDeviceList->enableScanning();
        } else if (auto* paired = dynamic_cast<cdroid::PreferenceGroup*>(
                findPreference("paired_devices"))) {
            mDeviceList->addDeviceCategory(paired, "已配对的设备",
                    DeviceListPreferenceFragment::FILTER_BONDED, true);
        }
    }
}

void SettingsFragment::onScanningStateChanged(bool started) {
    // PairingDetail.onScanningStateChanged: keep the spinner while the
    // screen wants scanning; show the empty text when a sweep ends empty.
    if (cdroid::Preference* listPref = findPreference("bt_device_list")) {
        if (auto* progress = dynamic_cast<preferencedemo::BluetoothProgressCategory*>(listPref)) {
            progress->setProgress(started);
        }
    }
}

// cdroid::BluetoothPairingListener (agent thread; marshaled to main).

void SettingsFragment::onPairingRequest(const cdroid::BluetoothDevice& device,
                                        int pairingVariant, uint32_t /*passkey*/) {
    if (!*mBtAlive) return;
    static cdroid::Handler sBtHandler(cdroid::Looper::getMainLooper());
    sBtHandler.post([this, alive = mBtAlive, device, pairingVariant]() mutable {
        if (!*alive) return;
        cdroid::Context* c = requireContext();
        if (c == nullptr) return;
        cdroid::BluetoothAdapter& bt = cdroid::BluetoothAdapter::getDefaultAdapter();
        const std::string display = device.getName().empty()
                ? device.getAddress() : device.getName();
        using cdroid::BluetoothDevice;
        if (pairingVariant == BluetoothDevice::PAIRING_VARIANT_PIN
                || pairingVariant == BluetoothDevice::PAIRING_VARIANT_PIN_16_DIGITS) {
            auto* input = new cdroid::EditText(c);
            input->setHint("PIN");
            cdroid::AlertDialog::Builder(c)
                .setTitle("输入 " + display + " 的 PIN")
                .setView(input)
                .setPositiveButton("配对", [input](cdroid::DialogInterface&, int) {
                    cdroid::BluetoothAdapter::getDefaultAdapter()
                        .replyPairingPin(std::string(input->getText()));
                })
                .setNegativeButton("取消", [](cdroid::DialogInterface&, int) {
                    cdroid::BluetoothAdapter::getDefaultAdapter()
                        .cancelPairingUserInput();
                })
                .show();
        } else if (pairingVariant == BluetoothDevice::PAIRING_VARIANT_PASSKEY) {
            auto* input = new cdroid::EditText(c);
            input->setHint("6 位数字密钥");
            cdroid::AlertDialog::Builder(c)
                .setTitle("输入 " + display + " 的密钥")
                .setView(input)
                .setPositiveButton("配对", [input](cdroid::DialogInterface&, int) {
                    const std::string t = std::string(input->getText());
                    try { cdroid::BluetoothAdapter::getDefaultAdapter()
                            .replyPairingPasskey((uint32_t)std::stoul(t)); }
                    catch (...) { cdroid::BluetoothAdapter::getDefaultAdapter()
                            .cancelPairingUserInput(); }
                })
                .setNegativeButton("取消", [](cdroid::DialogInterface&, int) {
                    cdroid::BluetoothAdapter::getDefaultAdapter()
                        .cancelPairingUserInput();
                })
                .show();
        } else if (pairingVariant == BluetoothDevice::PAIRING_VARIANT_PASSKEY_CONFIRMATION
                || pairingVariant == BluetoothDevice::PAIRING_VARIANT_CONSENT) {
            cdroid::AlertDialog::Builder(c)
                .setTitle("配对请求")
                .setMessage("与 " + display + " 配对?")
                .setPositiveButton("配对", [](cdroid::DialogInterface&, int) {
                    cdroid::BluetoothAdapter::getDefaultAdapter()
                        .replyPairingConfirmation(true);
                })
                .setNegativeButton("取消", [](cdroid::DialogInterface&, int) {
                    cdroid::BluetoothAdapter::getDefaultAdapter()
                        .replyPairingConfirmation(false);
                })
                .show();
        } else {
            bt.cancelPairingUserInput();
        }
    });
}

void SettingsFragment::onDisplayPasskey(const cdroid::BluetoothDevice& device,
                                        uint32_t passkey, int) {
    if (!*mBtAlive) return;
    static cdroid::Handler sBtHandler(cdroid::Looper::getMainLooper());
    sBtHandler.post([this, alive = mBtAlive, device, passkey]() mutable {
        if (!*alive) return;
        cdroid::Context* c = requireContext();
        if (c == nullptr) return;
        const std::string display = device.getName().empty()
                ? device.getAddress() : device.getName();
        cdroid::Toast::makeText(c, "在 " + display + " 上输入密钥 " +
                std::to_string(passkey), cdroid::Toast::LENGTH_LONG)->show();
    });
}

void SettingsFragment::onPairingCancelled(const cdroid::BluetoothDevice&) {
    if (!*mBtAlive) return;
    static cdroid::Handler sBtHandler(cdroid::Looper::getMainLooper());
    sBtHandler.post([this, alive = mBtAlive]{
        if (!*alive) return;
        cdroid::Context* c = requireContext();
        if (c != nullptr) {
            cdroid::Toast::makeText(c, "配对已取消", cdroid::Toast::LENGTH_SHORT)->show();
        }
    });
}

void SettingsFragment::cycleStep(int index) {
    // File-local main-looper handler: PreferenceFragment::mHandler is private.
    static cdroid::Handler sCycleHandler(cdroid::Looper::getMainLooper());
    cdroid::PreferenceScreen* screen = getPreferenceScreen();
    if (screen == nullptr || index >= screen->getPreferenceCount()) {
        cdroid::App::getInstance().exit(0);
        return;
    }
    cdroid::Preference* p = screen->getPreference(index);
    const bool isScreen = dynamic_cast<cdroid::PreferenceScreen*>(p) != nullptr;
    p->performClick();
    if (isScreen) {
        // Let the pushed sub-screen settle, pop it via the host Window's
        // virtual onBackPressed, then walk on.
        sCycleHandler.postDelayed([this, index]() {
            getActivity()->onBackPressed();
            sCycleHandler.postDelayed([this, index]() { cycleStep(index + 1); }, 3000);
        }, 6000);
    } else {
        sCycleHandler.postDelayed([this, index]() { cycleStep(index + 1); }, 2500);
    }
}

/** Hosts the settings hierarchy; nested screens push a new SettingsFragment. */
class SettingsActivity : public FragmentActivity, public cdroid::OnPreferenceStartScreenCallback {
public:
    SettingsActivity() : FragmentActivity(0, 0, -1, -1) {
        // Framework theme carries the whole palette (text appearances,
        // ripples, window background); the opaque backdrops below only
        // guarantee no desktop leak from the transparent window surface.
        setTheme((getenv("PREFDEMO_DARK") != nullptr)
                ? (int)internal::R::style::Theme_Material
                : (int)internal::R::style::Theme_Material_Light);
    }

    bool onPreferenceStartScreen(PreferenceFragment& /*caller*/,
            PreferenceScreen& pref) override {
        navigateTo(pref.getKey(), pref.getTitle());
        return true;
    }

    /** Route a second-level screen by orientation: portrait pushes it onto
     *  the back stack (Back pops), landscape swaps the detail pane in place
     *  (two-pane has no back navigation). */
    void navigateTo(const std::string& key, const std::string& title) {
        if (mLandscape) showDetail(key);
        else openScreen(key, title);
    }

    /** Portrait: push the second-level screen; Back pops it (onBackPressed). */
    void openScreen(const std::string& key, const std::string& /*title*/) {
        auto* fragment = newFragmentForKey(key);
        auto* tx = getSupportFragmentManager()->beginTransaction();
        // Default motion is the fragment Transitions set in onCreatePreferences
        // (SEC Priority 1). PREFDEMO_ANIM=1 opts this transaction back into the
        // legacy custom slides (SEC Priority 2, kept for comparison/regression).
        if (getenv("PREFDEMO_ANIM") != nullptr) {
            tx->setCustomAnimations((int)preferencedemo::R::anim::slide_in_right,
                                    (int)preferencedemo::R::anim::slide_out_left,
                                    (int)preferencedemo::R::anim::slide_in_left,
                                    (int)preferencedemo::R::anim::slide_out_right);
        }
        tx->replace((int)preferencedemo::R::id::prefdemo_single, fragment)
           .addToBackStack(key)
           .commit();
    }

    /** Landscape: swap the right pane to the selected second-level screen. */
    void showDetail(const std::string& key) {
        getSupportFragmentManager()->beginTransaction()
            ->replace((int)preferencedemo::R::id::prefdemo_detail, newFragmentForKey(key))
            .commit();
    }

    static SettingsFragment* newFragmentForKey(const std::string& key) {
        auto* fragment = new SettingsFragment();
        auto* args = new cdroid::Bundle();
        args->putString(PreferenceFragment::ARG_PREFERENCE_ROOT, key);
        fragment->setArguments(args);
        return fragment;
    }

protected:
    void onCreate(cdroid::Bundle* savedInstanceState) override {
        FragmentActivity::onCreate(savedInstanceState);
        mLandscape = getContext()->getResources().getConfiguration().orientation
                == cdroid::Configuration::ORIENTATION_LANDSCAPE;
        // Activity content comes from R::layout::prefdemo_main: layout/ is a
        // single slot, the layout-land/ twin is master + detail (two-pane).
        // The window surface is transparent: back the slots with an opaque
        // color too, so pane swaps never leak the desktop.
        auto* host = dynamic_cast<cdroid::ViewGroup*>(findViewById(getFragmentContainerId()));
        if (host == nullptr) {
            LOGW("no fragment container to host prefdemo_main");
            return;
        }
        auto* main = cdroid::LayoutInflater::from(getContext())->inflate(
                (int)preferencedemo::R::layout::prefdemo_main, host, true);
        main->setBackgroundColor(themeColor(*getContext(),
                (int)internal::R::attr::colorBackground, 0xFFF8F9FA));
        // Optional argv[1]: start directly at a nested screen (smoke-testing
        // every second-level page without touch input), e.g.
        //   ./preferencedemo screen_network
        std::string initialRoot;
        if (mLaunchArg != nullptr) initialRoot = mLaunchArg;
        if (mLandscape) {
            // Two-pane: master carries the top-level list, detail starts at
            // the given (or first) top-level entry's screen.
            auto* tx = getSupportFragmentManager()->beginTransaction();
            tx->replace((int)preferencedemo::R::id::prefdemo_master, new SettingsFragment());
            tx->replace((int)preferencedemo::R::id::prefdemo_detail,
                    newFragmentForKey(initialRoot.empty() ? "screen_network" : initialRoot));
            tx->commit();
        } else {
            auto* fragment = new SettingsFragment();
            if (!initialRoot.empty()) fragment = newFragmentForKey(initialRoot);
            getSupportFragmentManager()->beginTransaction()
                ->replace((int)preferencedemo::R::id::prefdemo_single, fragment)
                .commit();
        }
    }

private:
    const char* mLaunchArg = nullptr;
    bool mLandscape = false;   // set in onCreate from the resource config

public:
    void setLaunchRoot(const char* key) { mLaunchArg = key; }

    /** Pop the current second-level screen back to its parent. */
    void goBack() {
        getSupportFragmentManager()->popBackStackImmediate();
    }

protected:
    void onBackPressed() override {
        // The back stack pops the nested-screen fragment; its own header bar
        // shows the parent title again.
        if (!getSupportFragmentManager()->popBackStackImmediate()) {
            FragmentActivity::onBackPressed();
        }
    }
};

void SettingsFragment::requestGoBack() {
    auto* host = dynamic_cast<SettingsActivity*>(getActivity());
    if (host != nullptr) host->goBack();
}

bool SettingsFragment::onPreferenceTreeClick(Preference& preference) {
    // Bluetooth device rows (AOSP DeviceListPreferenceFragment's branch).
    if (mDeviceList != nullptr && mDeviceList->onPreferenceClick(&preference)) {
        return true;
    }
    if (screenXmlFor(preference.getKey()) != 0) {
        auto* host = dynamic_cast<SettingsActivity*>(getActivity());
        if (host != nullptr) {
            host->navigateTo(preference.getKey(), preference.getTitle());
            return true;
        }
    }
    return handleTreeClick(preference);
}

int main(int argc, const char* argv[]) {
    cdroid::App app(argc, argv);

    // Orientation -> resource config: PREFDEMO_ORIENTATION forces a variant
    // here (like PREFDEMO_DARK below); with no env the framework already
    // resolved it in App::applyOrientationConfig (the manifest's
    // android:screenOrientation > screen shape), so there is
    // nothing to redo. Either way this stays before the first inflate
    // (SettingsActivity below) so layouts resolve under the right config.
    int orientation = 0;
    const char* forced = getenv("PREFDEMO_ORIENTATION");
    if (forced != nullptr && forced[0] != '\0') {
        const std::string v = forced;
        if (v == "land" || v == "landscape")
            orientation = cdroid::Configuration::ORIENTATION_LANDSCAPE;
        else if (v == "port" || v == "portrait")
            orientation = cdroid::Configuration::ORIENTATION_PORTRAIT;
        else
            LOGW("PREFDEMO_ORIENTATION='%s' invalid (land|landscape|port|portrait)",
                 forced);
    }
    if (orientation != 0) {
        cdroid::Resources& res = app.getResources();
        cdroid::Configuration cfg = res.getConfiguration();
        cfg.orientation = orientation;
        res.updateConfiguration(&cfg, nullptr);
        LOGI("prefdemo orientation=%s (env override)",
             orientation == cdroid::Configuration::ORIENTATION_LANDSCAPE
                     ? "landscape" : "portrait");
    }
    // App-level theme: every LayoutInflater::from(ctx) in the preference
    // chain resolves ?android:attr/textAppearance against the App context,
    // so the Material Light palette reaches the row TextViews (a Window-only
    // setTheme would not propagate to those contexts).
    // PREFDEMO_DARK=1 selects the dark Material theme (chrome colors all
    // resolve from the live theme, so both palettes exercise the same code).
    const int themeId = (getenv("PREFDEMO_DARK") != nullptr)
            ? (int)internal::R::style::Theme_Material
            : (int)internal::R::style::Theme_Material_Light;
    app.setTheme(themeId);
    auto* w = new SettingsActivity();
    // Optional argv[1]: start directly at a nested screen (smoke-testing) —
    // but skip framework options (--auto-test etc.): App owns those. Their
    // VALUES too: "-f 3000" leaves a bare "3000" in argv, which the old
    // '-'-prefix check took as a root key and aborted the first inflation
    // (screenXmlFor -> 0 -> empty parser -> "No start tag found").
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-' && screenXmlFor(argv[i]) != 0) { w->setLaunchRoot(argv[i]); break; }
    }
    LOGD("settings demo window created");
    return app.exec();
}
