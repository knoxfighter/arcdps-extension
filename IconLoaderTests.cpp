#include "IconLoader.h"

#include "Singleton.h"
#include "test/resource.h"

#include <chrono>
#include <cstddef>
#include <filesystem>
#include <gtest/gtest.h>
#include <string>
#include <thread>

// Windows
#include <d3d11.h>
#include <d3dcommon.h>
#include <windows.h>

namespace {
	ID3D11Device* D11_DEVICE = nullptr;
	ID3D11DeviceContext* D11_DEVICE_CONTEXT = nullptr;
	HMODULE EXE_HANDLE = NULL;
} // namespace

using namespace ArcdpsExtension;

class IconLoaderTests : public ::testing::Test {
public:
	static void SetUpTestSuite() {
		/* SETUP DIRECTX11 */
		const D3D_FEATURE_LEVEL featureLevelArray[2] = {
				D3D_FEATURE_LEVEL_11_0,
		};

		D3D_FEATURE_LEVEL featureLevel;
		HRESULT res = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, featureLevelArray, 1, D3D11_SDK_VERSION, &D11_DEVICE, &featureLevel, &D11_DEVICE_CONTEXT);
		if (res != S_OK) {
			GTEST_FAIL();
			return;
		}

		EXE_HANDLE = GetModuleHandleA(NULL);

		ArcdpsExtension::IconLoader::init(EXE_HANDLE, D11_DEVICE);

		/* Delete cached downloaded file */
		auto fullPath = std::filesystem::temp_directory_path();
		fullPath /= "GW2-arcdps-extension";
		fullPath /= "wiki.guildwars2.com/images/4/4c/Alacrity.png";

		if (exists(fullPath)) {
			remove(fullPath);
		}
	}

	static void TearDownTestSuite() {
		D11_DEVICE_CONTEXT->Release();
		D11_DEVICE_CONTEXT = nullptr;

		D11_DEVICE->Release();
		D11_DEVICE = nullptr;

		g_singletonManagerInstance.Shutdown();
	}
};

TEST_F(IconLoaderTests, LoadFromFile) {
	using namespace std::chrono_literals;
	auto& iconLoader = ArcdpsExtension::IconLoader::instance();

	std::string file = TEST_DIR;
	file.append("Alacrity.png");
	iconLoader.RegisterFile(1, file);

	const auto& begin = std::chrono::steady_clock::now();
	while (true) {
		if (iconLoader.Draw(1) != nullptr) {
			break;
		}

		// check for timeout
		const auto& now = std::chrono::steady_clock::now();
		const auto& diff = now - begin;
		ASSERT_LT(diff, 10s);

		std::this_thread::sleep_for(100ms);
	}
}

TEST_F(IconLoaderTests, LoadFromUrl) {
	using namespace std::chrono_literals;
	auto& iconLoader = ArcdpsExtension::IconLoader::instance();

	iconLoader.RegisterUrl(2, "https://assets.gw2dat.com/102484.png");

	const auto& begin = std::chrono::steady_clock::now();
	while (true) {
		if (iconLoader.Draw(2) != nullptr) {
			break;
		}

		// check for timeout
		const auto& now = std::chrono::steady_clock::now();
		const auto& diff = now - begin;
		ASSERT_LT(diff, 10s);

		std::this_thread::sleep_for(100ms);
	}
}

TEST_F(IconLoaderTests, LoadFromGw2Dat) {
	using namespace std::chrono_literals;
	auto& iconLoader = ArcdpsExtension::IconLoader::instance();

	iconLoader.RegisterGw2Dat(4, "102484"); // https://assets.gw2dat.com/102484.png

	const auto& begin = std::chrono::steady_clock::now();
	while (true) {
		if (iconLoader.Draw(4) != nullptr) {
			break;
		}

		// check for timeout
		const auto& now = std::chrono::steady_clock::now();
		const auto& diff = now - begin;
		ASSERT_LT(diff, 10s);

		std::this_thread::sleep_for(100ms);
	}
}

TEST_F(IconLoaderTests, LoadFromResource) {
	using namespace std::chrono_literals;
	auto& iconLoader = ArcdpsExtension::IconLoader::instance();

	iconLoader.RegisterResource(3, ID_Alacrity);

	const auto& begin = std::chrono::steady_clock::now();
	while (true) {
		if (iconLoader.Draw(3) != nullptr) {
			break;
		}

		// check for timeout
		const auto& now = std::chrono::steady_clock::now();
		const auto& diff = now - begin;
		ASSERT_LT(diff, 10s);

		std::this_thread::sleep_for(100ms);
	}
}
