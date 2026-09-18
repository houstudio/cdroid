#!/usr/bin/env python3
"""
进程内存监控工具
- 支持按进程名或 PID 监控
- 定时采集 RSS / PSS / Private_Dirty 等指标
- 输出 CSV + 趋势图
"""

#现象	判断
#PSS 持续上升，不回落	可能存在内存泄漏
#只有匿名内存（Anon）增长	堆分配未释放，代码层面问题
#RSS 增长但 PSS 稳定	共享库页面增多，通常正常
#锯齿形波动	正常的分配-释放循环
#最简单的做法，把监控终端数据甩给AI。直接给你结果 
import os
import sys
import csv
import time
import signal
import argparse
import subprocess
from datetime import datetime
from pathlib import Path


def find_pids_by_name(name):
    """通过进程名查找所有匹配的 PID"""
    pids = []
    for entry in Path("/proc").iterdir():
        if not entry.name.isdigit():
            continue
        try:
            comm = (entry / "comm").read_text().strip()
            if comm == name:
                pids.append(int(entry.name))
        except (PermissionError, FileNotFoundError, ProcessLookupError):
            continue
    return sorted(pids)


def parse_smaps_rollup(pid):
    """解析 /proc/[pid]/smaps_rollup，返回关键内存指标（单位 kB）"""
    filepath = f"/proc/{pid}/smaps_rollup"
    metrics = {}
    try:
        with open(filepath) as f:
            for line in f:
                parts = line.split()
                if len(parts) >= 2 and parts[0].endswith(":"):
                    key = parts[0].rstrip(":")
                    try:
                        metrics[key] = int(parts[1])
                    except ValueError:
                        pass
    except (PermissionError, FileNotFoundError, ProcessLookupError):
        return None
    return metrics


def get_process_name(pid):
    """获取进程名"""
    try:
        with open(f"/proc/{pid}/comm") as f:
            return f.read().strip()
    except (PermissionError, FileNotFoundError, ProcessLookupError):
        return None


def monitor(pid, interval, duration, output_dir):
    """主监控循环"""
    proc_name = get_process_name(pid)
    if not proc_name:
        print(f"错误：进程 {pid} 不存在或无权限访问")
        sys.exit(1)

    # 输出文件
    output_dir = Path(output_dir)
    output_dir.mkdir(parents=True, exist_ok=True)
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    csv_file = output_dir / f"mem_{proc_name}_{pid}_{timestamp}.csv"
    png_file = output_dir / f"mem_{proc_name}_{pid}_{timestamp}.png"

    print(f"监控目标: {proc_name} (PID={pid})")
    print(f"采集间隔: {interval}s | 总时长: {duration}s")
    print(f"CSV 输出: {csv_file}")
    print(f"图表输出: {png_file}")
    print(f"按 Ctrl+C 提前结束\n")
    print(f"{'时间':>12s}  {'RSS(MB)':>9s}  {'PSS(MB)':>9s}  {'Anon(MB)':>9s}  {'File(MB)':>9s}  {'Priv_D(MB)':>11s}")
    print("-" * 78)

    # CSV 写入
    csv_f = open(csv_file, "w", newline="")
    writer = csv.writer(csv_f)
    writer.writerow(["timestamp", "rss_kb", "pss_kb", "pss_anon_kb", "pss_file_kb",
                      "private_dirty_kb", "private_clean_kb", "shared_clean_kb", "swap_kb"])

    start_time = time.time()
    records = []

    # 优雅退出
    running = True

    def handle_signal(sig, frame):
        nonlocal running
        running = False
        print("\n\n收到中断信号，正在保存数据...")

    signal.signal(signal.SIGINT, handle_signal)
    signal.signal(signal.SIGTERM, handle_signal)

    try:
        while running:
            elapsed = time.time() - start_time
            if duration > 0 and elapsed > duration:
                break

            metrics = parse_smaps_rollup(pid)
            if metrics is None:
                print(f"\n进程 {pid} 已退出，停止监控")
                break

            now = datetime.now().strftime("%H:%M:%S")
            rss = metrics.get("Rss", 0)
            pss = metrics.get("Pss", 0)
            pss_anon = metrics.get("Pss_Anon", 0)
            pss_file = metrics.get("Pss_File", 0)
            priv_dirty = metrics.get("Private_Dirty", 0)
            priv_clean = metrics.get("Private_Clean", 0)
            shared_clean = metrics.get("Shared_Clean", 0)
            swap = metrics.get("Swap", 0)

            row = [now, rss, pss, pss_anon, pss_file, priv_dirty, priv_clean, shared_clean, swap]
            writer.writerow(row)
            csv_f.flush()

            records.append(row)

            print(f"{now:>12s}  {rss/1024:>9.1f}  {pss/1024:>9.1f}  {pss_anon/1024:>9.1f}  "
                  f"{pss_file/1024:>9.1f}  {priv_dirty/1024:>11.1f}")

            # 最后一次不 sleep
            if duration > 0 and elapsed + interval >= duration:
                break
            time.sleep(interval)

    finally:
        csv_f.close()
        print(f"\n共采集 {len(records)} 条记录，已保存到 {csv_file}")

        # 生成趋势图
        if len(records) >= 2:
            try:
                plot_trend(records, str(png_file), proc_name, pid)
                print(f"趋势图已保存到 {png_file}")
            except ImportError:
                print("提示：未安装 matplotlib，跳过图表生成。可执行: pip install matplotlib")


def plot_trend(records, png_file, proc_name, pid):
    """生成内存趋势图"""
    import matplotlib
    matplotlib.use("Agg")  # 无 GUI 环境
    import matplotlib.pyplot as plt
    import matplotlib.dates as mdates

    times = []
    rss_vals = []
    pss_vals = []
    anon_vals = []
    dirty_vals = []

    for row in records:
        times.append(datetime.strptime(row[0], "%H:%M:%S"))
        rss_vals.append(row[1] / 1024)      # kB -> MB
        pss_vals.append(row[2] / 1024)
        anon_vals.append(row[3] / 1024)
        dirty_vals.append(row[5] / 1024)

    fig, ax = plt.subplots(figsize=(12, 5))
    ax.plot(times, rss_vals, label="RSS", linewidth=1.5, alpha=0.8)
    ax.plot(times, pss_vals, label="PSS (独占)", linewidth=1.5, alpha=0.8)
    ax.plot(times, anon_vals, label="匿名内存(堆/栈)", linewidth=1.2, alpha=0.7, linestyle="--")
    ax.plot(times, dirty_vals, label="Private_Dirty", linewidth=1.2, alpha=0.7, linestyle=":")

    ax.set_title(f"内存监控: {proc_name} (PID={pid})", fontsize=14)
    ax.set_xlabel("时间")
    ax.set_ylabel("内存 (MB)")
    ax.legend(loc="upper left", fontsize=9)
    ax.grid(True, alpha=0.3)
    ax.xaxis.set_major_formatter(mdates.DateFormatter("%H:%M:%S"))
    fig.autofmt_xdate(rotation=30)
    plt.tight_layout()
    plt.savefig(png_file, dpi=150)
    plt.close(fig)


def main():
    parser = argparse.ArgumentParser(
        description="进程内存监控工具 — 定时采集 PSS/RSS 并生成趋势图",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
示例:
  # 按进程名监控，每 5 秒采集一次，持续 600 秒
  %(prog)s -n printerdemo -i 5 -d 600

  # 按 PID 监控，每 1 秒采集一次，不限时长（Ctrl+C 停止）
  %(prog)s -p 1618454 -i 1

  # 指定输出目录
  %(prog)s -n printerdemo -o /tmp/mem_logs
        """,
    )

    group = parser.add_mutually_exclusive_group(required=True)
    group.add_argument("-n", "--name", help="按进程名匹配（支持多实例，全部监控）")
    group.add_argument("-p", "--pid", type=int, help="按 PID 监控")

    parser.add_argument("-i", "--interval", type=float, default=5,
                        help="采集间隔，单位秒（默认 5）")
    parser.add_argument("-d", "--duration", type=float, default=0,
                        help="总监控时长，单位秒（默认 0 = 不限，Ctrl+C 停止）")
    parser.add_argument("-o", "--output", default=".",
                        help="输出目录（默认当前目录）")

    args = parser.parse_args()

    # 确定要监控的 PID 列表
    if args.pid:
        pids = [args.pid]
        if not Path(f"/proc/{args.pid}").exists():
            print(f"错误：PID {args.pid} 不存在")
            sys.exit(1)
    else:
        pids = find_pids_by_name(args.name)
        if not pids:
            print(f"错误：未找到名为 '{args.name}' 的进程")
            sys.exit(1)
        print(f"找到 {len(pids)} 个匹配进程: {pids}")

    # 启动监控（多进程时逐个监控，这里简化为监控第一个）
    # 如果需要同时监控多个，可以 fork 子进程或用多线程
    for pid in pids:
        monitor(pid, args.interval, args.duration, args.output)


if __name__ == "__main__":
    main()
