#pragma once

#include <array>
#include <expected>
#include <filesystem>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <vector>
#include <windows.h>

namespace ArcdpsExtension {
	// Expected usage is as follows:
	// - Optionally, create a new class that inherits from UpdateCheckerBase to modify the functionality, in case your addon
	//   uses a different version format than the default one (3 numbers separated by dots, all letters in tag names ignored),
	//   or to provide a sink for the log lines
	// - Instantiate your derivative class (or UpdateCheckerBase if there is none) either during static initialization or in
	//   mod_init.
	// - In mod_init, call the following, in order
	//   - GetCurrentVersion() to get the current version of your addon in UpdateChecker format, store this globally
	//   - ClearFiles() to remove the artifacts from old updates and ensure that a new update would succeed
	//   - CheckForUpdate() to start the check for an update. The update is fetched asynchronously. Store the struct returned
	//     by the function globally
	// - In mod_release, call FinishPendingTasks() on the update struct and then destruct it
	// - In mod_imgui, acquire a lock on the update struct and look at UpdateStatus. Show a window displaying there is an
	//   update available if it's not Unknown or Dismissed. If the window is closed, set UpdateStatus to Dismissed. If the
	//   user chooses to perform an update, call PerformInstallOrUpdate() with the update struct still locked.
	class UpdateCheckerBase {
	public:
		using Version = std::array<WORD, 4>;

		//                            / Dismissed
		// Unknown -> UpdateAvailable                    / UpdateSuccessful
		//                            \ UpdateInProgress
		//                                               \ UpdateError
		enum class Status {
			Unknown,
			UpdateAvailable,
			Dismissed,
			UpdateInProgress,
			UpdateSuccessful,
			UpdateError
		};

		struct UpdateState {
			UpdateState(const std::optional<Version>& pVersion, std::string&& pInstallPath);
			~UpdateState();

			void FinishPendingTasks();

			// Changes the status to pNewStatus iff the current value is pExpectedStatus
			bool ChangeStatus(Status pExpectedStatus, Status pNewStatus);

			const std::optional<Version> CurrentVersion = {};
			const std::string InstallPath;

			std::mutex Lock; // Protects all non-const members
			Status UpdateStatus = Status::Unknown;
			std::vector<std::thread> Tasks;

			// The fields below are only defined if UpdateStatus != Status::Unknown, at which point they are constant
			Version NewVersion = {};
			std::string DownloadUrl;
		};

		static std::expected<Version, std::string> GetCurrentVersion(HMODULE dll) noexcept;
		void ClearFiles(HMODULE pDll) noexcept;
		std::unique_ptr<UpdateState> CheckForUpdate(HMODULE pDll, const Version& pCurrentVersion, std::string&& pRepo, bool pAllowPreRelease) noexcept;
		std::unique_ptr<UpdateState> GetInstallState(std::string&& pInstallPath, std::string&& pRepo, bool pAllowPreRelease) noexcept;
		void PerformInstallOrUpdate(UpdateState& pState) noexcept; // Requires lock to be held on pState already

		virtual std::optional<std::string> GetPathFromHModule(HMODULE pDll) noexcept;
		std::unique_ptr<UpdateState> GetUpdateInternal(std::string&& pInstallPath, const std::optional<Version>& pCurrentVersion, std::string&& pRepo, bool pAllowPreRelease) noexcept;
		static std::string GetVersionAsString(const Version& pVersion);
		virtual bool IsNewer(const Version& pRepoVersion, const Version& pCurrentVersion);
		virtual void Log(std::string&& pMessage);
		virtual Version ParseVersion(std::string_view versionString);
		bool PerformDownload(const std::string& pUrl, const std::string& pDestinationPath);

		// Can throw http errors
		std::optional<std::tuple<Version, std::string>> GetLatestRelease(std::string&& pRepo, bool pAllowPreRelease);
		virtual bool HttpDownload(const std::string& pUrl, const std::filesystem::path& pOutputFile) = 0;
		virtual std::optional<std::string> HttpGet(const std::string& pUrl) = 0;
	};
} // namespace ArcdpsExtension
