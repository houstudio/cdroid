/*********************************************************************************
 * WifiFragment — trial wiring of the cdnet WifiManager (android.net.wifi port
 * over wpa_supplicant ctrl_iface) into a real app page: state/SSID/RSSI card,
 * scan list, tap-to-connect (password dialog for secured networks), enable
 * switch, disconnect. The supplicant control socket defaults to
 * SupplicantClient::defaultCtrlPath(); WPA_CTRL_PATH overrides it (the
 * mac80211_hwsim bench runs the supplicant at /tmp/wpa-hwsim/wlan0).
 *
 * Threading: WifiManager callbacks arrive on the supplicant monitor thread —
 * every listener marshals through View::post and re-checks mAlive; listeners
 * are removed in onDestroyView.
 *********************************************************************************/
#include <core/app.h>
#include <cdroid.h>
#include <fragment/fragment.h>
#include <fragment/fragmentfactory.h>
#include <transition/slide.h>
#include <widget/textview.h>
#include <widget/button.h>
#include <widget/switch.h>
#include <widget/edittext.h>
#include <widget/toast.h>
#include <app/alertdialog.h>
#include <wifi/wifimanager.h>
#include <wifi/scanresult.h>
#include <wifi/wifiinfo.h>
#include <wifi/wificonfiguration.h>
#include "printer_common.h"
#include "R.h"

#include <atomic>
#include <cstdlib>
#include <vector>

using cdroid::WifiManager;
using cdroid::ScanResult;
using cdroid::WifiConfiguration;

static std::string quoted(const std::string& s) { return "\"" + s + "\""; }

// Trim the quotes WifiSsid#toString carries ("\"MyNetwork\"").
static std::string unquoted(const std::string& s) {
    if (s.size() >= 2 && s.front() == '"' && s.back() == '"') return s.substr(1, s.size() - 2);
    return s;
}

static bool isSecured(const std::string& capabilities) {
    return capabilities.find("WPA") != std::string::npos
        || capabilities.find("WEP") != std::string::npos
        || capabilities.find("SAE") != std::string::npos;
}

class WifiFragment : public cdroid::Fragment,
                     public WifiManager::NetworkStateListener,
                     public WifiManager::ScanResultsListener,
                     public WifiManager::ActionListener {
public:
    void onCreate(cdroid::Bundle* savedInstanceState) override {
        cdroid::Fragment::onCreate(savedInstanceState);
        setEnterTransition(new cdroid::Slide(cdroid::Gravity::END));
        setExitTransition(new cdroid::Slide(cdroid::Gravity::END));
    }

    cdroid::View* onCreateView(cdroid::LayoutInflater* inflater, cdroid::ViewGroup* container,
                               cdroid::Bundle*) override {
        return inflater->inflate(printerdemo::R::layout::fragment_wifi, container, false);
    }

    void onViewCreated(cdroid::View* view, cdroid::Bundle*) override {
        cdroid::Fragment::onViewCreated(view, nullptr);
        mRoot = view;
        mAlive = true;

        // The transport binds the library default (SupplicantClient::
        // defaultCtrlPath: WPA_CTRL_PATH if set, else the system socket) and
        // starts the event pump; idempotent.
        WifiManager& wifi = WifiManager::getInstance();
        wifi.initialize();
        wifi.addNetworkStateListener(this);
        wifi.addScanResultsListener(this);

        mState = (cdroid::TextView*)view->findViewById(printerdemo::R::id::tv_wifi_state);
        mDetail = (cdroid::TextView*)view->findViewById(printerdemo::R::id::tv_wifi_detail);
        mList = (cdroid::LinearLayout*)view->findViewById(printerdemo::R::id::list_wifi);
        if (cdroid::Button* scan = (cdroid::Button*)view->findViewById(printerdemo::R::id::btn_scan)) {
            scan->setOnClickListener([this](cdroid::View&){ doScan(); });
        }
        if (cdroid::Button* disc = (cdroid::Button*)view->findViewById(printerdemo::R::id::btn_disconnect)) {
            disc->setOnClickListener([](cdroid::View&){ WifiManager::getInstance().disconnect(); });
        }
        if (cdroid::Switch* sw = (cdroid::Switch*)view->findViewById(printerdemo::R::id::sw_wifi)) {
            sw->setChecked(wifi.isWifiEnabled());
            sw->setOnCheckedChangeListener([](cdroid::CompoundButton&, bool on){
                WifiManager::getInstance().setWifiEnabled(on);
            });
        }
        doScan();
    }

    void onDestroyView() override {
        detach();
        cdroid::Fragment::onDestroyView();
    }

    // --- WifiManager::NetworkStateListener (monitor thread) -----------------
    void onNetworkStateChanged(const cdroid::WifiInfo&) override {
        cdroid::View* root = mRoot;
        if (root == nullptr || !mAlive) return;
        root->post([this]{ if (mAlive) { refreshStatus(); refreshConnectedRow(); } });
    }

    // --- WifiManager::ScanResultsListener (monitor thread) ------------------
    void onScanResultsAvailable() override {
        cdroid::View* root = mRoot;
        if (root == nullptr || !mAlive) return;
        root->post([this]{ if (mAlive) refreshList(); });
    }

    // --- WifiManager::ActionListener (connect result; called back on the
    //     caller thread for connect(config) — we only call it from the UI) ---
    void onSuccess() override { toast("已连接"); }
    void onFailure(int) override { toast("连接失败"); }

private:
    void detach() {
        mAlive = false;
        WifiManager& wifi = WifiManager::getInstance();
        wifi.removeNetworkStateListener(this);
        wifi.removeScanResultsListener(this);
    }

    void doScan() {
        if (mDetail) mDetail->setText("扫描中…");
        WifiManager::getInstance().startScan();
        if (mRoot) mRoot->postDelayed([this]{ if (mAlive) refreshList(); }, 2500);
        refreshStatus();
    }

    void refreshStatus() {
        WifiManager& wifi = WifiManager::getInstance();
        if (!wifi.isWifiEnabled()) {
            if (mState) mState->setText("Wi-Fi 已关闭");
            if (mDetail) mDetail->setText("");
            return;
        }
        cdroid::WifiInfo info = wifi.getConnectionInfo();
        const std::string ssid = unquoted(info.getSSID());
        if (mState) {
            mState->setText(ssid.empty() || ssid == WifiManager::UNKNOWN_SSID
                            ? "Wi-Fi 未连接" : "已连接: " + ssid);
        }
        if (mDetail) {
            std::string ipStr = "未获取";
            if (const uint32_t ip = (uint32_t)info.getIpAddress()) {
                // AOSP little-endian packing: first octet is the low byte.
                ipStr = std::to_string(ip & 0xff) + "." + std::to_string((ip >> 8) & 0xff)
                      + "." + std::to_string((ip >> 16) & 0xff) + "." + std::to_string((ip >> 24) & 0xff);
            }
            mDetail->setText("信号 " + std::to_string(info.getRssi()) + " dBm   状态 "
                             + (info.getSupplicantState() == cdroid::SupplicantState::COMPLETED
                                ? "COMPLETED" : "…")
                             + "   IP " + ipStr
                             + "\nBSSID " + info.getBSSID()
                             + "   " + std::to_string(info.getFrequency()) + " MHz"
                             + "\nMAC " + info.getMacAddress());
        }
    }

    // Mark the connected network's row in the scan list.
    void refreshConnectedRow() { refreshList(); }

    void refreshList() {
        if (mList == nullptr) return;
        refreshStatus();
        mList->removeAllViews();
        cdroid::Context* ctx = getContext();

        const std::vector<ScanResult> results = WifiManager::getInstance().getScanResults();
        if (results.empty()) {
            auto* empty = new cdroid::TextView(ctx);
            empty->setText("(无扫描结果 — 点“扫描”)");
            empty->setTextSize(13);
            empty->setPadding(28, 20, 28, 20);
            mList->addView(empty);
            return;
        }
        // Strongest first.
        std::vector<ScanResult> sorted(results);
        std::sort(sorted.begin(), sorted.end(),
                  [](const ScanResult& a, const ScanResult& b){ return a.level > b.level; });
        const std::string cur = unquoted(WifiManager::getInstance().getConnectionInfo().getSSID());
        bool first = true;
        for (const ScanResult& r : sorted) {
            const std::string ssid = unquoted(r.SSID);
            if (ssid.empty()) continue;   // hidden APs — out of scope for the trial
            first = false;
            auto* row = new cdroid::TextView(ctx);
            row->setText((ssid == cur ? "✓ " : "") + ssid
                         + (isSecured(r.capabilities) ? "  🔒" : "")
                         + "   " + std::to_string(r.level) + " dBm");
            row->setTextSize(15);
            row->setPadding(28, 22, 28, 22);
            row->setClickable(true);
            // Copy what the click needs — the ScanResult itself outlives the loop.
            const std::string ssidCopy = ssid;
            const bool secured = isSecured(r.capabilities);
            row->setOnClickListener([this, ssidCopy, secured](cdroid::View&){
                connectTo(ssidCopy, secured);
            });
            mList->addView(row);
        }
    }

    void connectTo(const std::string& ssid, bool secured) {
        WifiConfiguration config;
        config.SSID = quoted(ssid);
        if (!secured) {
            WifiManager::getInstance().connect(config, this);
            toast("连接 " + ssid + " …");
            return;
        }
        cdroid::Context* ctx = getContext();
        auto* input = new cdroid::EditText(ctx);
        input->setHint("密码");
        cdroid::AlertDialog::Builder(ctx)
            .setTitle("连接 " + ssid)
            .setView(input)
            .setPositiveButton("连接", [this, input, ssid](cdroid::DialogInterface&, int){
                const std::string psk = input->getText();
                WifiConfiguration cfg;
                cfg.SSID = quoted(ssid);
                cfg.preSharedKey = quoted(psk);
                WifiManager::getInstance().connect(cfg, this);
                toast("连接 " + ssid + " …");
            })
            .setNegativeButton("取消", [](cdroid::DialogInterface&, int){})
            .show();
    }

    void toast(const std::string& text) {
        if (cdroid::Context* ctx = getContext())
            cdroid::Toast::makeText(ctx, text, cdroid::Toast::LENGTH_SHORT)->show();
    }

    std::atomic<bool> mAlive { false };
    cdroid::View* mRoot = nullptr;
    cdroid::TextView* mState = nullptr;
    cdroid::TextView* mDetail = nullptr;
    cdroid::LinearLayout* mList = nullptr;
};

REGISTER_FRAGMENT(WifiFragment);
