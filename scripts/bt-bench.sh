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

case "${1:-start}" in
start)
    if [ ! -x "$BLUEZ/emulator/btvirt" ] || [ ! -x "$BLUEZ/src/bluetoothd" ]; then
        echo "bluez build tree not found under $BLUEZ (set BLUEZ=<dir>)" >&2
        exit 1
    fi
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

    modprobe hci_vhci
    chmod 666 $VHCI_DEV 2>/dev/null || true

    # btvirt: -l2 = 两个本地控制器(经典+LE 双模),互为对端
    $BLUEZ/emulator/btvirt -d -l2 > $PIDDIR/btvirt.log 2>&1 &
    echo $! > $PIDDIR/btvirt.pid
    sleep 1

    # bluetoothd 直接挂系统总线
    $BLUEZ/src/bluetoothd -n -d > $PIDDIR/bluetoothd.log 2>&1 &
    echo $! > $PIDDIR/bluetoothd.pid
    sleep 1

    echo "== hci 控制器 =="
    ls /sys/class/bluetooth/ || true
    echo "== bluetoothd 状态 =="
    tail -3 $PIDDIR/bluetoothd.log || true
    echo "台架就绪。回归:./bttest state && ./bttest discover on && ./bttest listen 5"
    ;;
stop)
    for f in bluetoothd btvirt dbus; do
        [ -f $PIDDIR/$f.pid ] && kill $(cat $PIDDIR/$f.pid) 2>/dev/null || true
    done
    pkill -f 'bluetoothd -n' 2>/dev/null || true
    pkill -x btvirt 2>/dev/null || true
    echo stopped
    ;;
*)
    echo "usage: $0 [start|stop]"; exit 1;;
esac
