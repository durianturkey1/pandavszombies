#ifndef BACKBUTTON_H_
#define BACKBUTTON_H_

#include "../../dxco/Button.h"

namespace dxco {

/**
 * Leaves the current menu screen. The game was built for Android, where going
 * back was the hardware key handled by AbstractMenuLayer::keyBackClicked; on a
 * desktop there is no such key, so the menus need a visible way out.
 */
class BackButton: public ButtonDxCo {
public:
	BackButton(cocos2d::CCSprite* sprite);

	virtual void execute();
};

} /* namespace dxco */

#endif /* BACKBUTTON_H_ */
