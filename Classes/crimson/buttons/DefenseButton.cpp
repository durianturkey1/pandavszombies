#include "DefenseButton.h"

#include "../defense/DefenseChapterLayer.h"

namespace dxco {

DefenseButton::DefenseButton(cocos2d::CCSprite* sprite) : ButtonDxCo(sprite) {
	this->on();
}

void DefenseButton::execute() {
	cocos2d::CCDirector::sharedDirector()->pushScene(DefenseChapterLayer::scene());
}

} /* namespace dxco */
