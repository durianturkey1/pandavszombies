#include "DefenseZombie.h"

#include "../../dxco/SpriteUtil.h"
#include "../../dxco/StringUtil.h"

namespace dxco {

// Direction 7 of the 8 baked into each zombie sheet is the one walking towards
// the camera, which is what "down the lane" looks like here.
#define ZOMBIE_DOWN_DIRECTION 7
#define ZOMBIE_WALK_FRAMES 20
#define ZOMBIE_SIZE 46

DefenseZombie::DefenseZombie(std::string type, float x, float y, float speed, float life,
		float scale, cocos2d::CCNode* parent) {

	std::string dir = StringUtil::toString(ZOMBIE_DOWN_DIRECTION);

	// SpriteUtil::create places the bottom-left corner; everything here works in
	// centres (collisions included), so position it explicitly afterwards.
	this->scale = scale;
	this->sprite = SpriteUtil::create(type + "_" + dir + "_0000.png", 0, 0,
			ZOMBIE_SIZE * scale, ZOMBIE_SIZE * scale, true);
	SpriteUtil::moveToAbsolute(this->sprite, x, y);
	parent->addChild(this->sprite, 20);

	std::vector<cocos2d::CCSpriteFrame*> frames;
	for (int i = 0; i < ZOMBIE_WALK_FRAMES; i++) {
		frames.push_back(SpriteUtil::createSpriteFrame(
				type + "_" + dir + "_" + StringUtil::padLeft(i, 4) + ".png"));
	}

	this->walk = new Animation(frames, 0.05f);
	this->life = life;
	this->maxLife = life;
	this->speed = speed;
	this->dead = false;
}

void DefenseZombie::update(float dt, float slowFactor) {
	if (this->dead) {
		return;
	}

	this->walk->update(this->sprite, dt);

	float moved = this->speed * (1.0f - slowFactor) * dt;
	this->sprite->setPositionY(this->sprite->getPositionY() - moved);
}

void DefenseZombie::hurt(float damage) {
	this->life -= damage;

	if (this->life <= 0) {
		this->dead = true;
		return;
	}

	// Flash red so hits read clearly even on a fast-moving sprite.
	this->sprite->setColor(cocos2d::ccc3(255, 120, 120));
	this->sprite->runAction(cocos2d::CCTintTo::create(0.12f, 255, 255, 255));
}

bool DefenseZombie::isDead() {
	return this->dead;
}

void DefenseZombie::remove() {
	if (this->sprite) {
		this->sprite->removeFromParentAndCleanup(true);
		this->sprite = NULL;
	}

	if (this->walk) {
		delete this->walk;
		this->walk = NULL;
	}
}

float DefenseZombie::getX() {
	return this->sprite ? this->sprite->getPositionX() : 0;
}

float DefenseZombie::getY() {
	return this->sprite ? this->sprite->getPositionY() : 0;
}

float DefenseZombie::getRadius() {
	return ZOMBIE_SIZE * this->scale * 0.32f;
}

} /* namespace dxco */
