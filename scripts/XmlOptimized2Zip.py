import os
import shutil
import struct
import zlib
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
# 9-patch npTc chunk embedding (Android aapt-equivalent, build-time).
#
# A source .9.png carries its stretch/padding info only as the 1px guide border.
# We scan that border, serialize it into an Android npTc chunk (Res_png_9patch in
# big-endian "file" order — the same layout the C++ Res_png_9patch::deserialize
# consumes), and splice the chunk into the PNG just before IEND. At runtime
# NinePatch::Create then builds from the chunk instead of re-scanning the border.
# The image, its filename (.9.png) and its relative path inside the pak are all
# preserved — only the chunk is added.
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

def embed_9patch_assets(input_directory, zip_file_path):
    """Walk input_directory for *.9.png, embed an npTc chunk into each, and append them
    to the pak at their original relative path/name (ZIP_STORED — PNGs are already
    compressed, matching the binary-zip -0 policy)."""
    count = 0
    with zipfile.ZipFile(zip_file_path, 'a', zipfile.ZIP_STORED) as zipf:
        for root, dirs, files in os.walk(input_directory):
            for f in files:
                if not f.endswith(".9.png"):
                    continue
                path = os.path.join(root, f)
                rel = os.path.relpath(path, input_directory).replace(os.sep, "/")
                try:
                    im = Image.open(path); im.load()
                    if im.mode != "RGBA":
                        im = im.convert("RGBA")
                    xd, yd, pl, pr, pt, pb = _scan_9patch(im)
                    data = _serialize_nptc(xd, yd, pl, pr, pt, pb)
                    with open(path, "rb") as fh:
                        raw = fh.read()
                    raw = _insert_chunk_before_iend(raw, b"npTc", data)
                    zipf.writestr(rel, raw)
                    count += 1
                except Exception as e:
                    print(f"9patch embed failed for {path}: {e}; storing as-is")
                    zipf.write(path, rel)
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

