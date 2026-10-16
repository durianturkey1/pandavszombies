#include "EquipPandaItem.h"
#include "EquipPandaLayer.h"
#include "GameTypeSelectionLayer.h"
#include "../../dxco/SpriteUtil.h"
#include "../../dxco/LabelUtil.h"
#include "../../dxco/StringUtil.h"
#include "../../dxco/Language.h"
#include "../GameProperties.h"
#include "../daos/UserDAO.h"
#include "../buttons/BuyPowerUpButton.h"
#include "../defense/DefenseWeapon.h"
#include "cocos2d.h"
#include "SimpleAudioEngine.h"

#include <cstdlib>

namespace dxco {

EquipPandaItem::EquipPandaItem(std::string item, float x, float y, float width,
		float height,cocos2d::CCSprite* scaleMaster) :
		Container(x, y, width, height) {
	this->item = item;
	this->price = 0;

	cocos2d::CCSprite* background = SpriteUtil::create(
			this->getImagePath().c_str(), 0, height * 0.2, scaleMaster);
	SpriteUtil::topAlign(scaleMaster, background);
	CCLOG("Width: %f, Scaled: %f", width, SpriteUtil::getWidth(background));

	width = SpriteUtil::getWidth(background);
	this->addChild(background);

	cocos2d::CCLabelTTF* nameLabel = LabelUtil::create(this->getName(), 19, 0,
			0, 0, 0, "fonts/KBStickToThePlan.ttf");
	nameLabel->setAnchorPoint(ccp(0.5, 0.5));
	LabelUtil::setScaleByWidth(nameLabel, width * 0.5);

	cocos2d::CCPoint origin =
			cocos2d::CCDirector::sharedDirector()->getVisibleOrigin();
	CCLOG("Name Label Width: %f", LabelUtil::getWidth(nameLabel));
	nameLabel->setPositionX(width * 0.52 + origin.x);
	nameLabel->setPositionY(height * 0.59 + origin.y);

	nameLabel->setColor(cocos2d::ccc3(62, 62, 62));

	this->addChild(nameLabel);

	cocos2d::CCLabelTTF* detailLabel = cocos2d::CCLabelTTF::create(this->getDetail().c_str(), "fonts/KBStickToThePlan.ttf",
			10, cocos2d::CCSize(85,0), cocos2d::kCCTextAlignmentCenter);

	detailLabel->setPositionX(nameLabel->getPositionX());
	detailLabel->setPositionY(height * 0.43 + origin.y);
	detailLabel->setColor(cocos2d::ccc3(62, 62, 62));
	this->addChild(detailLabel);


	cocos2d::CCSprite* coin = SpriteUtil::create("coin.png", width * 0.09,
			height * 0.22, scaleMaster);
	this->addChild(coin);

	int fontSize = 13;

	if (this->getPrice() > 1000000) {
		fontSize = 9;
	}

	cocos2d::CCLabelTTF* coinsLabel = LabelUtil::create(StringUtil::toString(this->getPrice()), fontSize,
			0, 0, 0, 0, "fonts/KBStickToThePlan.ttf");

	coinsLabel->setAnchorPoint(ccp(0.5, 0.5));
	LabelUtil::setScaleByHeight(nameLabel, width * 0.12);

	float coinsLabelWidth = LabelUtil::getWidth(coinsLabel);
	float coinsLabelHeight = LabelUtil::getHeight(coinsLabel);

	float nameLabelX = width * 0.14 + origin.x + SpriteUtil::getWidth(coin) + 17;
	float nameLabelY = coin->getPositionY();

	coinsLabel->setPositionX(nameLabelX);
	coinsLabel->setPositionY(nameLabelY);

	coinsLabel->setColor(cocos2d::ccc3(62, 62, 62));

	this->addChild(coinsLabel);

	if (!(this->item == "bazooka.unlocked" ||this->item == "fire.unlocked" || this->item == "firebullet.unlocked" || this->item == "frenzy.unlocked"
			|| DefenseWeapon::isWeaponKey(this->item))) {
		std::string cantidadStr = StringUtil::toString(
				this->getCantidadDisponible());

		cocos2d::CCLabelTTF* cantidadLabel = LabelUtil::create(cantidadStr, 18, 0,
				0, 0, 0, "fonts/KBStickToThePlan.ttf");

		LabelUtil::setScaleByHeight(cantidadLabel, width * 0.15);

		float cantidadLabelHeight = LabelUtil::getHeight(cantidadLabel);
		float cantidadLabelWidth = LabelUtil::getWidth(cantidadLabel);

		float cantidadLabelX = (cantidadLabelWidth / 2) + origin.x + width * 0.17;
		float cantidadLabelY = height - (cantidadLabelHeight / 2) + origin.y
				- width * 0.19;

		cantidadLabel->setPositionX(cantidadLabelX);
		cantidadLabel->setPositionY(cantidadLabelY);

		if (!this->isActivo()) {
			cantidadLabel->setColor(cocos2d::ccc3(181, 181, 181));
		}

		this->addChild(cantidadLabel);
	}
	cocos2d::CCSprite* buyButton = SpriteUtil::create(this->getBuyImagePath(),
			0, 0, scaleMaster);
	SpriteUtil::bottomAlign(scaleMaster, buyButton);
	buyButton->setPositionX(background->getPositionX());
	this->addChild(buyButton);
	this->button = new BuyPowerUpButton(buyButton, this);

}


int EquipPandaItem::getCantidadDisponible() {

	int result = 5;

	if (DefenseWeapon::isWeaponKey(this->item)) {
		return DefenseWeapon::fromKey(this->item).isUnlocked() ? 0 : 1;
	}

	if (this->item == "bazooka.unlocked" || this->item == "fire.unlocked" || this->item == "firebullet.unlocked" || this->item == "frenzy.unlocked") {
		if (GameProperties::get(this->item)) {
			result = 0;
		} else {
			result = 1;
		}
	//FIXME sacar la opcion despues de desbloquear. El damage se mejora en conjunto con las otras armas
	} else if (this->item == "bazooka.damage") {
		if (!GameProperties::get("bazooka.unlocked")) {
			// si todavia no destrabamos la bazooka no podemos mejorarla.
			return 0;
		} else {
			result = GameProperties::get("bazooka.damage.level");
		}
	} else {
		result = GameProperties::get(this->item + std::string(".level"));
	}

	return result;
}

bool EquipPandaItem::isActivo() {
	return getCantidadDisponible() > 0;
}

std::vector<std::string> EquipPandaItem::getMejoras() {

	std::vector<std::string> mejoras;

	// Guns for defense mode come first: they are bought once here and then
	// equipped by tapping again, and they change the game far more than a
	// single stat point does.
	std::vector<std::string> weapons = DefenseWeapon::getStoreKeys();
	for (int i = 0; i < (int) weapons.size(); i++) {
		mejoras.push_back(weapons[i]);
	}

	mejoras.push_back("attack.damage");
	mejoras.push_back("player.life");
	mejoras.push_back("bonus.probability");
	mejoras.push_back("weapon.duration");
	mejoras.push_back("player.speed");

	mejoras.push_back("firebullet.unlocked");
	mejoras.push_back("bazooka.unlocked");
	mejoras.push_back("fire.unlocked");
	mejoras.push_back("frenzy.unlocked");

	return mejoras;
}

std::string EquipPandaItem::getBuyImagePath() {
	if (DefenseWeapon::isWeaponKey(this->item)) {
		// Owned-but-not-equipped guns stay tappable, so keep the live button.
		return DefenseWeapon::fromKey(this->item).isEquipped()
				? "buttons/buy-inactivo.png" : "buttons/buy.png";
	}

	if (this->isActivo()) {
		return "buttons/buy.png";
	}

	return "buttons/buy-inactivo.png";
}


std::string EquipPandaItem::getImagePath() {

	if (DefenseWeapon::isWeaponKey(this->item)) {
		return DefenseWeapon::fromKey(this->item).getImagePath();
	}

	std::string result = "equip/MEJORA_";

	if (this->item == "bazooka.unlocked") {
		result.append("bazooka.png");

		return result;
	} else if (this->item == "fire.unlocked" || this->item == "firebullet.unlocked") {
		result.append("fuego.png");
		return result;
	} else if (this->item == "player.life") {
		result.append("energia");
	} else if (this->item == "player.speed") {
		result.append("velocidad");
	} else if (this->item == "bonus.probability") {
		result.append("regalo");
	} else if (this->item == "frenzy.unlocked") {
		result.append("frensy.png");
		return result;
	} else if (this->item == "weapon.duration") {
		result.append("reloj");
	} else if (this->item == "attack.damage") {
		result.append("balasv");
	}

	if (this->isActivo()) {
		result.append("_act.png");
	} else {
		result.append("_desac.png");
	}

	return result;
}

std::string EquipPandaItem::getName() {

	if (DefenseWeapon::isWeaponKey(this->item)) {
		return DefenseWeapon::fromKey(this->item).getName();
	}

	if (this->item == "bazooka.unlocked") {
		return Language::get("equip.bazooka.name");
	} else if (this->item == "fire.unlocked") {
		return Language::get("equip.flame.name");
	} else if (this->item == "frenzy.unlocked") {
		return Language::get("equip.frenzy.name");
	} else if (this->item == "firebullet.unlocked") {
		return Language::get("equip.fireball.name");
	} else if (this->item == "player.life") {
		return Language::get("equip.life.name");
	} else if (this->item == "player.speed") {
		return Language::get("equip.speed.name");
	} else if (this->item == "bonus.probability") {
		return Language::get("equip.bonus.name");
	} else if (this->item == "weapon.duration") {
		return Language::get("equip.weapon.name");
	} else if (this->item == "attack.damage") {
		return Language::get("equip.attack.name");
	}

	return " ";
}

std::string EquipPandaItem::getDetail() {
	if (DefenseWeapon::isWeaponKey(this->item)) {
		DefenseWeapon weapon = DefenseWeapon::fromKey(this->item);

		if (weapon.isEquipped()) {
			return Language::get("weapon.equipped");
		}
		if (weapon.isUnlocked()) {
			return Language::get("weapon.owned");
		}
		return weapon.getDetail();
	}

	if (this->item == "bazooka.unlocked") {
		return Language::get("equip.bazooka.detail");
	} else if (this->item == "fire.unlocked") {
		return Language::get("equip.flame.detail");
	} else if (this->item == "firebullet.unlocked") {
		return Language::get("equip.fireball.detail");
	} else if (this->item == "frenzy.unlocked") {
		return Language::get("equip.frenzy.detail");
	} else if (this->item == "player.life") {
		return Language::get("equip.life.detail");
	} else if (this->item == "player.speed") {
		return Language::get("equip.speed.detail");
	} else if (this->item == "bonus.probability") {
		return Language::get("equip.bonus.detail");
	} else if (this->item == "weapon.duration") {
		return Language::get("equip.weapon.detail");
	} else if (this->item == "attack.damage") {
		return Language::get("equip.attack.detail");
	}
	return " ";
}

float EquipPandaItem::getMaxX() {
	return this->getPositionX() + (this->getWidth() / 2);
}

int EquipPandaItem::getPrice() {
	if (DefenseWeapon::isWeaponKey(this->item)) {
		return DefenseWeapon::fromKey(this->item).getPrice();
	}

	return GameProperties::getPrice(this->item);
}

void EquipPandaItem::applyPowerUp() {

	CCLOG("Upgrade item %s", this->item.c_str());
	int price = this->getPrice();

	if (DefenseWeapon::isWeaponKey(this->item)) {
		this->applyWeapon();
		return;
	}

	if (this->isActivo() && UserDAO::getCoins() >= price) {
		CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("sounds/bonus.ogg");
		CCLOG("Upgrade item %s active", this->item.c_str());

		UserDAO::addCoins(-price);
		GameProperties::powerUp(this->item);

		cocos2d::CCDirector* pDirector = cocos2d::CCDirector::sharedDirector();

		if (this->item == "frenzy.unlocked") {
			pDirector->popToSceneStackLevel(1);
			pDirector->replaceScene(GameTypeSelectionLayer::scene());
			CocosDenshion::SimpleAudioEngine::sharedEngine()->stopBackgroundMusic();
			CocosDenshion::SimpleAudioEngine::sharedEngine()->stopAllEffects();
		} else {//TODO reload scene, hay alguna forma mejor de hacerlo? Si, pero es bastante mas facill hacerlo asi =(.
			pDirector->popScene();
			pDirector->pushScene(EquipPandaLayer::scene());
		}

	} else if (UserDAO::getCoins() < price) {
		CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("sounds/cannotbuy.mp3");
		this->showMessage(Language::get("equip.nocoins"));
	} else {
		// Owned outright, or every level of the upgrade already bought.
		CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("sounds/cannotbuy.mp3");
		this->showMessage(Language::get("equip.maxed"));
	}
}

/**
 * Guns behave differently from the stat upgrades around them: the first tap
 * buys one, and any tap after that simply equips it.
 */
void EquipPandaItem::applyWeapon() {
	DefenseWeapon weapon = DefenseWeapon::fromKey(this->item);

	if (!weapon.isUnlocked()) {
		if (UserDAO::getCoins() < weapon.getPrice()) {
			CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("sounds/cannotbuy.mp3");
			this->showMessage(Language::get("equip.nocoins"));
			return;
		}

		UserDAO::addCoins(-weapon.getPrice());
		weapon.unlock();
	}

	weapon.equip();
	CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("sounds/bonus.ogg");
	this->showMessage(weapon.getName() + " " + Language::get("weapon.equipped"));

	// Rebuild the store so prices, badges and the equipped marker refresh.
	cocos2d::CCDirector* pDirector = cocos2d::CCDirector::sharedDirector();
	pDirector->popScene();
	pDirector->pushScene(EquipPandaLayer::scene());
}

/**
 * Briefly shows a message over the store. The sound effect alone gives no
 * feedback at all if effects are muted or the reason isn't obvious.
 */
void EquipPandaItem::showMessage(std::string text) {
	cocos2d::CCScene* scene = cocos2d::CCDirector::sharedDirector()->getRunningScene();

	if (!scene) {
		return;
	}

	cocos2d::CCSize visibleSize = cocos2d::CCDirector::sharedDirector()->getVisibleSize();
	cocos2d::CCPoint origin = cocos2d::CCDirector::sharedDirector()->getVisibleOrigin();

	cocos2d::CCLabelTTF* label = LabelUtil::create(text, 20, 0, 0, 0, 0, "fonts/KBStickToThePlan.ttf");
	LabelUtil::setScaleByHeight(label, visibleSize.height * 0.07);
	label->setAnchorPoint(ccp(0.5, 0.5));
	label->setPositionX(origin.x + visibleSize.width / 2);
	label->setPositionY(origin.y + visibleSize.height * 0.88);
	label->setColor(cocos2d::ccc3(222, 36, 36));

	scene->addChild(label, 1000);

	label->runAction(cocos2d::CCSequence::create(
			cocos2d::CCDelayTime::create(1.2f),
			cocos2d::CCFadeOut::create(0.4f),
			cocos2d::CCCallFuncN::create(this, (cocos2d::SEL_CallFuncN)(&EquipPandaItem::removeNode)),
			NULL));
}

void EquipPandaItem::removeNode(cocos2d::CCNode* node) {
	if (node) {
		node->removeFromParentAndCleanup(true);
	}
}

} /* namespace dxco */
