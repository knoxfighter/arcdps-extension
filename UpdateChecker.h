#pragma once

#include "Singleton.h"
#include "UpdateCheckerBase.h"

#include <memory>
#include <string>

namespace ArcdpsExtension {
	/**
	 * Use this only if you can live with the Dependencies:<br>
	 * - ImGui directly<br>
	 * - Localization.h<br>
	 * - Singleton of this library (it has to be setup correctly as well)
	 */
	class UpdateChecker final : public UpdateCheckerBase, public Singleton<UpdateChecker> {
	public:
		void Draw(const std::unique_ptr<UpdateState>& pUpdateState, const std::string& pPluginName, const std::string& pRepoReleaseLink);
		bool HttpDownload(const std::string& pUrl, const std::filesystem::path& pOutputFile) override;
		std::optional<std::string> HttpGet(const std::string& pUrl) override;
	};
} // namespace ArcdpsExtension
