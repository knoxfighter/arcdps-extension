#pragma once

#include "arcdps_structs.h"
#include "Singleton.h"
#include "UpdateCheckerBase.h"
#include <filesystem>

/**
 * Use this only if you can live with the Dependencies:
 * - ImGui directly
 * - Localization.h
 * - Singleton of this library (it has to be setup correctly as well)
 */
class UpdateChecker final : public UpdateCheckerBase, public Singleton<UpdateChecker> {
public:
	void Draw(const std::unique_ptr<UpdateState>& pUpdateState, const std::string& pPluginName, const std::string& pRepoReleaseLink);
	bool HttpDownload(const std::string& pUrl, const std::filesystem::path& pOutputFile) override;
	std::optional<std::string> HttpGet(const std::string& pUrl) override;
};
