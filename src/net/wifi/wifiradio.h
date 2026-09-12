#ifndef __WIFI_RADIO_H__
#define __WIFI_RADIO_H__

#include <string>
#include <vector>

#include <wifi/scanresult.h>

namespace cdroid {

/**
 * Platform-neutral seam for the wifi radio data plane — the Windows-porting
 * extension point, companion to NetworkEventMonitor:
 *
 *   Linux   : nl80211 generic-netlink dump (see nl80211radio.{h,cc}) —
 *             the AOSP wifi-HAL scan source (IEs / TSF / channel width).
 *   Windows : WlanApi (WlanEnumInterfaces + WlanGetNetworkBssList) later
 *             registers its own create().
 *
 * AOSP counterpart: IWifiStaIface#getScanResults via wificond/HIDL — scans
 * come from the radio, the supplicant stays the control plane.
 */
class WifiRadioData {
public:
    virtual ~WifiRadioData() = default;

    /* Full-fidelity scan dump for `iface`; empty when the radio source is
     * unavailable (permissions, no data) — callers fall back to the
     * supplicant ctrl_iface list. */
    virtual std::vector<ScanResult> getScanResults(const std::string& iface) = 0;

    static WifiRadioData* create();
};

} // namespace cdroid

#endif /* __WIFI_RADIO_H__ */
