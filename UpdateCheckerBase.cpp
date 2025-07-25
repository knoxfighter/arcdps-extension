#include "UpdateCheckerBase.h"

#include <cassert>
#include <charconv>
#include <filesystem>
#include <format>
#include <fstream>
#include <optional>

#include <nlohmann/json.hpp>

ArcdpsExtension::UpdateCheckerBase::UpdateState::UpdateState(const std::optional<Version>& pVersion, std::string&& pInstallPath)
	: CurrentVersion(pVersion), InstallPath(std::move(pInstallPath)) {}

ArcdpsExtension::UpdateCheckerBase::UpdateState::~UpdateState() {
	assert(Tasks.empty() && "All tasks should be awaited before shutdown");

	// Desparate attempt to not crash in release, this might crash itself because joining threads during static destruction is wonky on Windows
	FinishPendingTasks();
}

void ArcdpsExtension::UpdateCheckerBase::UpdateState::FinishPendingTasks() {
	std::vector<std::thread> tasks;
	{
		std::lock_guard lock(Lock);
		tasks = std::move(Tasks);
	}

	for (auto& task : tasks) {
		task.join();
	}
}

bool ArcdpsExtension::UpdateCheckerBase::UpdateState::ChangeStatus(Status pExpectedStatus, Status pNewStatus) {
	std::lock_guard lock(Lock);
	if (UpdateStatus != pExpectedStatus) {
		return false;
	}

	UpdateStatus = pNewStatus;
	return true;
}

std::expected<ArcdpsExtension::UpdateCheckerBase::Version, std::string> ArcdpsExtension::UpdateCheckerBase::GetCurrentVersion(HMODULE pDll) noexcept {
	// GetModuleFileName
	TCHAR moduleFileName[MAX_PATH + 1]{};
	if (!GetModuleFileName(pDll, moduleFileName, MAX_PATH)) {
		return std::unexpected(std::format("GetCurrentVersion: GetModuleFileName failed - {}", GetLastError()));
	}

	// GetFileVersionInfoSize
	DWORD dummy; // this will get set to 0 (wtf windows api)
	DWORD versionInfoSize = GetFileVersionInfoSize(moduleFileName, &dummy);
	if (versionInfoSize == 0) {
		return std::unexpected(std::format("GetCurrentVersion: GetFileVersionInfoSize failed - {}", GetLastError()));
	}
	std::vector<BYTE> data(versionInfoSize);

	// GetFileVersionInfo
	if (!GetFileVersionInfo(moduleFileName, NULL, versionInfoSize, &data[0])) {
		return std::unexpected(std::format("GetCurrentVersion: GetFileVersionInfo failed - {}", GetLastError()));
	}

	UINT randomPointer = 0;
	VS_FIXEDFILEINFO* fixedFileInfo = nullptr;
	if (!VerQueryValue(data.data(), TEXT("\\"), (void**) &fixedFileInfo, &randomPointer)) {
		return std::unexpected(std::format("GetCurrentVersion: VerQueryValue failed - {}", GetLastError()));
	}

	return Version({HIWORD(fixedFileInfo->dwProductVersionMS), LOWORD(fixedFileInfo->dwProductVersionMS), HIWORD(fixedFileInfo->dwProductVersionLS), LOWORD(fixedFileInfo->dwProductVersionLS)});
}

void ArcdpsExtension::UpdateCheckerBase::ClearFiles(HMODULE pDll) noexcept {
	std::error_code ec;

	std::optional<std::string> dllPath = GetPathFromHModule(pDll);
	if (dllPath.has_value() == false) {
		Log("ClearFiles: Failed to get self path");
		return;
	}

	std::string tmpPath = *dllPath + ".tmp";
	std::string oldPath = *dllPath + ".old";

	std::filesystem::remove(tmpPath, ec);
	if (ec != std::error_code{}) {
		Log(std::format("Failed to remove {} - value={} message={} category={}", tmpPath, ec.value(), ec.message(), ec.category().name()));
	}

	std::filesystem::remove(oldPath, ec);
	if (ec != std::error_code{}) {
		Log(std::format("Failed to remove {} - value={} message={} category={}", oldPath, ec.value(), ec.message(), ec.category().name()));
	}
}

std::unique_ptr<ArcdpsExtension::UpdateCheckerBase::UpdateState> ArcdpsExtension::UpdateCheckerBase::CheckForUpdate(
		HMODULE pDll, const Version& pCurrentVersion, std::string&& pRepo, bool pAllowPreRelease
) noexcept {
	std::optional<std::string> dllPath = GetPathFromHModule(pDll);
	if (dllPath.has_value() == false) {
		Log("GetUpdate: Failed to get self path");
		return nullptr;
	}

	return GetUpdateInternal(std::move(*dllPath), pCurrentVersion, std::move(pRepo), pAllowPreRelease);
}

std::unique_ptr<ArcdpsExtension::UpdateCheckerBase::UpdateState> ArcdpsExtension::UpdateCheckerBase::GetInstallState(
		std::string&& pInstallPath, std::string&& pRepo, bool pAllowPreRelease
) noexcept {
	// TODO: FIRST CHECK IF FILE ALREADY EXISTS? OR SHOULD WE USE INTERNAL STATE IN ADDON TO DETERMINE?
	return GetUpdateInternal(std::move(pInstallPath), std::nullopt, std::move(pRepo), pAllowPreRelease);
}

void ArcdpsExtension::UpdateCheckerBase::PerformInstallOrUpdate(UpdateState& pState) noexcept {
	assert(pState.Lock.try_lock() == false && "Lock should be held when this function is called");

	if (pState.UpdateStatus != Status::UpdateAvailable) {
		Log(std::format("Tried to download update when update status was {}", static_cast<int>(pState.UpdateStatus)));
		return;
	}
	pState.UpdateStatus = Status::UpdateInProgress;

	// Update
	if (pState.CurrentVersion.has_value()) {
		pState.Tasks.emplace_back([this, &pState]() {
			std::string dllPathTemp = pState.InstallPath + ".tmp";
			std::string dllPathOld = pState.InstallPath + ".old";

			// Reading DownloadUrl without lock is fine since it's constant as soon as it has been set
			if (PerformDownload(pState.DownloadUrl, dllPathTemp) == false) {
				pState.ChangeStatus(Status::UpdateInProgress, Status::UpdateError);
				return;
			}

			if (rename(pState.InstallPath.c_str(), dllPathOld.c_str()) != 0) {
				Log(std::format("Failed to rename {} to {} - errno={} GetLastError={}", pState.InstallPath, dllPathOld, errno, GetLastError()));

				pState.ChangeStatus(Status::UpdateInProgress, Status::UpdateError);
				return;
			}

			if (rename(dllPathTemp.c_str(), pState.InstallPath.c_str()) != 0) {
				Log(std::format("Failed to rename {} to {} - errno={} GetLastError={}", dllPathTemp, pState.InstallPath, errno, GetLastError()));

				pState.ChangeStatus(Status::UpdateInProgress, Status::UpdateError);
				return;
			}

			Log(std::format("Successfully performed update"));
			pState.ChangeStatus(Status::UpdateInProgress, Status::UpdateSuccessful);
		});
	} else // Install
	{
		pState.Tasks.emplace_back([this, &pState]() {
			// Reading DownloadUrl without lock is fine since it's constant as soon as it has been set
			if (PerformDownload(pState.DownloadUrl, pState.InstallPath) == false) {
				pState.ChangeStatus(Status::UpdateInProgress, Status::UpdateError);
				return;
			}

			Log(std::format("Successfully performed install"));
			pState.ChangeStatus(Status::UpdateInProgress, Status::UpdateSuccessful);
		});
	}
}

std::optional<std::string> ArcdpsExtension::UpdateCheckerBase::GetPathFromHModule(HMODULE pDll) noexcept {
	CHAR dllPath[MAX_PATH] = {};
	if (GetModuleFileNameA(pDll, dllPath, _countof(dllPath)) == 0) {
		Log(std::format("Getting path failed - GetLastError={}", GetLastError()));
		return std::nullopt;
	}

	return std::string(dllPath);
}

std::unique_ptr<ArcdpsExtension::UpdateCheckerBase::UpdateState> ArcdpsExtension::UpdateCheckerBase::GetUpdateInternal(
		std::string&& pInstallPath, const std::optional<Version>& pCurrentVersion, std::string&& pRepo,
		bool pAllowPreRelease
) noexcept {
	std::unique_ptr<UpdateState> result = std::make_unique<UpdateState>(pCurrentVersion, std::move(pInstallPath));

	// Perform the rest of the work in a newly spawned thread as a sort of poor man's async
	std::lock_guard lock(result->Lock);
	result->Tasks.emplace_back([this, result = result.get(), repo = std::move(pRepo), pAllowPreRelease]() mutable {
		std::optional<std::tuple<Version, std::string>> latestRelease = std::nullopt;
		try {
			latestRelease = GetLatestRelease(std::move(repo), pAllowPreRelease);
		} catch (std::exception& e) {
			Log(std::format("GetUpdateInternal: GetLatestRelease threw {}", e.what()));
			return;
		}

		if (latestRelease.has_value() == false) {
			Log("GetUpdate: GetUpdateInternal didn't find any release");
			return;
		}

		Version releaseVersion = std::get<0>(*latestRelease);
		if (result->CurrentVersion.has_value()) {
			if (IsNewer(releaseVersion, *result->CurrentVersion) == false) {
				Log(std::format(
						"GetUpdateInternal: Found new release {} which is not newer than current installed version {}",
						GetVersionAsString(releaseVersion), GetVersionAsString(*result->CurrentVersion)
				));
				return;
			}
		}

		std::lock_guard lock(result->Lock);
		result->UpdateStatus = Status::UpdateAvailable;
		result->NewVersion = releaseVersion;
		result->DownloadUrl = std::move(std::get<1>(*latestRelease));

		Log(std::format("GetUpdateInternal: Found new release {} with link {}", GetVersionAsString(result->NewVersion), result->DownloadUrl));
	});

	return result;
}

std::string ArcdpsExtension::UpdateCheckerBase::GetVersionAsString(const Version& pVersion) {
	return std::format("{}.{}.{}.{}", pVersion.at(0), pVersion.at(1), pVersion.at(2), pVersion.at(3));
}

bool ArcdpsExtension::UpdateCheckerBase::IsNewer(const Version& pRepoVersion, const Version& pCurrentVersion) {
	return std::tie(pCurrentVersion[0], pCurrentVersion[1], pCurrentVersion[2])
		   < std::tie(pRepoVersion[0], pRepoVersion[1], pRepoVersion[2]);
}

void ArcdpsExtension::UpdateCheckerBase::Log(std::string&&) {
	// Do nothing by default
}

ArcdpsExtension::UpdateCheckerBase::Version ArcdpsExtension::UpdateCheckerBase::ParseVersion(std::string_view versionString) {
	// TODO: use semver to calculate this. So all semver releases can be parsed and not only my hardcoded ones :)
	// libary for it: https://github.com/Neargye/semver

	Version result{};
	size_t tokenIndex = 0;
	size_t start = 0;
	do {
		size_t dotPos = versionString.find('.', start);
		if (dotPos == std::string_view::npos) {
			dotPos = versionString.size();
		}

		std::string_view token_str = versionString.substr(start, dotPos - start);

		// Remove all non-digit characters from the beginning of the token
		while (true) {
			if (token_str.empty()) {
				break;
			}
			if (isdigit(*token_str.begin())) {
				break;
			}
			token_str = token_str.substr(1);
		}

		// Parse the str token to an integer
		const auto from_chars_result = std::from_chars(
				token_str.data(),
				token_str.data() + token_str.size(),
				result[tokenIndex]
		);
		if (from_chars_result.ec != std::errc{}) {
			Log(std::format("Parsing version token '{}' from '{}' failed", versionString, token_str));
		} else {
			tokenIndex++;
		}

		start = dotPos + 1; // + 1 to skip over the '.'
	} while (start < versionString.size() && tokenIndex < 3);

	if (tokenIndex < 3) {
		Log(std::format("Failed to parse version from {} - only found {} tokens", versionString, tokenIndex));
		return Version{};
	}

	return result;
}

bool ArcdpsExtension::UpdateCheckerBase::PerformDownload(const std::string& pUrl, const std::string& pDestinationPath) {
	if (HttpDownload(pUrl, pDestinationPath) == false) {
		return false;
	}

	return true;
}

std::optional<std::tuple<ArcdpsExtension::UpdateCheckerBase::Version, std::string>> ArcdpsExtension::UpdateCheckerBase::GetLatestRelease(
		std::string&& pRepo, bool pAllowPreRelease
) {
	nlohmann::basic_json<> json;
	nlohmann::basic_json<>* release;
	if (pAllowPreRelease == false) {
		std::string link = std::format("https://api.github.com/repos/{}/releases/latest", pRepo);

		auto response = HttpGet(link);
		if (response.has_value() == false) {
			Log(std::format("Getting {} failed", link));
			return std::nullopt;
		}

		json = nlohmann::json::parse(response.value());
		release = &json;
	} else {
		std::string link = std::format("https://api.github.com/repos/{}/releases", pRepo);

		auto response = HttpGet(link);
		if (response.has_value() == false) {
			Log(std::format("Getting {} failed", link));
			return std::nullopt;
		}

		json = nlohmann::json::parse(response.value());
		release = &json[0];
	}

	const auto tagName = release->at("tag_name").get<std::string>();
	Version releaseVersion = ParseVersion(tagName);
	std::string releaseDownloadUrl;

	// load download URL (use the first asset that has a .dll ending)
	for (const auto& item : (*release)["assets"]) {
		const auto assetName = item["name"].get<std::string>();
		if (assetName.size() < 4) {
			continue;
		}

		if (std::string_view(assetName).substr(assetName.size() - 4) == ".dll") {
			releaseDownloadUrl = item["browser_download_url"].get<std::string>();
			Log(std::format("Found download url in {} - {}", assetName, releaseDownloadUrl));
			break;
		}
	}

	if (releaseDownloadUrl.empty()) {
		Log(std::format("Failed to find download url for release {}", tagName));
		return std::nullopt;
	}

	return std::make_tuple(releaseVersion, releaseDownloadUrl);
}
