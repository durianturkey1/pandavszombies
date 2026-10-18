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

The window is resizable and locked to the game's 3:2 aspect. The engine renders
at the window's real pixel count (Retina included) while the screens keep their
480x320 design resolution, letterboxed with `kResolutionShowAll` so nothing is
ever cropped or pushed off-screen -- on a laptop window, a 16:9 Android phone or
a 19.5:9 iPhone alike. Mouse input is converted from points to pixels, so the
on-screen joysticks work by click and drag.

Two things this fixes that were visibly wrong before: text and sprites were
rendered at 480x320 and then stretched by the display (blurry), and because
`AppDelegate` picks its asset scale from the *frame* size, a 480x320 frame chose
the 1x bucket -- leaving every sprite that is sized in raw pixels about four
times larger than on a phone, which is what pushed layouts off the right edge.

## What this port changes

Game code (`Classes/`) -- the Android JNI shims are now guarded so the same
files compile on both platforms. Changes under `Classes/crimson/` are committed
here; `Classes/dxco/` is the [lib-dxco](https://github.com/DxCo-Games/lib-dxco)
submodule, whose changes live in a fork
([durianturkey1/lib-dxco](https://github.com/durianturkey1/lib-dxco), branch
`macos-port`) that `.gitmodules` now points at.

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

6. **Word wrapping split words mid-word.** The Mac text path appended
   characters one at a time and inserted a break the moment a line overflowed,
   producing "bullets bounce o / ff wall / s". AppKit now does the wrapping.
7. **Mouse input ignored Retina.** Events arrive in points while the engine's
   frame is in pixels, so clicks landed at half their apparent position once the
   view backs a Retina drawable.

Fixes 4, 5 and 6 are what make the HUD, cards and hint text legible.

`main.mm` replaces the stock `NSApplicationMain` + `MainMenu.xib` startup (xib
compilation needs Xcode's `ibtool`) with a programmatic one. It also makes the
GL context current before handing control to cocos2d: `EAGLView` only does that
in `-prepareOpenGL`, which AppKit defers until the first draw, so
`CCConfiguration::gatherGPUInfo` used to `strlen(NULL)` on `glGetString` and
segfault at launch.

## Defense mode

`Classes/crimson/defense/` adds a third mode to the main menu, alongside the
original Story and Survival levels, which are untouched.

The panda holds the gate at the bottom of the lane and fires automatically at
whichever zombie is closest to breaking through. Kills earn levels; each level
pauses the action and offers three upgrade cards -- extra projectiles, bullets
that pierce or bounce, faster fire, more damage, faster bullets, slower zombies.
A chapter is a fixed set of waves: clear them all and the next chapter unlocks
(stored under `defense.chapter`); let one zombie cross the gate and the run ends.
Kills pay coins scaled by how tough the zombie was, and clearing a chapter pays
a bonus, so defense funds the store on its own -- you keep what you earned even
on a loss.

There are 16 chapters, picked from a grid that only opens the ones you have
cleared and resizes itself to however many the config declares. Difficulty is a plain formula in `DefenseLayer` (`getWaveCount`,
`getWaveSize`, `getZombieLife`, `getZombieSpeed`, `getSpawnInterval`): waves get
longer, zombie health compounds at 1.35x per chapter, spawns tighten, the zombie
roster widens (new sprite sets at chapters 2, 4, 7 and 10), elites start
appearing at chapter 3, and every chapter ends on a boss wave. Cards live in
`DefenseStats.cpp`; adding one means adding an enum value, its effect in
`apply`, two strings and an icon.

The store's **ATTACK** upgrade also scales defense damage (+50% per purchase).
That is deliberate: a run's cards reset every chapter while chapter difficulty
compounds, so without a permanent power source the later chapters cannot be
beaten with any gun. It is the reason to keep spending coins.

**Guns** (`DefenseWeapon`) are sold in the same store as the stat upgrades and
appear first in the list. The first tap buys one, any tap after that equips it,
and the equipped gun sets the starting stats for a defense run -- pistol, SMG,
shotgun, rifle, flamer, bazooka, each trading fire rate against damage and
coverage. Level-up cards stack on top, so MULTISHOT is worth far more on a
shotgun than on a rifle.

The menu button reuses the unused `buttons/challenge_button.png` art, so it
reads "challenge" on screen. Swap that file to rename the mode.

## Not ported

Everything behind the Android `CppConnectorActivity`: Google Analytics, the
native share sheet, interstitial ads, and the Facebook-backed ranking screen.
The `proj.android/` project is untouched and still reflects the original build.
