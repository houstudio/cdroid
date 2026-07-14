import os
import shutil
import struct
import zlib
import io
from lxml import etree
import zipfile
from PIL import Image

def remove_comments_and_blank_lines(input_file, output_file):
    try:
        parser = etree.XMLParser(remove_comments=True)
        tree = etree.parse(input_file, parser)
        with open(output_file, 'wb') as f:
            xml_content = etree.tostring(tree, encoding='UTF-8', xml_declaration=True, pretty_print=False)
            non_blank_lines = [line for line in xml_content.decode('utf-8').splitlines() if line.strip()]
            f.write('\n'.join(non_blank_lines).encode('utf-8'))
    except Exception as e:
        print(f"Error processing {input_file}: {e}")

def process_xml_files(input_directory, output_directory):
    if os.path.exists(output_directory):
        shutil.rmtree(output_directory)
    os.makedirs(output_directory)
    for root, dirs, files in os.walk(input_directory):
        for file in files:
            if file.endswith('.xml'):
                input_file_path = os.path.join(root, file)
                relative_path = os.path.relpath(input_file_path, input_directory)
                output_file_path = os.path.join(output_directory, relative_path)
                output_file_dir = os.path.dirname(output_file_path)
                if not os.path.exists(output_file_dir):
                    os.makedirs(output_file_dir)
                remove_comments_and_blank_lines(input_file_path, output_file_path)

def zip_xml_files(output_directory, zip_file_path):
    with zipfile.ZipFile(zip_file_path, 'w', zipfile.ZIP_DEFLATED) as zipf:
        for root, dirs, files in os.walk(output_directory):
            for file in files:
                if file.endswith('.xml'):
                    file_path = os.path.join(root, file)
                    relative_path = os.path.relpath(file_path, output_directory)
                    zipf.write(file_path, relative_path)

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
# common_functions.cmake shell-zips with -x "*.9.png" so only these processed
# borderless .png copies reach the pak. createAsDrawable detects a 9-patch by the
# cdNp chunk (not the filename). The <nine-patch> XML path is developer-guaranteed
# (its srcs are plain animation PNGs, not .9.png), so it's untouched.
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

def embed_9patch_assets(input_directory, zip_file_path):
    """aapt-compile each *.9.png: scan the guide border for stretch/padding (npTc),
    optical/layout-bounds (npLb) and outline (npOl); bundle them into one "cdNp"
    chunk; STRIP the 1px guide border and splice cdNp into the borderless PNG
    before IEND; store as <name>.png (drop the .9) at the same relative path.
    ZIP_STORED (PNGs are already compressed)."""
    count = 0
    with zipfile.ZipFile(zip_file_path, 'a', zipfile.ZIP_STORED) as zipf:
        for root, dirs, files in os.walk(input_directory):
            for f in files:
                if not f.endswith(".9.png"):
                    continue
                path = os.path.join(root, f)
                rel = os.path.relpath(path, input_directory).replace(os.sep, "/")
                arcname = rel[:-6] + ".png"   # foo.9.png -> foo.png
                try:
                    im = Image.open(path); im.load()
                    if im.mode != "RGBA":
                        im = im.convert("RGBA")
                    W, H = im.size
                    if W < 3 or H < 3:
                        zipf.write(path, arcname); continue
                    # scan the bordered source (divs/padding/optical/outline are
                    # content-relative, which == image-relative after the strip)
                    xd, yd, pl, pr, pt, pb = _scan_9patch(im)
                    lb_l, lb_t, lb_r, lb_b = _scan_layout_bounds(im)
                    ol_l, ol_t, ol_r, ol_b, ol_rad, ol_alpha = _compute_outline(im)
                    nptc = _serialize_nptc(xd, yd, pl, pr, pt, pb)
                    nplb = _serialize_nplb(lb_l, lb_t, lb_r, lb_b)
                    npol = _serialize_outline(ol_l, ol_t, ol_r, ol_b, ol_rad, ol_alpha)
                    cdNp = _serialize_cdNp(nptc, nplb, npol)
                    # strip the 1px guide border -> inner (W-2)x(H-2), re-encode PNG,
                    # splice cdNp before IEND
                    inner = im.crop((1, 1, W - 1, H - 1))
                    buf = io.BytesIO()
                    inner.save(buf, format="PNG")
                    raw = _insert_chunk_before_iend(buf.getvalue(), b"cdNp", cdNp)
                    zipf.writestr(arcname, raw)
                    count += 1
                except Exception as e:
                    print(f"9patch embed failed for {path}: {e}; storing bordered as-is")
                    zipf.write(path, arcname)
    return count

if __name__ == "__main__":
    import sys
    if len(sys.argv) != 4:
        print("Usage: python remove_xml_comments.py <input_directory> <output_directory> <zip_file_path>")
        sys.exit(1)
    input_directory = sys.argv[1]
    output_directory = sys.argv[2]
    zip_file_path = sys.argv[3]
    process_xml_files(input_directory, output_directory)
    zip_xml_files(output_directory, zip_file_path)
    embed_9patch_assets(input_directory, zip_file_path)
    # remove temp directory
    #shutil.rmtree(output_directory)

