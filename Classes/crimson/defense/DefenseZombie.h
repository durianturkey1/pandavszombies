#ifndef DEFENSEZOMBIE_H_
#define DEFENSEZOMBIE_H_

#include "cocos2d.h"
#include "../../dxco/Animation.h"

#include <string>

namespace dxco {

/**
 * A zombie walking straight down the lane in defense mode.
 *
 * Deliberately not the Enemy class from story mode: that one carries steering
 * behaviour, a GameModel pointer and the whole level/bonus system with it, none
 * of which applies when the only thing a zombie does is march at the gate.
 */
class DefenseZombie {
public:
	DefenseZombie(std::string type, float x, float y, float speed, float life,
			float scale, cocos2d::CCNode* parent);

	void update(float dt, float slowFactor);
	void hurt(float damage);
	bool isDead();
	void remove();

	float getX();
	float getY();
	float getRadius();

	cocos2d::CCSprite* sprite;
	Animation* walk;
	float life;
	float maxLife;
	float speed;
	float scale;
	bool dead;
};

} /* namespace dxco */

#endif /* DEFENSEZOMBIE_H_ */
