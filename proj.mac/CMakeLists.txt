# macOS desktop build for Panda vs Zombies.
#
# The game was written in 2014/2015 against cocos2d-x 2.2.x and only ever had an
# Android project (Eclipse + Ant + ndk-build). This builds the same C++ game code
# as a native Mac app with a current toolchain: no Xcode project, no prebuilt
# third-party binaries, everything compiled from source against the macOS SDK.
#
#   cmake -S proj.mac -B proj.mac/build && cmake --build proj.mac/build
#
# Expects the cocos2d-x 2.2.6 source tree at <repo>/cocos2d-x (override with
# -DCOCOS2DX_ROOT=/path/to/cocos2d-x).

cmake_minimum_required(VERSION 3.20)
project(PandaVsZombies LANGUAGES C CXX OBJC OBJCXX)

set(GAME_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(COCOS2DX_ROOT "${GAME_ROOT}/cocos2d-x" CACHE PATH "Path to the cocos2d-x 2.2.6 source tree")

if(NOT EXISTS "${COCOS2DX_ROOT}/cocos2dx/cocos2d.cpp")
    message(FATAL_ERROR "cocos2d-x 2.2.6 not found at ${COCOS2DX_ROOT}. "
                        "Download it with:\n"
                        "  curl -L -o cocos.tgz https://codeload.github.com/cocos2d/cocos2d-x/tar.gz/refs/tags/cocos2d-x-2.2.6\n"
                        "  tar xzf cocos.tgz && mv cocos2d-x-cocos2d-x-2.2.6 ${COCOS2DX_ROOT}")
endif()

# cocos2d-x 2.x predates C++11 idioms and was built with libstdc++; gnu++11 is the
# newest dialect its sources still compile cleanly under. Warnings are silenced
# because a decade-old engine emits thousands of them and none are actionable here.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0")
add_compile_options(-w -fno-objc-arc)

# The engine's Objective-C sources assume Foundation is already imported, which
# Xcode did through cocos2dx-Prefix.pch. Reproduce that as a forced include.
add_compile_options($<$<COMPILE_LANGUAGE:OBJC,OBJCXX>:-include$<SEMICOLON>${COCOS2DX_ROOT}/cocos2dx/proj.mac/cocos2dx-Prefix.pch>)
add_definitions(-DCC_TARGET_OS_MAC -DUSE_FILE32API -DCOCOS2D_DEBUG=1)

# libxml2 headers live inside the SDK, not /usr/include.
execute_process(COMMAND xcrun --show-sdk-path OUTPUT_VARIABLE OSX_SDK_PATH
                OUTPUT_STRIP_TRAILING_WHITESPACE)

include_directories(
    ${COCOS2DX_ROOT}/cocos2dx
    ${COCOS2DX_ROOT}/cocos2dx/include
    ${COCOS2DX_ROOT}/cocos2dx/kazmath/include
    # The Mac platform sources include engine headers by bare name (the Xcode
    # project relied on ALWAYS_SEARCH_USER_PATHS), so these need to be visible.
    ${COCOS2DX_ROOT}/cocos2dx/cocoa
    ${COCOS2DX_ROOT}/cocos2dx/platform
    ${COCOS2DX_ROOT}/cocos2dx/platform/mac
    ${COCOS2DX_ROOT}/cocos2dx/textures
    ${COCOS2DX_ROOT}/cocos2dx/touch_dispatcher
    ${COCOS2DX_ROOT}/cocos2dx/text_input_node
    ${COCOS2DX_ROOT}/cocos2dx/base_nodes
    ${COCOS2DX_ROOT}/cocos2dx/keypad_dispatcher
    ${COCOS2DX_ROOT}/cocos2dx/sprite_nodes
    ${COCOS2DX_ROOT}/cocos2dx/support
    ${COCOS2DX_ROOT}/cocos2dx/shaders
    ${COCOS2DX_ROOT}/CocosDenshion/include
    ${COCOS2DX_ROOT}/extensions
    ${GAME_ROOT}/Classes
    # rapidjson ships inside the dxco submodule, where the game's own JSON
    # loaders include it as <rapidjson/...>.
    ${GAME_ROOT}/Classes/dxco
    ${OSX_SDK_PATH}/usr/include/libxml2
)

# ---------------------------------------------------------------------------
# cocos2d-x engine
# ---------------------------------------------------------------------------
file(GLOB COCOS_SRC
    ${COCOS2DX_ROOT}/cocos2dx/*.cpp
    ${COCOS2DX_ROOT}/cocos2dx/*.c
    ${COCOS2DX_ROOT}/cocos2dx/actions/*.cpp
    ${COCOS2DX_ROOT}/cocos2dx/base_nodes/*.cpp
    ${COCOS2DX_ROOT}/cocos2dx/cocoa/*.cpp
    ${COCOS2DX_ROOT}/cocos2dx/draw_nodes/*.cpp
    ${COCOS2DX_ROOT}/cocos2dx/effects/*.cpp
    ${COCOS2DX_ROOT}/cocos2dx/kazmath/src/*.c
    ${COCOS2DX_ROOT}/cocos2dx/kazmath/src/GL/*.c
    ${COCOS2DX_ROOT}/cocos2dx/keypad_dispatcher/*.cpp
    ${COCOS2DX_ROOT}/cocos2dx/label_nodes/*.cpp
    ${COCOS2DX_ROOT}/cocos2dx/layers_scenes_transitions_nodes/*.cpp
    ${COCOS2DX_ROOT}/cocos2dx/menu_nodes/*.cpp
    ${COCOS2DX_ROOT}/cocos2dx/misc_nodes/*.cpp
    ${COCOS2DX_ROOT}/cocos2dx/particle_nodes/*.cpp
    ${COCOS2DX_ROOT}/cocos2dx/script_support/*.cpp
    ${COCOS2DX_ROOT}/cocos2dx/shaders/*.cpp
    ${COCOS2DX_ROOT}/cocos2dx/sprite_nodes/*.cpp
    ${COCOS2DX_ROOT}/cocos2dx/support/*.cpp
    ${COCOS2DX_ROOT}/cocos2dx/support/component/*.cpp
    ${COCOS2DX_ROOT}/cocos2dx/support/data_support/*.cpp
    ${COCOS2DX_ROOT}/cocos2dx/support/image_support/*.cpp
    ${COCOS2DX_ROOT}/cocos2dx/support/tinyxml2/*.cpp
    ${COCOS2DX_ROOT}/cocos2dx/support/zip_support/*.cpp
    ${COCOS2DX_ROOT}/cocos2dx/text_input_node/*.cpp
    ${COCOS2DX_ROOT}/cocos2dx/textures/*.cpp
    ${COCOS2DX_ROOT}/cocos2dx/tilemap_parallax_nodes/*.cpp
    ${COCOS2DX_ROOT}/cocos2dx/touch_dispatcher/*.cpp
    # Mac keeps its own NSUserDefaults-backed CCUserDefault.
    ${COCOS2DX_ROOT}/cocos2dx/support/user_default/CCUserDefault.mm
    # Platform layer: shared pieces plus the Cocoa/OpenGL backend.
    ${COCOS2DX_ROOT}/cocos2dx/platform/CCEGLViewProtocol.cpp
    ${COCOS2DX_ROOT}/cocos2dx/platform/CCFileUtils.cpp
    ${COCOS2DX_ROOT}/cocos2dx/platform/CCSAXParser.cpp
    ${COCOS2DX_ROOT}/cocos2dx/platform/CCThread.cpp
    ${COCOS2DX_ROOT}/cocos2dx/platform/platform.cpp
    ${COCOS2DX_ROOT}/cocos2dx/platform/mac/*.mm
    ${COCOS2DX_ROOT}/cocos2dx/platform/mac/*.m
    # Audio (OpenAL) and the HTTP client the game's SmartHttpClient wraps.
    ${COCOS2DX_ROOT}/CocosDenshion/mac/*.m
    ${COCOS2DX_ROOT}/CocosDenshion/mac/*.mm
    ${COCOS2DX_ROOT}/extensions/network/HttpClient.cpp
)

# WebP decoding. CCImage.mm always references _initWithWebpData, and 2.2.6 got
# libwebp from a prebuilt binary bundle that no longer makes sense to use (it was
# never built for arm64). Use a system/Homebrew libwebp when one is available,
# otherwise generate a stub -- this game ships no .webp assets either way.
find_path(WEBP_INCLUDE_DIR webp/decode.h HINTS /opt/homebrew/include /usr/local/include)
find_library(WEBP_LIBRARY NAMES webp HINTS /opt/homebrew/lib /usr/local/lib)

if(WEBP_INCLUDE_DIR AND WEBP_LIBRARY)
    message(STATUS "WebP support: ${WEBP_LIBRARY}")
    # The engine source includes "decode.h" unqualified on non-NaCl platforms.
    list(APPEND COCOS_SRC ${COCOS2DX_ROOT}/cocos2dx/platform/CCImageCommonWebp.cpp)
    include_directories(${WEBP_INCLUDE_DIR} ${WEBP_INCLUDE_DIR}/webp)
else()
    message(STATUS "WebP support: disabled (libwebp not found)")
    set(WEBP_STUB "${CMAKE_CURRENT_BINARY_DIR}/CCImageWebpStub.cpp")
    file(WRITE ${WEBP_STUB}
         "#include \"platform/CCImage.h\"\n"
         "NS_CC_BEGIN\n"
         "bool CCImage::_initWithWebpData(void*, int) { return false; }\n"
         "NS_CC_END\n")
    list(APPEND COCOS_SRC ${WEBP_STUB})
endif()

# ---------------------------------------------------------------------------
# Game code (Classes/, including the lib-dxco submodule)
# ---------------------------------------------------------------------------
file(GLOB_RECURSE GAME_SRC ${GAME_ROOT}/Classes/*.cpp)
list(REMOVE_ITEM GAME_SRC
    # Android-only JNI bridge to the RevMob ad SDK.
    ${GAME_ROOT}/Classes/dxco/RevMobAds_Android.cpp
    # Unused, and its only implementation is a JNI call into com/dxco/SoundUtil.
    ${GAME_ROOT}/Classes/dxco/SoundUtil.cpp
    # Dead file: superseded by dxco/TopDownItem.cpp and absent from Android.mk too.
    ${GAME_ROOT}/Classes/crimson/TopDownItem.cpp
    # AdMob banner node, unreferenced by the game and Android-only.
    ${GAME_ROOT}/Classes/dxco/CCAdView.cpp
)

add_executable(PandaVsZombies MACOSX_BUNDLE
    ${COCOS_SRC}
    ${GAME_SRC}
    ${CMAKE_CURRENT_SOURCE_DIR}/main.mm
)

set_target_properties(PandaVsZombies PROPERTIES
    OUTPUT_NAME "Panda vs Zombies"
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Info.plist
)

target_link_libraries(PandaVsZombies PRIVATE
    "-framework Cocoa" "-framework AppKit" "-framework Foundation"
    "-framework CoreFoundation" "-framework QuartzCore" "-framework OpenGL"
    # Deliberately NOT AVFoundation: CocosDenshion's Mac layer declares its own
    # AVAudioPlayer/AVAudioSession shims (NSSound-backed), and linking the real
    # framework would put two classes of the same name in the runtime.
    "-framework OpenAL" "-framework AudioToolbox" "-framework ImageIO" "-framework CoreText"
    z xml2 curl
)

if(WEBP_INCLUDE_DIR AND WEBP_LIBRARY)
    target_link_libraries(PandaVsZombies PRIVATE ${WEBP_LIBRARY})
endif()

# The game expects its assets at the root of the bundle's Resources directory.
# Symlinks keep the 160 MB of art out of the build tree and let asset edits show
# up on the next launch without rebuilding.
add_custom_command(TARGET PandaVsZombies POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_BUNDLE_CONTENT_DIR:PandaVsZombies>/Resources"
    COMMAND /bin/sh -c
            "ln -sfn '${GAME_ROOT}/Resources'/* '$<TARGET_BUNDLE_CONTENT_DIR:PandaVsZombies>/Resources/'"
    COMMENT "Linking game assets into the app bundle"
    VERBATIM
)
