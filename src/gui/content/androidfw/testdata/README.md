# androidfw test data

Source resource trees + reference binaries for the `*_fixture.h` byte arrays the
tests consume. Everything here is reproducible via `regen.sh` (aapt2 build-tools/36).

## Layout

| dir | produces | consumed by |
|---|---|---|
| `axml/` (res/layout/test.xml + res/values/attrs.xml) | binary AXML `test.xml` → `axml_fixture.h` (kAXML) | resxmltree_test, androidfw_axml_dump |
| `arsc/` (string/hello: default/zh/es; integer/grid: mdpi/hdpi/xhdpi) | resources.arsc → `arsc_fixture.h` (kARS) | androidfw_resource_demo |
| `arsc_verify/` (string/hello locale variants + greeting/chain + style/AppStyle + array/names) | resources.arsc → `arsc_verify_fixture.h` (kARSV) | traverse_test |
| `stringpool/` (the 5 pool strings) | global string pool → `stringpool_fixture.h` (kStringPool) | resourcetypes_test |

Each dir holds the original `res/` tree + `AndroidManifest.xml` (the human-readable
source) plus the committed binary reference (`test.xml` / `resources.arsc`).

## Regenerate

```sh
AAPT2=/opt/android-sdk/build-tools/36.0.0/aapt2 \
ANDROID_JAR=/opt/android-sdk/platforms/android-36/android.jar \
./regen.sh
```

This rebuilds each apk, extracts the binary, writes it back into the dir, and emits
`_gen/<name>.h` — compare against the committed `../<name>_fixture.h`. The committed
headers are canonical; regen verified byte-identical for arsc_verify (kARSV).

> `stringpool_fixture.h` is a hand-extracted global string pool; its source here is
> for reference and may not byte-match aapt2's pool internals exactly. The committed
> fixture is canonical.
