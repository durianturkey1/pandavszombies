#include "DefenseIcon.h"

#include "../../dxco/SpriteUtil.h"

namespace dxco {

// Fractions of the source card that hold the icon: clear of the badge in the
// top-left corner and above the divider line.
#define ICON_LEFT 0.26f
#define ICON_TOP 0.11f
#define ICON_WIDTH 0.52f
#define ICON_HEIGHT 0.30f

cocos2d::CCSprite* DefenseIcon::create(const std::string& path, float targetHeight) {
	cocos2d::CCSprite* sprite = SpriteUtil::create(path, 0, 0);

	if (sprite == NULL) {
		return NULL;
	}

	cocos2d::CCSize full = sprite->getContentSize();

	if (full.width > 0 && full.height > 0) {
		sprite->setTextureRect(cocos2d::CCRectMake(
				full.width * ICON_LEFT,
				full.height * ICON_TOP,
				full.width * ICON_WIDTH,
				full.height * ICON_HEIGHT));
	}

	float height = sprite->getContentSize().height;

	if (height > 0) {
		sprite->setScale(targetHeight / height);
	}

	return sprite;
}

} /* namespace dxco */
