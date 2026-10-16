#ifndef DEFENSECHAPTERLAYER_H_
#define DEFENSECHAPTERLAYER_H_

#include "cocos2d.h"

#include "../layers/AbstractMenuLayer.h"
#include "../../dxco/Button.h"

#include <vector>

#define DEFENSE_CHAPTER_COUNT 12

namespace dxco {

/** Chapter picker for defense mode: cleared chapters stay open for replay. */
class DefenseChapterLayer : public AbstractMenuLayer {
public:
	DefenseChapterLayer();

	virtual bool init();
	static cocos2d::CCScene* scene();

	CREATE_FUNC(DefenseChapterLayer);

	void ccTouchesBegan(cocos2d::CCSet *pTouches, cocos2d::CCEvent *pEvent);

private:
	std::vector<ButtonDxCo*> chapterButtons;
	ButtonDxCo* backButton;
	cocos2d::CCLabelTTF* messageLabel;
};

} /* namespace dxco */

#endif /* DEFENSECHAPTERLAYER_H_ */
