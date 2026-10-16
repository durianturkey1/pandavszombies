#ifndef DEFENSELAYER_H_
#define DEFENSELAYER_H_

#include "cocos2d.h"

#include "DefenseStats.h"
#include "../layers/AbstractMenuLayer.h"

#include <string>
#include <vector>

namespace dxco {

class DefenseZombie;
class DefenseBullet;

/**
 * Defense mode: the panda holds the gate at the bottom of the lane and fires
 * automatically while zombies walk down from the top. Kills earn levels, each
 * level offers three upgrade cards, and a chapter is cleared by surviving all
 * of its waves. One zombie through the gate ends the run.
 */
class DefenseLayer : public AbstractMenuLayer {
public:
	DefenseLayer();

	virtual bool init();
	static cocos2d::CCScene* scene(int chapter);

	void update(float dt);
	void ccTouchesBegan(cocos2d::CCSet *pTouches, cocos2d::CCEvent *pEvent);

	int chapter;

private:
	enum State { PLAYING, CHOOSING_CARD, FINISHED };

	// setup
	void buildBackground();
	void buildHud();
	void preloadZombies();

	// loop
	void updateSpawning(float dt);
	void updateZombies(float dt);
	void updateShooting(float dt);
	void updateBullets(float dt);
	void resolveCollisions();
	void onZombieKilled();
	void refreshHud();

	// flow
	void startWave(int wave);
	void levelUp();
	void showCards();
	void chooseCard(int index);
	void finish(bool won);
	void goToMenu();
	void clearMessage();

	// chapter plan
	int getWaveCount();
	int getWaveSize(int wave);
	float getZombieLife(int wave);
	float getZombieSpeed(int wave);
	float getSpawnInterval(int wave);
	bool isBossWave(int wave);
	bool shouldSpawnElite();

	State state;

	std::vector<DefenseZombie*> zombies;
	std::vector<DefenseBullet*> bullets;
	std::vector<std::string> zombieTypes;
	std::string bulletImage;
	std::string weaponName;

	DefenseStats stats;

	cocos2d::CCSprite* panda;
	cocos2d::CCLabelTTF* chapterLabel;
	cocos2d::CCLabelTTF* waveLabel;
	cocos2d::CCLabelTTF* levelLabel;
	cocos2d::CCLabelTTF* messageLabel;

	std::vector<DefenseCard> offered;
	std::vector<cocos2d::CCSprite*> cardSprites;
	std::vector<cocos2d::CCNode*> cardDecorations;

	float visibleWidth;
	float visibleHeight;
	float originX;
	float originY;
	float gateY;
	float laneTop;

	int wave;
	int spawnedThisWave;
	float spawnTimer;
	float waveBreakTimer;
	float fireTimer;

	int level;
	int kills;
	int killsForNextLevel;
};

} /* namespace dxco */

#endif /* DEFENSELAYER_H_ */
