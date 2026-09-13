#!/bin/bash
# cdblue 无硬件蓝牙台架:vhci 虚拟控制器 + btvirt + bluetoothd
# (from /tmp/bt-bench.sh, made durable; the /tmp copy stays the working one
#  until the bluez build tree below is relocated somewhere permanent).
#
# 用法: sudo bash scripts/bt-bench.sh [start|stop]
#  start:
#    1) 写系统总线 policy(root 可拥有 org.bluez,所有人可与之对话)
#    2) modprobe hci_vhci(虚拟控制器内核模块)
#    3) btvirt -l2 建两个本地双模控制器(互为对端 —— 扫描/配对测试的对端即彼此)
#    4) bluetoothd 挂系统总线(前台调试)
#  可覆写环境变量:
#    BLUEZ   bluez 源码/构建树(须含 emulator/btvirt 与 src/bluetoothd)
#    PIDDIR  运行时 pid/日志目录
#  回归直接 ./bttest state(默认系统总线,无需环境变量)
set -e
BLUEZ=${BLUEZ:-/tmp/bluez-5.79}
VHCI_DEV=/dev/vhci
PIDDIR=${PIDDIR:-/tmp/bt-bench}

stop_bench() {
    for f in bluetoothd btvirt dbus; do
        [ -f $PIDDIR/$f.pid ] && kill $(cat $PIDDIR/$f.pid) 2>/dev/null || true
    done
    pkill -f 'bluetoothd -n' 2>/dev/null || true
    pkill -x btvirt 2>/dev/null || true
    [ -f $PIDDIR/dnsmasq-pan.pid ] && kill $(cat $PIDDIR/dnsmasq-pan.pid) 2>/dev/null || true
    ip link del bt-pan 2>/dev/null || true
    # Wait for the daemons to actually exit: bluetoothd holds raw hci
    # sockets and btvirt its /dev/vhci fds; rmmod below races them (and
    # silently fails, leaving the zombie controllers) if we rush.
    for i in $(seq 1 20); do
        pgrep -x btvirt >/dev/null || pgrep -f 'bluetoothd -n' >/dev/null || break
        sleep 0.5
    done
    # A dead btvirt leaves its kernel vhci controllers behind (zombie hciN
    # with no live peer); bluetoothd then defaults to the FIRST adapter —
    # the zombie — and every discovery finds nothing. Unloading the module
    # clears all of them; start re-modprobes a clean set.
    rmmod hci_vhci 2>/dev/null || true
    if ls /sys/class/bluetooth 2>/dev/null | grep -q hci; then
        echo "WARN: hci controllers survived the unload (kernel holds them;" >&2
        echo "      a reboot is the only cleaner)." >&2
    fi
}

case "${1:-start}" in
start)
    if [ ! -x "$BLUEZ/emulator/btvirt" ] || [ ! -x "$BLUEZ/src/bluetoothd" ]; then
        echo "bluez build tree not found under $BLUEZ (set BLUEZ=<dir>)" >&2
        exit 1
    fi
    # Idempotent start: a leftover bluetoothd keeps owning org.bluez (the
    # new daemon dies with "Name already in use") and duplicate btvirt runs
    # stack controllers — clear any previous instance first.
    stop_bench
    modprobe hci_vhci
    chmod 666 $VHCI_DEV 2>/dev/null || true
    mkdir -p $PIDDIR

    # org.bluez 总线策略(机器没装系统 bluez,默认 policy 拒绝 own)
    cat > /etc/dbus-1/system.d/cdblue-bluez.conf <<POLICY
<!DOCTYPE busconfig PUBLIC "-//freedesktop//DTD D-BUS Bus Configuration 1.0//EN"
 "http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd">
<busconfig>
  <policy user="root">
    <allow own="org.bluez"/>
  </policy>
  <policy context="default">
    <allow send_destination="org.bluez"/>
    <allow send_destination="org.bluez" send_interface="org.freedesktop.DBus.ObjectManager"/>
    <allow send_destination="org.bluez" send_interface="org.freedesktop.DBus.Properties"/>
  </policy>
</busconfig>
POLICY

    # btvirt: -l2 = 两个本地控制器(经典+LE 双模),互为对端
    $BLUEZ/emulator/btvirt -d -l2 > $PIDDIR/btvirt.log 2>&1 &
    echo $! > $PIDDIR/btvirt.pid
    sleep 1

    # bluetoothd 直接挂系统总线
    $BLUEZ/src/bluetoothd -n -d > $PIDDIR/bluetoothd.log 2>&1 &
    echo $! > $PIDDIR/bluetoothd.pid
    sleep 1

    # PAN tethering 数据面:bt-pan 桥(BluetoothPan 的 NetworkServer1 目标),
    # bluetoothd 把对端 bnepX 填进桥;DHCP + NAT 走默认路由(hwsim wlan1 同配方)。
    # AOSP 里这半边由 Tethering/netd 做——这里先由台架(root)供给,app 只控制
    # NetworkServer1.Register(cdblue 的 BluetoothPan::setBluetoothTethering)。
    modprobe bnep 2>/dev/null || true
    ip link add bt-pan type bridge 2>/dev/null || true
    ip link set bt-pan up
    ip addr add 192.168.47.1/24 dev bt-pan 2>/dev/null || true
    sysctl -qw net.ipv4.ip_forward=1
    iptables -t nat -C POSTROUTING -s 192.168.47.0/24 ! -o bt-pan -j MASQUERADE 2>/dev/null \
        || iptables -t nat -A POSTROUTING -s 192.168.47.0/24 ! -o bt-pan -j MASQUERADE
    if [ ! -f $PIDDIR/dnsmasq-pan.pid ] || ! kill -0 $(cat $PIDDIR/dnsmasq-pan.pid) 2>/dev/null; then
        dnsmasq --interface=bt-pan --bind-interfaces \
            --dhcp-range=192.168.47.50,192.168.47.60,12h \
            --log-dhcp --log-facility=$PIDDIR/dnsmasq-pan.log \
            --pid-file=$PIDDIR/dnsmasq-pan.pid || echo "dnsmasq-pan failed (see log)"
    fi

    echo "== hci 控制器 =="
    ls /sys/class/bluetooth/ || true
    echo "== bluetoothd 状态 =="
    tail -3 $PIDDIR/bluetoothd.log || true
    echo "台架就绪。回归:./bttest state && ./bttest discover on && ./bttest listen 5"
    ;;
stop)
    stop_bench
    echo stopped
    ;;
*)
    echo "usage: $0 [start|stop]"; exit 1;;
esac
