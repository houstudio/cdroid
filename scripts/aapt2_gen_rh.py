#!/usr/bin/env python3
# ----------------------------------------------------------------------------
# aapt2_gen_rh.py — generate R.h from aapt2 dump resources (arsc real IDs).
#
# Replaces idgen.py's dict2RH: reads the actual resource IDs that aapt2 baked
# into the arsc (0x01020000 framework / 0x7f020000 app), so R::id::xxx matches
# what runtime arsc resolution returns.
#
# Flags:
#   --only-public true/false  true  = ONLY public resources.
#                             false = ALL resources (public + private).
#                             Default: false.
#   --namespace <ns>          Top-level C++ namespace. Accepts qualified names
#                             like "cdroid::internal" → wraps in <ns>::internal::R.
#                             Plain "cdroid" or "printerdemo" → <ns>::R.
#   --attrs <app attrs.xml>   Optional. Emit R::styleable from the app's
#                             <declare-styleable> blocks: index constants +
#                             inline id arrays (header-only, static per TU).
#                             App attrs resolve from the dumped arsc (own 0x7f
#                             ids); android:-prefixed attrs resolve from
#                             --fw-ids (+ sibling cdroid_attrids.txt override).
#   --fw-ids <map file>       '0xID NAME' map for framework attr ids, used only
#                             with --attrs (android:-prefixed styleable attrs).
#
# Framework (cdroid) build calls this TWICE via pakbuilder:
#   1. --only-public true  --namespace cdroid           -o R.h
#   2.                        --namespace cdroid::internal -o internal_R.h
#
# App build calls once:
#   aapt2_gen_rh.py app.apk --namespace printerdemo -o R.h   → printerdemo::R (ALL)
#
# App build calls once (no flags):
#   aapt2_gen_rh.py app.apk -o R.h    → app::R (all app resources)
# ----------------------------------------------------------------------------
import sys, os, re, argparse, subprocess
import xml.etree.ElementTree as ET

RES_RE = re.compile(r'resource (0x[0-9a-fA-F]{8})\s+([^/\s]+)/(\S+)')

def load_id_map(path):
    """Load a '0xID NAME' file -> {name: '0xID'} ('' entries skipped)."""
    m = {}
    if not path or not os.path.exists(path):
        return m
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith('#'):
                continue
            tok = line.split()
            if len(tok) == 2 and tok[0].startswith('0x'):
                m[tok[1]] = tok[0]
    return m

def parse_styleables(path):
    """attrs.xml -> [(styleable_name, [attr_name, ...])] preserving order.
    Attr names keep their 'android:' prefix when present."""
    out = []
    root = ET.parse(path).getroot()
    for ds in root.findall('declare-styleable'):
        name = ds.get('name')
        if not name:
            continue
        attrs = []
        for a in ds.findall('attr'):
            an = a.get('name')
            if an:
                attrs.append(an)
        out.append((name, attrs))
    return out

CPP_KEYWORDS = {
    'alignas', 'alignof', 'and', 'and_eq', 'asm', 'auto', 'bitand', 'bitor',
    'bool', 'break', 'case', 'catch', 'char', 'char8_t', 'char16_t', 'char32_t',
    'class', 'compl', 'concept', 'const', 'consteval', 'constexpr', 'constinit',
    'const_cast', 'continue', 'co_await', 'co_return', 'co_yield', 'decltype',
    'default', 'delete', 'do', 'double', 'dynamic_cast', 'else', 'enum',
    'explicit', 'export', 'extern', 'false', 'float', 'for', 'friend', 'goto',
    'if', 'inline', 'int', 'long', 'mutable', 'namespace', 'new', 'noexcept',
    'not', 'not_eq', 'nullptr', 'operator', 'or', 'or_eq', 'private',
    'protected', 'public', 'register', 'reinterpret_cast', 'requires', 'return',
    'short', 'signed', 'sizeof', 'static', 'static_assert', 'static_cast',
    'struct', 'switch', 'template', 'this', 'thread_local', 'throw', 'true',
    'try', 'typedef', 'typeid', 'typename', 'union', 'unsigned', 'using',
    'virtual', 'void', 'volatile', 'wchar_t', 'while', 'xor', 'xor_eq',
}

TYPE_RENAME = {'bool': 'boolean'}

def cident(name):
    s = re.sub(r'[^0-9A-Za-z_]', '_', name)
    if s and s[0].isdigit(): s = '_' + s
    if s in CPP_KEYWORDS: s = s + '_'
    return s

def ctype(rtype):
    return TYPE_RENAME.get(rtype, cident(rtype))

def main():
    ap = argparse.ArgumentParser(description="Generate R.h from aapt2 dump resources.")
    ap.add_argument('apk')
    ap.add_argument('--aapt2', required=True)
    ap.add_argument('--namespace', required=True)
    ap.add_argument('-o', '--output', required=True)
    ap.add_argument('--only-public', choices=['true', 'false'], default='false',
                    help='true = public only; false = ALL. Default: false.')
    ap.add_argument('--attrs', default=None,
                    help='app attrs.xml with declare-styleables -> emit R::styleable')
    ap.add_argument('--fw-ids', default=None,
                    help="'0xID NAME' framework attr id map (for android: styleable attrs)")
    args = ap.parse_args()
    only_public = (args.only_public == 'true')
    # Detect ::internal qualifier in namespace → cdroid::internal → wrap in internal::R
    ns_parts = args.namespace.split('::')
    internal = (len(ns_parts) >= 2 and ns_parts[-1] == 'internal')
    ns_root = '::'.join(ns_parts[:-1]) if internal else args.namespace

    r = subprocess.run([args.aapt2, 'dump', 'resources', args.apk],
                       capture_output=True, text=True)
    if r.returncode != 0:
        sys.stderr.write("aapt2 dump failed: %s\n" % r.stderr[:500])
        sys.exit(1)

    by_type = {}  # ctype -> {cident_key: hex_id}
    for line in r.stdout.splitlines():
        m = RES_RE.search(line)
        if not m or '$' in m.group(3):
            continue
        rid, rtype, rkey = m.group(1), m.group(2), m.group(3)
        is_public = ' PUBLIC' in line
        if only_public and not is_public:
            continue  # skip private when --only-public
        ck = cident(rkey)
        tns = 'attr' if rtype == '^attr-private' else ctype(rtype)
        by_type.setdefault(tns, {})[ck] = rid

    # Write output
    with open(args.output, 'w') as f:
        f.write('#pragma once\n\n/*Generated by aapt2_gen_rh.py, do not edit*/\n\n')
        f.write('namespace %s{\n\n' % ns_root)
        if internal:
            f.write('namespace internal{\n')
        f.write('namespace R{\n')
        for tns in sorted(by_type):
            f.write('    namespace %s{\n' % tns)
            f.write('        enum : int {\n')
            for key in sorted(by_type[tns]):
                f.write('            %s = %s,\n' % (key, by_type[tns][key]))
            f.write('        };\n')
            f.write('    }/*namespace %s*/\n\n' % tns)

        # R::styleable from the app's declare-styleables (--attrs). Arrays are
        # static per TU (header-only; the app is one executable — no ODR concern).
        if args.attrs and os.path.exists(args.attrs):
            fw = load_id_map(args.fw_ids)
            # cdroid_attrids.txt (sibling of --fw-ids) overrides: cdroid-private
            # attrs are pinned there at their real framework ids.
            fw.update(load_id_map(os.path.join(
                os.path.dirname(os.path.abspath(args.fw_ids or '.')),
                'cdroid_attrids.txt')))
            app_attrs = by_type.get('attr', {})
            styleables = parse_styleables(args.attrs)
            f.write('    namespace styleable{\n')
            emitted = 0
            for sname, attrs in styleables:
                ids = []
                ok = True
                for an in attrs:
                    if an.startswith('android:'):
                        key = cident(an.split(':', 1)[1])
                        idv = fw.get(key)
                        if idv is None:
                            sys.stderr.write("styleable %s: framework attr '%s' "
                                             "not in fw-ids map — skipped\n"
                                             % (sname, an))
                            ok = False
                            break
                        ids.append(idv)
                    else:
                        idv = app_attrs.get(cident(an))
                        if idv is None:
                            sys.stderr.write("styleable %s: app attr '%s' not in "
                                             "arsc dump — skipped (not linked?)\n"
                                             % (sname, an))
                            ok = False
                            break
                        ids.append(idv)
                if not ok:
                    continue
                ids.append('0')  # sentinel, matching cdroid styleable arrays
                sn = cident(sname)
                f.write('        // %s (%d attrs)\n' % (sname, len(attrs)))
                f.write('        static const uint32_t %s[] = {%s};\n'
                        % (sn, ', '.join(ids)))
                for i, an in enumerate(attrs):
                    f.write('        constexpr int %s_%s = %d;\n'
                            % (sn, cident(an.split(':', 1)[-1]), i))
                f.write('\n')
                emitted += 1
            f.write('    }/*namespace styleable*/\n\n')
            sys.stderr.write('aapt2_gen_rh: %d/%d styleables emitted\n'
                             % (emitted, len(styleables)))

        f.write('};//endof namespace R\n\n')
        if internal:
            f.write('}//namespace internal\n')
        f.write('}//endof namespace\n')

    count = sum(len(v) for v in by_type.values())
    scope = '%s::internal::R' % ns_root if internal else '%s::R' % ns_root
    pub = ' (public only)' if only_public else ' (ALL)'
    sys.stderr.write('aapt2_gen_rh: %d types/%d resources%s -> %s\n'
                     % (len(by_type), count, pub, scope))

if __name__ == '__main__':
    main()
