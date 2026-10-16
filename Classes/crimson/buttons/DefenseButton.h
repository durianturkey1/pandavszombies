#ifndef DEFENSEBUTTON_H_
#define DEFENSEBUTTON_H_

#include "../../dxco/Button.h"

namespace dxco {

/** Starts defense mode at the highest chapter the player has unlocked. */
class DefenseButton: public ButtonDxCo {
public:
	DefenseButton(cocos2d::CCSprite* sprite);

	virtual void execute();
};

} /* namespace dxco */

#endif /* DEFENSEBUTTON_H_ */
