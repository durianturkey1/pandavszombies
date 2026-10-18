#include "DefenseBullet.h"

#include "DefenseZombie.h"
#include "../../dxco/SpriteUtil.h"

#include <algorithm>
#include <math.h>

namespace dxco {

#define BULLET_SIZE 10

DefenseBullet::DefenseBullet(const std::string& image, float x, float y, float angleDegrees,
		float speed, float damage, int pierce, int bounce, cocos2d::CCNode* parent) {

	this->sprite = SpriteUtil::create(image, 0, 0, BULLET_SIZE, BULLET_SIZE);
	SpriteUtil::moveToAbsolute(this->sprite, x, y);
	parent->addChild(this->sprite, 25);

	float radians = angleDegrees * (float) M_PI / 180.0f;
	this->velocityX = cos(radians) * speed;
	this->velocityY = sin(radians) * speed;

	this->damage = damage;
	this->pierce = pierce;
	this->bounce = bounce;
}

DefenseBullet::~DefenseBullet() {
	this->remove();
}

bool DefenseBullet::update(float dt, float minX, float maxX, float maxY) {
	if (!this->sprite) {
		return false;
	}

	float x = this->sprite->getPositionX() + this->velocityX * dt;
	float y = this->sprite->getPositionY() + this->velocityY * dt;

	if (x < minX || x > maxX) {
		if (this->bounce <= 0) {
			return false;
		}

		this->bounce--;
		this->velocityX = -this->velocityX;
		x = (x < minX) ? minX : maxX;
	}

	// Bullets always travel upwards, so the top edge is the end of the line.
	if (y > maxY) {
		return false;
	}

	this->sprite->setPosition(ccp(x, y));
	return true;
}

bool DefenseBullet::canHit(DefenseZombie* zombie) {
	return std::find(this->alreadyHit.begin(), this->alreadyHit.end(), zombie) == this->alreadyHit.end();
}

void DefenseBullet::registerHit(DefenseZombie* zombie) {
	this->alreadyHit.push_back(zombie);
}

void DefenseBullet::remove() {
	if (this->sprite) {
		this->sprite->removeFromParentAndCleanup(true);
		this->sprite = NULL;
	}
}

float DefenseBullet::getX() {
	return this->sprite ? this->sprite->getPositionX() : 0;
}

float DefenseBullet::getY() {
	return this->sprite ? this->sprite->getPositionY() : 0;
}

} /* namespace dxco */
