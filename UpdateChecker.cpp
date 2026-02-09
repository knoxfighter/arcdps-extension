#include "UpdateChecker.h"

#include "ExtensionTranslations.h"
#include "Localization.h"
#include "SimpleNetworkStack.h"
#include "Widgets.h"

#include <imgui/imgui.h>
#include <expected>
#include <format>
#include <future>
#include <magic_enum/magic_enum.hpp>
#include <optional>
#include <string_view>
#include <thread>
#include <mutex>
#include <type_traits>

// Windows
#include <shellapi.h>
#include <winuser.h>

void ArcdpsExtension::UpdateChecker::Draw(const std::unique_ptr<UpdateState>& pUpdateState, const std::string& pPluginName, const std::string& pRepoReleaseLink) {
	if (!pUpdateState) {
		// wrongly initialized UpdateChecker, nothing to do here!
		return;
	}
	std::lock_guard guard(pUpdateState->Lock);

	const Status& updateStatus = pUpdateState->UpdateStatus;
	if (updateStatus != Status::Unknown && updateStatus != Status::Dismissed) {
		bool open = true;
		if (ImGui::Begin(std::format("{}Update###{} Update", pPluginName, pPluginName).c_str(), &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize)) {
			const Version& currentVersion = *pUpdateState->CurrentVersion;
			const Version& newVersion = pUpdateState->NewVersion;

			ImGuiEx::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), Localization::STranslate(ET_UpdateDesc), pPluginName);
			ImGuiEx::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "{}: {}.{}.{}", Localization::STranslate(ET_UpdateCurrentVersion), currentVersion[0], currentVersion[1], currentVersion[2]);
			ImGuiEx::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "{}: {}.{}.{}", Localization::STranslate(ET_UpdateNewVersion), newVersion[0], newVersion[1], newVersion[2]);
			if (ImGui::Button(Localization::STranslate(ET_UpdateOpenPage).data())) {
				std::thread([pRepoReleaseLink]() {
					ShellExecuteA(nullptr, nullptr, pRepoReleaseLink.c_str(), nullptr, nullptr, SW_SHOW);
				}).detach();
			}

			switch (updateStatus) {
				case Status::UpdateAvailable: {
					if (ImGui::Button(Localization::STranslate(ET_UpdateAutoButton).data())) {
						PerformInstallOrUpdate(*pUpdateState);
					}
					break;
				}
				case Status::UpdateInProgress: {
					ImGui::TextUnformatted(Localization::STranslate(ET_UpdateInProgress).data());
					break;
				}
				case Status::UpdateSuccessful: {
					ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), "%s", Localization::STranslate(ET_UpdateRestartPending).data());
					break;
				}
				case Status::UpdateError: {
					ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), "%s", Localization::STranslate(ET_UpdateError).data());
				}
				// nothing to print when user doesn't want to update
				case Status::Unknown:
				case Status::Dismissed: break;
			}

			ImGui::End();
		}

		if (!open) {
			pUpdateState->UpdateStatus = Status::Dismissed;
		}
	}
}

bool ArcdpsExtension::UpdateChecker::HttpDownload(const std::string& pUrl, const std::filesystem::path& pOutputFile) {
	auto& networkStack = SimpleNetworkStack::instance();

	std::promise<SimpleNetworkStack::Result> promise;
	auto future = promise.get_future();
	networkStack.QueueGet(pUrl, std::move(promise), pOutputFile);
	auto response = future.get();
	if (!response.has_value()) {
		Log(std::format("Downloading {} failed - networkStack error {} - {}", pUrl, magic_enum::enum_name(response.error().Type), response.error().Message));
		return false;
	} else if (response.value().Code != 200) {
		Log(std::format("Downloading {} failed - http failure {} {}", pUrl, response.value().Code, response.value().Message));
		return false;
	}

	return true;
}

std::optional<std::string> ArcdpsExtension::UpdateChecker::HttpGet(const std::string& pUrl) {
	auto& networkStack = SimpleNetworkStack::instance();

	std::promise<SimpleNetworkStack::Result> promise;
	auto future = promise.get_future();
	networkStack.QueueGet(pUrl, std::move(promise));
	auto response = future.get();

	if (!response) {
		auto& error = response.error();
		Log(std::format("Getting {} failed - {} - {}", pUrl, magic_enum::enum_name(error.Type), error.Message));
		return std::nullopt;
	}

	auto& result = response.value();
	if (result.Code != 200) {
		Log(std::format("Getting {} failed - {} {}", pUrl, result.Code, result.Message));
		return std::nullopt;
	}

	return result.Message;
}
