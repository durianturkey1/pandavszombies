#include "DefenseOfferLayer.h"

#include "DefenseIcon.h"
#include "../daos/UserDAO.h"
#include "../../dxco/LabelUtil.h"
#include "../../dxco/Language.h"
#include "../../dxco/SpriteUtil.h"
#include "../../dxco/StringUtil.h"

#include "SimpleAudioEngine.h"

namespace dxco {

#define OFFER_FONT "fonts/KBStickToThePlan.ttf"

DefenseOfferLayer::DefenseOfferLayer() {
	this->runReward = 0;
	this->closed = false;
	this->claimButton = NULL;
	this->dismissLabel = NULL;
	this->errorLabel = NULL;
}

DefenseOfferLayer* DefenseOfferLayer::create(DefenseOffer offer, int runReward) {
	DefenseOfferLayer* layer = new DefenseOfferLayer();

	if (layer && layer->build(offer, runReward)) {
		layer->autorelease();
		return layer;
	}

	CC_SAFE_DELETE(layer);
	return NULL;
}

bool DefenseOfferLayer::build(DefenseOffer offer, int runReward) {
	if (!cocos2d::CCLayer::init() || !offer.isValid()) {
		return false;
	}

	this->offer = offer;
	this->runReward = runReward;

	cocos2d::CCSize visibleSize = cocos2d::CCDirector::sharedDirector()->getVisibleSize();
	cocos2d::CCPoint origin = cocos2d::CCDirector::sharedDirector()->getVisibleOrigin();
	float centerX = origin.x + visibleSize.width / 2;
	float centerY = origin.y + visibleSize.height / 2;

	// Dim everything behind the popup so it reads as a modal.
	cocos2d::CCLayerColor* dim = cocos2d::CCLayerColor::create(
			cocos2d::ccc4(0, 0, 0, 170), visibleSize.width, visibleSize.height);
	dim->setPosition(ccp(origin.x, origin.y));
	this->addChild(dim, 0);

	float panelWidth = visibleSize.width * 0.62f;
	float panelHeight = visibleSize.height * 0.70f;
	float panelLeft = centerX - panelWidth / 2;
	float panelBottom = centerY - panelHeight / 2;

	// The panel is drawn rather than taken from the store's card art: that art
	// carries a baked-in quantity badge and divider line, which looked like
	// rendering bugs behind the text.
	cocos2d::CCLayerColor* border = cocos2d::CCLayerColor::create(
			cocos2d::ccc4(250, 220, 90, 255), panelWidth + 6, panelHeight + 6);
	border->setPosition(ccp(panelLeft - 3, panelBottom - 3));
	this->addChild(border, 1);

	cocos2d::CCLayerColor* panel = cocos2d::CCLayerColor::create(
			cocos2d::ccc4(28, 28, 34, 250), panelWidth, panelHeight);
	panel->setPosition(ccp(panelLeft, panelBottom));
	this->addChild(panel, 2);

	cocos2d::CCLabelTTF* title = LabelUtil::create(offer.getTitle(), 20, 0, 0, 0, 0, OFFER_FONT);
	title->setAnchorPoint(ccp(0.5, 0.5));
	title->setPositionX(centerX);
	title->setPositionY(panelBottom + panelHeight * 0.88f);
	title->setColor(cocos2d::ccc3(255, 214, 80));
	LabelUtil::setScaleByWidth(title, panelWidth * 0.76f);
	this->addChild(title, 3);

	cocos2d::CCSprite* icon = DefenseIcon::create(offer.getImagePath(), panelHeight * 0.24f);
	if (icon != NULL) {
		SpriteUtil::moveToAbsolute(icon, centerX, panelBottom + panelHeight * 0.66f);
		this->addChild(icon, 3);
	}

	cocos2d::CCLabelTTF* detail = cocos2d::CCLabelTTF::create(offer.getDetail().c_str(),
			OFFER_FONT, 11, cocos2d::CCSize(panelWidth * 0.80f, 0),
			cocos2d::kCCTextAlignmentCenter);
	detail->setAnchorPoint(ccp(0.5, 1));
	detail->setPositionX(centerX);
	detail->setPositionY(panelBottom + panelHeight * 0.54f);
	detail->setColor(cocos2d::ccc3(228, 228, 228));
	this->addChild(detail, 3);

	cocos2d::CCLabelTTF* reward = LabelUtil::create(offer.describeReward(runReward), 16,
			0, 0, 0, 0, OFFER_FONT);
	reward->setAnchorPoint(ccp(0.5, 0.5));
	reward->setPositionX(centerX);
	reward->setPositionY(panelBottom + panelHeight * 0.33f);
	reward->setColor(cocos2d::ccc3(130, 230, 130));
	LabelUtil::setScaleByHeight(reward, panelHeight * 0.075f);
	this->addChild(reward, 3);

	// The action button is drawn too. buttons/buy.png has the word "buy" baked
	// into the artwork, which collided with the price label on top of it.
	float buttonWidth = panelWidth * 0.56f;
	float buttonHeight = panelHeight * 0.15f;
	float buttonLeft = centerX - buttonWidth / 2;
	float buttonBottom = panelBottom + panelHeight * 0.14f;

	cocos2d::CCLayerColor* buttonBorder = cocos2d::CCLayerColor::create(
			cocos2d::ccc4(255, 255, 255, 255), buttonWidth + 4, buttonHeight + 4);
	buttonBorder->setPosition(ccp(buttonLeft - 2, buttonBottom - 2));
	this->addChild(buttonBorder, 3);

	cocos2d::CCLayerColor* buttonFill = cocos2d::CCLayerColor::create(
			cocos2d::ccc4(206, 32, 42, 255), buttonWidth, buttonHeight);
	buttonFill->setPosition(ccp(buttonLeft, buttonBottom));
	this->addChild(buttonFill, 4);

	// Invisible sprite kept as the hit target so the touch test stays simple.
	this->claimButton = SpriteUtil::create("white.png", 0, 0, buttonWidth, buttonHeight);
	SpriteUtil::moveToAbsolute(this->claimButton, centerX, buttonBottom + buttonHeight / 2);
	this->claimButton->setOpacity(0);
	this->addChild(this->claimButton, 5);

	std::string action = offer.isFree()
			? Language::get("offer.claim")
			: StringUtil::toString(offer.getCost()) + " " + Language::get("defense.coins");

	cocos2d::CCLabelTTF* actionLabel = LabelUtil::create(action, 18, 0, 0, 0, 0, OFFER_FONT);
	actionLabel->setAnchorPoint(ccp(0.5, 0.5));
	actionLabel->setPositionX(centerX);
	actionLabel->setPositionY(buttonBottom + buttonHeight / 2);
	LabelUtil::setScaleByHeight(actionLabel, buttonHeight * 0.62f);
	this->addChild(actionLabel, 6);

	// Dismissing is a labelled button, not an icon: an offer must never feel
	// like a dead end.
	cocos2d::CCLabelTTF* dismiss = LabelUtil::create(Language::get("offer.dismiss"), 14,
			0, 0, 0, 0, OFFER_FONT);
	dismiss->setAnchorPoint(ccp(0.5, 0.5));
	dismiss->setPositionX(centerX);
	dismiss->setPositionY(panelBottom + panelHeight * 0.055f);
	dismiss->setColor(cocos2d::ccc3(190, 190, 190));
	LabelUtil::setScaleByHeight(dismiss, visibleSize.height * 0.045f);
	this->addChild(dismiss, 3);
	this->dismissLabel = dismiss;

	this->errorLabel = LabelUtil::create(" ", 14, 0, 0, 0, 0, OFFER_FONT);
	this->errorLabel->setAnchorPoint(ccp(0.5, 0.5));
	this->errorLabel->setPositionX(centerX);
	this->errorLabel->setPositionY(panelBottom - visibleSize.height * 0.045f);
	this->errorLabel->setColor(cocos2d::ccc3(240, 90, 90));
	LabelUtil::setScaleByHeight(this->errorLabel, visibleSize.height * 0.045f);
	this->addChild(this->errorLabel, 3);

	this->setTouchEnabled(true);

	return true;
}

void DefenseOfferLayer::ccTouchesBegan(cocos2d::CCSet *pTouches, cocos2d::CCEvent *pEvent) {
	if (this->closed) {
		return;
	}

	cocos2d::CCTouch* touch = (cocos2d::CCTouch*) (pTouches->anyObject());
	cocos2d::CCPoint location = touch->getLocationInView();
	location = cocos2d::CCDirector::sharedDirector()->convertToGL(location);

	if (this->dismissLabel->boundingBox().containsPoint(location)) {
		this->close();
		return;
	}

	if (this->claimButton->boundingBox().containsPoint(location)) {
		if (this->offer.claim(this->runReward)) {
			CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("sounds/bonus.ogg");
			this->close();
		} else {
			CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("sounds/cannotbuy.mp3");
			this->errorLabel->setString(Language::get("equip.nocoins").c_str());
		}
	}
}

void DefenseOfferLayer::close() {
	this->closed = true;
	this->setTouchEnabled(false);
	this->setVisible(false);

	// Removal is deferred by a frame: this runs from inside the layer's own
	// touch handler, and freeing the delegate while the dispatcher is walking
	// its list is how that turns into a crash.
	this->runAction(cocos2d::CCSequence::create(
			cocos2d::CCDelayTime::create(0),
			cocos2d::CCCallFunc::create(this,
					(cocos2d::SEL_CallFunc)(&DefenseOfferLayer::removeSelf)),
			NULL));
}

void DefenseOfferLayer::removeSelf() {
	this->removeFromParentAndCleanup(true);
}

bool DefenseOfferLayer::isClosed() {
	return this->closed;
}

} /* namespace dxco */
