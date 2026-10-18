#include "DefenseChapterLayer.h"

#include "DefenseChapterButton.h"
#include "DefenseWeapon.h"
#include "DefenseConfig.h"
#include "DefenseIcon.h"
#include "DefenseOffer.h"
#include "DefenseOfferLayer.h"
#include "../buttons/BackButton.h"
#include "../layers/EquipPandaLayer.h"
#include "../daos/UserDAO.h"
#include "../../dxco/StringUtil.h"
#include "../../dxco/DB.h"
#include "../../dxco/LabelUtil.h"
#include "../../dxco/Language.h"
#include "../../dxco/SpriteUtil.h"

namespace dxco {

#define CHAPTER_COLUMNS 6
#define CHAPTER_FONT "fonts/KBStickToThePlan.ttf"
#define OFFER_POPUP_TAG 7001

DefenseChapterLayer::DefenseChapterLayer() {
	this->backButton = NULL;
	this->shopButton = NULL;
}

DefenseChapterLayer::~DefenseChapterLayer() {
	for (int i = 0; i < (int) this->chapterButtons.size(); i++) {
		delete this->chapterButtons[i];
	}

	this->chapterButtons.clear();
	delete this->backButton;
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
	int chapterCount = DefenseConfig::get()->getChapterInt("count", DEFENSE_CHAPTER_COUNT);

	cocos2d::CCLabelTTF* title = LabelUtil::create(Language::get("defense.chapters"), 20,
			0, 0, 0, 0, CHAPTER_FONT);
	title->setAnchorPoint(ccp(0.5, 0.5));
	title->setPositionX(origin.x + visibleSize.width * 0.5f);
	title->setPositionY(origin.y + visibleSize.height * 0.9f);
	LabelUtil::setScaleByHeight(title, visibleSize.height * 0.08f);
	this->addChild(title, 10);

	// The grid sizes itself to however many chapters the config declares: the
	// tiles shrink to fit the band between the title and the bottom margin, and
	// every row -- including a short last one -- is centred on its own.
	int rows = (chapterCount + CHAPTER_COLUMNS - 1) / CHAPTER_COLUMNS;
	float bandTop = origin.y + visibleSize.height * 0.82f;
	float bandBottom = origin.y + visibleSize.height * 0.10f;
	float bandHeight = bandTop - bandBottom;
	float rowGap = visibleSize.height * 0.035f;

	float cellWidth = visibleSize.width * 0.88f / CHAPTER_COLUMNS;
	float tileWidth = cellWidth * 0.80f;
	float tileHeight = tileWidth * 1.5f;

	float neededHeight = rows * tileHeight + (rows - 1) * rowGap;
	if (neededHeight > bandHeight) {
		float shrink = (bandHeight - (rows - 1) * rowGap) / (rows * tileHeight);
		tileHeight *= shrink;
		tileWidth *= shrink;
		neededHeight = rows * tileHeight + (rows - 1) * rowGap;
	}

	float firstRowY = bandBottom + neededHeight - tileHeight / 2;

	for (int i = 0; i < chapterCount; i++) {
		int chapter = i + 1;
		int row = i / CHAPTER_COLUMNS;
		int column = i % CHAPTER_COLUMNS;

		// How many tiles share this row, so a partial row stays centred.
		int inRow = chapterCount - row * CHAPTER_COLUMNS;
		if (inRow > CHAPTER_COLUMNS) {
			inRow = CHAPTER_COLUMNS;
		}

		float rowLeft = origin.x + (visibleSize.width - inRow * cellWidth) / 2 + cellWidth / 2;
		float x = rowLeft + column * cellWidth;
		float y = firstRowY - row * (tileHeight + rowGap);

		DefenseChapterButton* button = new DefenseChapterButton(chapter,
				chapter <= unlockedUpTo, chapter < unlockedUpTo, x, y, tileWidth, tileHeight);

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

	// Coin balance doubles as the way into the store: the guns are sold there,
	// and backing out to the main menu to find them was a detour.
	this->shopButton = SpriteUtil::create("equip/coins.png", 0, 0);
	float shopScale = (visibleSize.height * 0.11f) / SpriteUtil::getHeight(this->shopButton);
	this->shopButton->setScale(shopScale);
	SpriteUtil::moveToAbsolute(this->shopButton,
			origin.x + SpriteUtil::getWidth(this->shopButton) * 0.7f,
			origin.y + visibleSize.height - SpriteUtil::getHeight(this->shopButton) * 0.75f);
	this->addChild(this->shopButton, 20);

	// equip/coins.png is a pill with the coin on the left and room for the
	// number inside it, which is how the store uses it -- putting the number
	// beside the pill ran it into the centred title.
	cocos2d::CCLabelTTF* coins = LabelUtil::create(StringUtil::toString(UserDAO::getCoins()),
			18, 0, 0, 0, 0, CHAPTER_FONT);
	coins->setAnchorPoint(ccp(0, 0.5));
	coins->setPositionX(this->shopButton->getPositionX()
			- SpriteUtil::getWidth(this->shopButton) * 0.12f);
	coins->setPositionY(this->shopButton->getPositionY());
	coins->setColor(cocos2d::ccc3(255, 255, 255));
	LabelUtil::setScaleByHeight(coins, SpriteUtil::getHeight(this->shopButton) * 0.42f);
	this->addChild(coins, 21);

	// Which gun the run will start with, so the choice is visible before
	// committing to a chapter rather than only once the HUD appears.
	DefenseWeapon equipped = DefenseWeapon::getEquipped();
	cocos2d::CCSprite* gunIcon = DefenseIcon::create(equipped.getImagePath(),
			visibleSize.height * 0.09f);

	if (gunIcon != NULL) {
		SpriteUtil::moveToAbsolute(gunIcon,
				origin.x + visibleSize.width * 0.68f,
				origin.y + visibleSize.height * 0.945f);
		this->addChild(gunIcon, 20);
	}

	cocos2d::CCLabelTTF* gunName = LabelUtil::create(equipped.getName(), 16, 0, 0, 0, 0, CHAPTER_FONT);
	gunName->setAnchorPoint(ccp(0, 0.5));
	gunName->setPositionX(origin.x + visibleSize.width * 0.715f);
	gunName->setPositionY(origin.y + visibleSize.height * 0.945f);
	gunName->setColor(cocos2d::ccc3(210, 210, 210));
	LabelUtil::setScaleByHeight(gunName, visibleSize.height * 0.05f);
	this->addChild(gunName, 20);

	this->messageLabel = LabelUtil::create(" ", 18, 0, 0, 0, 0, CHAPTER_FONT);
	this->messageLabel->setAnchorPoint(ccp(0.5, 0.5));
	this->messageLabel->setPositionX(origin.x + visibleSize.width * 0.5f);
	this->messageLabel->setPositionY(origin.y + visibleSize.height * 0.045f);
	this->messageLabel->setColor(cocos2d::ccc3(240, 90, 90));
	LabelUtil::setScaleByHeight(this->messageLabel, visibleSize.height * 0.055f);
	this->addChild(this->messageLabel, 20);

	this->showLastResult();

	this->setTouchEnabled(true);
	this->setKeypadEnabled(true);

	this->showPendingOffer();

	return true;
}

/**
 * A lost run drops straight back to this list, so without this the only trace
 * of it was the coin balance having gone up a little.
 */
void DefenseChapterLayer::showLastResult() {
	int failed = DB::getInteger("defense.lastFailed", 0);

	if (failed <= 0) {
		return;
	}

	int reward = DB::getInteger("defense.lastReward", 0);
	int wave = DB::getInteger("defense.lastWave", 0);

	this->messageLabel->setColor(cocos2d::ccc3(240, 150, 90));
	this->messageLabel->setString((Language::get("defense.failed") + "  "
			+ Language::get("defense.wave") + " " + StringUtil::toString(wave)
			+ "  +" + StringUtil::toString(reward)).c_str());

	DB::putInteger("defense.lastFailed", 0);
}

/**
 * Offers surface here rather than mid-run: the welcome gift the first time the
 * mode is opened, and a pack on returning from a chapter that was just cleared.
 */
void DefenseChapterLayer::showPendingOffer() {
	DefenseOffer offer;
	int runReward = DB::getInteger("defense.lastReward", 0);
	int clearedChapter = DB::getInteger("defense.lastCleared", 0);

	if (clearedChapter > 0) {
		offer = DefenseOffer::forChapterCleared(clearedChapter);
		DB::putInteger("defense.lastCleared", 0);
	} else {
		offer = DefenseOffer::forFirstRun();
	}

	if (!offer.isValid()) {
		return;
	}

	DefenseOfferLayer* popup = DefenseOfferLayer::create(offer, runReward);

	if (popup != NULL) {
		popup->setTag(OFFER_POPUP_TAG);
		this->addChild(popup, 100);
	}
}

void DefenseChapterLayer::ccTouchesBegan(cocos2d::CCSet *pTouches, cocos2d::CCEvent *pEvent) {
	cocos2d::CCTouch* touch = (cocos2d::CCTouch*) (pTouches->anyObject());
	cocos2d::CCPoint location = touch->getLocationInView();
	location = cocos2d::CCDirector::sharedDirector()->convertToGL(location);

	// An open offer popup owns the screen.
	if (this->getChildByTag(OFFER_POPUP_TAG) != NULL) {
		return;
	}

	if (this->backButton->isTouched(location)) {
		this->backButton->execute();
		return;
	}

	if (this->shopButton != NULL && this->shopButton->boundingBox().containsPoint(location)) {
		cocos2d::CCDirector::sharedDirector()->pushScene(EquipPandaLayer::scene());
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
