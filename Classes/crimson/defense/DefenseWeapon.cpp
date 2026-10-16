#include "DefenseWeapon.h"

#include "../../dxco/DB.h"
#include "../../dxco/Language.h"

namespace dxco {

#define EQUIPPED_KEY "weapon.equipped"

DefenseWeapon::DefenseWeapon(Id id) {
	this->id = id;
}

std::string DefenseWeapon::getKey() const {
	switch (this->id) {
		case WEAPON_PISTOL:  return "weapon.pistol";
		case WEAPON_SMG:     return "weapon.smg";
		case WEAPON_SHOTGUN: return "weapon.shotgun";
		case WEAPON_RIFLE:   return "weapon.rifle";
		case WEAPON_FLAMER:  return "weapon.flamer";
		case WEAPON_BAZOOKA: return "weapon.bazooka";
	}
	return "weapon.pistol";
}

std::string DefenseWeapon::getName() const {
	return Language::get(this->getKey() + ".name");
}

std::string DefenseWeapon::getDetail() const {
	return Language::get(this->getKey() + ".detail");
}

std::string DefenseWeapon::getImagePath() const {
	switch (this->id) {
		case WEAPON_PISTOL:  return "equip/MEJORA_balas_act.png";
		case WEAPON_SMG:     return "equip/MEJORA_balasv_act.png";
		case WEAPON_SHOTGUN: return "equip/MEJORA_bomba_act.png";
		case WEAPON_RIFLE:   return "equip/MEJORA_mochila_act.png";
		case WEAPON_FLAMER:  return "equip/MEJORA_fuego.png";
		case WEAPON_BAZOOKA: return "equip/MEJORA_bazooka.png";
	}
	return "equip/MEJORA_balas_act.png";
}

std::string DefenseWeapon::getBulletImagePath() const {
	switch (this->id) {
		case WEAPON_FLAMER:  return "fire.png";
		case WEAPON_BAZOOKA: return "bazooka_bullet.png";
		default:             return "bullet.png";
	}
}

int DefenseWeapon::getPrice() const {
	switch (this->id) {
		case WEAPON_PISTOL:  return 0;
		case WEAPON_SMG:     return 1200;
		case WEAPON_SHOTGUN: return 2600;
		case WEAPON_RIFLE:   return 4500;
		case WEAPON_FLAMER:  return 7000;
		case WEAPON_BAZOOKA: return 11000;
	}
	return 0;
}

// Each gun trades fire rate against damage and coverage, so no single one is
// strictly best: the shotgun clears crowds, the rifle kills armoured leaders.
DefenseStats DefenseWeapon::getBaseStats() const {
	DefenseStats stats;

	switch (this->id) {
		case WEAPON_PISTOL:
			break;  // the defaults in DefenseStats are the pistol
		case WEAPON_SMG:
			stats.damage = 3.5f;
			stats.fireInterval = 0.20f;
			stats.bulletSpeed = 320.0f;
			stats.spreadDegrees = 7.0f;
			break;
		case WEAPON_SHOTGUN:
			stats.damage = 4.0f;
			stats.fireInterval = 0.85f;
			stats.projectiles = 4;
			stats.bulletSpeed = 250.0f;
			stats.spreadDegrees = 13.0f;
			break;
		case WEAPON_RIFLE:
			stats.damage = 16.0f;
			stats.fireInterval = 0.90f;
			stats.bulletSpeed = 460.0f;
			stats.spreadDegrees = 4.0f;
			stats.pierce = 1;
			break;
		case WEAPON_FLAMER:
			stats.damage = 2.2f;
			stats.fireInterval = 0.08f;
			stats.bulletSpeed = 190.0f;
			stats.spreadDegrees = 16.0f;
			break;
		case WEAPON_BAZOOKA:
			stats.damage = 45.0f;
			stats.fireInterval = 1.70f;
			stats.bulletSpeed = 230.0f;
			stats.spreadDegrees = 3.0f;
			stats.pierce = 2;
			break;
	}

	return stats;
}

bool DefenseWeapon::isFree() const {
	return this->id == WEAPON_PISTOL;
}

bool DefenseWeapon::isUnlocked() const {
	return this->isFree() || DB::getInteger(this->getKey(), 0) == 1;
}

bool DefenseWeapon::isEquipped() const {
	return DefenseWeapon::getEquipped().id == this->id;
}

void DefenseWeapon::unlock() const {
	DB::putInteger(this->getKey(), 1);
}

void DefenseWeapon::equip() const {
	DB::putInteger(EQUIPPED_KEY, (int) this->id);
}

std::vector<DefenseWeapon> DefenseWeapon::all() {
	std::vector<DefenseWeapon> weapons;
	weapons.push_back(DefenseWeapon(WEAPON_PISTOL));
	weapons.push_back(DefenseWeapon(WEAPON_SMG));
	weapons.push_back(DefenseWeapon(WEAPON_SHOTGUN));
	weapons.push_back(DefenseWeapon(WEAPON_RIFLE));
	weapons.push_back(DefenseWeapon(WEAPON_FLAMER));
	weapons.push_back(DefenseWeapon(WEAPON_BAZOOKA));
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

// Matches against the real list rather than a "weapon." prefix: the store
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

	return DefenseWeapon(WEAPON_PISTOL);
}

DefenseWeapon DefenseWeapon::getEquipped() {
	int stored = DB::getInteger(EQUIPPED_KEY, (int) WEAPON_PISTOL);
	DefenseWeapon weapon((Id) stored);

	// Never hand back a gun the player does not actually own.
	if (!weapon.isUnlocked()) {
		return DefenseWeapon(WEAPON_PISTOL);
	}

	return weapon;
}

} /* namespace dxco */
