#!/usr/bin/env bash
# hwsim-setup — one-stop mac80211_hwsim WiFi testbed for the cdnet stack.
# Brings up the whole bench and bakes in every lesson learned so far:
#   AP (hostapd on wlan1) + STA supplicant (wlan0, ctrl socket for the
#   invoking user) + DHCP server (dnsmasq, --log-dhcp into a file — the
#   server side is always observable) + ufw pinhole for udp/67 (the
#   default-deny INPUT was eating our DISCOVERs) + CAP_NET_RAW/NET_ADMIN
#   grants on the test binaries (rebuilds wipe them; DHCP needs them,
#   AF_PACKET + interface config, without running the app as root).
#
# usage: ./hwsim-setup.sh [--start|--stop|--fresh|--status]
#   --start   bring everything up (default)
#   --stop    tear the three daemons down (caps and firewall rule stay)
#   --fresh   --start with a wiped supplicant config (no saved networks)
#   --caps    re-grant capabilities only (rebuilds wipe them; no daemon
#             restart) — the most common re-trigger after `make`
#   --status  non-privileged overview: daemons, supplicant state, leases,
#             addresses, caps, firewall
set -euo pipefail

TESTDIR=/tmp/hwsim-test
CTRL_DIR=/tmp/wpa-hwsim
IF_AP=wlan1
IF_STA=wlan0
SSID=hwsim-test
PSK=hwsim12345
DHCP_POOL="192.168.77.50,192.168.77.60,255.255.255.0,12h"
GW=192.168.77.1
BUILD="$(cd "$(dirname "$0")/../../.." && pwd)/outX64-Debug"
LEASES=/var/lib/misc/dnsmasq.leases
CAPS="cap_net_raw,cap_net_admin+ep"

MODE=--start
[ $# -ge 1 ] && MODE=$1

if [ "$(id -u)" -ne 0 ] && [ "$MODE" != "--status" ]; then
    exec sudo bash "$0" "$@"
fi
WGROUP="${SUDO_USER:-root}"   # ctrl socket group = the invoking user

ensure_hostapd_conf() {
    [ -f "$TESTDIR/hostapd.conf" ] && return 0
    mkdir -p "$TESTDIR"
    cat > "$TESTDIR/hostapd.conf" <<EOF
interface=$IF_AP
driver=nl80211
ssid=$SSID
hw_mode=g
channel=6
wpa=2
wpa_passphrase=$PSK
wpa_key_mgmt=WPA-PSK
rsn_pairwise=CCMP
EOF
}

ensure_supplicant_conf() {
    [ -f "$TESTDIR/wpa_supplicant.conf" ] && return 0
    cat > "$TESTDIR/wpa_supplicant.conf" <<EOF
ctrl_interface=DIR=$CTRL_DIR GROUP=$WGROUP
update_config=1
EOF
}

stop_daemons() {
    for name in dnsmasq wpa hostapd; do
        if [ -f "$TESTDIR/$name.pid" ]; then
            kill "$(cat "$TESTDIR/$name.pid")" 2>/dev/null || true
            rm -f "$TESTDIR/$name.pid"
        fi
    done
    pkill -f "dnsmasd.*interface=$IF_AP" 2>/dev/null || true
    pkill -f "dnsmasq.*interface=$IF_AP" 2>/dev/null || true
    sleep 1
}

start_bench() {
    lsmod | grep -q mac80211_hwsim || { echo "loading mac80211_hwsim"; modprobe mac80211_hwsim; }
    ip link show "$IF_AP" >/dev/null 2>&1 || { echo "E: $IF_AP missing (hwsim radios)"; exit 1; }
    ip link show "$IF_STA" >/dev/null 2>&1 || { echo "E: $IF_STA missing (hwsim radios)"; exit 1; }

    stop_daemons
    ensure_hostapd_conf
    ensure_supplicant_conf

    hostapd -B -i "$IF_AP" "$TESTDIR/hostapd.conf" \
        -P "$TESTDIR/hostapd.pid" -f "$TESTDIR/hostapd.log"
    # dnsmasq drops every DHCP request on an interface that has no address
    # inside the --dhcp-range subnet ("no address range available") — the
    # AP side must own the gateway address first.
    ip addr add "$GW/24" dev "$IF_AP" 2>/dev/null || true
    wpa_supplicant -B -i "$IF_STA" -c "$TESTDIR/wpa_supplicant.conf" \
        -P "$TESTDIR/wpa.pid" -f "$TESTDIR/wpa.log"
    : > "$TESTDIR/dnsmasq.log"
    dnsmasq --interface="$IF_AP" --bind-interfaces \
        --dhcp-range="$DHCP_POOL" \
        --dhcp-option=option:dns-server,"$GW" \
        --log-dhcp --log-facility="$TESTDIR/dnsmasq.log" \
        --pid-file="$TESTDIR/dnsmasq.pid"
    chmod a+r "$TESTDIR/dnsmasq.log"
    ufw allow in on "$IF_AP" to any port 67 proto udp >/dev/null 2>&1 || true

    for bin in "$BUILD/src/net/wpatest" \
               "$BUILD/apps/printerdemo/printerdemo" \
               "$BUILD/apps/preferencedemo/preferencedemo"; do
        [ -x "$bin" ] && setcap "$CAPS" "$bin" && echo "capped: $bin"
    done

    echo "bench up: AP=$SSID on $IF_AP, supplicant ctrl=$CTRL_DIR/$IF_STA, dhcp pool=$DHCP_POOL"
    echo "server log: $TESTDIR/dnsmasq.log   (leases: $LEASES)"
    echo "next: export WPA_CTRL_PATH=$CTRL_DIR/$IF_STA"
    echo "      $BUILD/src/net/wpatest --ctrl $CTRL_DIR/$IF_STA connect $SSID $PSK"
}

status_bench() {
    for name in hostapd wpa dnsmasq; do
        pid="$(cat "$TESTDIR/$name.pid" 2>/dev/null || true)"
        if [ -n "$pid" ] && [ -d "/proc/$pid" ]; then
            echo "$name: running (pid $pid)"
        else
            echo "$name: DOWN"
        fi
    done
    echo "--- supplicant"
    wpa_cli -p "$CTRL_DIR" status 2>/dev/null | grep -E "wpa_state|ssid=" || echo "(no ctrl socket)"
    echo "--- leases"
    cat "$LEASES" 2>/dev/null || echo "(none)"
    echo "--- addresses"
    ip -4 -br addr show "$IF_STA" "$IF_AP" 2>/dev/null || true
    echo "--- caps"
    for bin in "$BUILD/src/net/wpatest" "$BUILD/apps/printerdemo/printerdemo"; do
        [ -x "$bin" ] && getcap "$bin"
    done
    echo "--- dhcp server log (last 3)"
    tail -3 "$TESTDIR/dnsmasq.log" 2>/dev/null || echo "(no log yet)"
}

cap_only() {
    for bin in "$BUILD/src/net/wpatest"                "$BUILD/apps/printerdemo/printerdemo"                "$BUILD/apps/preferencedemo/preferencedemo"; do
        [ -x "$bin" ] && setcap "$CAPS" "$bin" && echo "capped: $bin"
    done
    echo "hint: export WPA_CTRL_PATH=$CTRL_DIR/$IF_STA"
}

case "$MODE" in
    --start) start_bench ;;
    --caps) cap_only ;;
    --fresh) stop_daemons; rm -f "$TESTDIR/wpa_supplicant.conf"; start_bench ;;
    --stop)  stop_daemons; echo "bench down (caps + firewall rule left as-is)" ;;
    --status) status_bench ;;
    *) echo "usage: $0 [--start|--stop|--fresh|--status]"; exit 2 ;;
esac
