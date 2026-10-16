#ifndef DEFENSESTATS_H_
#define DEFENSESTATS_H_

#include <string>
#include <vector>

namespace dxco {

/**
 * The panda's weapon in defense mode. Starts deliberately weak: one slow
 * bullet, so the first few level-up cards are immediately noticeable.
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
 * One level-up choice. Cards are data, not subclasses: each knows how to
 * change the stats and how to describe itself.
 */
class DefenseCard {
public:
	enum Kind {
		CARD_MULTISHOT,
		CARD_PIERCE,
		CARD_BOUNCE,
		CARD_RAPID,
		CARD_POWER,
		CARD_VELOCITY,
		CARD_SLOW,
		CARD_CRIT
	};

	DefenseCard(Kind kind);

	void apply(DefenseStats& stats) const;
	std::string getName() const;
	std::string getDetail() const;
	std::string getImagePath() const;

	Kind kind;

	/** Three distinct random cards to offer on level up. */
	static std::vector<DefenseCard> draw(int count);
};

} /* namespace dxco */

#endif /* DEFENSESTATS_H_ */
