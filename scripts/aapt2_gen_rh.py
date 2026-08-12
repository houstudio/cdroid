#!/usr/bin/env python3
# ----------------------------------------------------------------------------
# aapt2_gen_rh.py — generate R.h from aapt2 dump resources (arsc real IDs).
#
# Replaces idgen.py's dict2RH: instead of scanning @id/@+id and assigning
# sequential ints (1000+/10000+), this reads the actual resource IDs that aapt2
# baked into the arsc (0x01020000 framework / 0x7f020000 app), so R::id::xxx
# matches what runtime arsc resolution (ResTable::getIdentifier) returns →
# single id source, fixes the idgen-vs-arsc mismatch (LayerDrawable lost layer).
#
# Usage:
#   aapt2_gen_rh.py <apk/pak> --aapt2 <path> --namespace cdroid -o <R.h>
#
# `aapt2 dump resources <apk>` lines look like:
#   " resource 0x01020000 id/background PUBLIC"
# → R::id::background = 0x01020000
# ----------------------------------------------------------------------------
import sys
import os
import re
import argparse
import subprocess

# Match the resource type token up to '/'. Includes '^attr-private' (aapt2's
# marker for private framework attrs): '^' and '-' aren't word chars, so \w+
# silently dropped all 302 private attrs — use [^/\s]+ to capture them too.
RES_RE = re.compile(r'resource (0x[0-9a-fA-F]{8})\s+([^/\s]+)/(\S+)')

# C++ reserved words. aapt2 dumps the FULL framework arsc, which contains
# resource names that are C++ keywords (e.g. id/auto, id/bool, id/default,
# id/signed, string/delete). Using them verbatim as `constexpr int <name>` is a
# hard compile error, so cident() appends a trailing '_' (idiomatic C++).
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


def cident(name):
    """resource key -> C++ identifier (illegal chars -> _, leading digit -> _,
    C++ keyword -> trailing '_')."""
    s = re.sub(r'[^0-9A-Za-z_]', '_', name)
    if s and s[0].isdigit():
        s = '_' + s
    if s in CPP_KEYWORDS:
        s = s + '_'
    return s


# Resource TYPE -> C++ namespace name. Android ships a `<bool>` resource type
# (R.bool.*), but `bool` is a C++ keyword so `namespace bool{}` is a hard error.
# Rename it to `boolean` (the only keyword type the framework defines); any other
# keyword type falls back to cident()'s trailing '_'.
TYPE_RENAME = {'bool': 'boolean'}


def ctype(rtype):
    if rtype in TYPE_RENAME:
        return TYPE_RENAME[rtype]
    return cident(rtype)


def main():
    ap = argparse.ArgumentParser(description="Generate R.h from aapt2 dump resources.")
    ap.add_argument('apk', help='apk/pak with resources.arsc (e.g. cdroid.pak / app.apk)')
    ap.add_argument('--aapt2', required=True, help='aapt2 binary path')
    ap.add_argument('--namespace', required=True, help='C++ namespace (cdroid / app name)')
    ap.add_argument('-o', '--output', required=True, help='output R.h path')
    # The public/private split is a FRAMEWORK-only concept: aapt2 marks the
    # public API subset with a trailing PUBLIC, and the framework mirrors
    # Android's android.R (public) vs com.android.internal.R (private) by routing
    # everything aapt2 did NOT mark PUBLIC into internal_R.h. Apps have no such
    # distinction — every app resource is public to the app — and aapt2 never
    # emits PUBLIC for them, so enabling the split on an app would move almost
    # every resource (id/layout/string/...) into internal_R.h and leave R.h empty.
    # Hence default OFF; only the framework (cdroid) build passes it.
    ap.add_argument('--split-private', action='store_true',
                    help='Route non-PUBLIC resources into internal_R.h '
                         '(cdroid::internal::R). Framework-only.')
    args = ap.parse_args()

    r = subprocess.run([args.aapt2, 'dump', 'resources', args.apk],
                       capture_output=True, text=True)
    if r.returncode != 0:
        sys.stderr.write("aapt2 dump failed: %s\n" % r.stderr[:500])
        sys.exit(1)

    by_type = {}  # public ctype -> {cident_key: hex_id}  -> cdroid::R
    internal_by_type = {}  # private ctype -> {cident_key: hex_id}  -> cdroid::internal::R
    for line in r.stdout.splitlines():
        m = RES_RE.search(line)
        # Skip aapt2 synthetic inline resources (<aapt:attr> children, e.g. the
        # nested <aapt:attr name="android:animation"> blocks inside an
        # animated-vector). aapt2 names them "$$xxx" / "$xxx" and bakes them into
        # the arsc, but Android's R.java never exports them — no code references an
        # anonymous inline resource by name. Without this filter, cident() turns
        # the "$" into "_" and R.h fills up with "__xxx__0__0" garbage.
        if m and '$' not in m.group(3):
            rid, rtype, rkey = m.group(1), m.group(2), m.group(3)
            ck = cident(rkey)
            tns = 'attr' if rtype == '^attr-private' else ctype(rtype)
            # A resource is "private" when aapt2 did NOT mark it PUBLIC (this
            # covers ^attr-private attrs AND plain styles/drawables/etc. that the
            # framework keeps out of android.jar). The framework wants these in
            # internal_R.h; an app does not — so only split when --split-private.
            is_private = (rtype == '^attr-private') or (' PUBLIC' not in line)
            target = internal_by_type if (args.split_private and is_private) else by_type
            d = target.setdefault(tns, {})
            if ck in d and d[ck] != rid:
                sys.stderr.write("WARNING: %s id collision in %s: %s (0x%s vs 0x%s)\n"
                                 % ('internal_R' if target is internal_by_type else 'R.h', tns, ck, d[ck], rid))
            d[ck] = rid

    # Public resources -> R.h (cdroid::R).
    with open(args.output, 'w') as f:
        f.write('#pragma once\n\n/*Generated by aapt2_gen_rh.py, do not edit*/\n\n')
        f.write('namespace %s{\n\n' % args.namespace)
        f.write('namespace R{\n')
        for tns in sorted(by_type):
            # enum : int (not static constexpr int): enumerators are constants,
            # not objects, so they have no address and are never ODR-used -> no
            # linker errors and no need for (int) casts at use sites (C++14 has
            # no inline constexpr). All resource IDs fit in int (0x01../0x7f..).
            f.write('    namespace %s{\n' % tns)
            f.write('        enum : int {\n')
            for key, rid in sorted(by_type[tns].items()):
                f.write('            %s = %s,\n' % (key, rid))
            f.write('        };\n')
            f.write('    }/*namespace %s*/\n\n' % tns)
        f.write('};//endof namespace R\n\n')
        f.write('}//endof namespace\n')

    internal_out = os.path.join(os.path.dirname(os.path.abspath(args.output)), 'internal_R.h')
    if args.split_private:
        # Private framework resources -> internal_R.h (cdroid::internal::R), the
        # com.android.internal.R equivalent — separate file, never mixed into the
        # public cdroid::R. Contains every resource aapt2 did NOT mark PUBLIC:
        # private attrs (^attr-private) plus private styles/drawables/layouts/etc.
        with open(internal_out, 'w') as f:
            f.write('#pragma once\n\n/*Generated by aapt2_gen_rh.py: private framework')
            f.write(' resources (com.android.internal.R equivalent). Do not edit*/\n\n')
            f.write('namespace %s{\n\n' % args.namespace)
            f.write('namespace internal{\n')
            f.write('namespace R{\n')
            for tns in sorted(internal_by_type):
                f.write('    namespace %s{\n' % tns)
                f.write('        enum : int {\n')
                for key, rid in sorted(internal_by_type[tns].items()):
                    f.write('            %s = %s,\n' % (key, rid))
                f.write('        };\n')
                f.write('    }/*namespace %s*/\n\n' % tns)
            f.write('};//endof namespace R\n\n')
            f.write('}//endof namespace internal\n')
            f.write('}//endof namespace\n')
        sys.stderr.write('aapt2_gen_rh: %d public types/%d resources -> %s; %d private types/%d resources -> %s\n'
                         % (len(by_type), sum(len(v) for v in by_type.values()), args.output,
                            len(internal_by_type), sum(len(v) for v in internal_by_type.values()), internal_out))
    else:
        # App mode: no public/private split — every resource is already in R.h.
        # Remove a stale internal_R.h left by an earlier framework-style run so it
        # does not linger (nothing includes it, but it is misleading on disk).
        if os.path.exists(internal_out):
            os.remove(internal_out)
        sys.stderr.write('aapt2_gen_rh: %d types/%d resources -> %s (no private split)\n'
                         % (len(by_type), sum(len(v) for v in by_type.values()), args.output))


if __name__ == '__main__':
    main()
