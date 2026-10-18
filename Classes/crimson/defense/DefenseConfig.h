#ifndef DEFENSECONFIG_H_
#define DEFENSECONFIG_H_

#include "rapidjson/document.h"

#include <string>
#include <vector>

namespace dxco {

/**
 * Reads defense/config.json, which is where the mode's content lives: guns,
 * level-up cards, the difficulty curve and the store offers. Adding a weapon or
 * retuning a chapter is an edit to that file, not to the code.
 *
 * Every accessor takes a default, so a config that is missing a field (or an
 * older config shipped with an older build) still produces a playable game
 * rather than a crash.
 */
class DefenseConfig {
public:
	static DefenseConfig* get();

	/** Number of entries under a top-level array, e.g. "weapons". */
	int getCount(const std::string& section);

	int getInt(const std::string& section, int index, const std::string& field, int fallback);
	float getFloat(const std::string& section, int index, const std::string& field, float fallback);
	std::string getString(const std::string& section, int index, const std::string& field,
			const std::string& fallback);
	bool has(const std::string& section, int index, const std::string& field);

	/** Strings may be given inline, or as "<field>Key" pointing into lang/*.json. */
	std::string getText(const std::string& section, int index, const std::string& field,
			const std::string& fallback);

	/** Array of strings inside an entry, e.g. a pack's "rewardWeapons". */
	std::vector<std::string> getStringArray(const std::string& section, int index,
			const std::string& field);

	/** Values under the "chapters" object. */
	int getChapterInt(const std::string& field, int fallback);
	float getChapterFloat(const std::string& field, float fallback);

	/** Zombie roster: types unlocked at or before this chapter. */
	std::vector<std::string> getZombieTypes(int chapter);
	std::vector<std::string> getZombieSheets(int chapter);

	/** A per-type modifier, e.g. "speed" or "life"; 1.0 when unset. */
	float getZombieModifier(const std::string& type, const std::string& field, float fallback);
	/** How often this type shows up relative to the others. */
	int getZombieWeight(const std::string& type);

private:
	DefenseConfig();

	const rapidjson::Value* entry(const std::string& section, int index);

	static DefenseConfig* instance;
	rapidjson::Document* document;
};

} /* namespace dxco */

#endif /* DEFENSECONFIG_H_ */
