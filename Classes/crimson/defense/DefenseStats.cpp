#include "DefenseStats.h"

#include "../../dxco/Language.h"

#include <cstdlib>

namespace dxco {

DefenseStats::DefenseStats() {
	this->damage = 5.0f;
	this->fireInterval = 0.55f;
	this->projectiles = 1;
	this->pierce = 0;
	this->bounce = 0;
	this->bulletSpeed = 260.0f;
	this->spreadDegrees = 11.0f;
	this->enemySlow = 0.0f;
}

DefenseCard::DefenseCard(Kind kind) {
	this->kind = kind;
}

void DefenseCard::apply(DefenseStats& stats) const {
	switch (this->kind) {
		case CARD_MULTISHOT:
			stats.projectiles += 1;
			break;
		case CARD_PIERCE:
			stats.pierce += 1;
			break;
		case CARD_BOUNCE:
			stats.bounce += 1;
			break;
		case CARD_RAPID:
			// Multiplicative so it keeps mattering, floored so the game cannot
			// turn into a solid wall of bullets.
			stats.fireInterval *= 0.80f;
			if (stats.fireInterval < 0.08f) {
				stats.fireInterval = 0.08f;
			}
			break;
		case CARD_POWER:
			stats.damage *= 1.30f;
			break;
		case CARD_VELOCITY:
			stats.bulletSpeed *= 1.20f;
			break;
		case CARD_SLOW:
			stats.enemySlow += 0.10f;
			if (stats.enemySlow > 0.5f) {
				stats.enemySlow = 0.5f;
			}
			break;
	}
}

std::string DefenseCard::getName() const {
	switch (this->kind) {
		case CARD_MULTISHOT: return Language::get("defense.card.multishot.name");
		case CARD_PIERCE:    return Language::get("defense.card.pierce.name");
		case CARD_BOUNCE:    return Language::get("defense.card.bounce.name");
		case CARD_RAPID:     return Language::get("defense.card.rapid.name");
		case CARD_POWER:     return Language::get("defense.card.power.name");
		case CARD_VELOCITY:  return Language::get("defense.card.velocity.name");
		case CARD_SLOW:      return Language::get("defense.card.slow.name");
	}
	return " ";
}

std::string DefenseCard::getDetail() const {
	switch (this->kind) {
		case CARD_MULTISHOT: return Language::get("defense.card.multishot.detail");
		case CARD_PIERCE:    return Language::get("defense.card.pierce.detail");
		case CARD_BOUNCE:    return Language::get("defense.card.bounce.detail");
		case CARD_RAPID:     return Language::get("defense.card.rapid.detail");
		case CARD_POWER:     return Language::get("defense.card.power.detail");
		case CARD_VELOCITY:  return Language::get("defense.card.velocity.detail");
		case CARD_SLOW:      return Language::get("defense.card.slow.detail");
	}
	return " ";
}

// Reuses the upgrade icons the store already ships.
std::string DefenseCard::getImagePath() const {
	switch (this->kind) {
		case CARD_MULTISHOT: return "equip/MEJORA_balas_act.png";
		case CARD_PIERCE:    return "equip/MEJORA_balasv_act.png";
		case CARD_BOUNCE:    return "equip/MEJORA_bomba_act.png";
		case CARD_RAPID:     return "equip/MEJORA_reloj_act.png";
		case CARD_POWER:     return "equip/MEJORA_mochila_act.png";
		case CARD_VELOCITY:  return "equip/MEJORA_velocidad_act.png";
		case CARD_SLOW:      return "equip/MEJORA_congelar_act.png";
	}
	return "equip/MEJORA_balas_act.png";
}

std::vector<DefenseCard> DefenseCard::draw(int count) {
	std::vector<Kind> pool;
	pool.push_back(CARD_MULTISHOT);
	pool.push_back(CARD_PIERCE);
	pool.push_back(CARD_BOUNCE);
	pool.push_back(CARD_RAPID);
	pool.push_back(CARD_POWER);
	pool.push_back(CARD_VELOCITY);
	pool.push_back(CARD_SLOW);

	std::vector<DefenseCard> result;

	for (int i = 0; i < count && !pool.empty(); i++) {
		int index = rand() % pool.size();
		result.push_back(DefenseCard(pool[index]));
		pool.erase(pool.begin() + index);
	}

	return result;
}

} /* namespace dxco */
