#include "DefenseLayer.h"

#include "DefenseBullet.h"
#include "DefenseZombie.h"
#include "DefenseWeapon.h"
#include "DefenseChapterLayer.h"
#include "../SoundUtil.h"
#include "../daos/UserDAO.h"
#include "../layers/GameTypeSelectionLayer.h"
#include "../../dxco/DB.h"
#include "../../dxco/LabelUtil.h"
#include "../../dxco/Language.h"
#include "../../dxco/SpriteUtil.h"
#include "../../dxco/StringUtil.h"

#include "SimpleAudioEngine.h"

#include <cstdlib>
#include <math.h>

namespace dxco {

#define DEFENSE_FONT "fonts/KBStickToThePlan.ttf"
#define DEFENSE_CHAPTER_KEY "defense.chapter"
#define PANDA_SIZE 54
#define CARD_COUNT 3
#define WAVE_BREAK 2.5f

DefenseLayer::DefenseLayer() {
	this->chapter = 1;
	this->state = PLAYING;

	this->panda = NULL;
	this->chapterLabel = NULL;
	this->waveLabel = NULL;
	this->levelLabel = NULL;
	this->messageLabel = NULL;

	this->visibleWidth = 0;
	this->visibleHeight = 0;
	this->originX = 0;
	this->originY = 0;
	this->gateY = 0;
	this->laneTop = 0;

	this->wave = 0;
	this->spawnedThisWave = 0;
	this->spawnTimer = 0;
	this->waveBreakTimer = 0;
	this->fireTimer = 0;

	this->level = 1;
	this->kills = 0;
	this->killsForNextLevel = 6;
	this->coinsEarned = 0;
	this->coinsLabel = NULL;
	this->xpBar = NULL;
	this->xpBarWidth = 0;
}

cocos2d::CCScene* DefenseLayer::scene(int chapter) {
	cocos2d::CCScene* scene = cocos2d::CCScene::create();

	DefenseLayer* layer = new DefenseLayer();
	layer->chapter = chapter;
	layer->init();
	scene->addChild(layer);

	return scene;
}

bool DefenseLayer::init() {
	if (!cocos2d::CCLayer::init()) {
		return false;
	}

	if (!SoundUtil::isSoundEffectsOn()) {
		CocosDenshion::SimpleAudioEngine::sharedEngine()->setEffectsVolume(0);
	}

	cocos2d::CCSize visibleSize = cocos2d::CCDirector::sharedDirector()->getVisibleSize();
	cocos2d::CCPoint origin = cocos2d::CCDirector::sharedDirector()->getVisibleOrigin();

	this->visibleWidth = visibleSize.width;
	this->visibleHeight = visibleSize.height;
	this->originX = origin.x;
	this->originY = origin.y;
	this->gateY = origin.y + visibleSize.height * 0.11f;
	this->laneTop = origin.y + visibleSize.height;

	// Whichever gun is equipped in the store sets the starting weapon profile;
	// level-up cards stack on top of it.
	DefenseWeapon weapon = DefenseWeapon::getEquipped();
	this->stats = weapon.getBaseStats();
	this->bulletImage = weapon.getBulletImagePath();
	this->weaponName = weapon.getName();

	this->preloadZombies();
	this->buildBackground();
	this->buildHud();

	this->startWave(1);

	this->setTouchEnabled(true);
	this->setKeypadEnabled(true);
	this->scheduleUpdate();

	return true;
}

void DefenseLayer::preloadZombies() {
	SpriteUtil::preloadTextureWithFile("sprite_sheets/campesino.plist");
	SpriteUtil::preloadTextureWithFile("sprite_sheets/oficinista.plist");
	SpriteUtil::preloadTextureWithFile("sprite_sheets/cirujano.plist");
	SpriteUtil::preloadTextureWithFile("sprite_sheets/sangre.plist");
	SpriteUtil::preloadTextureWithFile("sprite_sheets/panda1.plist");

	this->zombieTypes.push_back("campesino");
	this->zombieTypes.push_back("oficinista");

	// The roster widens as chapters go up, so later ones read differently.
	if (this->chapter >= 2) {
		this->zombieTypes.push_back("cirujano");
	}
	if (this->chapter >= 4) {
		SpriteUtil::preloadTextureWithFile("sprite_sheets/cura.plist");
		this->zombieTypes.push_back("cura");
	}
	if (this->chapter >= 7) {
		SpriteUtil::preloadTextureWithFile("sprite_sheets/elvis.plist");
		this->zombieTypes.push_back("elvis");
	}
	if (this->chapter >= 10) {
		SpriteUtil::preloadTextureWithFile("sprite_sheets/basquet.plist");
		this->zombieTypes.push_back("basquet");
	}
}

void DefenseLayer::buildBackground() {
	cocos2d::CCSprite* background = SpriteUtil::create("fondo_cementerio.jpg", 0, 0,
			this->visibleWidth, this->visibleHeight);
	this->addChild(background, 0);

	// The gate: the line the zombies must not cross.
	cocos2d::CCLayerColor* gate = cocos2d::CCLayerColor::create(
			cocos2d::ccc4(170, 30, 30, 170), this->visibleWidth, this->visibleHeight * 0.035f);
	gate->setPosition(ccp(this->originX, this->gateY));
	this->addChild(gate, 5);

	// The panda holds the middle of the lane, just above the gate, facing up the
	// lane (direction 6 of the 16 baked into the panda sheet).
	this->panda = SpriteUtil::create("caminata_6_0000.png", 0, 0,
			PANDA_SIZE, PANDA_SIZE, true);
	SpriteUtil::moveToAbsolute(this->panda,
			this->originX + this->visibleWidth / 2,
			this->gateY + this->visibleHeight * 0.10f);
	this->addChild(this->panda, 30);
}

void DefenseLayer::buildHud() {
	float top = this->originY + this->visibleHeight * 0.955f;

	this->chapterLabel = LabelUtil::create(" ", 20, 0, 0, 0, 0, DEFENSE_FONT);
	this->chapterLabel->setAnchorPoint(ccp(0, 0.5));
	this->chapterLabel->setPositionX(this->originX + this->visibleWidth * 0.03f);
	this->chapterLabel->setPositionY(top);
	LabelUtil::setScaleByHeight(this->chapterLabel, this->visibleHeight * 0.05f);
	this->addChild(this->chapterLabel, 60);

	this->waveLabel = LabelUtil::create(" ", 20, 0, 0, 0, 0, DEFENSE_FONT);
	this->waveLabel->setAnchorPoint(ccp(0.5, 0.5));
	this->waveLabel->setPositionX(this->originX + this->visibleWidth * 0.5f);
	this->waveLabel->setPositionY(top);
	LabelUtil::setScaleByHeight(this->waveLabel, this->visibleHeight * 0.05f);
	this->addChild(this->waveLabel, 60);

	this->levelLabel = LabelUtil::create(" ", 20, 0, 0, 0, 0, DEFENSE_FONT);
	this->levelLabel->setAnchorPoint(ccp(1, 0.5));
	this->levelLabel->setPositionX(this->originX + this->visibleWidth * 0.97f);
	this->levelLabel->setPositionY(top);
	LabelUtil::setScaleByHeight(this->levelLabel, this->visibleHeight * 0.05f);
	this->addChild(this->levelLabel, 60);

	this->messageLabel = LabelUtil::create(" ", 22, 0, 0, 0, 0, DEFENSE_FONT);
	this->messageLabel->setAnchorPoint(ccp(0.5, 0.5));
	this->messageLabel->setPositionX(this->originX + this->visibleWidth * 0.5f);
	this->messageLabel->setPositionY(this->originY + this->visibleHeight * 0.68f);
	this->messageLabel->setColor(cocos2d::ccc3(255, 225, 90));
	LabelUtil::setScaleByHeight(this->messageLabel, this->visibleHeight * 0.085f);
	this->addChild(this->messageLabel, 80);

	// Coins earned this run, so the reward for pushing one wave further is
	// visible while you play rather than only on the results screen.
	this->coinsLabel = LabelUtil::create(" ", 18, 0, 0, 0, 0, DEFENSE_FONT);
	this->coinsLabel->setAnchorPoint(ccp(0, 0.5));
	this->coinsLabel->setPositionX(this->originX + this->visibleWidth * 0.03f);
	this->coinsLabel->setPositionY(top - this->visibleHeight * 0.06f);
	this->coinsLabel->setColor(cocos2d::ccc3(255, 214, 80));
	LabelUtil::setScaleByHeight(this->coinsLabel, this->visibleHeight * 0.042f);
	this->addChild(this->coinsLabel, 60);

	// Progress towards the next card.
	this->xpBarWidth = this->visibleWidth * 0.34f;
	float barHeight = this->visibleHeight * 0.016f;
	float barX = this->originX + (this->visibleWidth - this->xpBarWidth) / 2;
	float barY = top - this->visibleHeight * 0.055f;

	cocos2d::CCLayerColor* xpBack = cocos2d::CCLayerColor::create(
			cocos2d::ccc4(0, 0, 0, 120), this->xpBarWidth, barHeight);
	xpBack->setPosition(ccp(barX, barY));
	this->addChild(xpBack, 59);

	this->xpBar = cocos2d::CCLayerColor::create(
			cocos2d::ccc4(120, 220, 120, 220), 1, barHeight);
	this->xpBar->setPosition(ccp(barX, barY));
	this->addChild(this->xpBar, 60);

	this->refreshHud();
}

void DefenseLayer::refreshHud() {
	this->chapterLabel->setString((Language::get("defense.chapter") + " "
			+ StringUtil::toString(this->chapter)).c_str());
	this->waveLabel->setString((Language::get("defense.wave") + " "
			+ StringUtil::toString(this->wave) + "/"
			+ StringUtil::toString(this->getWaveCount())).c_str());
	this->coinsLabel->setString((Language::get("defense.coins") + " "
			+ StringUtil::toString(this->coinsEarned)).c_str());

	if (this->xpBar) {
		float ratio = (float) this->kills / (float) this->killsForNextLevel;
		if (ratio > 1.0f) { ratio = 1.0f; }
		float width = this->xpBarWidth * ratio;
		if (width < 1.0f) { width = 1.0f; }
		this->xpBar->setContentSize(cocos2d::CCSize(width, this->xpBar->getContentSize().height));
	}

	this->levelLabel->setString((this->weaponName + "  "
			+ Language::get("defense.level") + " "
			+ StringUtil::toString(this->level)).c_str());
}

// ---------------------------------------------------------------------------
// Chapter plan
// ---------------------------------------------------------------------------

// The chapter curve. Everything scales off (chapter, wave) so there is one
// place to tune: waves get longer, zombies get tougher and faster, and the gaps
// between them close up.
int DefenseLayer::getWaveCount() {
	int count = 3 + this->chapter / 2;
	return count > 9 ? 9 : count;
}

int DefenseLayer::getWaveSize(int wave) {
	return 5 + 2 * wave + 3 * this->chapter;
}

float DefenseLayer::getZombieLife(int wave) {
	// Compounding so late chapters genuinely need upgraded guns, not just patience.
	float chapterScale = pow(1.35f, (float) (this->chapter - 1));
	return (7.0f + 2.5f * wave) * chapterScale;
}

float DefenseLayer::getZombieSpeed(int wave) {
	float speed = 14.0f + 1.2f * wave + 1.6f * (this->chapter - 1);
	float cap = 46.0f;  // still slow enough to shoot something before it lands
	if (speed > cap) {
		speed = cap;
	}
	return speed * (this->visibleHeight / 320.0f);
}

float DefenseLayer::getSpawnInterval(int wave) {
	float interval = 1.5f - 0.07f * wave - 0.06f * this->chapter;
	return interval < 0.22f ? 0.22f : interval;
}

bool DefenseLayer::isBossWave(int wave) {
	return wave == this->getWaveCount();
}

// Every few zombies in later chapters is an elite: bigger, much tougher, slower.
bool DefenseLayer::shouldSpawnElite() {
	if (this->chapter < 3) {
		return false;
	}
	int chance = 6 + this->chapter;  // percent-ish, climbs with the chapter
	return (rand() % 100) < chance;
}

void DefenseLayer::startWave(int wave) {
	this->wave = wave;
	this->spawnedThisWave = 0;
	this->spawnTimer = 0;
	this->refreshHud();
}

// ---------------------------------------------------------------------------
// Main loop
// ---------------------------------------------------------------------------

void DefenseLayer::update(float dt) {
	if (this->state != PLAYING) {
		return;
	}

	this->updateSpawning(dt);
	this->updateShooting(dt);
	this->updateZombies(dt);
	this->updateBullets(dt);
	this->resolveCollisions();
}

void DefenseLayer::updateSpawning(float dt) {
	if (this->waveBreakTimer > 0) {
		this->waveBreakTimer -= dt;

		if (this->waveBreakTimer <= 0) {
			this->messageLabel->setString(" ");
			this->startWave(this->wave + 1);
		}
		return;
	}

	if (this->spawnedThisWave < this->getWaveSize(this->wave)) {
		this->spawnTimer -= dt;

		if (this->spawnTimer <= 0) {
			this->spawnTimer = this->getSpawnInterval(this->wave);
			this->spawnedThisWave++;

			std::string type = this->zombieTypes[rand() % this->zombieTypes.size()];
			float margin = this->visibleWidth * 0.12f;
			float x = this->originX + margin + (rand() % (int)(this->visibleWidth - 2 * margin));

			float life = this->getZombieLife(this->wave);
			float speed = this->getZombieSpeed(this->wave);
			float scale = 1.0f;

			// The last zombie of a boss wave is a single huge one.
			if (this->isBossWave(this->wave)
					&& this->spawnedThisWave == this->getWaveSize(this->wave)) {
				life *= 14.0f;
				speed *= 0.55f;
				scale = 2.4f;
				this->messageLabel->setString(Language::get("defense.boss").c_str());
				this->messageLabel->runAction(cocos2d::CCSequence::create(
						cocos2d::CCDelayTime::create(2.0f),
						cocos2d::CCCallFunc::create(this,
								(cocos2d::SEL_CallFunc)(&DefenseLayer::clearMessage)),
						NULL));
			} else if (this->shouldSpawnElite()) {
				life *= 3.0f;
				speed *= 0.8f;
				scale = 1.5f;
			}

			this->zombies.push_back(new DefenseZombie(type, x, this->laneTop + 20,
					speed, life, scale, this));
		}
		return;
	}

	// Wave is fully spawned: once it is also fully cleared, move on.
	if (this->zombies.empty()) {
		if (this->wave >= this->getWaveCount()) {
			this->finish(true);
		} else {
			this->waveBreakTimer = WAVE_BREAK;
			this->messageLabel->setString(Language::get("defense.wavecleared").c_str());
		}
	}
}

void DefenseLayer::updateZombies(float dt) {
	for (int i = 0; i < (int) this->zombies.size(); i++) {
		DefenseZombie* zombie = this->zombies[i];
		zombie->update(dt, this->stats.enemySlow);

		if (zombie->getY() <= this->gateY) {
			this->finish(false);
			return;
		}
	}
}

void DefenseLayer::updateShooting(float dt) {
	this->fireTimer -= dt;

	if (this->fireTimer > 0 || this->zombies.empty()) {
		return;
	}

	this->fireTimer = this->stats.fireInterval;

	// Aim at whichever zombie is closest to the gate: that is the threat.
	DefenseZombie* target = this->zombies[0];
	for (int i = 1; i < (int) this->zombies.size(); i++) {
		if (this->zombies[i]->getY() < target->getY()) {
			target = this->zombies[i];
		}
	}

	float originX = this->panda->getPositionX();
	float originY = this->panda->getPositionY() + PANDA_SIZE * 0.25f;
	float baseAngle = atan2(target->getY() - originY, target->getX() - originX) * 180.0f / (float) M_PI;

	// Fan the volley out around the aim line.
	float spread = this->stats.spreadDegrees;
	float start = baseAngle - spread * (this->stats.projectiles - 1) / 2.0f;

	for (int i = 0; i < this->stats.projectiles; i++) {
		this->bullets.push_back(new DefenseBullet(this->bulletImage, originX, originY,
				start + spread * i, this->stats.bulletSpeed, this->stats.damage,
				this->stats.pierce, this->stats.bounce, this));
	}

	CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("sounds/shoot.ogg");
}

void DefenseLayer::updateBullets(float dt) {
	std::vector<DefenseBullet*> alive;

	for (int i = 0; i < (int) this->bullets.size(); i++) {
		DefenseBullet* bullet = this->bullets[i];

		if (bullet->update(dt, this->originX, this->originX + this->visibleWidth, this->laneTop + 30)) {
			alive.push_back(bullet);
		} else {
			bullet->remove();
			delete bullet;
		}
	}

	this->bullets = alive;
}

void DefenseLayer::resolveCollisions() {
	std::vector<DefenseBullet*> liveBullets;

	for (int i = 0; i < (int) this->bullets.size(); i++) {
		DefenseBullet* bullet = this->bullets[i];
		bool spent = false;

		for (int j = 0; j < (int) this->zombies.size() && !spent; j++) {
			DefenseZombie* zombie = this->zombies[j];

			if (zombie->isDead() || !bullet->canHit(zombie)) {
				continue;
			}

			float dx = bullet->getX() - zombie->getX();
			float dy = bullet->getY() - zombie->getY();
			float radius = zombie->getRadius();

			if (dx * dx + dy * dy > radius * radius) {
				continue;
			}

			zombie->hurt(bullet->damage);
			bullet->registerHit(zombie);

			if (bullet->pierce > 0) {
				bullet->pierce--;
			} else {
				spent = true;
			}
		}

		if (spent) {
			bullet->remove();
			delete bullet;
		} else {
			liveBullets.push_back(bullet);
		}
	}

	this->bullets = liveBullets;

	std::vector<DefenseZombie*> liveZombies;
	for (int i = 0; i < (int) this->zombies.size(); i++) {
		DefenseZombie* zombie = this->zombies[i];

		if (zombie->isDead()) {
			float maxLife = zombie->maxLife;
			zombie->remove();
			delete zombie;
			this->onZombieKilled(maxLife);
		} else {
			liveZombies.push_back(zombie);
		}
	}

	this->zombies = liveZombies;
}

void DefenseLayer::onZombieKilled(float maxLife) {
	this->kills++;

	// Pay out by how tough the zombie was, so elites and bosses are worth
	// chasing and the store is reachable by playing defense alone.
	float base = this->getZombieLife(this->wave);
	int reward = 1 + (int) (maxLife / (base > 0 ? base : 1.0f));
	this->coinsEarned += reward;

	if (this->kills >= this->killsForNextLevel) {
		this->levelUp();
	} else {
		this->refreshHud();
	}
}

int DefenseLayer::getChapterReward() {
	return 150 + 120 * this->chapter;
}

// ---------------------------------------------------------------------------
// Levelling and cards
// ---------------------------------------------------------------------------

void DefenseLayer::levelUp() {
	this->level++;
	this->kills = 0;
	this->killsForNextLevel = 6 + 2 * this->level;
	this->refreshHud();

	CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("sounds/bonus.ogg");
	this->showCards();
}

void DefenseLayer::showCards() {
	this->state = CHOOSING_CARD;
	this->messageLabel->setString(Language::get("defense.levelup").c_str());

	this->offered = DefenseCard::draw(CARD_COUNT);

	float cardWidth = this->visibleWidth * 0.25f;
	float cardHeight = this->visibleHeight * 0.40f;
	float gap = this->visibleWidth * 0.035f;
	float totalWidth = CARD_COUNT * cardWidth + (CARD_COUNT - 1) * gap;
	float left = this->originX + (this->visibleWidth - totalWidth) / 2;
	float centerY = this->originY + this->visibleHeight * 0.34f;

	for (int i = 0; i < (int) this->offered.size(); i++) {
		float centerX = left + i * (cardWidth + gap) + cardWidth / 2;

		cocos2d::CCSprite* card = SpriteUtil::create(this->offered[i].getImagePath(),
				0, 0, cardWidth, cardHeight);
		SpriteUtil::moveToAbsolute(card, centerX, centerY);
		this->addChild(card, 70);
		this->cardSprites.push_back(card);

		// Same proportions the store uses, so the text sits in the blank part
		// of the card art rather than under it.
		cocos2d::CCLabelTTF* name = LabelUtil::create(this->offered[i].getName(), 18, 0, 0, 0, 0, DEFENSE_FONT);
		name->setAnchorPoint(ccp(0.5, 0.5));
		name->setPositionX(centerX);
		name->setPositionY(centerY + cardHeight * 0.04f);
		name->setColor(cocos2d::ccc3(62, 62, 62));
		LabelUtil::setScaleByWidth(name, cardWidth * 0.60f);
		this->addChild(name, 71);
		this->cardDecorations.push_back(name);

		cocos2d::CCLabelTTF* detail = cocos2d::CCLabelTTF::create(
				this->offered[i].getDetail().c_str(), DEFENSE_FONT, 11,
				cocos2d::CCSize(cardWidth * 0.62f, 0), cocos2d::kCCTextAlignmentCenter);
		detail->setAnchorPoint(ccp(0.5, 1));
		detail->setPositionX(centerX);
		detail->setPositionY(centerY - cardHeight * 0.06f);
		detail->setColor(cocos2d::ccc3(62, 62, 62));
		this->addChild(detail, 71);
		this->cardDecorations.push_back(detail);
	}
}

void DefenseLayer::chooseCard(int index) {
	if (index < 0 || index >= (int) this->offered.size()) {
		return;
	}

	this->offered[index].apply(this->stats);

	for (int i = 0; i < (int) this->cardSprites.size(); i++) {
		this->cardSprites[i]->removeFromParentAndCleanup(true);
	}
	for (int i = 0; i < (int) this->cardDecorations.size(); i++) {
		this->cardDecorations[i]->removeFromParentAndCleanup(true);
	}

	this->cardSprites.clear();
	this->cardDecorations.clear();
	this->offered.clear();

	this->messageLabel->setString(" ");
	this->state = PLAYING;
}

// ---------------------------------------------------------------------------
// End of run
// ---------------------------------------------------------------------------

void DefenseLayer::finish(bool won) {
	this->state = FINISHED;

	int payout = this->coinsEarned;

	if (won) {
		payout += this->getChapterReward();
		this->messageLabel->setString((Language::get("defense.cleared") + " +"
				+ StringUtil::toString(payout)).c_str());
		this->messageLabel->setColor(cocos2d::ccc3(120, 230, 120));

		// Unlock the next chapter, without ever walking progress backwards.
		if (DB::getInteger(DEFENSE_CHAPTER_KEY, 1) <= this->chapter) {
			DB::putInteger(DEFENSE_CHAPTER_KEY, this->chapter + 1);
		}
	} else {
		// Keep what was earned even on a loss: a run is never wasted time.
		this->messageLabel->setString((Language::get("defense.failed") + " +"
				+ StringUtil::toString(payout)).c_str());
		this->messageLabel->setColor(cocos2d::ccc3(240, 90, 90));
	}

	UserDAO::addCoins(payout);

	// Let the player read the result, then drop back to the menu.
	this->runAction(cocos2d::CCSequence::create(
			cocos2d::CCDelayTime::create(2.5f),
			cocos2d::CCCallFunc::create(this, (cocos2d::SEL_CallFunc)(&DefenseLayer::goToMenu)),
			NULL));
}

// The chapter was entered with replaceScene, so the default "pop the scene"
// would drop out to the main menu. Go back to the chapter list instead.
void DefenseLayer::keyBackClicked() {
	this->goToMenu();
}

void DefenseLayer::clearMessage() {
	this->messageLabel->setString(" ");
}

void DefenseLayer::goToMenu() {
	cocos2d::CCDirector::sharedDirector()->replaceScene(DefenseChapterLayer::scene());
}

void DefenseLayer::ccTouchesBegan(cocos2d::CCSet *pTouches, cocos2d::CCEvent *pEvent) {
	if (this->state != CHOOSING_CARD) {
		return;
	}

	cocos2d::CCTouch* touch = (cocos2d::CCTouch*) (pTouches->anyObject());
	cocos2d::CCPoint location = touch->getLocationInView();
	location = cocos2d::CCDirector::sharedDirector()->convertToGL(location);

	for (int i = 0; i < (int) this->cardSprites.size(); i++) {
		if (this->cardSprites[i]->boundingBox().containsPoint(location)) {
			this->chooseCard(i);
			return;
		}
	}
}

} /* namespace dxco */
