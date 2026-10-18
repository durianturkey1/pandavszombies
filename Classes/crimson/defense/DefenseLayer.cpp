#include "DefenseLayer.h"

#include "DefenseBullet.h"
#include "DefenseZombie.h"
#include "DefenseWeapon.h"
#include "DefenseChapterLayer.h"
#include "DefenseConfig.h"
#include "DefenseIcon.h"
#include "../SoundUtil.h"
#include "../daos/UserDAO.h"
#include "../GameProperties.h"
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
	this->quitButton = NULL;
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

	// The store's ATTACK upgrade is the progression that carries between runs:
	// chapters scale faster than a single run's cards can, so without this the
	// later ones are unwinnable no matter which gun is equipped. It starts at 10
	// and each purchase adds 10, worth +50% damage apiece.
	float attack = (float) GameProperties::get("attack.damage");
	if (attack > 10.0f) {
		this->stats.damage *= 1.0f + (attack - 10.0f)
				/ DefenseConfig::get()->getChapterFloat("attackUpgradeDivisor", 20.0f);
	}
	this->bulletImage = weapon.getBulletImagePath();
	this->fireSound = weapon.getSoundPath();
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
	DefenseConfig* config = DefenseConfig::get();

	SpriteUtil::preloadTextureWithFile("sprite_sheets/panda1.plist");
	SpriteUtil::preloadTextureWithFile("sprite_sheets/sangre.plist");

	// Which zombies exist in this chapter, and their sheets, are configured.
	std::vector<std::string> sheets = config->getZombieSheets(this->chapter);
	for (int i = 0; i < (int) sheets.size(); i++) {
		SpriteUtil::preloadTextureWithFile(sheets[i]);
	}

	this->zombieTypes = config->getZombieTypes(this->chapter);
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

	// A strip behind the readouts: zombies walk in from the top of the lane and
	// used to pass straight through the text.
	cocos2d::CCLayerColor* strip = cocos2d::CCLayerColor::create(
			cocos2d::ccc4(0, 0, 0, 120), this->visibleWidth, this->visibleHeight * 0.115f);
	strip->setPosition(ccp(this->originX, this->originY + this->visibleHeight * 0.885f));
	this->addChild(strip, 55);

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

	// Leaving a run was Escape-only, which a mouse-only player cannot discover.
	this->quitButton = SpriteUtil::create("buttons/menu_icon_button.png", 0, 0);
	float quitScale = (this->visibleHeight * 0.105f) / SpriteUtil::getHeight(this->quitButton);
	this->quitButton->setScale(quitScale);
	SpriteUtil::moveToAbsolute(this->quitButton,
			this->originX + this->visibleWidth - SpriteUtil::getWidth(this->quitButton) * 0.7f,
			this->originY + this->visibleHeight * 0.80f);
	this->addChild(this->quitButton, 62);

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
	DefenseConfig* config = DefenseConfig::get();
	float count = config->getChapterFloat("waveCountBase", 3)
			+ config->getChapterFloat("waveCountPerChapter", 0.5f) * this->chapter;
	float cap = config->getChapterFloat("waveCountMax", 9);

	return (int) (count > cap ? cap : count);
}

int DefenseLayer::getWaveSize(int wave) {
	DefenseConfig* config = DefenseConfig::get();

	return config->getChapterInt("waveSizeBase", 5)
			+ config->getChapterInt("waveSizePerWave", 2) * wave
			+ config->getChapterInt("waveSizePerChapter", 1) * this->chapter;
}

float DefenseLayer::getZombieLife(int wave) {
	DefenseConfig* config = DefenseConfig::get();

	// Compounding so late chapters genuinely need upgraded guns, not just
	// patience -- but gently: at 1.35x chapter 12 reached 800hp zombies, which
	// no combination of guns and cards can chew through in a wave.
	float scale = pow(config->getChapterFloat("lifeChapterScale", 1.15f),
			(float) (this->chapter - 1));

	return (config->getChapterFloat("lifeBase", 7.0f)
			+ config->getChapterFloat("lifePerWave", 2.5f) * wave) * scale;
}

float DefenseLayer::getZombieSpeed(int wave) {
	DefenseConfig* config = DefenseConfig::get();

	float speed = config->getChapterFloat("speedBase", 14.0f)
			+ config->getChapterFloat("speedPerWave", 1.2f) * wave
			+ config->getChapterFloat("speedPerChapter", 1.6f) * (this->chapter - 1);
	float cap = config->getChapterFloat("speedMax", 46.0f);

	if (speed > cap) {
		speed = cap;  // still slow enough to shoot something before it lands
	}

	return speed * (this->visibleHeight / 320.0f);
}

float DefenseLayer::getSpawnInterval(int wave) {
	DefenseConfig* config = DefenseConfig::get();

	// Spawn pressure rises mostly inside a chapter, not across chapters: a
	// chapter has to open gently enough to build levels before it squeezes.
	float interval = config->getChapterFloat("spawnBase", 1.6f)
			- config->getChapterFloat("spawnPerWave", 0.08f) * wave
			- config->getChapterFloat("spawnPerChapter", 0.03f) * this->chapter;
	float floor_value = config->getChapterFloat("spawnMin", 0.35f);

	return interval < floor_value ? floor_value : interval;
}

bool DefenseLayer::isBossWave(int wave) {
	return wave == this->getWaveCount();
}

// A share of the zombies in later chapters are elites: bigger, much tougher.
bool DefenseLayer::shouldSpawnElite() {
	DefenseConfig* config = DefenseConfig::get();

	if (this->chapter < config->getChapterInt("eliteFromChapter", 3)) {
		return false;
	}

	int chance = config->getChapterInt("eliteChanceBase", 6)
			+ config->getChapterInt("eliteChancePerChapter", 1) * this->chapter;

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
			DefenseConfig* config = DefenseConfig::get();

			if (this->isBossWave(this->wave)
					&& this->spawnedThisWave == this->getWaveSize(this->wave)) {
				life *= config->getChapterFloat("bossLifeMultiplier", 14.0f);
				speed *= config->getChapterFloat("bossSpeedMultiplier", 0.55f);
				scale = config->getChapterFloat("bossScale", 2.4f);
				this->messageLabel->setString(Language::get("defense.boss").c_str());
				this->messageLabel->runAction(cocos2d::CCSequence::create(
						cocos2d::CCDelayTime::create(2.0f),
						cocos2d::CCCallFunc::create(this,
								(cocos2d::SEL_CallFunc)(&DefenseLayer::clearMessage)),
						NULL));
			} else if (this->shouldSpawnElite()) {
				life *= config->getChapterFloat("eliteLifeMultiplier", 3.0f);
				speed *= config->getChapterFloat("eliteSpeedMultiplier", 0.8f);
				scale = config->getChapterFloat("eliteScale", 1.5f);
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

	// Lead the shot. Aiming straight at where a zombie is right now means the
	// bullet arrives where it was: by chapter 12 a zombie walks ~40 points in
	// the time a bazooka round crosses the lane, which is wider than its hit
	// radius, so every single shot missed. Two refinement passes are plenty for
	// a target moving in a straight line.
	float targetX = target->getX();
	float targetY = target->getY();
	float zombieSpeed = target->speed * (1.0f - this->stats.enemySlow);

	for (int pass = 0; pass < 2; pass++) {
		float dx = targetX - originX;
		float dy = targetY - originY;
		float travel = sqrt(dx * dx + dy * dy) / this->stats.bulletSpeed;
		targetY = target->getY() - zombieSpeed * travel;
	}

	if (targetY < originY) {
		targetY = originY;  // never aim back down into the gate
	}

	float baseAngle = atan2(targetY - originY, targetX - originX) * 180.0f / (float) M_PI;

	// Fan the volley out around the aim line.
	float spread = this->stats.spreadDegrees;
	float start = baseAngle - spread * (this->stats.projectiles - 1) / 2.0f;

	for (int i = 0; i < this->stats.projectiles; i++) {
		float damage = this->stats.damage;

		if (this->stats.critChance > 0 && (rand() % 100) < (this->stats.critChance * 100)) {
			damage *= 2.5f;
		}

		this->bullets.push_back(new DefenseBullet(this->bulletImage, originX, originY,
				start + spread * i, this->stats.bulletSpeed, damage,
				this->stats.pierce, this->stats.bounce, this));
	}

	CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect(this->fireSound.c_str());
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
			CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("sounds/splat.ogg");
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
	DefenseConfig* config = DefenseConfig::get();

	return config->getChapterInt("rewardBase", 150)
			+ config->getChapterInt("rewardPerChapter", 120) * this->chapter;
}

// ---------------------------------------------------------------------------
// Levelling and cards
// ---------------------------------------------------------------------------

void DefenseLayer::levelUp() {
	this->level++;
	this->kills = 0;
	this->killsForNextLevel = DefenseConfig::get()->getChapterInt("killsForFirstLevel", 6)
			+ DefenseConfig::get()->getChapterInt("killsPerLevel", 2) * this->level;
	this->refreshHud();

	CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("sounds/bonus.ogg");
	this->showCards();
}

void DefenseLayer::showCards() {
	this->state = CHOOSING_CARD;
	this->messageLabel->setString(Language::get("defense.levelup").c_str());

	this->offered = DefenseCard::draw(DefenseConfig::get()->getChapterInt("cardsOffered", 3));

	int cardCount = (int) this->offered.size();
	if (cardCount <= 0) {
		this->state = PLAYING;
		return;
	}

	float cardWidth = this->visibleWidth * 0.25f;
	float cardHeight = this->visibleHeight * 0.40f;
	float gap = this->visibleWidth * 0.035f;
	float totalWidth = cardCount * cardWidth + (cardCount - 1) * gap;
	float left = this->originX + (this->visibleWidth - totalWidth) / 2;
	float centerY = this->originY + this->visibleHeight * 0.34f;

	for (int i = 0; i < (int) this->offered.size(); i++) {
		float centerX = left + i * (cardWidth + gap) + cardWidth / 2;
		float cardLeft = centerX - cardWidth / 2;
		float cardBottom = centerY - cardHeight / 2;

		// Drawn panels rather than the store's card art, which carries a badge
		// and a divider line that read as rendering glitches behind the text.
		cocos2d::CCLayerColor* border = cocos2d::CCLayerColor::create(
				cocos2d::ccc4(250, 220, 90, 255), cardWidth + 4, cardHeight + 4);
		border->setPosition(ccp(cardLeft - 2, cardBottom - 2));
		this->addChild(border, 69);
		this->cardDecorations.push_back(border);

		cocos2d::CCLayerColor* panel = cocos2d::CCLayerColor::create(
				cocos2d::ccc4(28, 28, 34, 245), cardWidth, cardHeight);
		panel->setPosition(ccp(cardLeft, cardBottom));
		this->addChild(panel, 70);
		this->cardDecorations.push_back(panel);

		// The hit target stays a sprite so the touch test is unchanged, but it
		// is invisible: the panels above are what the player sees.
		cocos2d::CCSprite* card = SpriteUtil::create("white.png", 0, 0, cardWidth, cardHeight);
		SpriteUtil::moveToAbsolute(card, centerX, centerY);
		card->setOpacity(0);
		this->addChild(card, 71);
		this->cardSprites.push_back(card);

		cocos2d::CCSprite* icon = DefenseIcon::create(this->offered[i].getImagePath(),
				cardHeight * 0.34f);
		if (icon != NULL) {
			SpriteUtil::moveToAbsolute(icon, centerX, cardBottom + cardHeight * 0.70f);
			this->addChild(icon, 72);
			this->cardDecorations.push_back(icon);
		}

		cocos2d::CCLabelTTF* name = LabelUtil::create(this->offered[i].getName(), 18, 0, 0, 0, 0, DEFENSE_FONT);
		name->setAnchorPoint(ccp(0.5, 0.5));
		name->setPositionX(centerX);
		name->setPositionY(cardBottom + cardHeight * 0.42f);
		name->setColor(cocos2d::ccc3(255, 214, 80));
		LabelUtil::setScaleByWidth(name, cardWidth * 0.80f);
		this->addChild(name, 72);
		this->cardDecorations.push_back(name);

		cocos2d::CCLabelTTF* detail = cocos2d::CCLabelTTF::create(
				this->offered[i].getDetail().c_str(), DEFENSE_FONT, 11,
				cocos2d::CCSize(cardWidth * 0.84f, 0), cocos2d::kCCTextAlignmentCenter);
		detail->setAnchorPoint(ccp(0.5, 1));
		detail->setPositionX(centerX);
		detail->setPositionY(cardBottom + cardHeight * 0.32f);
		detail->setColor(cocos2d::ccc3(225, 225, 225));
		this->addChild(detail, 72);
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

	// Hand the result to the chapter list, which is where offers are shown.
	DB::putInteger("defense.lastReward", payout);
	DB::putInteger("defense.lastCleared", won ? this->chapter : 0);

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
	if (this->state == FINISHED) {
		return;
	}

	cocos2d::CCTouch* touch = (cocos2d::CCTouch*) (pTouches->anyObject());
	cocos2d::CCPoint location = touch->getLocationInView();
	location = cocos2d::CCDirector::sharedDirector()->convertToGL(location);

	// Picking a card comes first: the cards cover most of the screen.
	if (this->state == CHOOSING_CARD) {
		for (int i = 0; i < (int) this->cardSprites.size(); i++) {
			if (this->cardSprites[i]->boundingBox().containsPoint(location)) {
				this->chooseCard(i);
				return;
			}
		}
		return;
	}

	if (this->quitButton != NULL && this->quitButton->boundingBox().containsPoint(location)) {
		this->goToMenu();
	}
}

} /* namespace dxco */
