# src/cmds — device-side package tools (the `frameworks/base/cmds` role)

`pm` and `am` turn a CDROID target into something `adb install`-able: paks
carry the app binary (`bin/<exe>` slot, `CreatePAK(... EMBED_EXE)`), `pm`
installs/registers them, `am` launches them.

## Bundle build

```cmake
CreatePAK(myapp ${PROJECT_SOURCE_DIR}/assets ${PROJECT_BINARY_DIR}/myapp.pak
          ${PROJECT_SOURCE_DIR}/R.h EMBED_EXE)
```

The distributable single file is `<out>/apps/myapp/myapp.pak` (the binary-root
copy is refreshed after embedding too).

## pm

```
pm install [-r] <app.pak>            # -r = replace/upgrade (versionCode guard)
pm install-create [-r] ...           # staged session (modern adb install)
pm install-write <id> <name> (-|f)   # stream the pak into the session
pm install-commit <id>               # apply; install-abandon <id> drops it
pm uninstall <package>
pm list packages [-f]                # AOSP output format
pm path <package>
```

Data root: `$CDROID_DATA_DIR` (default `/data`); registry at
`<root>/system/packages.xml`, installs at `<root>/app/<pkg>-<serial>/`
holding `<exe>` + `<exe>.pak` side by side — exactly what `App` probes in its
own directory, so installed apps run with zero runtime changes.

## am

```
am start [-n] <pkg>[/.Activity]      # fork + detach (adb shell friendly)
am start --exec <pkg>                # replace THIS process with the app
am force-stop <pkg>
```

## Boot / auto-start recipes

CDROID is a single-foreground-GUI-process model (one process holds the drm
master + evdev), so "launch supervision" is serial scheduling, and the system
init already ships a supervisor. Prefer borrowing it over running a new daemon:

**busybox init (buildroot)** — `/etc/inittab`:

```
::sysinit:/usr/bin/pm install -r /usr/share/cdroid/apps/mypkg.pak
::respawn:/usr/bin/am start --exec cdroid.myapp
```

The respawn line gives persistent-app restart (crash → relaunch) and
Home-return (exit → relaunch) in one line; `--exec` makes the app BE the
respawned process (no fork, exit status propagates).

**systemd** — `/etc/systemd/system/cdroid-app.service`:

```
[Unit]
Description=CDROID foreground app
After=systemd-modules-load.service

[Service]
ExecStart=/usr/bin/am start --exec cdroid.myapp
Restart=always
RestartSec=1

[Install]
WantedBy=multi-user.target
```

## adb

TCP adbd comes from the buildroot package (Target packages → Development
tools → `android-tools`, enable `adbd`); without a USB transport it falls
back to listening on :5555. Then from the host:

```
adb connect <device-ip>:5555
adb install myapp.pak          # goes through pm install-create/-write/-commit
adb shell am start cdroid.myapp
```

`touch /var/usb-debugging-enabled` if your adbd build checks the flag.
