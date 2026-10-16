#ifndef DEFENSEBULLET_H_
#define DEFENSEBULLET_H_

#include "cocos2d.h"

#include <vector>

namespace dxco {

class DefenseZombie;

/**
 * A bullet fired by the stationary panda. Carries its own pierce and bounce
 * budget so upgrades picked mid-run apply to everything fired afterwards.
 */
class DefenseBullet {
public:
	DefenseBullet(float x, float y, float angleDegrees, float speed, float damage,
			int pierce, int bounce, cocos2d::CCNode* parent);

	/** Moves the bullet; returns false once it should be removed. */
	bool update(float dt, float minX, float maxX, float maxY);

	/** True if this bullet has not already hit that zombie. */
	bool canHit(DefenseZombie* zombie);
	void registerHit(DefenseZombie* zombie);

	void remove();

	float getX();
	float getY();

	cocos2d::CCSprite* sprite;
	float velocityX;
	float velocityY;
	float damage;
	int pierce;
	int bounce;

private:
	std::vector<DefenseZombie*> alreadyHit;
};

} /* namespace dxco */

#endif /* DEFENSEBULLET_H_ */
