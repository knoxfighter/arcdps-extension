#pragma warning(push, 0)
#pragma warning(disable : 4005)
#pragma warning(disable : 4389)
#pragma warning(disable : 26439)
#pragma warning(disable : 26495)
#include <gtest/gtest.h>
#pragma warning(pop)

#include <UpdateCheckerBase.h>

#include <filesystem>
#include <fstream>
#include <queue>

#if ARCDPS_EXTENSION_IMGUI
		static_assert(false);
#endif

using namespace ArcdpsExtension;

namespace {
	constexpr char MOCK_RESPONSE_ALL_RELEASES[] = R"DELIMITER([
  {
    "url": "https://api.github.com/repos/knoxfighter/arcdps-killproof.me-plugin/releases/45627408",
    "assets_url": "https://api.github.com/repos/knoxfighter/arcdps-killproof.me-plugin/releases/45627408/assets",
    "upload_url": "https://uploads.github.com/repos/knoxfighter/arcdps-killproof.me-plugin/releases/45627408/assets{?name,label}",
    "html_url": "https://github.com/knoxfighter/arcdps-killproof.me-plugin/releases/tag/v2.5.2",
    "id": 45627408,
    "author": {
      "login": "knoxfighter",
      "id": 18680546,
      "node_id": "MDQ6VXNlcjE4NjgwNTQ2",
      "avatar_url": "https://avatars.githubusercontent.com/u/18680546?v=4",
      "gravatar_id": "",
      "url": "https://api.github.com/users/knoxfighter",
      "html_url": "https://github.com/knoxfighter",
      "followers_url": "https://api.github.com/users/knoxfighter/followers",
      "following_url": "https://api.github.com/users/knoxfighter/following{/other_user}",
      "gists_url": "https://api.github.com/users/knoxfighter/gists{/gist_id}",
      "starred_url": "https://api.github.com/users/knoxfighter/starred{/owner}{/repo}",
      "subscriptions_url": "https://api.github.com/users/knoxfighter/subscriptions",
      "organizations_url": "https://api.github.com/users/knoxfighter/orgs",
      "repos_url": "https://api.github.com/users/knoxfighter/repos",
      "events_url": "https://api.github.com/users/knoxfighter/events{/privacy}",
      "received_events_url": "https://api.github.com/users/knoxfighter/received_events",
      "type": "User",
      "site_admin": false
    },
    "node_id": "MDc6UmVsZWFzZTQ1NjI3NDA4",
    "tag_name": "v2.5.2",
    "target_commitish": "master",
    "name": "v2.5.2",
    "draft": false,
    "prerelease": false,
    "created_at": "2021-07-02T14:23:47Z",
    "published_at": "2021-07-02T14:29:54Z",
    "assets": [
      {
        "url": "https://api.github.com/repos/knoxfighter/arcdps-killproof.me-plugin/releases/assets/39626428",
        "id": 39626428,
        "node_id": "MDEyOlJlbGVhc2VBc3NldDM5NjI2NDI4",
        "name": "d3d9_arcdps_killproof_me.dll",
        "label": null,
        "uploader": {
          "login": "knoxfighter",
          "id": 18680546,
          "node_id": "MDQ6VXNlcjE4NjgwNTQ2",
          "avatar_url": "https://avatars.githubusercontent.com/u/18680546?v=4",
          "gravatar_id": "",
          "url": "https://api.github.com/users/knoxfighter",
          "html_url": "https://github.com/knoxfighter",
          "followers_url": "https://api.github.com/users/knoxfighter/followers",
          "following_url": "https://api.github.com/users/knoxfighter/following{/other_user}",
          "gists_url": "https://api.github.com/users/knoxfighter/gists{/gist_id}",
          "starred_url": "https://api.github.com/users/knoxfighter/starred{/owner}{/repo}",
          "subscriptions_url": "https://api.github.com/users/knoxfighter/subscriptions",
          "organizations_url": "https://api.github.com/users/knoxfighter/orgs",
          "repos_url": "https://api.github.com/users/knoxfighter/repos",
          "events_url": "https://api.github.com/users/knoxfighter/events{/privacy}",
          "received_events_url": "https://api.github.com/users/knoxfighter/received_events",
          "type": "User",
          "site_admin": false
        },
        "content_type": "application/x-msdownload",
        "state": "uploaded",
        "size": 1437184,
        "download_count": 10343,
        "created_at": "2021-07-02T14:27:36Z",
        "updated_at": "2021-07-02T14:27:40Z",
        "browser_download_url": "https://github.com/knoxfighter/arcdps-killproof.me-plugin/releases/download/v2.5.2/d3d9_arcdps_killproof_me.dll"
      }
    ],
    "tarball_url": "https://api.github.com/repos/knoxfighter/arcdps-killproof.me-plugin/tarball/v2.5.2",
    "zipball_url": "https://api.github.com/repos/knoxfighter/arcdps-killproof.me-plugin/zipball/v2.5.2",
    "body": "**Make sure you have the newest C++ 2015-2019 redistributable installed:\r\n[Microsoft download page](https://support.microsoft.com/en-us/topic/the-latest-supported-visual-c-downloads-2647da03-1eea-4433-9aff-95f26a218cc0)**\r\n**[Direct download Link](https://aka.ms/vs/16/release/vc_redist.x64.exe)**\r\n\r\n- fixed coffers counted twice for LI/LD",
    "reactions": {
      "url": "https://api.github.com/repos/knoxfighter/arcdps-killproof.me-plugin/releases/45627408/reactions",
      "total_count": 1,
      "+1": 0,
      "-1": 0,
      "laugh": 0,
      "hooray": 0,
      "confused": 0,
      "heart": 0,
      "rocket": 1,
      "eyes": 0
    }
  },
  {
    "url": "https://api.github.com/repos/knoxfighter/arcdps-killproof.me-plugin/releases/45369082",
    "assets_url": "https://api.github.com/repos/knoxfighter/arcdps-killproof.me-plugin/releases/45369082/assets",
    "upload_url": "https://uploads.github.com/repos/knoxfighter/arcdps-killproof.me-plugin/releases/45369082/assets{?name,label}",
    "html_url": "https://github.com/knoxfighter/arcdps-killproof.me-plugin/releases/tag/v2.5.1",
    "id": 45369082,
    "author": {
      "login": "knoxfighter",
      "id": 18680546,
      "node_id": "MDQ6VXNlcjE4NjgwNTQ2",
      "avatar_url": "https://avatars.githubusercontent.com/u/18680546?v=4",
      "gravatar_id": "",
      "url": "https://api.github.com/users/knoxfighter",
      "html_url": "https://github.com/knoxfighter",
      "followers_url": "https://api.github.com/users/knoxfighter/followers",
      "following_url": "https://api.github.com/users/knoxfighter/following{/other_user}",
      "gists_url": "https://api.github.com/users/knoxfighter/gists{/gist_id}",
      "starred_url": "https://api.github.com/users/knoxfighter/starred{/owner}{/repo}",
      "subscriptions_url": "https://api.github.com/users/knoxfighter/subscriptions",
      "organizations_url": "https://api.github.com/users/knoxfighter/orgs",
      "repos_url": "https://api.github.com/users/knoxfighter/repos",
      "events_url": "https://api.github.com/users/knoxfighter/events{/privacy}",
      "received_events_url": "https://api.github.com/users/knoxfighter/received_events",
      "type": "User",
      "site_admin": false
    },
    "node_id": "MDc6UmVsZWFzZTQ1MzY5MDgy",
    "tag_name": "v2.5.1",
    "target_commitish": "master",
    "name": "v2.5.1",
    "draft": false,
    "prerelease": false,
    "created_at": "2021-06-28T16:18:31Z",
    "published_at": "2021-06-28T16:20:37Z",
    "assets": [
      {
        "url": "https://api.github.com/repos/knoxfighter/arcdps-killproof.me-plugin/releases/assets/39365466",
        "id": 39365466,
        "node_id": "MDEyOlJlbGVhc2VBc3NldDM5MzY1NDY2",
        "name": "d3d9_arcdps_killproof_me.dll",
        "label": null,
        "uploader": {
          "login": "knoxfighter",
          "id": 18680546,
          "node_id": "MDQ6VXNlcjE4NjgwNTQ2",
          "avatar_url": "https://avatars.githubusercontent.com/u/18680546?v=4",
          "gravatar_id": "",
          "url": "https://api.github.com/users/knoxfighter",
          "html_url": "https://github.com/knoxfighter",
          "followers_url": "https://api.github.com/users/knoxfighter/followers",
          "following_url": "https://api.github.com/users/knoxfighter/following{/other_user}",
          "gists_url": "https://api.github.com/users/knoxfighter/gists{/gist_id}",
          "starred_url": "https://api.github.com/users/knoxfighter/starred{/owner}{/repo}",
          "subscriptions_url": "https://api.github.com/users/knoxfighter/subscriptions",
          "organizations_url": "https://api.github.com/users/knoxfighter/orgs",
          "repos_url": "https://api.github.com/users/knoxfighter/repos",
          "events_url": "https://api.github.com/users/knoxfighter/events{/privacy}",
          "received_events_url": "https://api.github.com/users/knoxfighter/received_events",
          "type": "User",
          "site_admin": false
        },
        "content_type": "application/x-msdownload",
        "state": "uploaded",
        "size": 1437184,
        "download_count": 1919,
        "created_at": "2021-06-28T16:20:14Z",
        "updated_at": "2021-06-28T16:20:18Z",
        "browser_download_url": "https://github.com/knoxfighter/arcdps-killproof.me-plugin/releases/download/v2.5.1/d3d9_arcdps_killproof_me.dll"
      }
    ],
    "tarball_url": "https://api.github.com/repos/knoxfighter/arcdps-killproof.me-plugin/tarball/v2.5.1",
    "zipball_url": "https://api.github.com/repos/knoxfighter/arcdps-killproof.me-plugin/zipball/v2.5.1",
    "body": "**Make sure you have the newest C++ 2015-2019 redistributable installed:\r\n[Microsoft download page](https://support.microsoft.com/en-us/topic/the-latest-supported-visual-c-downloads-2647da03-1eea-4433-9aff-95f26a218cc0)**\r\n**[Direct download Link](https://aka.ms/vs/16/release/vc_redist.x64.exe)**\r\n\r\n- fixed broken automatic update download\r\n- sorting respects coffers\r\n- fixed players shown twice after adding them twice\r\n- fixed clear button not clearing already tracked and manually readded players"
  }
])DELIMITER";

	constexpr char MOCK_RESPONSE_LATEST_RELEASE[] = R"DELIMITER({
  "url": "https://api.github.com/repos/knoxfighter/arcdps-killproof.me-plugin/releases/45627408",
  "assets_url": "https://api.github.com/repos/knoxfighter/arcdps-killproof.me-plugin/releases/45627408/assets",
  "upload_url": "https://uploads.github.com/repos/knoxfighter/arcdps-killproof.me-plugin/releases/45627408/assets{?name,label}",
  "html_url": "https://github.com/knoxfighter/arcdps-killproof.me-plugin/releases/tag/v2.5.2",
  "id": 45627408,
  "author": {
    "login": "knoxfighter",
    "id": 18680546,
    "node_id": "MDQ6VXNlcjE4NjgwNTQ2",
    "avatar_url": "https://avatars.githubusercontent.com/u/18680546?v=4",
    "gravatar_id": "",
    "url": "https://api.github.com/users/knoxfighter",
    "html_url": "https://github.com/knoxfighter",
    "followers_url": "https://api.github.com/users/knoxfighter/followers",
    "following_url": "https://api.github.com/users/knoxfighter/following{/other_user}",
    "gists_url": "https://api.github.com/users/knoxfighter/gists{/gist_id}",
    "starred_url": "https://api.github.com/users/knoxfighter/starred{/owner}{/repo}",
    "subscriptions_url": "https://api.github.com/users/knoxfighter/subscriptions",
    "organizations_url": "https://api.github.com/users/knoxfighter/orgs",
    "repos_url": "https://api.github.com/users/knoxfighter/repos",
    "events_url": "https://api.github.com/users/knoxfighter/events{/privacy}",
    "received_events_url": "https://api.github.com/users/knoxfighter/received_events",
    "type": "User",
    "site_admin": false
  },
  "node_id": "MDc6UmVsZWFzZTQ1NjI3NDA4",
  "tag_name": "v2.5.2",
  "target_commitish": "master",
  "name": "v2.5.2",
  "draft": false,
  "prerelease": false,
  "created_at": "2021-07-02T14:23:47Z",
  "published_at": "2021-07-02T14:29:54Z",
  "assets": [
    {
      "url": "https://api.github.com/repos/knoxfighter/arcdps-killproof.me-plugin/releases/assets/39626428",
      "id": 39626428,
      "node_id": "MDEyOlJlbGVhc2VBc3NldDM5NjI2NDI4",
      "name": "d3d9_arcdps_killproof_me.dll",
      "label": null,
      "uploader": {
        "login": "knoxfighter",
        "id": 18680546,
        "node_id": "MDQ6VXNlcjE4NjgwNTQ2",
        "avatar_url": "https://avatars.githubusercontent.com/u/18680546?v=4",
        "gravatar_id": "",
        "url": "https://api.github.com/users/knoxfighter",
        "html_url": "https://github.com/knoxfighter",
        "followers_url": "https://api.github.com/users/knoxfighter/followers",
        "following_url": "https://api.github.com/users/knoxfighter/following{/other_user}",
        "gists_url": "https://api.github.com/users/knoxfighter/gists{/gist_id}",
        "starred_url": "https://api.github.com/users/knoxfighter/starred{/owner}{/repo}",
        "subscriptions_url": "https://api.github.com/users/knoxfighter/subscriptions",
        "organizations_url": "https://api.github.com/users/knoxfighter/orgs",
        "repos_url": "https://api.github.com/users/knoxfighter/repos",
        "events_url": "https://api.github.com/users/knoxfighter/events{/privacy}",
        "received_events_url": "https://api.github.com/users/knoxfighter/received_events",
        "type": "User",
        "site_admin": false
      },
      "content_type": "application/x-msdownload",
      "state": "uploaded",
      "size": 1437184,
      "download_count": 10344,
      "created_at": "2021-07-02T14:27:36Z",
      "updated_at": "2021-07-02T14:27:40Z",
      "browser_download_url": "https://github.com/knoxfighter/arcdps-killproof.me-plugin/releases/download/v2.5.2/d3d9_arcdps_killproof_me.dll"
    }
  ],
  "tarball_url": "https://api.github.com/repos/knoxfighter/arcdps-killproof.me-plugin/tarball/v2.5.2",
  "zipball_url": "https://api.github.com/repos/knoxfighter/arcdps-killproof.me-plugin/zipball/v2.5.2",
  "body": "**Make sure you have the newest C++ 2015-2019 redistributable installed:\r\n[Microsoft download page](https://support.microsoft.com/en-us/topic/the-latest-supported-visual-c-downloads-2647da03-1eea-4433-9aff-95f26a218cc0)**\r\n**[Direct download Link](https://aka.ms/vs/16/release/vc_redist.x64.exe)**\r\n\r\n- fixed coffers counted twice for LI/LD",
  "reactions": {
    "url": "https://api.github.com/repos/knoxfighter/arcdps-killproof.me-plugin/releases/45627408/reactions",
    "total_count": 1,
    "+1": 0,
    "-1": 0,
    "laugh": 0,
    "hooray": 0,
    "confused": 0,
    "heart": 0,
    "rocket": 1,
    "eyes": 0
  }
}
)DELIMITER";

	class UpdateCheckerMock final : public UpdateCheckerBase {
	public:
		std::string DllName = "update_checker_test.dll";
		std::queue<std::optional<std::string>> QueuedResponses;

		bool HttpDownload(const std::string&, const std::filesystem::path& pOutputFile) override {
			assert(QueuedResponses.size() > 0);

			std::optional<std::string> result = QueuedResponses.front();
			QueuedResponses.pop();

			if (result.has_value() == false) {
				return false;
			}

			std::ofstream outputFile(pOutputFile);

			if (!outputFile.is_open()) {
				return false;
			}
			outputFile.write(result->data(), result->size());
			outputFile.close();
			return true;
		}

		std::optional<std::string> HttpGet(const std::string&) override {
			assert(QueuedResponses.size() > 0);

			std::optional<std::string> result = QueuedResponses.front();
			QueuedResponses.pop();
			return result;
		}

		std::optional<std::string> GetPathFromHModule(HMODULE) noexcept override {
			return DllName;
		}
	};

	// parameters are <use_prerelease, window_dismissed>
	class UpdateCheckerTestFixture : public ::testing::TestWithParam<std::tuple<bool, bool>> {
	protected:
		UpdateCheckerMock mUpdater;

		void SetUp() override {
			remove(mUpdater.DllName.c_str());
			mUpdater.ClearFiles(NULL);
		}

		void TearDown() override {
			remove(mUpdater.DllName.c_str());
			mUpdater.ClearFiles(NULL);
		}
	};
} // anonymous namespace

TEST(UpdateCheckerTest, ParseVersion) {
	UpdateCheckerMock updater;
	// "Good path"
	EXPECT_EQ(updater.ParseVersion("1.2.3"), UpdateCheckerMock::Version({1, 2, 3, 0}));
	EXPECT_EQ(updater.ParseVersion("v1.2.3"), UpdateCheckerMock::Version({1, 2, 3, 0}));
	EXPECT_EQ(updater.ParseVersion("1v.2.3"), UpdateCheckerMock::Version({1, 2, 3, 0}));
	EXPECT_EQ(updater.ParseVersion("rc1v.r2c.hh3_"), UpdateCheckerMock::Version({1, 2, 3, 0}));

	// Tokens after the third one should be ignored
	EXPECT_EQ(updater.ParseVersion("v1.2.rc3.4"), UpdateCheckerMock::Version({1, 2, 3, 0}));
	EXPECT_EQ(updater.ParseVersion("v1.2.rc3.4.5.6"), UpdateCheckerMock::Version({1, 2, 3, 0}));

	// tokens that contain no digits should be ignored
	EXPECT_EQ(updater.ParseVersion("5.4.a.3.4"), UpdateCheckerMock::Version({5, 4, 3, 0}));

	// Invalid strings should return all zero version
	EXPECT_EQ(updater.ParseVersion("1.2"), UpdateCheckerMock::Version({0, 0, 0, 0}));
	EXPECT_EQ(updater.ParseVersion("v"), UpdateCheckerMock::Version({0, 0, 0, 0}));
	EXPECT_EQ(updater.ParseVersion(".."), UpdateCheckerMock::Version({0, 0, 0, 0}));
	EXPECT_EQ(updater.ParseVersion("..1"), UpdateCheckerMock::Version({0, 0, 0, 0}));
	EXPECT_EQ(updater.ParseVersion("a.b"), UpdateCheckerMock::Version({0, 0, 0, 0}));
}

TEST(UpdateCheckerTest, IsNewer) {
	UpdateCheckerMock updater;
	// Last version token is ignored
	EXPECT_FALSE(updater.IsNewer(UpdateCheckerMock::Version({1, 2, 3, 0}), UpdateCheckerMock::Version({1, 2, 3, 0})));
	EXPECT_FALSE(updater.IsNewer(UpdateCheckerMock::Version({1, 2, 3, 0}), UpdateCheckerMock::Version({1, 2, 3, 200})));
	EXPECT_FALSE(updater.IsNewer(UpdateCheckerMock::Version({1, 2, 3, 200}), UpdateCheckerMock::Version({1, 2, 3, 0})));

	// Newer version can be detected
	EXPECT_TRUE(updater.IsNewer(UpdateCheckerMock::Version({1, 2, 4, 0}), UpdateCheckerMock::Version({1, 2, 3, 0})));
	EXPECT_FALSE(updater.IsNewer(UpdateCheckerMock::Version({1, 2, 3, 0}), UpdateCheckerMock::Version({1, 2, 4, 0})));

	// Numbers earlier in the version number take precedence
	EXPECT_TRUE(updater.IsNewer(UpdateCheckerMock::Version({1, 3, 1, 0}), UpdateCheckerMock::Version({1, 2, 4, 0})));
	EXPECT_FALSE(updater.IsNewer(UpdateCheckerMock::Version({1, 2, 4, 0}), UpdateCheckerMock::Version({1, 3, 1, 0})));
	EXPECT_TRUE(updater.IsNewer(UpdateCheckerMock::Version({10, 0, 0, 0}), UpdateCheckerMock::Version({9, 9, 9, 9})));
	EXPECT_FALSE(updater.IsNewer(UpdateCheckerMock::Version({9, 9, 9, 9}), UpdateCheckerMock::Version({10, 0, 0, 0})));
}

TEST_P(UpdateCheckerTestFixture, Update_Positive) {
	auto [prerelease, dismiss_window] = GetParam();
	if (prerelease == false) {
		mUpdater.QueuedResponses.push(MOCK_RESPONSE_LATEST_RELEASE);
	} else {
		mUpdater.QueuedResponses.push(MOCK_RESPONSE_ALL_RELEASES);
	}

	std::unique_ptr<UpdateCheckerMock::UpdateState> state = mUpdater.CheckForUpdate(NULL, UpdateCheckerMock::Version({2, 5, 1, 0}), "knoxfighter/arcdps-killproof.me-plugin", prerelease);
	ASSERT_NE(state, nullptr);
	EXPECT_EQ(state->CurrentVersion, UpdateCheckerMock::Version({2, 5, 1, 0}));
	EXPECT_EQ(state->InstallPath, mUpdater.DllName);

	state->FinishPendingTasks();

	EXPECT_EQ(state->UpdateStatus, UpdateCheckerMock::Status::UpdateAvailable);
	EXPECT_EQ(state->NewVersion, UpdateCheckerMock::Version({2, 5, 2, 0}));
	EXPECT_EQ(state->DownloadUrl, "https://github.com/knoxfighter/arcdps-killproof.me-plugin/releases/download/v2.5.2/d3d9_arcdps_killproof_me.dll");

	{
		std::ofstream mockDll(mUpdater.DllName, std::ios::binary);
		mockDll << "OLD_VERSION";
	}

	mUpdater.QueuedResponses.push("DATA_IN_NEW_VERSION");
	{
		std::lock_guard lock(state->Lock);
		mUpdater.PerformInstallOrUpdate(*state);

		EXPECT_EQ(state->UpdateStatus, UpdateCheckerMock::Status::UpdateInProgress);
		if (dismiss_window == true) {
			state->UpdateStatus = UpdateCheckerMock::Status::Dismissed;
		}
	}

	state->FinishPendingTasks();

	if (dismiss_window == true) {
		EXPECT_EQ(state->UpdateStatus, UpdateCheckerMock::Status::Dismissed);
	} else {
		EXPECT_EQ(state->UpdateStatus, UpdateCheckerMock::Status::UpdateSuccessful);
	}

	{
		std::ifstream mockDll(state->InstallPath, std::ios::binary);
		std::string content;
		mockDll.seekg(0, std::ios::end);
		content.resize(mockDll.tellg());
		mockDll.seekg(0, std::ios::beg);
		mockDll.read(content.data(), content.size());

		EXPECT_EQ(content, "DATA_IN_NEW_VERSION");
	}
}

// Like the positive case, except it doesn't create the old file the update is performed from, so the rename will fail
TEST_P(UpdateCheckerTestFixture, Update_RenameFailure) {
	auto [prerelease, dismiss_window] = GetParam();
	if (prerelease == false) {
		mUpdater.QueuedResponses.push(MOCK_RESPONSE_LATEST_RELEASE);
	} else {
		mUpdater.QueuedResponses.push(MOCK_RESPONSE_ALL_RELEASES);
	}

	std::unique_ptr<UpdateCheckerMock::UpdateState> state = mUpdater.CheckForUpdate(NULL, UpdateCheckerMock::Version({2, 5, 1, 0}), "knoxfighter/arcdps-killproof.me-plugin", prerelease);
	ASSERT_NE(state, nullptr);
	EXPECT_EQ(state->CurrentVersion, UpdateCheckerMock::Version({2, 5, 1, 0}));
	EXPECT_EQ(state->InstallPath, mUpdater.DllName);

	state->FinishPendingTasks();

	EXPECT_EQ(state->UpdateStatus, UpdateCheckerMock::Status::UpdateAvailable);
	EXPECT_EQ(state->NewVersion, UpdateCheckerMock::Version({2, 5, 2, 0}));
	EXPECT_EQ(state->DownloadUrl, "https://github.com/knoxfighter/arcdps-killproof.me-plugin/releases/download/v2.5.2/d3d9_arcdps_killproof_me.dll");

	mUpdater.QueuedResponses.push("DATA_IN_NEW_VERSION");
	{
		std::lock_guard lock(state->Lock);
		mUpdater.PerformInstallOrUpdate(*state);

		EXPECT_EQ(state->UpdateStatus, UpdateCheckerMock::Status::UpdateInProgress);
		if (dismiss_window == true) {
			state->UpdateStatus = UpdateCheckerMock::Status::Dismissed;
		}
	}

	state->FinishPendingTasks();

	if (dismiss_window == true) {
		EXPECT_EQ(state->UpdateStatus, UpdateCheckerMock::Status::Dismissed);
	} else {
		EXPECT_EQ(state->UpdateStatus, UpdateCheckerMock::Status::UpdateError);
	}
}

// Like the positive case, except it pre-creates the temp file as a directory, so opening it as a file will fail
TEST_P(UpdateCheckerTestFixture, Update_TempFileFailure) {
	auto [prerelease, dismiss_window] = GetParam();
	if (prerelease == false) {
		mUpdater.QueuedResponses.push(MOCK_RESPONSE_LATEST_RELEASE);
	} else {
		mUpdater.QueuedResponses.push(MOCK_RESPONSE_ALL_RELEASES);
	}

	std::unique_ptr<UpdateCheckerMock::UpdateState> state = mUpdater.CheckForUpdate(NULL, UpdateCheckerMock::Version({2, 5, 1, 0}), "knoxfighter/arcdps-killproof.me-plugin", prerelease);
	ASSERT_NE(state, nullptr);
	EXPECT_EQ(state->CurrentVersion, UpdateCheckerMock::Version({2, 5, 1, 0}));
	EXPECT_EQ(state->InstallPath, mUpdater.DllName);

	state->FinishPendingTasks();

	EXPECT_EQ(state->UpdateStatus, UpdateCheckerMock::Status::UpdateAvailable);
	EXPECT_EQ(state->NewVersion, UpdateCheckerMock::Version({2, 5, 2, 0}));
	EXPECT_EQ(state->DownloadUrl, "https://github.com/knoxfighter/arcdps-killproof.me-plugin/releases/download/v2.5.2/d3d9_arcdps_killproof_me.dll");

	{
		std::ofstream mockDll(mUpdater.DllName, std::ios::binary);
		mockDll << "OLD_VERSION";
	}
	std::filesystem::create_directory(mUpdater.DllName + ".tmp");

	mUpdater.QueuedResponses.push("DATA_IN_NEW_VERSION");
	{
		std::lock_guard lock(state->Lock);
		mUpdater.PerformInstallOrUpdate(*state);

		EXPECT_EQ(state->UpdateStatus, UpdateCheckerMock::Status::UpdateInProgress);
		if (dismiss_window == true) {
			state->UpdateStatus = UpdateCheckerMock::Status::Dismissed;
		}
	}

	state->FinishPendingTasks();

	if (dismiss_window == true) {
		EXPECT_EQ(state->UpdateStatus, UpdateCheckerMock::Status::Dismissed);
	} else {
		EXPECT_EQ(state->UpdateStatus, UpdateCheckerMock::Status::UpdateError);
	}
}

// Like the positive case, except it returns a http error on download
TEST_P(UpdateCheckerTestFixture, Update_HttpError) {
	auto [prerelease, dismiss_window] = GetParam();
	if (prerelease == false) {
		mUpdater.QueuedResponses.push(MOCK_RESPONSE_LATEST_RELEASE);
	} else {
		mUpdater.QueuedResponses.push(MOCK_RESPONSE_ALL_RELEASES);
	}

	std::unique_ptr<UpdateCheckerMock::UpdateState> state = mUpdater.CheckForUpdate(NULL, UpdateCheckerMock::Version({2, 5, 1, 0}), "knoxfighter/arcdps-killproof.me-plugin", prerelease);
	ASSERT_NE(state, nullptr);
	EXPECT_EQ(state->CurrentVersion, UpdateCheckerMock::Version({2, 5, 1, 0}));
	EXPECT_EQ(state->InstallPath, mUpdater.DllName);

	state->FinishPendingTasks();

	EXPECT_EQ(state->UpdateStatus, UpdateCheckerMock::Status::UpdateAvailable);
	EXPECT_EQ(state->NewVersion, UpdateCheckerMock::Version({2, 5, 2, 0}));
	EXPECT_EQ(state->DownloadUrl, "https://github.com/knoxfighter/arcdps-killproof.me-plugin/releases/download/v2.5.2/d3d9_arcdps_killproof_me.dll");

	{
		std::ofstream mockDll(mUpdater.DllName, std::ios::binary);
		mockDll << "OLD_VERSION";
	}

	mUpdater.QueuedResponses.push(std::nullopt);
	{
		std::lock_guard lock(state->Lock);
		mUpdater.PerformInstallOrUpdate(*state);

		EXPECT_EQ(state->UpdateStatus, UpdateCheckerMock::Status::UpdateInProgress);
		if (dismiss_window == true) {
			state->UpdateStatus = UpdateCheckerMock::Status::Dismissed;
		}
	}

	state->FinishPendingTasks();

	if (dismiss_window == true) {
		EXPECT_EQ(state->UpdateStatus, UpdateCheckerMock::Status::Dismissed);
	} else {
		EXPECT_EQ(state->UpdateStatus, UpdateCheckerMock::Status::UpdateError);
	}
}

TEST_P(UpdateCheckerTestFixture, Install_Positive) {
	auto [prerelease, dismiss_window] = GetParam();
	if (prerelease == false) {
		mUpdater.QueuedResponses.push(MOCK_RESPONSE_LATEST_RELEASE);
	} else {
		mUpdater.QueuedResponses.push(MOCK_RESPONSE_ALL_RELEASES);
	}

	std::unique_ptr<UpdateCheckerMock::UpdateState> state = mUpdater.GetInstallState(std::string(mUpdater.DllName), "knoxfighter/arcdps-killproof.me-plugin", prerelease);
	ASSERT_NE(state, nullptr);
	EXPECT_EQ(state->CurrentVersion, std::nullopt);
	EXPECT_EQ(state->InstallPath, mUpdater.DllName);

	state->FinishPendingTasks();

	EXPECT_EQ(state->UpdateStatus, UpdateCheckerMock::Status::UpdateAvailable);
	EXPECT_EQ(state->NewVersion, UpdateCheckerMock::Version({2, 5, 2, 0}));
	EXPECT_EQ(state->DownloadUrl, "https://github.com/knoxfighter/arcdps-killproof.me-plugin/releases/download/v2.5.2/d3d9_arcdps_killproof_me.dll");

	mUpdater.QueuedResponses.push("DATA_IN_NEW_VERSION");
	{
		std::lock_guard lock(state->Lock);
		mUpdater.PerformInstallOrUpdate(*state);

		EXPECT_EQ(state->UpdateStatus, UpdateCheckerMock::Status::UpdateInProgress);
		if (dismiss_window == true) {
			state->UpdateStatus = UpdateCheckerMock::Status::Dismissed;
		}
	}

	state->FinishPendingTasks();

	if (dismiss_window == true) {
		EXPECT_EQ(state->UpdateStatus, UpdateCheckerMock::Status::Dismissed);
	} else {
		EXPECT_EQ(state->UpdateStatus, UpdateCheckerMock::Status::UpdateSuccessful);
	}

	{
		std::ifstream mockDll(state->InstallPath, std::ios::binary);
		std::string content;
		mockDll.seekg(0, std::ios::end);
		content.resize(mockDll.tellg());
		mockDll.seekg(0, std::ios::beg);
		mockDll.read(content.data(), content.size());

		EXPECT_EQ(content, "DATA_IN_NEW_VERSION");
	}
}

// Like the positive case, except it returns a http error on download
TEST_P(UpdateCheckerTestFixture, Install_HttpError) {
	auto [prerelease, dismiss_window] = GetParam();
	if (prerelease == false) {
		mUpdater.QueuedResponses.push(MOCK_RESPONSE_LATEST_RELEASE);
	} else {
		mUpdater.QueuedResponses.push(MOCK_RESPONSE_ALL_RELEASES);
	}

	std::unique_ptr<UpdateCheckerMock::UpdateState> state = mUpdater.GetInstallState(std::string(mUpdater.DllName), "knoxfighter/arcdps-killproof.me-plugin", prerelease);
	ASSERT_NE(state, nullptr);
	EXPECT_EQ(state->CurrentVersion, std::nullopt);
	EXPECT_EQ(state->InstallPath, mUpdater.DllName);

	state->FinishPendingTasks();

	EXPECT_EQ(state->UpdateStatus, UpdateCheckerMock::Status::UpdateAvailable);
	EXPECT_EQ(state->NewVersion, UpdateCheckerMock::Version({2, 5, 2, 0}));
	EXPECT_EQ(state->DownloadUrl, "https://github.com/knoxfighter/arcdps-killproof.me-plugin/releases/download/v2.5.2/d3d9_arcdps_killproof_me.dll");

	mUpdater.QueuedResponses.push(std::nullopt);
	{
		std::lock_guard lock(state->Lock);
		mUpdater.PerformInstallOrUpdate(*state);

		EXPECT_EQ(state->UpdateStatus, UpdateCheckerMock::Status::UpdateInProgress);
		if (dismiss_window == true) {
			state->UpdateStatus = UpdateCheckerMock::Status::Dismissed;
		}
	}

	state->FinishPendingTasks();

	if (dismiss_window == true) {
		EXPECT_EQ(state->UpdateStatus, UpdateCheckerMock::Status::Dismissed);
	} else {
		EXPECT_EQ(state->UpdateStatus, UpdateCheckerMock::Status::UpdateError);
	}
}

TEST_P(UpdateCheckerTestFixture, CheckForUpdate_NoUpdate) {
	auto [prerelease, dismiss_window] = GetParam();
	if (prerelease == false) {
		mUpdater.QueuedResponses.push(MOCK_RESPONSE_LATEST_RELEASE);
	} else {
		mUpdater.QueuedResponses.push(MOCK_RESPONSE_ALL_RELEASES);
	}

	std::unique_ptr<UpdateCheckerMock::UpdateState> state = mUpdater.CheckForUpdate(NULL, UpdateCheckerMock::Version({3, 0, 0, 0}), "knoxfighter/arcdps-killproof.me-plugin", prerelease);
	ASSERT_NE(state, nullptr);
	EXPECT_EQ(state->CurrentVersion, UpdateCheckerMock::Version({3, 0, 0, 0}));
	EXPECT_EQ(state->InstallPath, mUpdater.DllName);

	state->FinishPendingTasks();

	EXPECT_EQ(state->UpdateStatus, UpdateCheckerMock::Status::Unknown);
}

TEST_P(UpdateCheckerTestFixture, CheckForUpdate_BadJson) {
	auto [prerelease, dismiss_window] = GetParam();
	mUpdater.QueuedResponses.push("[ not json data }");
	std::unique_ptr<UpdateCheckerMock::UpdateState> state = mUpdater.CheckForUpdate(NULL, UpdateCheckerMock::Version({0, 0, 1, 0}), "Krappa322/arcdps_unofficial_extras_releases", prerelease);
	ASSERT_NE(state, nullptr);
	EXPECT_EQ(state->CurrentVersion, UpdateCheckerMock::Version({0, 0, 1, 0}));
	EXPECT_EQ(state->InstallPath, mUpdater.DllName);

	state->FinishPendingTasks();

	EXPECT_EQ(state->UpdateStatus, UpdateCheckerMock::Status::Unknown);
}

TEST_P(UpdateCheckerTestFixture, CheckForUpdate_NoReleases) {
	auto [prerelease, dismiss_window] = GetParam();
	mUpdater.QueuedResponses.push("[]");
	std::unique_ptr<UpdateCheckerMock::UpdateState> state = mUpdater.CheckForUpdate(NULL, UpdateCheckerMock::Version({0, 0, 1, 0}), "Krappa322/arcdps_unofficial_extras_releases", prerelease);
	ASSERT_NE(state, nullptr);
	EXPECT_EQ(state->CurrentVersion, UpdateCheckerMock::Version({0, 0, 1, 0}));
	EXPECT_EQ(state->InstallPath, mUpdater.DllName);

	state->FinishPendingTasks();

	EXPECT_EQ(state->UpdateStatus, UpdateCheckerMock::Status::Unknown);
}

INSTANTIATE_TEST_SUITE_P(
		Stable,
		UpdateCheckerTestFixture,
		::testing::Values(std::make_pair(false, false), std::make_pair(false, true))
);

INSTANTIATE_TEST_SUITE_P(
		PreRelease,
		UpdateCheckerTestFixture,
		::testing::Values(std::make_pair(true, false), std::make_pair(true, true))
);
