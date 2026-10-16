#include "DefenseChapterLayer.h"

#include "DefenseChapterButton.h"
#include "DefenseWeapon.h"
#include "../buttons/BackButton.h"
#include "../../dxco/DB.h"
#include "../../dxco/LabelUtil.h"
#include "../../dxco/Language.h"
#include "../../dxco/SpriteUtil.h"

namespace dxco {

#define CHAPTER_COLUMNS 6
#define CHAPTER_FONT "fonts/KBStickToThePlan.ttf"

DefenseChapterLayer::DefenseChapterLayer() {
	this->backButton = NULL;
}

cocos2d::CCScene* DefenseChapterLayer::scene() {
	cocos2d::CCScene* scene = cocos2d::CCScene::create();

	DefenseChapterLayer* layer = new DefenseChapterLayer();
	layer->init();
	scene->addChild(layer);

	return scene;
}

bool DefenseChapterLayer::init() {
	if (!cocos2d::CCLayer::init()) {
		return false;
	}

	cocos2d::CCSize visibleSize = cocos2d::CCDirector::sharedDirector()->getVisibleSize();
	cocos2d::CCPoint origin = cocos2d::CCDirector::sharedDirector()->getVisibleOrigin();

	cocos2d::CCSprite* background = SpriteUtil::create("fondo_cementerio.jpg", 0, 0,
			visibleSize.width, visibleSize.height);
	this->addChild(background);

	int unlockedUpTo = DB::getInteger("defense.chapter", 1);

	cocos2d::CCLabelTTF* title = LabelUtil::create(Language::get("defense.chapters"), 20,
			0, 0, 0, 0, CHAPTER_FONT);
	title->setAnchorPoint(ccp(0.5, 0.5));
	title->setPositionX(origin.x + visibleSize.width * 0.5f);
	title->setPositionY(origin.y + visibleSize.height * 0.9f);
	LabelUtil::setScaleByHeight(title, visibleSize.height * 0.08f);
	this->addChild(title, 10);

	// Two rows of six, sized from the space they have to share.
	int rows = (DEFENSE_CHAPTER_COUNT + CHAPTER_COLUMNS - 1) / CHAPTER_COLUMNS;
	float cellWidth = visibleSize.width * 0.84f / CHAPTER_COLUMNS;
	float tileWidth = cellWidth * 0.78f;
	float tileHeight = tileWidth * 1.5f;
	float rowGap = visibleSize.height * 0.06f;
	float blockHeight = rows * tileHeight + (rows - 1) * rowGap;
	float firstRowY = origin.y + visibleSize.height * 0.46f + blockHeight / 2 - tileHeight / 2;
	float left = origin.x + (visibleSize.width - CHAPTER_COLUMNS * cellWidth) / 2 + cellWidth / 2;

	for (int i = 0; i < DEFENSE_CHAPTER_COUNT; i++) {
		int chapter = i + 1;
		int row = i / CHAPTER_COLUMNS;
		int column = i % CHAPTER_COLUMNS;

		float x = left + column * cellWidth;
		float y = firstRowY - row * (tileHeight + rowGap);

		DefenseChapterButton* button = new DefenseChapterButton(chapter,
				chapter <= unlockedUpTo, x, y, tileWidth, tileHeight);

		this->addChild(button->sprite, 10);
		this->addChild(button->label, 11);
		this->chapterButtons.push_back(button);
	}

	cocos2d::CCSprite* backSprite = SpriteUtil::create("buttons/menu_icon_button.png", 0, 0);
	float backScale = (visibleSize.height * 0.14f) / SpriteUtil::getHeight(backSprite);
	backSprite->setScale(backScale);
	SpriteUtil::moveToAbsolute(backSprite,
			origin.x + visibleSize.width - SpriteUtil::getWidth(backSprite) * 0.75f,
			origin.y + visibleSize.height - SpriteUtil::getHeight(backSprite) * 0.75f);
	this->addChild(backSprite, 20);
	this->backButton = new BackButton(backSprite);

	this->messageLabel = LabelUtil::create(" ", 18, 0, 0, 0, 0, CHAPTER_FONT);
	this->messageLabel->setAnchorPoint(ccp(0.5, 0.5));
	this->messageLabel->setPositionX(origin.x + visibleSize.width * 0.5f);
	this->messageLabel->setPositionY(origin.y + visibleSize.height * 0.07f);
	this->messageLabel->setColor(cocos2d::ccc3(240, 90, 90));
	LabelUtil::setScaleByHeight(this->messageLabel, visibleSize.height * 0.055f);
	this->addChild(this->messageLabel, 20);

	this->setTouchEnabled(true);
	this->setKeypadEnabled(true);

	return true;
}

void DefenseChapterLayer::ccTouchesBegan(cocos2d::CCSet *pTouches, cocos2d::CCEvent *pEvent) {
	cocos2d::CCTouch* touch = (cocos2d::CCTouch*) (pTouches->anyObject());
	cocos2d::CCPoint location = touch->getLocationInView();
	location = cocos2d::CCDirector::sharedDirector()->convertToGL(location);

	if (this->backButton->isTouched(location)) {
		this->backButton->execute();
		return;
	}

	for (int i = 0; i < (int) this->chapterButtons.size(); i++) {
		if (!this->chapterButtons[i]->isTouched(location)) {
			continue;
		}

		// Say why nothing happened instead of just refusing silently.
		DefenseChapterButton* button = (DefenseChapterButton*) this->chapterButtons[i];
		if (!button->unlocked) {
			this->messageLabel->setString(Language::get("defense.locked").c_str());
		}

		this->chapterButtons[i]->execute();
		return;
	}
}

} /* namespace dxco */
