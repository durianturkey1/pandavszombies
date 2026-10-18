#ifndef DEFENSESTATS_H_
#define DEFENSESTATS_H_

#include <string>
#include <vector>

namespace dxco {

/**
 * The panda's weapon profile for a run: the equipped gun sets these, then every
 * level-up card modifies them.
 */
struct DefenseStats {
	float damage;
	float fireInterval;   // seconds between volleys
	int projectiles;      // bullets per volley
	int pierce;           // extra zombies a bullet passes through
	int bounce;           // times a bullet reflects off the side walls
	float bulletSpeed;    // points per second
	float spreadDegrees;  // angle between projectiles in a volley
	float enemySlow;      // 0..0.5, fraction shaved off zombie speed
	float critChance;     // 0..1, chance a bullet lands for 2.5x

	DefenseStats();
};

/**
 * One level-up choice, backed by an entry in defense/config.json. Cards are
 * data, not subclasses: each knows how to change the stats and how to describe
 * itself, and a new one is an entry in that file.
 */
class DefenseCard {
public:
	DefenseCard(int index = 0);

	/** Position in the config's "cards" array. */
	int index;

	void apply(DefenseStats& stats) const;
	std::string getName() const;
	std::string getDetail() const;
	std::string getImagePath() const;

	/** `count` distinct random cards to offer on level up. */
	static std::vector<DefenseCard> draw(int count);
};

} /* namespace dxco */

#endif /* DEFENSESTATS_H_ */
