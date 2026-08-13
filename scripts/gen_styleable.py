#!/usr/bin/env python3
"""gen_styleable.py — generate a C++ styleable header from an attrs.xml.

Parses <declare-styleable> blocks, resolves each attr's resource ID, and emits
the fw_attr::* (attr ID constants) + styleable::* (index enums + IDS[] arrays)
used by CDROID's obtainStyledAttributesTyped / TypedArray path.

ID resolution:
  * framework (android:) attrs   -> looked up in --fw-ids (android.jar map)
  * custom (app/cdroid:) attrs   -> looked up / auto-assigned in --custom-ids
    (0x7f010000+, append-only stable; the aapt2 public.xml equivalent)

The fw_attr constants are SELF-CONTAINED per styleable: each styleable's
fw_attr namespace holds its own attrs (shared attrs like gravity are duplicated
across namespaces — harmless constexpr). Widget code references only the
styleable::* index enums (by name), never fw_attr:: directly, so this is safe.

A --name-map file maps attrs.xml declare-styleable names to output C++ names
(e.g. LinearLayout_Layout -> LinearLayoutLayout); absent entries are identity.
"""
import argparse
import os
import re
import sys
import xml.etree.ElementTree as ET

CUSTOM_ID_BASE = 0x02010000


def load_id_map(path):
    """Load a '0xID NAME' (or 'NAME 0xID') file -> {name: int_id}."""
    m = {}
    if not path or not os.path.exists(path):
        return m
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            # accept "0xID NAME" or "NAME 0xID"
            tok = line.split()
            if len(tok) != 2:
                continue
            if tok[0].startswith('0x'):
                idv, name = tok[0], tok[1]
            else:
                name, idv = tok[0], tok[1]
            m[name] = int(idv, 16)
    return m


def load_name_map(path):
    """Load a 'attrsname outname' file -> {attrsname: outname}."""
    m = {}
    if not path or not os.path.exists(path):
        return m
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            tok = line.split()
            if len(tok) == 2:
                m[tok[0]] = tok[1]
    return m


def load_include_file(path):
    """Load an include file -> [styleable name, ...] (one per line; '#' lines
    and blank lines ignored)."""
    names = []
    if not path or not os.path.exists(path):
        return names
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            names.append(line)
    return names



def parse_attrs_xml(path):
    """Return {declare-styleable name: [attr names in order]}.

    Attr names have any 'android:' namespace prefix stripped (framework ref).
    <enum>/<flag> children are skipped — only direct <attr> children count.
    """
    out = {}
    tree = ET.parse(path)
    root = tree.getroot()
    for ds in root.findall('declare-styleable'):
        name = ds.get('name')
        if not name:
            continue
        attrs = []
        for a in ds.findall('attr'):
            an = a.get('name')
            if an and ':' in an:
                an = an.split(':', 1)[1]
            if an:
                attrs.append(an)
        out[name] = attrs
    return out


def resolve_id(attr_name, fw_ids, custom_ids, custom_next, auto_custom):
    """Return (id_int|None, custom_assigned_bool, new_custom_next).

    None => skip this attr (e.g. framework placeholder '__removed*' attrs that
    have no resource id) — only happens when auto_custom is False (framework
    mode). When auto_custom is True (custom-ids given), unknown attrs are
    assigned a new custom id rather than skipped."""
    if attr_name in fw_ids:
        return fw_ids[attr_name], False, custom_next
    if attr_name in custom_ids:
        return custom_ids[attr_name], False, custom_next
    if auto_custom:
        cid = custom_next
        return cid, True, custom_next + 1
    return None, False, custom_next  # framework placeholder/unknown — skip


def cident(name):
    """attr/styleable name -> C++ identifier (illegal chars -> _)."""
    return re.sub(r'[^0-9A-Za-z_]', '_', name)


def wrap_items(items, width=88):
    """Join `items` into comma-separated line strings, wrapping so each line's
    item text stays <= `width` chars. Yields ~2-3 short entries per line (1-2 for
    long qualified names) instead of one unreadable giant line. Returns a list of
    line strings (no trailing commas — the caller adds them)."""
    lines = []
    cur = ''
    for it in items:
        if not cur:
            cur = it
        elif len(cur) + 2 + len(it) <= width:   # +2 for ', '
            cur = cur + ', ' + it
        else:
            lines.append(cur)
            cur = it
    if cur:
        lines.append(cur)
    return lines


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--attrs', required=True)
    ap.add_argument('--fw-ids', required=True)
    ap.add_argument('--custom-ids', default=None)
    ap.add_argument('--name-map', default=None)
    ap.add_argument('--include', default=None,
                    help='comma list of OUTPUT styleable names to emit (default: all)')
    ap.add_argument('--include-file', default=None,
                    help='file with one OUTPUT styleable name per line '
                         '(# comments and blank lines ignored); merged with --include')
    ap.add_argument('--out-h', required=True)
    ap.add_argument('--out-cc', required=True)
    ap.add_argument('--guard', default='__GENERATED_STYLEABLE_H__')
    ap.add_argument('--header', default=None,
                    help='header filename for the .cc #include (default: basename of out-h)')
    args = ap.parse_args()

    fw_ids = load_id_map(args.fw_ids)
    name_map = load_name_map(args.name_map)
    # --attrs accepts a comma-separated list (per-component attrs.xml files);
    # merge their declare-styleables. A styleable defined in two files is a
    # build misconfiguration — warn and keep the first.
    ds_attrs = {}
    for attrs_path in [p.strip() for p in args.attrs.split(',') if p.strip()]:
        for k, v in parse_attrs_xml(attrs_path).items():
            if k in ds_attrs:
                print(f"warning: declare-styleable '{k}' duplicated; {attrs_path} ignored",
                      file=sys.stderr)
                continue
            ds_attrs[k] = v

    # custom-ids: load existing + find next free id
    custom_ids = load_id_map(args.custom_ids)
    custom_next = CUSTOM_ID_BASE
    if custom_ids:
        custom_next = max(custom_ids.values()) + 1
    auto_custom = args.custom_ids is not None
    new_custom = {}  # name -> id, freshly assigned this run

    # Determine output styleables: map output-name -> attrs-xml declare-styleable name
    # name_map is {attrsname: outname}; build reverse.
    rev = {v: k for k, v in name_map.items()}
    # Merge --include-file (one per line) with --include (csv); file first.
    # When neither is given, emit all declare-styleables (applying name_map).
    inc_names = load_include_file(args.include_file)
    if args.include:
        inc_names += [s.strip() for s in args.include.split(',') if s.strip()]
    if inc_names:
        seen = set()
        out_names = [n for n in inc_names if not (n in seen or seen.add(n))]
    else:
        # all, applying name_map forward
        out_names = [name_map.get(k, k) for k in ds_attrs]

    # Build per-styleable resolved data
    styleables = []  # (out_name, [(attr_name, id_int), ...])
    for out_name in out_names:
        attrs_name = rev.get(out_name, out_name)
        if attrs_name not in ds_attrs:
            print(f"warning: '{attrs_name}' (for output '{out_name}') not in {args.attrs}; skipped",
                  file=sys.stderr)
            continue
        resolved = []
        for an in ds_attrs[attrs_name]:
            idv, assigned, custom_next = resolve_id(an, fw_ids, custom_ids, custom_next, auto_custom)
            if idv is None:
                print(f"skip: attr '{an}' (in {out_name}) has no id — placeholder/unknown",
                      file=sys.stderr)
                continue
            if assigned:
                custom_ids[an] = idv
                new_custom[an] = idv
            resolved.append((an, idv))
        styleables.append((out_name, resolved))

    # Persist newly-assigned custom ids (rewritten below, after .cc emit)
    header_basename = args.header or os.path.basename(args.out_h)

    # ---- emit .h ----
    L = []
    L.append('// GENERATED by gen_styleable.py — do not edit by hand.')
    attrs_src = ', '.join(os.path.basename(p.strip()) for p in args.attrs.split(',') if p.strip())
    L.append(f'// Source: {attrs_src}  (fw-ids: {os.path.basename(args.fw_ids)})')
    L.append(f'#ifndef {args.guard}')
    L.append(f'#define {args.guard}')
    L.append('#include <cstdint>')
    L.append('#include <cstddef>')
    L.append('namespace cdroid {')
    L.append('namespace internal { namespace R { namespace styleable {')
    for out_name, resolved in styleables:
        sn = cident(out_name)
        n_attrs = len(resolved) if resolved else 0
        L.append(f'    // {out_name} ({n_attrs} attrs)')
        L.append(f'    extern const uint32_t {sn}[];')
        if resolved:
            for idx, (an, _) in enumerate(resolved):
                L.append(f'    constexpr int {sn}_{cident(an)} = {idx};')
        else:
            L.append(f'    constexpr int {sn}___none = 0;')
        L.append('')  # blank line after each styleable (readability)
    L.append('} } } // namespace internal::R::styleable')
    L.append('} // namespace cdroid')
    L.append(f'#endif // {args.guard}')
    with open(args.out_h, 'w') as f:
        f.write('\n'.join(L) + '\n')

    # ---- emit .cc ----
    C = []
    C.append('// GENERATED by gen_styleable.py — do not edit by hand.')
    C.append(f'#include "{header_basename}"')
    C.append('namespace cdroid {')
    C.append('namespace internal { namespace R { namespace styleable {')
    for out_name, resolved in styleables:
        sn = cident(out_name)
        entries = [f'0x{idv:08x}' for _, idv in resolved]
        entries.append('0')   # trailing sentinel
        wl = wrap_items(entries)
        if len(wl) == 1:
            C.append(f'    const uint32_t {sn}[] = {{ {wl[0]} }};')
        else:
            C.append(f'    const uint32_t {sn}[] = {{')
            for bl in wl[:-1]:
                C.append(f'        {bl},')
            C.append(f'        {wl[-1]} }};')
    C.append('} } } // namespace internal::R::styleable')
    C.append('} // namespace cdroid')
    with open(args.out_cc, 'w') as f:
        f.write('\n'.join(C) + '\n')

    # ---- persist custom ids cleanly (rewrite whole file: existing + new, id-sorted) ----
    if args.custom_ids and custom_ids:
        with open(args.custom_ids, 'w') as f:
            f.write('# Custom (app/cdroid) attr name -> resource ID. Append-only stable.\n')
            f.write('# Single source of truth; aapt2 public.xml will pin the same IDs.\n')
            f.write('# Format: 0xID NAME\n\n')
            for n in sorted(custom_ids, key=lambda k: custom_ids[k]):
                f.write(f'0x{custom_ids[n]:08x} {n}\n')

    print(f"generated {args.out_h} + {args.out_cc}: {len(styleables)} styleables"
          + (f", {len(new_custom)} new custom ids" if new_custom else ""))


if __name__ == '__main__':
    main()
