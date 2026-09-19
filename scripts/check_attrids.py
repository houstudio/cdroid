#!/usr/bin/env python3
"""check_attrids.py — guard the attr id tables against the compiled framework arsc.

The 0x010d private-attr block ids are ASSIGNED BY aapt2 (alphabetical, not
pinned) whenever src/gui/res/values/attrs.xml gains/loses private attrs. The
id tables (framework_attrids.txt / cdroid_attrids.txt) are hand-snapshotted
inputs to gen_styleable.py — when they drift from what aapt2 actually
assigned, every styleable array silently carries a NEIGHBOR's attr id and
binary AXML reads resolve the wrong theme entry (the 2026-08-15 TimePicker
crash: legacyLayout -> lightZ).

This script is the build-time tripwire: it dumps the real arsc (aapt2 dump
resources on framework.apk) and cross-checks every table entry. Any mismatch
fails the build. With --fix it rewrites the drifted entries instead.

Usage:
  check_attrids.py --framework-apk framework.apk [--aapt2 aapt2]
                   [--table scripts/framework_attrids.txt ...] [--fix]
"""
import argparse
import re
import shutil
import subprocess
import sys

RESOURCE_RE = re.compile(
    r'^\s*resource (0x[0-9a-fA-F]{8}) (?:\^)?attr(?:-private)?/(\w+)')


def dump_attr_ids(aapt2: str, apk: str) -> dict:
    """aapt2 dump resources -> {attr name: id}."""
    out = subprocess.run([aapt2, 'dump', 'resources', apk],
                         capture_output=True, text=True, check=True).stdout
    attrs = {}
    for line in out.splitlines():
        m = RESOURCE_RE.match(line)
        if m:
            attrs[m.group(2)] = m.group(1).lower()
    if not attrs:
        raise RuntimeError('no attr entries parsed from %s' % apk)
    return attrs


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument('--framework-apk', required=True)
    ap.add_argument('--aapt2', default='aapt2')
    ap.add_argument('--table', action='append', required=True,
                    help='id table (0xID NAME lines) to verify; repeatable')
    ap.add_argument('--fix', action='store_true',
                    help='rewrite drifted entries from the arsc instead of failing')
    args = ap.parse_args()

    if shutil.which(args.aapt2) is None:
        print('check_attrids: aapt2 not found (%s) — skipping' % args.aapt2)
        return 0
    arsc = dump_attr_ids(args.aapt2, args.framework_apk)

    bad = 0
    for table in args.table:
        lines = open(table).readlines()
        changed = False
        for i, line in enumerate(lines):
            m = re.match(r'(0x[0-9a-fA-F]{8}) (\w+)\s*$', line)
            if not m:
                continue
            cid, name = m.group(1).lower(), m.group(2)
            real = arsc.get(name)
            if real is None:
                # Attr not compiled into the framework arsc (e.g. widgetEx-only
                # or app-package attrs) — the arsc can't validate it.
                continue
            if real != cid:
                bad += 1
                if args.fix:
                    lines[i] = '%s %s\n' % (real, name)
                    changed = True
                print('check_attrids: %s: %s drifts: table=%s arsc=%s'
                      % (table, name, cid, real))
        if args.fix and changed:
            open(table, 'w').writelines(lines)
            print('check_attrids: rewrote drifted entries in %s' % table)

    if bad and not args.fix:
        print('check_attrids: FAILED — %d drifted attr id(s). Regenerate the '
              'table (scripts/gen_framework_attrids.sh or --fix) or the '
              'styleable arrays will resolve NEIGHBOR attrs.' % bad)
        return 1
    if not bad:
        print('check_attrids: OK (validated against %s)' % args.framework_apk)
    return 0


if __name__ == '__main__':
    sys.exit(main())
