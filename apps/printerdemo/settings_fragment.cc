/*********************************************************************************
 * SettingsFragment — brightness seekbar and the resource-driven language
 * picker: AssetManager.getNonSystemLocales() enumerates the app pak's locales,
 * PopupMenu offers them by self-name, applyLocale() switches and recreates.
 *********************************************************************************/
#include <core/app.h>
#include <cdroid.h>
#include <algorithm>
#include <vector>
#include <fragment/fragment.h>
#include <fragment/fragmentfactory.h>
#include <transition/slide.h>
#include <widget/textview.h>
#include <widget/seekbar.h>
#include <menu/menu.h>
#include <menu/menuitem.h>
#include <menu/popupmenu.h>
#include <navigation/navigation.h>
#include <navigation/navcontroller.h>
#include <app/alertdialog.h>
#include <widget/scrollview.h>
#include <connectivitymanager.h>          // cdnet: active-network aggregation
#include <ethernet/ethernetmanager.h>     // cdnet: per-iface DhcpInfo
#include <dhcpinfo.h>
#include <wifi/wifimanager.h>
#include <wifi/wifiinfo.h>
#include <content/assetmanager.h>
#include "printer_common.h"
#include "R.h"

// ---------------------------------------------------------------------------
class SettingsFragment : public cdroid::Fragment{
public:
    void onCreate(cdroid::Bundle* savedInstanceState) override{
        cdroid::Fragment::onCreate(savedInstanceState);
        setEnterTransition(new cdroid::Slide(cdroid::Gravity::END));
        setExitTransition(new cdroid::Slide(cdroid::Gravity::END));
    }
    cdroid::View* onCreateView(cdroid::LayoutInflater* inflater, cdroid::ViewGroup* container,
                               cdroid::Bundle*) override{
        return inflater->inflate(printerdemo::R::layout::fragment_settings, container, false);
    }
    void onViewCreated(cdroid::View* view, cdroid::Bundle*) override{
        cdroid::Fragment::onViewCreated(view, nullptr);
        cdroid::SeekBar* seek = (cdroid::SeekBar*)view->findViewById(printerdemo::R::id::seek_brightness);
        cdroid::TextView* tv = (cdroid::TextView*)view->findViewById(printerdemo::R::id::tv_brightness);
        if(seek && tv){
            cdroid::SeekBar::OnSeekBarChangeListener l;
            l.onProgressChanged = [tv](cdroid::SeekBar&, int progress, bool){
                tv->setText(std::to_string(progress) + "%");
            };
            l.onStartTrackingTouch = [](cdroid::SeekBar&){};
            l.onStopTrackingTouch  = [](cdroid::SeekBar&){};
            seek->setOnSeekBarChangeListener(l);
        }
        // Language picker: the offered set is what the app's resources actually
        // carry — AssetManager.getNonSystemLocales() enumerates the app pak's
        // values-<locale> tables; the unqualified values/ base (stored in the
        // arsc without a locale tag) is the app's base language, en-US here.
        // Live IP row (ConnectivityManager aggregation, ETHERNET > WIFI) and
        // the full network-details dialog on tap.
        mIpValue = (cdroid::TextView*)view->findViewById(printerdemo::R::id::tv_ip_value);
        refreshIpRow();
        if(cdroid::View* ipRow = view->findViewById(printerdemo::R::id::row_ip)){
            ipRow->setOnClickListener([this](cdroid::View&){ showNetworkDetails(); });
        }
        // Wi-Fi trial page (cdnet WifiManager over wpa_supplicant ctrl_iface).
        if(cdroid::View* wifiRow = view->findViewById(printerdemo::R::id::row_wifi)){
            wifiRow->setOnClickListener([](cdroid::View& v){
                if(cdroid::NavController* nc = cdroid::Navigation::findNavController(&v))
                    nc->navigate("wifi");
            });
        }
        if(cdroid::View* row = view->findViewById(printerdemo::R::id::row_language)){
            row->setOnClickListener([](cdroid::View& v){
                std::vector<std::string> tags{ "en-US" };   // the values/ base language
                for(const std::string& t : cdroid::App::getInstance().getAssets().getNonSystemLocales())
                    if(!t.empty() && std::find(tags.begin(), tags.end(), t) == tags.end())
                        tags.push_back(t);
                // Fire-and-forget (the unified transient-popup contract): the
                // menu owns itself after show() and self-destructs once its
                // dismiss cascade completes — no member, no delete, one fresh
                // menu per click. Gravity.RIGHT aligns the popup's right edge
                // with the row's right edge (the only horizontal alignment
                // PopupWindow special-cases, same as AOSP).
                cdroid::PopupMenu* menu = new cdroid::PopupMenu(v.getContext(), &v, cdroid::Gravity::RIGHT);
                cdroid::Menu* m = menu->getMenu();
                for(size_t i = 0; i < tags.size(); i++){
                    const cdroid::Locale l = cdroid::Locale::forLanguageTag(tags[i]);
                    cdroid::MenuItem* mi = m->add(cdroid::Menu::NONE, (int)i, (int)i,
                            l.getDisplayName(l));   // self-name, the picker convention
                    mi->setCheckable(true);
                    mi->setChecked(tags[i] == sLocaleTag);
                }
                menu->setOnMenuItemClickListener([tags](cdroid::MenuItem& item){
                    applyLocale(tags[item.getItemId()]);
                    return true;
                });
                menu->show();
            });
        }
    }

private:
    // WifiInfo's int is little-endian packed (AOSP): first octet = low byte.
    static std::string ipToString(uint32_t ip) {
        if (!ip) return std::string();
        return std::to_string(ip & 0xff) + "." + std::to_string((ip >> 8) & 0xff)
             + "." + std::to_string((ip >> 16) & 0xff) + "." + std::to_string((ip >> 24) & 0xff);
    }

    void refreshIpRow() {
        if (mIpValue == nullptr) return;
        cdroid::NetworkInfo ni =
            cdroid::ConnectivityManager::getInstance().getActiveNetworkInfo();
        if (!ni.isConnected()) { mIpValue->setText("未连接"); return; }
        if (ni.getType() == cdroid::ConnectivityManager::TYPE_WIFI) {
            const std::string ip = ipToString((uint32_t)cdroid::WifiManager::getInstance()
                                                  .getConnectionInfo().getIpAddress());
            mIpValue->setText("Wi-Fi · " + (ip.empty() ? "未获取" : ip));
        } else if (ni.getType() == cdroid::ConnectivityManager::TYPE_ETHERNET) {
            mIpValue->setText("以太网" + ethernetSummary());
        } else {
            mIpValue->setText(ni.getTypeName());
        }
    }

    // " · 192.168.1.10" for the first addressed ethernet iface, else "".
    static std::string ethernetSummary() {
        cdroid::EthernetManager& em = cdroid::EthernetManager::getInstance();
        for (const std::string& iface : em.getAvailableInterfaces()) {
            const cdroid::DhcpInfo d = em.getDhcpInfo(iface);
            const std::string ip = cdroid::DhcpInfo::intToStr(d.ipAddress);
            if (!ip.empty() && ip != "0.0.0.0") return " · " + ip;
        }
        return std::string();
    }

    void showNetworkDetails() {
        cdroid::Context* ctx = getContext();
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
            .setPositiveButton("确定", [](cdroid::DialogInterface&, int){})
            .show();
    }

    std::string networkDetailsText() {
        cdroid::ConnectivityManager& cm = cdroid::ConnectivityManager::getInstance();
        std::string text;
        cdroid::NetworkInfo ni = cm.getActiveNetworkInfo();
        text += "活动网络: " + (ni.isConnected() ? ni.getTypeName() + " 已连接"
                                                 : ni.getTypeName() + " 未连接");
        if (!ni.getExtraInfo().empty()) text += "  (" + ni.getExtraInfo() + ")";
        text += "\n";

        // Wi-Fi block (supplicant state; IP comes from the platform address
        // push, so static assignments show too).
        cdroid::WifiManager& wifi = cdroid::WifiManager::getInstance();
        text += std::string("\nWi-Fi  ") + (wifi.isWifiEnabled() ? "开" : "关");
        if (wifi.isWifiEnabled()) {
            cdroid::WifiInfo info = wifi.getConnectionInfo();
            const std::string ssid = info.getSSID();
            const std::string ip = ipToString((uint32_t)info.getIpAddress());
            text += std::string("\n  IP ") + (ip.empty() ? "未获取" : ip)
                  + "   信号 " + std::to_string(info.getRssi()) + " dBm"
                  + "\n  BSSID " + info.getBSSID()
                  + "   MAC " + info.getMacAddress();
            if (!ssid.empty()) text += "\n  SSID " + ssid;
        }

        // Ethernet blocks: full DhcpInfo dump per available iface.
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

    cdroid::TextView* mIpValue = nullptr;
};
REGISTER_FRAGMENT(SettingsFragment);
