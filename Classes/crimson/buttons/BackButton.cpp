#include "BackButton.h"

#include "cocos2d.h"

namespace dxco {

BackButton::BackButton(cocos2d::CCSprite* sprite) : ButtonDxCo(sprite) {
	this->on();
}

void BackButton::execute() {
	// Only ever used on pushed scenes, so there is always something underneath.
	cocos2d::CCDirector::sharedDirector()->popScene();
}

} /* namespace dxco */
