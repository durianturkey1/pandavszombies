#ifndef DEFENSEOFFER_H_
#define DEFENSEOFFER_H_

#include <string>
#include <vector>

namespace dxco {

/**
 * A store offer defined in defense/config.json: the welcome gift, a pack sold
 * after a particular chapter, or the end-of-run coin bonus.
 *
 * Offers are paid for in coins. The grant path is deliberately one method
 * (`claim`), which is where a real in-app purchase would be wired in: ask the
 * platform to bill, and call `claim` from the success callback.
 */
class DefenseOffer {
public:
	DefenseOffer(int index = -1);

	int index;

	bool isValid() const;
	std::string getId() const;
	std::string getTitle() const;
	std::string getDetail() const;
	std::string getImagePath() const;

	/** Price in coins; 0 means it is a gift. */
	int getCost() const;
	bool isFree() const;
	/** Offers marked "once" disappear after they are taken. */
	bool isOnce() const;
	bool isTaken() const;
	/** Doubles whatever the finished run paid out instead of a fixed reward. */
	bool doublesRunReward() const;

	int getRewardCoins() const;
	int getRewardAttackLevels() const;
	std::vector<std::string> getRewardWeapons() const;

	/** A one-line summary of what the player gets, for the popup. */
	std::string describeReward(int runReward) const;

	/**
	 * Grants the rewards and records that it was taken. `runReward` is what the
	 * finished run paid, used by doubling offers. Returns false when the player
	 * cannot afford it.
	 */
	bool claim(int runReward);

	/** The offer to show on first launch, if it has not been taken. */
	static DefenseOffer forFirstRun();
	/** The offer tied to clearing this chapter, if any is still available. */
	static DefenseOffer forChapterCleared(int chapter);
};

} /* namespace dxco */

#endif /* DEFENSEOFFER_H_ */
