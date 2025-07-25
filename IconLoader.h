#pragma once

#include "Singleton.h"

#include <atlbase.h>
#include <condition_variable>
#include <d3d11.h>
#include <filesystem>
#include <map>
#include <queue>
#include <ranges>
#include <thread>
#include <utility>
#include <variant>
#include <wincodec.h>
#include <Windows.h>

namespace ArcdpsExtension {
	template<typename From, typename To>
	concept static_castable = requires(From f) {
		static_cast<To>(f);
	};

	using IconLoaderKeyType = uint64_t;

	template<typename T>
	concept IconLoaderKey = static_castable<T, IconLoaderKeyType>;

	/**
	 * Load icons in a separate thread.<br>
	 * Always have an enum or any other list of numbers as unique ID.<br>
	 * Make sure to call `Setup()` before you use this class.
	 * <p>
	 * Class-Internal flow:
	 * @code
	 * MainThread::Register()
	 *   ↳ lock mThreadMutex
	 *   ↳ mThreadQueue
	 * ↓
	 * IconLoaderThread::runner()
	 *   ↳ Load()
	 *   ↳ lock mLoadQueueMutex
	 *   ↳ mLoadQueue
	 * ↓
	 * MainThread::Draw()
	 *   ↳ lock mLoadQueueMutex
	 *   ↳ extract from mLoadQueue
	 *   ↳ mIcons
	 * @endcode
	 * </p>
	 */
	class IconLoader final : public Singleton<IconLoader> {
	public:
		IconLoader();
		~IconLoader() override;

		/**
		 * Call in `mod_init()`. If this is not called properly, it will crash when icons are loaded.
		 * @param pDll The DLL of this module (the one to load resources from)
		 * @param pD11Device The D3D11 device
		 */
		void Setup(HMODULE pDll, ID3D11Device* pD11Device);

		/**
		 * Register an Icon that will be loaded from a file.
		 * @param pName UID of the Icon (will be casted to `IconLoaderKeyType`)
		 * @param pFilepath The Path to the file that will be loaded (has to exist, else nothing happens)
		 */
		void RegisterFile(IconLoaderKey auto pName, const std::filesystem::path& pFilepath);

		/**
		 * Register an Icon that will be downloaded and then loaded. It will cache files in the Temp Dir.
		 * @param pName UID of the Icon (will be casted to `IconLoaderKeyType`)
		 * @param pUrl The URL to download (has to be a full link, like `https://wiki.guildwars2.com/images/4/4c/Alacrity.png`)
		 */
		void RegisterUrl(IconLoaderKey auto pName, const std::string& pUrl);

		/**
		 * Register an Icon that will be downloaded from gw2dat.com. It will cache files in the Temp Dir.
		 * @param pName UID of the Icon (will be casted to `IconLoaderKeyType`)
		 * @param pId The ID to download from gw2dat.com (`.png` will be added to the id)
		 */
		void RegisterGw2Dat(IconLoaderKey auto pName, const std::string& pId);

		/**
		 * Register an Icon that will be loaded from the Resource of the module defined in Setup().
		 * @param pName UID of the Icon (will be casted to `IconLoaderKeyType`)
		 * @param pId The ID of the resource to load (normally defined in resource.h)
		 */
		void RegisterResource(IconLoaderKey auto pName, UINT pId);

		/**
		 * Get the ResourceView* from a previously Registered Icon.
		 * @param pName UID of the Icon (will be casted to `IconLoaderKeyType`)
		 * @return either the ResourceView* or nullptr if loading failed or is in progress.
		 */
		ID3D11ShaderResourceView* Draw(IconLoaderKey auto pName);

	private:
		struct Icon {
			UINT Width;
			UINT Height;
			ID3D11ShaderResourceView* Texture;

			Icon(UINT pWidth, UINT pHeight, ID3D11ShaderResourceView* pTexture) : Width(pWidth), Height(pHeight), Texture(pTexture) {}
		};

		enum class LoadWay {
			File,
			Url,
			Gw2Dat,
			Resource,
		};
		class QueueIcon {
		public:
			QueueIcon(IconLoaderKeyType pName, IconLoader& pIconLoader, LoadWay pWay, const auto& pResource) : mIconLoader(pIconLoader), mName(pName), mWay(pWay), mResource(pResource) {}

		protected:
			IconLoaderKeyType mName;
			LoadWay mWay;
			std::variant<std::filesystem::path, std::string, UINT> mResource;
			IconLoader& mIconLoader;
			UINT mWidth = 0;
			UINT mHeight = 0;
			std::vector<uint8_t> mPixelBuffer;
			DXGI_FORMAT mDxgiFormat = DXGI_FORMAT_UNKNOWN;

			void Load();

		private:
			void LoadFile(const std::filesystem::path& pFilepath);
#if ARCDPS_EXTENSION_CURL
			void LoadUrl(const std::string& pUrl);
			void LoadGw2Dat(const std::string& pId);
#endif
			void LoadResource(UINT pId);
			void LoadFrame(const CComPtr<IWICBitmapFrameDecode>& pIDecodeFrame, const CComPtr<IWICImagingFactory>& pIWICFactory);
			void DeviceLoad();
			static DXGI_FORMAT GetFormatDx11(WICPixelFormatGUID pPixelFormat);

			friend IconLoader;
		};

		HMODULE mDll;
		ID3D11Device* mD11Device;

		std::unordered_map<IconLoaderKeyType, Icon> mIcons;
		std::unordered_map<IconLoaderKeyType, QueueIcon> mLoadQueue;
		std::mutex mLoadQueueMutex;
		std::queue<QueueIcon> mThreadQueue;

		std::jthread mThread;
		std::mutex mThreadMutex;
		std::condition_variable_any mThreadVariable;

		void runner(std::stop_token pToken);
		void queueLoad(const QueueIcon& pIcon);
		void loadDone(const IconLoader::QueueIcon& pIcon, ID3D11ShaderResourceView* pTexture);
	};
} // namespace ArcdpsExtension

void ArcdpsExtension::IconLoader::RegisterFile(IconLoaderKey auto pName, const std::filesystem::path& pFilepath) {
	std::lock_guard guard(mThreadMutex);

	mThreadQueue.emplace(static_cast<IconLoaderKeyType>(pName), *this, LoadWay::File, pFilepath);

	mThreadVariable.notify_one();
}

void ArcdpsExtension::IconLoader::RegisterUrl(IconLoaderKey auto pName, const std::string& pUrl) {
	std::lock_guard guard(mThreadMutex);

	mThreadQueue.emplace(static_cast<IconLoaderKeyType>(pName), *this, LoadWay::Url, pUrl);

	mThreadVariable.notify_one();
}

void ArcdpsExtension::IconLoader::RegisterGw2Dat(IconLoaderKey auto pName, const std::string& pId) {
	std::lock_guard guard(mThreadMutex);

	mThreadQueue.emplace(static_cast<IconLoaderKeyType>(pName), *this, LoadWay::Gw2Dat, pId);

	mThreadVariable.notify_one();
}

void ArcdpsExtension::IconLoader::RegisterResource(IconLoaderKey auto pName, UINT pId) {
	std::lock_guard guard(mThreadMutex);

	mThreadQueue.emplace(static_cast<IconLoaderKeyType>(pName), *this, LoadWay::Resource, pId);

	mThreadVariable.notify_one();
}

ID3D11ShaderResourceView* ArcdpsExtension::IconLoader::Draw(IconLoaderKey auto pName) {
	auto name = static_cast<IconLoaderKeyType>(pName);
	if (const auto& icon = mIcons.find(name); icon != mIcons.end()) {
		return icon->second.Texture;
	}
	if (const auto& icon = mLoadQueue.find(name); icon != mLoadQueue.end()) {
		std::unique_lock lock(mLoadQueueMutex);
		auto element = mLoadQueue.extract(name);
		lock.unlock();

		element.mapped().DeviceLoad();
	}
	return nullptr;
}
