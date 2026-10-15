# Panda vs Zombies

![](ciru.gif)![](basquet.gif)![](cura.gif)

Panda vs Zombies is a top-down shooter videogame for Android, implemented using [cocos2d-x](https://github.com/cocos2d/cocos2d-x). The game is [available in Google Play](https://play.google.com/store/apps/details?id=com.dxco.pandavszombies).

## Installation

Clone the repo and init the [lib-dxco submodule](https://github.com/DxCo-Games/lib-dxco):

```
$ git clone git@github.com:DxCo-Games/pandavszombies.git
$ cd pandavszombies
$ git submodule init
$ git submodule update
```

To build the project follow the [instructions for Android](https://github.com/DxCo-Games/pandavszombies/blob/dev/proj.android/README.md).

## Running on macOS

The Android project dates from 2014 and no longer builds: it needs Ant, an NDK
old enough to ship `gnustl`, and several SDKs (Facebook, Parse, Google Play
Services) that were never committed and are no longer distributed.

`proj.mac/` builds the same game as a native macOS app instead, using only
CMake and the Command Line Tools:

```
$ ./proj.mac/setup.sh
$ cmake -S proj.mac -B proj.mac/build -DCMAKE_BUILD_TYPE=Release
$ cmake --build proj.mac/build -j8
$ open "proj.mac/build/Panda vs Zombies.app"
```

See [proj.mac/README.md](proj.mac/README.md) for what the port changes.
