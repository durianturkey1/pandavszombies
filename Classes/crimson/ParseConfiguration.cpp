#include "ParseConfiguration.h"
#include "cocos2d.h"

#if (CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID)
#include "platform/android/jni/JniHelper.h"
#include <jni.h>
#endif

namespace dxco {

#if (CC_TARGET_PLATFORM != CC_PLATFORM_ANDROID)

// The Parse backend these values came from was shut down in 2017, so desktop
// builds just use the same defaults the Android path falls back to.
int ParseConfiguration::getAdMinLevel() {
	return 15;
}

int ParseConfiguration::getShowAdRateSurvival() {
	return 15;
}

int ParseConfiguration::getShowAdRateStory() {
	return 15;
}

#else

int ParseConfiguration::getAdMinLevel() {

	cocos2d::JniMethodInfo t;

	if (cocos2d::JniHelper::getStaticMethodInfo(t, "com/dxco/pandavszombies/ParseConfiguration", "getAdMinLevel", "()I")) {

		return (int) t.env->CallStaticIntMethod(t.classID, t.methodID);
	}

	return 15;
}

int ParseConfiguration::getShowAdRateSurvival() {

	cocos2d::JniMethodInfo t;

	if (cocos2d::JniHelper::getStaticMethodInfo(t, "com/dxco/pandavszombies/ParseConfiguration", "getShowAdRateSurvival", "()I")) {

		return (int) t.env->CallStaticIntMethod(t.classID, t.methodID);
	}

	return 15;
}

int ParseConfiguration::getShowAdRateStory() {

	cocos2d::JniMethodInfo t;

	if (cocos2d::JniHelper::getStaticMethodInfo(t, "com/dxco/pandavszombies/ParseConfiguration", "getShowAdRateStory", "()I")) {

		return (int) t.env->CallStaticIntMethod(t.classID, t.methodID);
	}

	return 15;
}

#endif

} /* namespace dxco */
