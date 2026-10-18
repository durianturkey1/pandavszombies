#ifndef DEFENSEICON_H_
#define DEFENSEICON_H_

#include "cocos2d.h"

#include <string>

namespace dxco {

/**
 * The store's upgrade art (equip/MEJORA_*.png) is a whole card: a rounded panel
 * with a red quantity badge in one corner, the icon near the top and a divider
 * line below it. Defense mode draws its own panels, so it wants the icon only --
 * otherwise every card and popup shows a stray empty badge.
 */
class DefenseIcon {
public:
	/** The icon region of a store card, scaled to `targetHeight` points. */
	static cocos2d::CCSprite* create(const std::string& path, float targetHeight);
};

} /* namespace dxco */

#endif /* DEFENSEICON_H_ */
