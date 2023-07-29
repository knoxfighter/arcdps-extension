#include "UpdateChecker.h"

#include "ExtensionTranslations.h"
#include "Localization.h"
#include "Widgets.h"

#if __has_include(<imgui/imgui.h>)
#include <imgui/imgui.h>
#else
#include "../imgui/imgui.h"
#endif

void UpdateChecker::Draw(const std::unique_ptr<UpdateState>& pUpdateState, const std::string& pPluginName, const std::string& pRepoReleaseLink) {
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
			if (ImGui::Button(Localization::STranslate(ET_UpdateOpenPage).c_str())) {
				std::thread([pRepoReleaseLink]() {
					ShellExecuteA(nullptr, nullptr, pRepoReleaseLink.c_str(), nullptr, nullptr, SW_SHOW);
				}).detach();
			}

			switch (updateStatus) {
				case Status::UpdateAvailable: {
					if (ImGui::Button(Localization::STranslate(ET_UpdateAutoButton).c_str())) {
						PerformInstallOrUpdate(*pUpdateState);
					}
					break;
				}
				case Status::UpdateInProgress: {
					ImGui::TextUnformatted(Localization::STranslate(ET_UpdateInProgress).c_str());
					break;
				}
				case Status::UpdateSuccessful: {
					ImGui::TextColored(ImVec4(0.f, 1.f, 0.f, 1.f), Localization::STranslate(ET_UpdateRestartPending).c_str());
					break;
				}
				case Status::UpdateError: {
					ImGui::TextColored(ImVec4(1.f, 0.f, 0.f, 1.f), Localization::STranslate(ET_UpdateError).c_str());
				}
			}

			ImGui::End();
		}

		if (!open) {
			pUpdateState->UpdateStatus = Status::Dismissed;
		}
	}
}
