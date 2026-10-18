#include "DefenseStats.h"

#include "DefenseConfig.h"

#include <cstdlib>

namespace dxco {

#define CARDS "cards"

DefenseStats::DefenseStats() {
	this->damage = 5.0f;
	this->fireInterval = 0.55f;
	this->projectiles = 1;
	this->pierce = 0;
	this->bounce = 0;
	this->bulletSpeed = 260.0f;
	this->spreadDegrees = 11.0f;
	this->enemySlow = 0.0f;
	this->critChance = 0.0f;
}

DefenseCard::DefenseCard(int index) {
	this->index = index;
}

/**
 * Effects are named in the config so a card is pure data. "min"/"max" clamp the
 * result, which is what stops RAPID from collapsing the fire interval to zero.
 */
void DefenseCard::apply(DefenseStats& stats) const {
	DefenseConfig* config = DefenseConfig::get();

	std::string effect = config->getString(CARDS, this->index, "effect", "");
	float value = config->getFloat(CARDS, this->index, "value", 0);
	float minimum = config->getFloat(CARDS, this->index, "min", 0);
	float maximum = config->getFloat(CARDS, this->index, "max", 0);
	bool hasMin = config->has(CARDS, this->index, "min");
	bool hasMax = config->has(CARDS, this->index, "max");

	if (effect == "addProjectiles") {
		stats.projectiles += (int) value;
	} else if (effect == "addPierce") {
		stats.pierce += (int) value;
	} else if (effect == "addBounce") {
		stats.bounce += (int) value;
	} else if (effect == "mulFireInterval") {
		stats.fireInterval *= value;
		if (hasMin && stats.fireInterval < minimum) {
			stats.fireInterval = minimum;
		}
	} else if (effect == "mulDamage") {
		stats.damage *= value;
	} else if (effect == "mulBulletSpeed") {
		stats.bulletSpeed *= value;
	} else if (effect == "mulSpread") {
		stats.spreadDegrees *= value;
		if (hasMax && stats.spreadDegrees > maximum) {
			stats.spreadDegrees = maximum;
		}
	} else if (effect == "addSlow") {
		stats.enemySlow += value;
		if (hasMax && stats.enemySlow > maximum) {
			stats.enemySlow = maximum;
		}
	} else if (effect == "addCrit") {
		stats.critChance += value;
		if (hasMax && stats.critChance > maximum) {
			stats.critChance = maximum;
		}
	} else if (effect == "focus") {
		// Trades coverage for punch, the mirror of WIDE.
		stats.spreadDegrees *= value;
		stats.damage *= 1.15f;
	}
}

std::string DefenseCard::getName() const {
	return DefenseConfig::get()->getText(CARDS, this->index, "name", " ");
}

std::string DefenseCard::getDetail() const {
	return DefenseConfig::get()->getText(CARDS, this->index, "detail", " ");
}

std::string DefenseCard::getImagePath() const {
	return DefenseConfig::get()->getString(CARDS, this->index, "icon",
			"equip/MEJORA_balas_act.png");
}

std::vector<DefenseCard> DefenseCard::draw(int count) {
	std::vector<int> pool;
	int total = DefenseConfig::get()->getCount(CARDS);

	for (int i = 0; i < total; i++) {
		pool.push_back(i);
	}

	std::vector<DefenseCard> result;

	for (int i = 0; i < count && !pool.empty(); i++) {
		int index = rand() % pool.size();
		result.push_back(DefenseCard(pool[index]));
		pool.erase(pool.begin() + index);
	}

	return result;
}

} /* namespace dxco */
