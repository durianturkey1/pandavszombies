#include "DefenseConfig.h"

#include "../../dxco/JsonParser.h"
#include "../../dxco/Language.h"

#include "cocos2d.h"

namespace dxco {

#define DEFENSE_CONFIG_FILE "defense/config.json"

DefenseConfig* DefenseConfig::instance = NULL;

DefenseConfig::DefenseConfig() {
	std::string path = DEFENSE_CONFIG_FILE;
	this->document = JsonParser::parseJsonFile(path);

	if (this->document == NULL || this->document->HasParseError()) {
		CCLOG("DefenseConfig: could not read %s, falling back to built-in defaults",
				DEFENSE_CONFIG_FILE);
		this->document = NULL;
	}
}

DefenseConfig* DefenseConfig::get() {
	if (instance == NULL) {
		instance = new DefenseConfig();
	}

	return instance;
}

const rapidjson::Value* DefenseConfig::entry(const std::string& section, int index) {
	if (this->document == NULL || !this->document->HasMember(section.c_str())) {
		return NULL;
	}

	const rapidjson::Value& array = (*this->document)[section.c_str()];

	if (!array.IsArray() || index < 0 || index >= (int) array.Size()) {
		return NULL;
	}

	return &array[(rapidjson::SizeType) index];
}

int DefenseConfig::getCount(const std::string& section) {
	if (this->document == NULL || !this->document->HasMember(section.c_str())) {
		return 0;
	}

	const rapidjson::Value& array = (*this->document)[section.c_str()];
	return array.IsArray() ? (int) array.Size() : 0;
}

bool DefenseConfig::has(const std::string& section, int index, const std::string& field) {
	const rapidjson::Value* value = this->entry(section, index);
	return value != NULL && value->HasMember(field.c_str());
}

int DefenseConfig::getInt(const std::string& section, int index, const std::string& field,
		int fallback) {
	const rapidjson::Value* value = this->entry(section, index);

	if (value == NULL || !value->HasMember(field.c_str())) {
		return fallback;
	}

	const rapidjson::Value& field_value = (*value)[field.c_str()];
	return field_value.IsNumber() ? (int) field_value.GetDouble() : fallback;
}

float DefenseConfig::getFloat(const std::string& section, int index, const std::string& field,
		float fallback) {
	const rapidjson::Value* value = this->entry(section, index);

	if (value == NULL || !value->HasMember(field.c_str())) {
		return fallback;
	}

	const rapidjson::Value& field_value = (*value)[field.c_str()];
	return field_value.IsNumber() ? (float) field_value.GetDouble() : fallback;
}

std::string DefenseConfig::getString(const std::string& section, int index,
		const std::string& field, const std::string& fallback) {
	const rapidjson::Value* value = this->entry(section, index);

	if (value == NULL || !value->HasMember(field.c_str())) {
		return fallback;
	}

	const rapidjson::Value& field_value = (*value)[field.c_str()];
	return field_value.IsString() ? field_value.GetString() : fallback;
}

std::string DefenseConfig::getText(const std::string& section, int index,
		const std::string& field, const std::string& fallback) {
	// A "<field>Key" entry means "look this up in lang/*.json", which is how a
	// translated string is kept out of the config; a plain "<field>" is used
	// as-is, so a new weapon needs only this one file.
	std::string key = this->getString(section, index, field + "Key", "");

	if (!key.empty()) {
		return Language::get(key);
	}

	return this->getString(section, index, field, fallback);
}

std::vector<std::string> DefenseConfig::getStringArray(const std::string& section, int index,
		const std::string& field) {
	std::vector<std::string> result;
	const rapidjson::Value* value = this->entry(section, index);

	if (value == NULL || !value->HasMember(field.c_str())) {
		return result;
	}

	const rapidjson::Value& array = (*value)[field.c_str()];

	if (!array.IsArray()) {
		return result;
	}

	for (rapidjson::SizeType i = 0; i < array.Size(); i++) {
		if (array[i].IsString()) {
			result.push_back(array[i].GetString());
		}
	}

	return result;
}

int DefenseConfig::getChapterInt(const std::string& field, int fallback) {
	return (int) this->getChapterFloat(field, (float) fallback);
}

float DefenseConfig::getChapterFloat(const std::string& field, float fallback) {
	if (this->document == NULL || !this->document->HasMember("chapters")) {
		return fallback;
	}

	const rapidjson::Value& chapters = (*this->document)["chapters"];

	if (!chapters.IsObject() || !chapters.HasMember(field.c_str())) {
		return fallback;
	}

	const rapidjson::Value& value = chapters[field.c_str()];
	return value.IsNumber() ? (float) value.GetDouble() : fallback;
}

static const rapidjson::Value* zombieList(rapidjson::Document* document) {
	if (document == NULL || !document->HasMember("chapters")) {
		return NULL;
	}

	const rapidjson::Value& chapters = (*document)["chapters"];

	if (!chapters.IsObject() || !chapters.HasMember("zombies")) {
		return NULL;
	}

	const rapidjson::Value& zombies = chapters["zombies"];
	return zombies.IsArray() ? &zombies : NULL;
}

std::vector<std::string> DefenseConfig::getZombieTypes(int chapter) {
	std::vector<std::string> result;
	const rapidjson::Value* zombies = zombieList(this->document);

	if (zombies == NULL) {
		// Without a config there is still a game to play.
		result.push_back("campesino");
		result.push_back("oficinista");
		return result;
	}

	for (rapidjson::SizeType i = 0; i < zombies->Size(); i++) {
		const rapidjson::Value& zombie = (*zombies)[i];

		if (!zombie.HasMember("type")) {
			continue;
		}

		int from = zombie.HasMember("fromChapter") ? (int) zombie["fromChapter"].GetDouble() : 1;

		if (chapter >= from) {
			result.push_back(zombie["type"].GetString());
		}
	}

	if (result.empty()) {
		result.push_back("campesino");
	}

	return result;
}

float DefenseConfig::getZombieModifier(const std::string& type, const std::string& field,
		float fallback) {
	const rapidjson::Value* zombies = zombieList(this->document);

	if (zombies == NULL) {
		return fallback;
	}

	for (rapidjson::SizeType i = 0; i < zombies->Size(); i++) {
		const rapidjson::Value& zombie = (*zombies)[i];

		if (!zombie.HasMember("type") || type != zombie["type"].GetString()) {
			continue;
		}

		if (zombie.HasMember(field.c_str()) && zombie[field.c_str()].IsNumber()) {
			return (float) zombie[field.c_str()].GetDouble();
		}

		return fallback;
	}

	return fallback;
}

int DefenseConfig::getZombieWeight(const std::string& type) {
	int weight = (int) this->getZombieModifier(type, "weight", 1.0f);
	return weight < 1 ? 1 : weight;
}

std::vector<std::string> DefenseConfig::getZombieSheets(int chapter) {
	std::vector<std::string> result;
	const rapidjson::Value* zombies = zombieList(this->document);

	if (zombies == NULL) {
		result.push_back("sprite_sheets/campesino.plist");
		result.push_back("sprite_sheets/oficinista.plist");
		return result;
	}

	for (rapidjson::SizeType i = 0; i < zombies->Size(); i++) {
		const rapidjson::Value& zombie = (*zombies)[i];

		if (!zombie.HasMember("sheet")) {
			continue;
		}

		int from = zombie.HasMember("fromChapter") ? (int) zombie["fromChapter"].GetDouble() : 1;

		if (chapter >= from) {
			result.push_back(zombie["sheet"].GetString());
		}
	}

	return result;
}

} /* namespace dxco */
