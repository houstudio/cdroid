#!/usr/bin/env python3
# ----------------------------------------------------------------------------
# pakbuilder.py — single-class resource compiler for CreatePAK.
#
# Replaces the former idgen.py + XmlOptimized2Zip.py two-script dance:
#  - ID generation (R.h + values/ID.xml) is REUSED by inheriting idgen.py's
#    IDGenerater (that file is not modified);
#  - XML comment/blank-line stripping, 9-patch aapt-style compile, and .pak
#    packaging (formerly XmlOptimized2Zip.py) live on this class.
#
# lxml + Pillow are HARD dependencies: 9-patch compile needs PIL and there is no
# correct fallback without it (a .9.png stored without a cdNp chunk is treated
# as a plain bitmap at runtime). Missing either fails loud.
#
# CLI mirrors CreatePAK's 4 args:
#   pakbuilder.py <namespace> <resdir> <pakpath> <rhpath>
# Extra cmake args (e.g. kaidu_ms7's PIXMAN_INCLUDE_DIRS) are ignored.
# ----------------------------------------------------------------------------
import os
import sys
import io
import shutil
import struct
import zlib
import tempfile
import filecmp
import zipfile

try:
    from lxml import etree
    from PIL import Image
except ImportError as e:
    sys.stderr.write("pakbuilder requires lxml and Pillow: %s\n" % e)
    sys.exit(1)

import idgen  # sibling module: Python prepends this script's dir to sys.path.
              # idgen.py guards its work in __main__, so importing has no side effect.

# ----------------------------------------------------------------------------
# 9-patch aapt-style compile (Android aapt2-equivalent, build-time).
#
# A source .9.png carries stretch/padding as the 1px guide border. We scan that
# border for three things — npTc (stretch/padding, big-endian Res_png_9patch),
# npLb (optical/layout-bounds from the red ticks), npOl (outline rect/radius/alpha
# via findMaxOpacity) — bundle all three into ONE custom "cdNp" chunk, STRIP the
# 1px guide border, and splice cdNp into the borderless PNG before IEND. The asset
# is stored as <name>.png (the ".9" is dropped) — aapt-style. The divs/padding/
# optical/outline are content-relative on the bordered source, which after the
# strip equals image-relative on the borderless image, so the same values apply.
# createAsDrawable detects a 9-patch by the cdNp chunk (not the filename).
# cdNp DATA = [u8 version=1][u16 npTcLen BE + bytes][u16 npLbLen BE + bytes]
#             [u16 npOlLen BE + bytes]. Sub-blob byte orders match the C++ side:
# npTc = Res_png_9patch big-endian; npLb/npOl = native little-endian (memcpy).
# The chunk TYPE is "cdNp" (not "cd9p"): PNG chunk-type names must be 4 ASCII
# letters, and byte 2's reserved bit must be 0 (uppercase) — a digit like '9' is
# rejected by libpng with "bad header (invalid type)". c/d=ancillary+private,
# N=reserved-bit-0, p=safe-to-copy.
# ----------------------------------------------------------------------------

def _np_guide(px, x, y):
    """A 9-patch guide pixel is opaque black (0xFF000000)."""
    p = px[x, y]
    return p[3] == 255 and p[0] == 0 and p[1] == 0 and p[2] == 0

def _np_runs(count, is_guide):
    """Return [(start, end), ...] inclusive-start/exclusive-end guide runs over [0,count)."""
    spans, s = [], None
    for i in range(count):
        if is_guide(i):
            if s is None:
                s = i
        elif s is not None:
            spans.append((s, i)); s = None
    if s is not None:
        spans.append((s, count))
    return spans

def _scan_9patch(im):
    """Scan the 1px guide border. Returns content-relative x_divs, y_divs (start/end
    pairs) and padding (left/right/top/bottom). Content = the inner image excluding
    the 1px border, so coords are in [0, W-2]/[0, H-2)."""
    W, H = im.size
    px = im.load()
    cw, ch = W - 2, H - 2
    h_stretch = _np_runs(cw, lambda i: _np_guide(px, i + 1, 0))        # top edge
    v_stretch = _np_runs(ch, lambda i: _np_guide(px, 0, i + 1))        # left edge
    x_divs = [v for s in h_stretch for v in (s[0], s[1])]
    y_divs = [v for s in v_stretch for v in (s[0], s[1])]
    # Padding: bottom edge = horizontal, right edge = vertical. Fall back to the
    # first/last stretch spans when no padding guide is drawn (matches PopulateBounds).
    h_pad = _np_runs(cw, lambda i: _np_guide(px, i + 1, H - 1))        # bottom edge
    if h_pad:
        s, e = h_pad[0]; pad_l, pad_r = s, cw - e
    elif h_stretch:
        pad_l, pad_r = h_stretch[0][0], cw - h_stretch[-1][1]
    else:
        pad_l = pad_r = 0
    v_pad = _np_runs(ch, lambda i: _np_guide(px, W - 1, i + 1))        # right edge
    if v_pad:
        s, e = v_pad[0]; pad_t, pad_b = s, ch - e
    elif v_stretch:
        pad_t, pad_b = v_stretch[0][0], ch - v_stretch[-1][1]
    else:
        pad_t = pad_b = 0
    return x_divs, y_divs, pad_l, pad_r, pad_t, pad_b

def _serialize_nptc(x_divs, y_divs, pad_l, pad_r, pad_t, pad_b):
    """Serialize an npTc chunk DATA block (Res_png_9patch in big-endian file order),
    matching the C++ Res_png_9patch struct layout EXACTLY:
      [0..3]   wasDeserialized/numXDivs/numYDivs/numColors
      [4..7]   xDivsOffset
      [8..11]  yDivsOffset
      [12..27] paddingLeft/Right/Top/Bottom (4 x int32)
      [28..31] colorsOffset
      [32..]   xDivs[], yDivs[]  (colors omitted: numColors=0 — CDROID's renderer
               doesn't consume region_colors, so all-NO_COLOR is fine).
    NOTE: colorsOffset comes AFTER padding (not before) — getting this order wrong
    makes the C++ read paddingLeft as colorsOffset (e.g. 48 for a 22px image)."""
    nx, ny, nc = len(x_divs), len(y_divs), 0
    x_off = 32
    y_off = x_off + nx * 4
    c_off = y_off + ny * 4
    out = struct.pack(">BBBB", 0, nx, ny, nc)                # [0..3] wasDeserialized + counts
    out += struct.pack(">II", x_off, y_off)                  # [4..11] xDivsOffset, yDivsOffset
    out += struct.pack(">iiii", pad_l, pad_r, pad_t, pad_b)  # [12..27] padding
    out += struct.pack(">I", c_off)                          # [28..31] colorsOffset
    for d in x_divs:
        out += struct.pack(">i", d)
    for d in y_divs:
        out += struct.pack(">i", d)
    return out

def _insert_chunk_before_iend(png_bytes, chunk_type, chunk_data):
    """Splice a PNG chunk (length+type+data+CRC) in just before the IEND chunk."""
    crc = zlib.crc32(chunk_type + chunk_data) & 0xffffffff
    chunk = struct.pack(">I", len(chunk_data)) + chunk_type + chunk_data + struct.pack(">I", crc)
    idx = png_bytes.rfind(b"IEND")
    if idx < 4:                       # IEND not found / malformed — leave untouched
        return png_bytes
    return png_bytes[:idx - 4] + chunk + png_bytes[idx - 4:]

# ----------------------------------------------------------------------------
# npLb (optical / layout-bounds) and npOl (outline) — computed on the BORDERED
# source (before the border is stripped), replicating NinePatchRenderer's pixel
# logic so the chunk values match today's runtime. Byte order is native LE
# (matches NinePatch::SerializeLayoutBounds / SerializeRoundedRectOutline, which
# memcpy without swapping; CDROID targets are little-endian).
#
# AUDIT of the referenced C++ (ninepatchrenderer.cc) before porting:
#  - findMaxOpacity (line 236): correct (march, track inset of new opacity max,
#    stop at 0xff). Ported verbatim.
#  - radius via diagonal findMaxOpacity: correct (this is what yields 3/6 today).
#  - outline alpha (line 339-340): BUG in C++ — maxAlphaOverCol is called with
#    endY=innerStartY (should be innerEndY), so the column scan is empty and the
#    column term is always 0; alpha degenerates to row-only. FIXED here: scan the
#    column over [innerStartY, innerEndY) and take max(row, col).
# ----------------------------------------------------------------------------

_TICK_NONE, _TICK_TICK, _TICK_LB = 0, 1, 2
_K_WHITE = 0xffffffff
_K_TICK  = 0xff000000       # opaque black  (stretch/padding guide)
_K_LB    = 0xff0000ff       # opaque red    (layout-bounds / optical guide)

def _tick_type(p, transparent):
    """Classify a guide pixel like ninepatchrenderer.cc tickType. p = (R,G,B,A)."""
    r, g, b, a = p
    color = r | (g << 8) | (b << 16) | (a << 24)
    if transparent:
        if a == 0:            return _TICK_NONE
        if color == _K_LB:    return _TICK_LB
        if color == _K_TICK:  return _TICK_TICK
        return _TICK_TICK
    if color == _K_WHITE:    return _TICK_NONE
    if color == _K_TICK:     return _TICK_TICK
    if color == _K_LB:       return _TICK_LB
    return _TICK_TICK

def _scan_layout_bounds(im):
    """Optical/layout-bounds insets from the bottom-row (L/R) and right-col (T/B) red
    guide ticks. Returns (left, top, right, bottom), content-relative. Usually all-zero."""
    W, H = im.size
    px = im.load()
    transparent = px[0, 0][3] == 0
    left = right = top = bottom = 0
    # horizontal: bottom row (y = H-1), cols 1..W-2
    if _tick_type(px[1, H - 1], transparent) == _TICK_LB:
        i = 1
        while i < W - 1:
            left += 1; i += 1
            if _tick_type(px[i, H - 1], transparent) != _TICK_LB: break
    if _tick_type(px[W - 2, H - 1], transparent) == _TICK_LB:
        i = W - 2
        while i > 1:
            right += 1; i -= 1
            if _tick_type(px[i, H - 1], transparent) != _TICK_LB: break
    # vertical: right col (x = W-1), rows 1..H-2
    if _tick_type(px[W - 1, 1], transparent) == _TICK_LB:
        i = 1
        while i < H - 1:
            top += 1; i += 1
            if _tick_type(px[W - 1, i], transparent) != _TICK_LB: break
    if _tick_type(px[W - 1, H - 2], transparent) == _TICK_LB:
        i = H - 2
        while i > 1:
            bottom += 1; i -= 1
            if _tick_type(px[W - 1, i], transparent) != _TICK_LB: break
    return left, top, right, bottom

def _find_max_opacity(px, startX, startY, endX, endY, dX, dY):
    """Port of findMaxOpacity: march (startX,startY)->(endX,endY) by (dX,dY); return the
    inset (step count) at which opacity last rose to a new max; stop at fully opaque."""
    max_op = 0; out = 0; inset = 0
    x, y = startX, startY
    while x != endX and y != endY:
        op = px[x, y][3]
        if op > max_op:
            max_op = op; out = inset
        if op == 0xff:
            return out
        x += dX; y += dY; inset += 1
    return out

def _compute_outline(im):
    """Replicate NinePatchRenderer's outline computation on the BORDERED source.
    Returns (left, top, right, bottom, radius, alpha) — edge insets + corner radius
    + content max alpha (alpha column-scan FIXED vs the C++ bug)."""
    W, H = im.size
    px = im.load()
    midX, midY = W // 2, H // 2
    endX, endY = W - 2, H - 2
    left = right = top = bottom = 0
    if W > 4:
        left  = _find_max_opacity(px, 1,    midY, midX, -1, 1, 0)
        right = _find_max_opacity(px, endX, midY, midX, -1, -1, 0)
    if H > 4:
        top    = _find_max_opacity(px, midX, 1,    -1, midY, 0, 1)
        bottom = _find_max_opacity(px, midX, endY, -1, midY, 0, -1)
    innerSX = 1 + left
    innerSY = 1 + top
    innerEX = endX - right
    innerEY = endY - bottom
    innerMX = (innerEX + innerSX) // 2
    innerMY = (innerEY + innerSY) // 2
    # alpha: max over the center row AND center column of the outline area.
    alpha = 0
    for x in range(innerSX, innerEX):
        op = px[x, innerMY][3]
        if op > alpha: alpha = op
    for y in range(innerSY, innerEY):           # FIXED: C++ passed innerStartY twice (dead col)
        op = px[innerMX, y][3]
        if op > alpha: alpha = op
    diagonal = _find_max_opacity(px, innerSX, innerSY, innerMX, innerMY, 1, 1)
    radius = int(3.4142 * diagonal)
    return left, top, right, bottom, radius, alpha

def _serialize_nplb(left, top, right, bottom):
    return struct.pack("<iiii", left, top, right, bottom)

def _serialize_outline(left, top, right, bottom, radius, alpha):
    return struct.pack("<iiiifI", left, top, right, bottom, radius, alpha)

def _serialize_cdNp(nptc, nplb, npol):
    """Bundle npTc + npLb + npOl into one "cdNp" chunk DATA blob:
    [u8 version=1] then, for each sub-blob, [u16 BE length][bytes]. Matches the
    C++ NinePatch::Create cdNp parser exactly."""
    out = bytes([1])  # version
    for blob in (nptc, nplb, npol):
        if len(blob) > 0xffff:
            raise ValueError(f"cdNp sub-blob too large ({len(blob)} bytes)")
        out += struct.pack(">H", len(blob)) + blob
    return out


# Binary asset extensions stored verbatim (PNGs are already compressed -> ZIP_STORED).
BIN_EXTS = (".png", ".jpg", ".jpeg", ".gif", ".apng", ".webp", ".ttf", ".otf", ".ttc")


class PakBuilder(idgen.IDGenerater):
    """One-shot resource compiler: generates R.h/ID.xml (via inherited idgen) and
    builds the .pak zip (stripped XML + aapt-compiled 9-patch + verbatim binaries).

    When aapt2_path + android_jar are provided, layout/drawable XML is compiled
    to binary AXML via aapt2 (opt-in; default is the existing text-XML path)."""

    def __init__(self, namespace, res_dir, pak_path, rh_path,
                 aapt2_path=None, android_jar=None, sdk_res=None):
        idstart = 1000 if namespace == "cdroid" else 10000   # preserve idgen __main__ rule
        super().__init__(idstart, namespace)                 # wires self.Handler/scanxml/dict2ID/dict2RH
        self.res_dir = res_dir
        self.pak_path = pak_path
        self.rh_path = rh_path
        self.aapt2_path = aapt2_path
        self.android_jar = android_jar
        self.use_aapt2 = bool(aapt2_path and android_jar)
        self.sdk_res = sdk_res
        self.use_sdk = bool(sdk_res and aapt2_path)

    # ----- ID generation: drive inherited idgen, with its filecmp change gate -----
    def generate_ids(self):
        values = os.path.join(self.res_dir, "values")
        os.makedirs(values, exist_ok=True)
        self.scanxml(self.res_dir)                           # inherited -> fills self.Handler.idlist
        fd, tmp = tempfile.mkstemp(prefix=self.namespace + "_ID", suffix=".xml")
        os.close(fd)
        self.dict2ID(tmp)                                    # inherited
        id_xml = os.path.join(values, "ID.xml")
        same = os.path.exists(id_xml) and filecmp.cmp(tmp, id_xml)
        if not same or not os.path.exists(self.rh_path):
            shutil.copyfile(tmp, id_xml)
            self.dict2RH(self.rh_path)                       # inherited -> R.h
        try:
            os.remove(tmp)
        except OSError:
            pass

    # ----- XML processing (in-memory strip; no temp dir) -----
    def _strip_xml(self, src):
        parser = etree.XMLParser(remove_comments=True)
        tree = etree.parse(src, parser)
        s = etree.tostring(tree, encoding="UTF-8", xml_declaration=True, pretty_print=False)
        non_blank = [ln for ln in s.decode("utf-8").splitlines() if ln.strip()]
        return "\n".join(non_blank).encode("utf-8")

    # ----- 9-patch aapt compile: returns borderless+cdNp bytes, or None to store as-is -----
    def _compile_9patch(self, path):
        im = Image.open(path); im.load()
        if im.mode != "RGBA":
            im = im.convert("RGBA")
        W, H = im.size
        if W < 3 or H < 3:
            return None
        xd, yd, pl, pr, pt, pb = _scan_9patch(im)
        cdNp = _serialize_cdNp(
            _serialize_nptc(xd, yd, pl, pr, pt, pb),
            _serialize_nplb(*_scan_layout_bounds(im)),
            _serialize_outline(*_compute_outline(im)))
        # strip the 1px guide border -> inner (W-2)x(H-2), re-encode, splice cdNp
        buf = io.BytesIO()
        im.crop((1, 1, W - 1, H - 1)).save(buf, format="PNG")
        return _insert_chunk_before_iend(buf.getvalue(), b"cdNp", cdNp)

    # ----- SDK framework build: compile SDK data/res/ via aapt2 -x (framework mode) -----
    def _compile_sdk_res(self):
        """Build framework.apk from SDK data/res/ via aapt2 -x. Returns {rel_path: bytes}
        with binary AXML layouts + resources.arsc + drawables (all with 'res/' prefix
        stripped to match pak naming convention)."""
        import subprocess, shutil
        tmpdir = tempfile.mkdtemp(prefix="sdk_aapt2_")
        try:
            tmpres = os.path.join(tmpdir, "res")
            shutil.copytree(self.sdk_res, tmpres)
            # Fix 1: strip android:featureFlag lines from dimens.xml.
            dimens = os.path.join(tmpres, "values", "dimens.xml")
            if os.path.exists(dimens):
                with open(dimens) as f: lines = f.readlines()
                with open(dimens, "w") as f:
                    f.writelines(l for l in lines if "android:featureFlag" not in l)
            # Fix 2: add widget dimen stubs + remove their public-final declarations.
            with open(os.path.join(tmpres, "values", "_sdk_fixes.xml"), "w") as f:
                f.write('<?xml version="1.0" encoding="utf-8"?>\n<resources>\n'
                        '  <dimen name="system_app_widget_background_radius">0dp</dimen>\n'
                        '  <dimen name="system_app_widget_inner_radius">0dp</dimen>\n'
                        '</resources>\n')
            pf = os.path.join(tmpres, "values", "public-final.xml")
            if os.path.exists(pf):
                with open(pf) as f: lines = f.readlines()
                with open(pf, "w") as f:
                    f.writelines(l for l in lines
                                 if "system_app_widget_background_radius" not in l
                                 and "system_app_widget_inner_radius" not in l)
            # Synthesize manifest (framework: package=android).
            manifest = ('<?xml version="1.0" encoding="utf-8"?>\n'
                        '<manifest xmlns:android="http://schemas.android.com/apk/res/android"'
                        ' package="android">'
                        '<uses-sdk android:minSdkVersion="26" android:targetSdkVersion="36"/>'
                        '</manifest>')
            mpath = os.path.join(tmpdir, "AndroidManifest.xml")
            with open(mpath, "w") as f: f.write(manifest)
            compiled = os.path.join(tmpdir, "compiled.zip")
            out_apk = os.path.join(tmpdir, "framework.apk")
            sys.stderr.write("SDK res: compiling %s via aapt2 -x...\n" % self.sdk_res)
            subprocess.run([self.aapt2_path, "compile", "--dir", tmpres, "-o", compiled],
                           check=True, capture_output=True)
            subprocess.run([self.aapt2_path, "link", "-x", "--manifest", mpath,
                            "-o", out_apk, compiled],
                           check=True, capture_output=True)
            # Extract everything, stripping 'res/' prefix to match pak convention.
            result = {}
            with zipfile.ZipFile(out_apk) as zf:
                for name in zf.namelist():
                    if name.endswith("/"): continue
                    rel = name[4:] if name.startswith("res/") else name
                    result[rel] = zf.read(name)
            sys.stderr.write("SDK res: %d entries (binary AXML + arsc + drawables)\n" % len(result))
            return result
        except Exception as e:
            sys.stderr.write("SDK res compile failed (%s); falling back\n" % e)
            return {}
        finally:
            shutil.rmtree(tmpdir, ignore_errors=True)

    # ----- aapt2 compile: compile res/ to binary AXML, return {rel_path: bytes} -----
    def _compile_aapt2(self):
        """Run aapt2 compile+link on res/, return a dict mapping relative XML
        paths (e.g. 'layout/main.xml') to their binary AXML bytes."""
        import subprocess, shutil
        tmpdir = tempfile.mkdtemp(prefix="aapt2_")
        try:
            # Synthesize a minimal manifest (package name = namespace).
            pkg = self.namespace if "." in self.namespace else "com." + self.namespace
            manifest = ('<?xml version="1.0" encoding="utf-8"?>\n'
                        '<manifest xmlns:android="http://schemas.android.com/apk/res/android"'
                        ' package="%s">'
                        '<uses-sdk android:minSdkVersion="26" android:targetSdkVersion="36"/>'
                        '</manifest>' % pkg)
            mpath = os.path.join(tmpdir, "AndroidManifest.xml")
            with open(mpath, "w") as fh:
                fh.write(manifest)
            compiled = os.path.join(tmpdir, "compiled.zip")
            out_apk = os.path.join(tmpdir, "out.apk")
            subprocess.run([self.aapt2_path, "compile", "--dir", self.res_dir, "-o", compiled],
                           check=True, capture_output=True)
            subprocess.run([self.aapt2_path, "link", "-I", self.android_jar,
                            "--manifest", mpath, "-o", out_apk, compiled],
                           check=True, capture_output=True)
            # Extract binary XML from the apk (paths like res/layout/main.xml).
            result = {}
            with zipfile.ZipFile(out_apk) as zf:
                for name in zf.namelist():
                    if name.startswith("res/") and name.endswith(".xml"):
                        rel = name[4:]  # strip "res/" prefix -> layout/main.xml
                        result[rel] = zf.read(name)
            return result
        except Exception as e:
            sys.stderr.write("aapt2 compile failed (%s); falling back to text XML\n" % e)
            return {}
        finally:
            shutil.rmtree(tmpdir, ignore_errors=True)

    # ----- packaging: one walk, XML deflated / binaries stored -----
    def build(self):
        # SDK mode: build complete framework from SDK data/res/ via aapt2 -x.
        sdk_data = self._compile_sdk_res() if self.use_sdk else {}
        # App mode: compile cdroid's own res/ via aapt2 (optional).
        binary_xmls = self._compile_aapt2() if self.use_aapt2 else {}
        with zipfile.ZipFile(self.pak_path, "w") as zf:
            # SDK framework: store all entries (binary AXML + arsc + drawables).
            for rel, data in sorted(sdk_data.items()):
                zf.writestr(rel, data, zipfile.ZIP_DEFLATED)
            # Walk cdroid's res/ for files NOT already provided by SDK.
            for root, dirs, files in os.walk(self.res_dir):
                dirs.sort(); files.sort()
                for f in files:
                    p = os.path.join(root, f)
                    rel = os.path.relpath(p, self.res_dir).replace(os.sep, "/")
                    if self.use_sdk and rel in sdk_data:
                        continue  # SDK already provides this entry
                    if f.endswith(".xml"):
                        if rel in binary_xmls:
                            zf.writestr(rel, binary_xmls[rel], zipfile.ZIP_DEFLATED)
                        else:
                            # cdroid's values/*.xml kept as text (SDK provides
                            # layouts/drawables as binary, but NOT values/ files).
                            zf.writestr(rel, self._strip_xml(p), zipfile.ZIP_DEFLATED)
                    elif f.endswith(".9.png"):               # MUST precede the .png branch
                        arc = rel[:-6] + ".png"              # foo.9.png -> foo.png
                        try:
                            data = self._compile_9patch(p)
                            zf.writestr(arc, data if data else open(p, "rb").read(),
                                        zipfile.ZIP_STORED)
                        except Exception as e:
                            sys.stderr.write("9patch embed failed for %s: %s; storing as-is\n" % (p, e))
                            zf.writestr(arc, open(p, "rb").read(), zipfile.ZIP_STORED)
                    elif f.endswith(BIN_EXTS):
                        zf.writestr(rel, open(p, "rb").read(), zipfile.ZIP_STORED)
                    # other extensions skipped


if __name__ == "__main__":
    if len(sys.argv) < 5:
        sys.exit("Usage: pakbuilder.py <namespace> <resdir> <pakpath> <rhpath> [aapt2] [android.jar] [sdk_res]")
    aapt2 = sys.argv[5] if len(sys.argv) > 5 else None
    ajar  = sys.argv[6] if len(sys.argv) > 6 else None
    sres  = sys.argv[7] if len(sys.argv) > 7 else None
    pb = PakBuilder(*sys.argv[1:5], aapt2_path=aapt2, android_jar=ajar, sdk_res=sres)
    pb.generate_ids()
    pb.build()
