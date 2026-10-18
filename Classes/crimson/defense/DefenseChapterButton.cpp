#include "DefenseChapterButton.h"

#include "DefenseLayer.h"
#include "../../dxco/SpriteUtil.h"
#include "../../dxco/StringUtil.h"

#include "SimpleAudioEngine.h"

namespace dxco {

DefenseChapterButton::DefenseChapterButton(int chapter, bool unlocked, bool cleared,
		bool milestone, float x, float y, float width, float height) {

	this->chapter = chapter;
	this->unlocked = unlocked;
	this->cleared = cleared;
	this->milestone = milestone;

	// Three states worth telling apart at a glance: cleared, open, locked.
	std::string texture = "buttons/LEVELS-bloqueado.png";
	if (cleared) {
		texture = "buttons/LEVELS-x3.png";
	} else if (unlocked) {
		texture = "buttons/LEVELS-x0.png";
	}
	this->sprite = SpriteUtil::create(texture, 0, 0, width, height);
	SpriteUtil::moveToAbsolute((cocos2d::CCSprite*) this->sprite, x, y);
	// Boss chapters are tinted so the run-up to one is visible on the grid.
	if (milestone) {
		((cocos2d::CCSprite*) this->sprite)->setColor(unlocked
				? cocos2d::ccc3(150, 90, 230) : cocos2d::ccc3(90, 60, 130));
	}

	this->on();

	this->label = cocos2d::CCLabelTTF::create(StringUtil::toString(chapter).c_str(),
			"fonts/KBStickToThePlan.ttf", 16);
	// Locked numbers used to be near-black on red and unreadable.
	this->label->setColor(unlocked ? cocos2d::ccc3(255, 255, 255) : cocos2d::ccc3(232, 150, 160));

	if (milestone && unlocked) {
		this->label->setColor(cocos2d::ccc3(255, 225, 90));
	}
	this->label->setPositionX(x);
	this->label->setPositionY(y + height * 0.22f);
}

void DefenseChapterButton::execute() {
	if (!this->unlocked) {
		CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("sounds/cannotbuy.mp3");
		return;
	}

	CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("sounds/bonus.ogg");
	CocosDenshion::SimpleAudioEngine::sharedEngine()->stopBackgroundMusic();
	cocos2d::CCDirector::sharedDirector()->replaceScene(DefenseLayer::scene(this->chapter));
}

} /* namespace dxco */
