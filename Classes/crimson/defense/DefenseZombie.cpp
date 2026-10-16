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

	// Anything tougher than a basic zombie carries a health bar, so a boss
	// reads as "keep shooting" instead of "nothing is happening".
	this->healthBar = NULL;
	this->healthBarWidth = ZOMBIE_SIZE * scale * 0.7f;

	if (scale > 1.05f) {
		cocos2d::CCLayerColor* back = cocos2d::CCLayerColor::create(
				cocos2d::ccc4(0, 0, 0, 150), this->healthBarWidth, 4);
		back->ignoreAnchorPointForPosition(false);
		back->setAnchorPoint(ccp(0.5, 0.5));
		parent->addChild(back, 21);

		this->healthBar = cocos2d::CCLayerColor::create(
				cocos2d::ccc4(230, 70, 70, 230), this->healthBarWidth, 4);
		this->healthBar->ignoreAnchorPointForPosition(false);
		this->healthBar->setAnchorPoint(ccp(0, 0.5));
		parent->addChild(this->healthBar, 22);

		this->healthBarBack = back;
	} else {
		this->healthBarBack = NULL;
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

	if (this->healthBar) {
		float barY = this->sprite->getPositionY()
				+ SpriteUtil::getHeight(this->sprite) * 0.55f;
		float ratio = (this->maxLife > 0) ? this->life / this->maxLife : 0;
		if (ratio < 0) { ratio = 0; }

		this->healthBarBack->setPosition(ccp(this->sprite->getPositionX(), barY));
		this->healthBar->setPosition(ccp(this->sprite->getPositionX() - this->healthBarWidth / 2, barY));
		this->healthBar->setContentSize(cocos2d::CCSize(this->healthBarWidth * ratio, 4));
	}
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
	if (this->healthBar) {
		this->healthBar->removeFromParentAndCleanup(true);
		this->healthBar = NULL;
	}

	if (this->healthBarBack) {
		this->healthBarBack->removeFromParentAndCleanup(true);
		this->healthBarBack = NULL;
	}

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
