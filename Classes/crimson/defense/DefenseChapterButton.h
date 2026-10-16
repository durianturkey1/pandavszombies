#ifndef DEFENSECHAPTERBUTTON_H_
#define DEFENSECHAPTERBUTTON_H_

#include "../../dxco/Button.h"

namespace dxco {

/** One chapter tile: starts the chapter, or refuses if it is still locked. */
class DefenseChapterButton: public ButtonDxCo {
public:
	DefenseChapterButton(int chapter, bool unlocked, float x, float y, float width, float height);

	virtual void execute();

	int chapter;
	bool unlocked;
	cocos2d::CCLabelTTF* label;
};

} /* namespace dxco */

#endif /* DEFENSECHAPTERBUTTON_H_ */
