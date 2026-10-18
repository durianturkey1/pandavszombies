#include "DefenseWeapon.h"

#include "DefenseConfig.h"
#include "../../dxco/DB.h"

namespace dxco {

#define EQUIPPED_KEY "weapon.equipped.key"
#define WEAPONS "weapons"

DefenseWeapon::DefenseWeapon(int index) {
	this->index = index;
}

std::string DefenseWeapon::getKey() const {
	return DefenseConfig::get()->getString(WEAPONS, this->index, "key", "weapon.pistol");
}

std::string DefenseWeapon::getName() const {
	return DefenseConfig::get()->getText(WEAPONS, this->index, "name", " ");
}

std::string DefenseWeapon::getDetail() const {
	return DefenseConfig::get()->getText(WEAPONS, this->index, "detail", " ");
}

std::string DefenseWeapon::getImagePath() const {
	return DefenseConfig::get()->getString(WEAPONS, this->index, "icon",
			"equip/MEJORA_balas_act.png");
}

std::string DefenseWeapon::getBulletImagePath() const {
	return DefenseConfig::get()->getString(WEAPONS, this->index, "bullet", "bullet.png");
}

std::string DefenseWeapon::getSoundPath() const {
	return DefenseConfig::get()->getString(WEAPONS, this->index, "sound", "sounds/gun1.ogg");
}

int DefenseWeapon::getPrice() const {
	return DefenseConfig::get()->getInt(WEAPONS, this->index, "price", 0);
}

DefenseStats DefenseWeapon::getBaseStats() const {
	DefenseConfig* config = DefenseConfig::get();
	DefenseStats stats;

	stats.damage = config->getFloat(WEAPONS, this->index, "damage", stats.damage);
	stats.fireInterval = config->getFloat(WEAPONS, this->index, "fireInterval", stats.fireInterval);
	stats.projectiles = config->getInt(WEAPONS, this->index, "projectiles", stats.projectiles);
	stats.pierce = config->getInt(WEAPONS, this->index, "pierce", stats.pierce);
	stats.bounce = config->getInt(WEAPONS, this->index, "bounce", stats.bounce);
	stats.bulletSpeed = config->getFloat(WEAPONS, this->index, "bulletSpeed", stats.bulletSpeed);
	stats.spreadDegrees = config->getFloat(WEAPONS, this->index, "spread", stats.spreadDegrees);
	stats.enemySlow = config->getFloat(WEAPONS, this->index, "enemySlow", stats.enemySlow);
	stats.critChance = config->getFloat(WEAPONS, this->index, "critChance", stats.critChance);

	return stats;
}

bool DefenseWeapon::isFree() const {
	return DefenseConfig::get()->getInt(WEAPONS, this->index, "free", 0) == 1;
}

bool DefenseWeapon::isUnlocked() const {
	return this->isFree() || DB::getInteger(this->getKey(), 0) == 1;
}

bool DefenseWeapon::isEquipped() const {
	return DefenseWeapon::getEquipped().getKey() == this->getKey();
}

void DefenseWeapon::unlock() const {
	DB::putInteger(this->getKey(), 1);
}

void DefenseWeapon::equip() const {
	// Stored by key rather than by position so reordering the config, or
	// inserting a gun in the middle of it, cannot silently re-equip another one.
	DB::putString(EQUIPPED_KEY, this->getKey());
}

bool DefenseWeapon::isValid() const {
	return this->index >= 0 && this->index < DefenseConfig::get()->getCount(WEAPONS);
}

std::vector<DefenseWeapon> DefenseWeapon::all() {
	std::vector<DefenseWeapon> weapons;
	int count = DefenseConfig::get()->getCount(WEAPONS);

	for (int i = 0; i < count; i++) {
		weapons.push_back(DefenseWeapon(i));
	}

	return weapons;
}

std::vector<std::string> DefenseWeapon::getStoreKeys() {
	std::vector<DefenseWeapon> weapons = DefenseWeapon::all();
	std::vector<std::string> keys;

	for (int i = 0; i < (int) weapons.size(); i++) {
		if (!weapons[i].isFree()) {
			keys.push_back(weapons[i].getKey());
		}
	}

	return keys;
}

// Matches against the configured list rather than a "weapon." prefix: the store
// already sells an upgrade called "weapon.duration" that is not a gun.
bool DefenseWeapon::isWeaponKey(const std::string& key) {
	std::vector<DefenseWeapon> weapons = DefenseWeapon::all();

	for (int i = 0; i < (int) weapons.size(); i++) {
		if (weapons[i].getKey() == key) {
			return true;
		}
	}

	return false;
}

DefenseWeapon DefenseWeapon::fromKey(const std::string& key) {
	std::vector<DefenseWeapon> weapons = DefenseWeapon::all();

	for (int i = 0; i < (int) weapons.size(); i++) {
		if (weapons[i].getKey() == key) {
			return weapons[i];
		}
	}

	return DefenseWeapon::getDefault();
}

DefenseWeapon DefenseWeapon::getDefault() {
	std::vector<DefenseWeapon> weapons = DefenseWeapon::all();

	for (int i = 0; i < (int) weapons.size(); i++) {
		if (weapons[i].isFree()) {
			return weapons[i];
		}
	}

	return DefenseWeapon(0);
}

DefenseWeapon DefenseWeapon::getEquipped() {
	std::string key = DB::getString(EQUIPPED_KEY);

	if (!key.empty()) {
		DefenseWeapon weapon = DefenseWeapon::fromKey(key);

		// Never hand back a gun the player does not actually own.
		if (weapon.isUnlocked()) {
			return weapon;
		}
	}

	return DefenseWeapon::getDefault();
}

} /* namespace dxco */
