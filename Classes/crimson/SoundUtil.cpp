#include "SoundUtil.h"
#include "cocos2d.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "platform/android/jni/JniHelper.h"
#include <jni.h>
#endif


namespace dxco {

#if (CC_TARGET_PLATFORM != CC_PLATFORM_ANDROID)

// Desktop builds have no Java settings screen, so sound is simply always on.
bool SoundUtil::isMusicOn() {
	return true;
}

bool SoundUtil::isSoundEffectsOn() {
	return true;
}

#else

bool SoundUtil::isMusicOn() {

	bool result = false;

	cocos2d::JniMethodInfo t;

	if (cocos2d::JniHelper::getStaticMethodInfo(t, "com/dxco/pandavszombies/SoundUtil", "isMusicOnCpp", "()I")) {

		int ret = (int) t.env->CallStaticIntMethod(t.classID, t.methodID);

		if (ret) {
			result = true;
		}
	}

	return result;
}

bool SoundUtil::isSoundEffectsOn() {

	bool result = false;

	cocos2d::JniMethodInfo t;

	if (cocos2d::JniHelper::getStaticMethodInfo(t, "com/dxco/pandavszombies/SoundUtil", "isSoundEffectsCpp", "()I")) {

		int ret = (int) t.env->CallStaticIntMethod(t.classID, t.methodID);

		if (ret) {
			result = true;
		}
	}

	return result;
}

#endif

} /* namespace dxco */
