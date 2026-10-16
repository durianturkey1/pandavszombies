#include "DefenseButton.h"

#include "../defense/DefenseLayer.h"
#include "../../dxco/DB.h"

#include "SimpleAudioEngine.h"

namespace dxco {

DefenseButton::DefenseButton(cocos2d::CCSprite* sprite) : ButtonDxCo(sprite) {
	this->on();
}

void DefenseButton::execute() {
	int chapter = DB::getInteger("defense.chapter", 1);

	if (chapter < 1) {
		chapter = 1;
	}

	CocosDenshion::SimpleAudioEngine::sharedEngine()->stopBackgroundMusic();
	cocos2d::CCDirector::sharedDirector()->replaceScene(DefenseLayer::scene(chapter));
}

} /* namespace dxco */
