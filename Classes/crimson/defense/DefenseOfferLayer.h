#ifndef DEFENSEOFFERLAYER_H_
#define DEFENSEOFFERLAYER_H_

#include "cocos2d.h"

#include "DefenseOffer.h"

#include <vector>

namespace dxco {

/**
 * The popup that presents an offer: welcome gift on first launch, a pack after
 * a chapter, or the coin bonus at the end of a run. Shown as a child layer over
 * whatever screen triggered it rather than as its own scene, so the screen
 * underneath keeps its state.
 */
class DefenseOfferLayer : public cocos2d::CCLayer {
public:
	DefenseOfferLayer();

	static DefenseOfferLayer* create(DefenseOffer offer, int runReward);

	bool build(DefenseOffer offer, int runReward);
	void ccTouchesBegan(cocos2d::CCSet *pTouches, cocos2d::CCEvent *pEvent);

	/** True once the player has taken or dismissed the offer. */
	bool isClosed();

private:
	void close();

	DefenseOffer offer;
	int runReward;
	bool closed;

	cocos2d::CCSprite* claimButton;
	cocos2d::CCLabelTTF* dismissLabel;
	cocos2d::CCLabelTTF* errorLabel;
};

} /* namespace dxco */

#endif /* DEFENSEOFFERLAYER_H_ */
