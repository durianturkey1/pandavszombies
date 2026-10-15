# macOS build

Panda vs Zombies was written in 2014/2015 for Android only: Eclipse + Ant +
`ndk-build`, cocos2d-x 2.2.x, and a handful of SDKs (Facebook, Parse, Google
Play Services) that were never committed and are no longer distributed. That
project can't be built today, and the native libraries it produced are 32-bit
ARM, which current Apple-silicon emulators can't run.

The game's actual C++ is portable, though. This directory builds it as a native
macOS app with a current toolchain: no Xcode project, no prebuilt third-party
binaries, everything compiled from source against the macOS SDK.

## Requirements

* macOS with the Command Line Tools (`xcode-select --install`) -- full Xcode is
  not needed
* CMake 3.20+ (`brew install cmake`)
* Optional: `brew install webp` for WebP decoding (the game ships no `.webp`
  assets, so the build stubs it out when libwebp is missing)

## Build and run

```sh
./proj.mac/setup.sh                                        # cocos2d-x 2.2.6 + patches
cmake -S proj.mac -B proj.mac/build -DCMAKE_BUILD_TYPE=Release
cmake --build proj.mac/build -j8
open "proj.mac/build/Panda vs Zombies.app"
```

Assets are symlinked into the app bundle, so changes under `Resources/` show up
on the next launch without rebuilding.

The window runs the engine at the game's native 480x320 landscape frame and
scales it 2x for a desktop display (`kWindowZoom` in `main.mm`). Mouse input is
translated back through that zoom, so the on-screen joysticks work by click and
drag.

## What this port changes

Game code (`Classes/`) -- the Android JNI shims are now guarded so the same
files compile on both platforms. Changes under `Classes/crimson/` are committed
here; `Classes/dxco/` is the separate [lib-dxco](https://github.com/DxCo-Games/lib-dxco)
submodule, so its changes ship as `patches/lib-dxco-macos.patch` and are applied
by `setup.sh`.

| Area | Desktop behaviour |
| --- | --- |
| Sound settings (`SoundUtil`) | Music and effects always on -- the toggles lived in the Java menu |
| Remote config (`ParseConfiguration`) | Built-in defaults; Parse shut down in 2017 |
| Facebook (`FacebookAPI`) | Reports no session, so `UserDAO` skips all Facebook paths |
| Ads (`AdsAPI`, `AdmobHelper`, RevMob) | No-ops |
| Rate dialog (`RateHelper`) | Never prompts |
| Share button | Writes the screenshot to the writable path and logs the message |

`Classes/dxco/RevMobAds_Android.cpp`, `Classes/dxco/CCAdView.cpp`,
`Classes/dxco/SoundUtil.cpp` and `Classes/crimson/TopDownItem.cpp` are excluded
from this build (Android-only or dead code -- the last one isn't in
`Android.mk` either).

Engine fixes live in `patches/cocos2d-x-2.2.6-macos.patch`, applied by
`setup.sh`:

1. **CocosDenshion audio session.** The 2010-era Mac shims redefined
   `kAudioSessionProperty_*` and `AudioSessionGetProperty`, which the macOS SDK
   now declares itself (as unavailable on macOS). Removed, and the two
   `CDAudioManager` call sites short-circuit on macOS.
2. **Duplicate Objective-C classes.** CocosDenshion declares its own
   NSSound-backed `AVAudioPlayer`/`AVAudioSession`; the system now loads real
   classes with those names, so the runtime had two of each ("may cause
   mysterious crashes"). Renamed to `CDXAVAudio*`.
3. **`CCImage::saveToFile` was `assert(false)`** on Mac, silently doing nothing
   in release builds and breaking `CCRenderTexture` screenshots. Implemented
   with ImageIO.
4. **Garbled text on Retina displays.** `_initWithString` rendered through
   `-[NSImage lockFocus]` and grabbed the result with
   `-initWithFocusedViewRect:`, which returns a 2x backing store, then copied it
   as if it were 1x -- every label came out as a quarter of its glyphs smeared
   across the texture. Now draws into a bitmap of exactly the requested size.
5. **Custom fonts never loaded.** Font names arrive as resource paths
   (`fonts/KBStickToThePlan.ttf`) but were only looked up as installed font
   families, so every label silently fell back to the system font. Now
   registered with CoreText and resolved by PostScript name.

Fixes 4 and 5 are what make the HUD and hint text legible.

`main.mm` replaces the stock `NSApplicationMain` + `MainMenu.xib` startup (xib
compilation needs Xcode's `ibtool`) with a programmatic one. It also makes the
GL context current before handing control to cocos2d: `EAGLView` only does that
in `-prepareOpenGL`, which AppKit defers until the first draw, so
`CCConfiguration::gatherGPUInfo` used to `strlen(NULL)` on `glGetString` and
segfault at launch.

## Not ported

Everything behind the Android `CppConnectorActivity`: Google Analytics, the
native share sheet, interstitial ads, and the Facebook-backed ranking screen.
The `proj.android/` project is untouched and still reflects the original build.
