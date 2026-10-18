#include "DefenseOffer.h"

#include "DefenseConfig.h"
#include "DefenseWeapon.h"
#include "../GameProperties.h"
#include "../daos/UserDAO.h"
#include "../../dxco/DB.h"
#include "../../dxco/Language.h"
#include "../../dxco/StringUtil.h"

namespace dxco {

#define OFFERS "offers"

DefenseOffer::DefenseOffer(int index) {
	this->index = index;
}

bool DefenseOffer::isValid() const {
	return this->index >= 0 && this->index < DefenseConfig::get()->getCount(OFFERS);
}

std::string DefenseOffer::getId() const {
	return DefenseConfig::get()->getString(OFFERS, this->index, "id", "");
}

std::string DefenseOffer::getTitle() const {
	return DefenseConfig::get()->getText(OFFERS, this->index, "title", " ");
}

std::string DefenseOffer::getDetail() const {
	return DefenseConfig::get()->getText(OFFERS, this->index, "detail", " ");
}

std::string DefenseOffer::getImagePath() const {
	return DefenseConfig::get()->getString(OFFERS, this->index, "icon",
			"equip/MEJORA_regalo_act.png");
}

int DefenseOffer::getCost() const {
	return DefenseConfig::get()->getInt(OFFERS, this->index, "cost", 0);
}

bool DefenseOffer::isFree() const {
	return this->getCost() <= 0;
}

bool DefenseOffer::isOnce() const {
	return DefenseConfig::get()->getInt(OFFERS, this->index, "once", 1) == 1;
}

bool DefenseOffer::isTaken() const {
	return DB::getInteger("offer.taken." + this->getId(), 0) == 1;
}

bool DefenseOffer::doublesRunReward() const {
	return DefenseConfig::get()->getInt(OFFERS, this->index, "doubleRunReward", 0) == 1;
}

int DefenseOffer::getRewardCoins() const {
	return DefenseConfig::get()->getInt(OFFERS, this->index, "rewardCoins", 0);
}

int DefenseOffer::getRewardAttackLevels() const {
	return DefenseConfig::get()->getInt(OFFERS, this->index, "rewardAttackLevels", 0);
}

std::vector<std::string> DefenseOffer::getRewardWeapons() const {
	return DefenseConfig::get()->getStringArray(OFFERS, this->index, "rewardWeapons");
}

std::string DefenseOffer::describeReward(int runReward) const {
	if (this->doublesRunReward()) {
		return "+" + StringUtil::toString(runReward) + " " + Language::get("defense.coins");
	}

	std::string summary;
	int coins = this->getRewardCoins();

	if (coins > 0) {
		summary = "+" + StringUtil::toString(coins) + " " + Language::get("defense.coins");
	}

	std::vector<std::string> weapons = this->getRewardWeapons();
	for (int i = 0; i < (int) weapons.size(); i++) {
		if (!summary.empty()) {
			summary += " + ";
		}
		summary += DefenseWeapon::fromKey(weapons[i]).getName();
	}

	int attack = this->getRewardAttackLevels();
	if (attack > 0) {
		if (!summary.empty()) {
			summary += " + ";
		}
		summary += StringUtil::toString(attack) + "x " + Language::get("equip.attack.name");
	}

	return summary;
}

bool DefenseOffer::claim(int runReward) {
	int cost = this->getCost();

	if (cost > 0 && UserDAO::getCoins() < cost) {
		return false;
	}

	if (cost > 0) {
		UserDAO::addCoins(-cost);
	}

	if (this->doublesRunReward()) {
		UserDAO::addCoins(runReward);
	} else {
		UserDAO::addCoins(this->getRewardCoins());
	}

	std::vector<std::string> weapons = this->getRewardWeapons();
	for (int i = 0; i < (int) weapons.size(); i++) {
		DefenseWeapon weapon = DefenseWeapon::fromKey(weapons[i]);
		weapon.unlock();

		// Equipping the gun that was just handed over saves a trip to the store.
		if (i == 0) {
			weapon.equip();
		}
	}

	for (int i = 0; i < this->getRewardAttackLevels(); i++) {
		GameProperties::powerUp("attack.damage");
	}

	if (this->isOnce()) {
		DB::putInteger("offer.taken." + this->getId(), 1);
	}

	return true;
}

DefenseOffer DefenseOffer::forFirstRun() {
	DefenseConfig* config = DefenseConfig::get();
	int count = config->getCount(OFFERS);

	for (int i = 0; i < count; i++) {
		DefenseOffer offer(i);

		if (config->getString(OFFERS, i, "trigger", "") == "firstRun" && !offer.isTaken()) {
			return offer;
		}
	}

	return DefenseOffer();
}

DefenseOffer DefenseOffer::forChapterCleared(int chapter) {
	DefenseConfig* config = DefenseConfig::get();
	int count = config->getCount(OFFERS);
	DefenseOffer fallback;

	for (int i = 0; i < count; i++) {
		if (config->getString(OFFERS, i, "trigger", "") != "chapterCleared") {
			continue;
		}

		DefenseOffer offer(i);

		if (offer.isOnce() && offer.isTaken()) {
			continue;
		}

		int target = config->getInt(OFFERS, i, "chapter", 0);

		if (target == chapter) {
			return offer;  // a pack tied to this exact chapter wins
		}

		// chapter 0 means "after any chapter", kept as the fallback.
		if (target == 0 && !fallback.isValid()) {
			fallback = offer;
		}
	}

	return fallback;
}

} /* namespace dxco */
