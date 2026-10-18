#ifndef DEFENSEWEAPON_H_
#define DEFENSEWEAPON_H_

#include "DefenseStats.h"

#include <string>
#include <vector>

namespace dxco {

/**
 * A gun the panda can carry in defense mode, backed by an entry in
 * defense/config.json. Guns are bought once in the store and then equipped;
 * level-up cards stack on top of whichever one is equipped, so a card like
 * MULTISHOT is worth far more on a shotgun than on a rifle.
 */
class DefenseWeapon {
public:
	DefenseWeapon(int index = 0);

	/** Position in the config's "weapons" array. */
	int index;

	/** Save key, e.g. "weapon.shotgun". */
	std::string getKey() const;
	std::string getName() const;
	std::string getDetail() const;
	std::string getImagePath() const;
	std::string getBulletImagePath() const;
	std::string getSoundPath() const;
	int getPrice() const;

	bool isFree() const;
	bool isUnlocked() const;
	bool isEquipped() const;
	bool isValid() const;
	void unlock() const;
	void equip() const;

	/** The gun's own stats, before any level-up cards are applied. */
	DefenseStats getBaseStats() const;

	static std::vector<DefenseWeapon> all();
	/** Everything except the free starter gun, which the player always owns. */
	static std::vector<std::string> getStoreKeys();
	static bool isWeaponKey(const std::string& key);
	static DefenseWeapon fromKey(const std::string& key);
	static DefenseWeapon getDefault();
	static DefenseWeapon getEquipped();
};

} /* namespace dxco */

#endif /* DEFENSEWEAPON_H_ */
