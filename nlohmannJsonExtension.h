#pragma once
#include <optional>

#include <nlohmann/json.hpp>

#define NLOHMANN_JSON_FROM_NON_THROWING(v1) \
	if (nlohmann_json_j.contains(#v1)) nlohmann_json_j.at(#v1).get_to(nlohmann_json_t.v1);
#define NLOHMANN_DEFINE_TYPE_INTRUSIVE_NON_THROWING(Type, ...)                                                                                                     \
	friend void to_json(nlohmann::json& nlohmann_json_j, const Type& nlohmann_json_t) { NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_TO, __VA_ARGS__)) } \
	friend void from_json(const nlohmann::json& nlohmann_json_j, Type& nlohmann_json_t) { NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM_NON_THROWING, __VA_ARGS__)) }

#define NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_NON_THROWING(Type, ...)                                                                                                 \
	inline void to_json(nlohmann::json& nlohmann_json_j, const Type& nlohmann_json_t) { NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_TO, __VA_ARGS__)) } \
	inline void from_json(const nlohmann::json& nlohmann_json_j, Type& nlohmann_json_t) { NLOHMANN_JSON_EXPAND(NLOHMANN_JSON_PASTE(NLOHMANN_JSON_FROM_NON_THROWING, __VA_ARGS__)) }


// add support for std::optional (3.11.3), previously used code removed
template<typename T>
struct nlohmann::adl_serializer<std::optional<T>> {
	static void to_json(nlohmann::json& j, const std::optional<T>& v) {
		if (v.has_value()) {
			j = v.value();
		} else {
			j = nullptr;
		}
	}
	static void from_json(const nlohmann::json& j, std::optional<T>& v) {
		if(j.is_null()) {
			v = std::nullopt;
		} else {
			v = j.get<T>();
		}
	}
};
