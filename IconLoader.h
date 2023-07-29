#pragma once

#include "Singleton.h"

#include <atlbase.h>
#include <condition_variable>
#include <d3d11.h>
#include <d3d9.h>
#include <filesystem>
#include <map>
#include <ranges>
#include <thread>
#include <utility>
#include <variant>
#include <wincodec.h>
#include <Windows.h>

#ifndef ARCDPS_EXTENSION_NO_CPR
#include <cpr/cpr.h>
#endif

/**
 * Call `Setup()` in `mod_init()`. This is needed, so this class knows about the dll and the directx device!
 * The IconLoader is not thread-safe, only call it in the render frame (or on startup)
 *
 * Always have an enum or any other list of numbers as unique ID.
 * Load textures:
 * - GetTexture(pResourceId):
 *     Load Texture from a resource within this DLL
 * - GetTexture(pFilePath):
 *	   Load Texture from file.
 *	   There is a direct conversion from string to filesystem::path.
 *	   If such a file is not found, you will always get a nullptr as result (no file reload performed)
 * - GetTexture(pUrl):
 *     Load Texture from url.
 *     It will be saved in AppData/Local/Temp/GW2-arcdps-extension and loaded from there.
 *	   If the file already exists, it will just be loaded.
 *	   Only available if cpr is loaded (can be disabled with `ARCDPS_EXTENSION_NO_CPR`)
 */
class IconLoader : public Singleton<IconLoader> {
public:
	IconLoader() {
		// mLoadThread = std::jthread([this](std::stop_token pToken){LoadThreadFunc(pToken);});
		mLoadThread = std::jthread(std::bind_front(&IconLoader::LoadThreadFunc, this));
	}

	~IconLoader() override;

	void Setup(HMODULE new_dll, IDirect3DDevice9* d3d9Device, ID3D11Device* new_d3d11device);

	/*************/
	/* GetTexture - used when already there, does NOT load */
	/*************/
	void* GetTexture(size_t pId);

	/*************/
	/* LoadTexture - used to request to load a texture and to get it's unique ID */
	/*************/
	size_t LoadTexture(UINT pResourceId);
	size_t LoadTexture(const std::filesystem::path& pFilePath);
#ifndef ARCDPS_EXTENSION_NO_CPR
	size_t LoadTexture(const cpr::Url& pUrl);
#endif

private:
	/**
	 *          /  PendingResource  \
	 * Pending                         Loaded  -  Finished
	 *          \  PendingFile      /
	 *
	 *	Fallout: Error
	 */
	enum class Status {
		Pending,
		PendingResource,
		PendingFile,
		PendingCpr,
		Loaded,
		Finished,
		Error,
	};

#ifndef ARCDPS_EXTENSION_NO_CPR
	using TargetVariant = std::variant<UINT, std::filesystem::path, cpr::Url>;
#else
	using TargetVariant = std::variant<UINT, std::filesystem::path>;

#endif

	class Texture {
	public:
		UINT Width = 0;
		UINT Height = 0;
		Status Status = Status::Pending;

		Texture() = default;

		// copy/move
		Texture(const Texture& pOther) = default;
		Texture(Texture&& pOther) noexcept = default;
		Texture& operator=(const Texture& pOther) = default;
		Texture& operator=(Texture&& pOther) noexcept = default;

		friend ::IconLoader;

	private:
		std::vector<uint8_t> mPixelBuffer;
		IDirect3DTexture9* mD9Texture = nullptr;
		ID3D11ShaderResourceView* mD11Texture = nullptr;
		TargetVariant mLoadTarget;
		DXGI_FORMAT mDxgiFormat = DXGI_FORMAT_UNKNOWN;

		void LoadTexture(IconLoader& pIconLoader, UINT pResourceId);
		void LoadTexture(IconLoader& pIconLoader, const std::filesystem::path& pFilePath);
#ifndef ARCDPS_EXTENSION_NO_CPR
		void LoadTexture(IconLoader& pIconLoader, const cpr::Url& pUrl);
		void LoadFromCpr();
#endif
		void Load(IconLoader& pIconLoader);

		void* GetTexture(IconLoader& pIconLoader);
		void LoadFrame(const CComPtr<IWICBitmapFrameDecode>& pIDecodeFrame, const CComPtr<IWICImagingFactory>& pIWICFactory);
		DXGI_FORMAT GetFormatDx11(WICPixelFormatGUID pPixelFormat);
		_D3DFORMAT GetFormatDx9();
		void LoadIntoDirectXDevice(IconLoader& pIconLoader);
		void LoadFromFile();
		void LoadFromResource(IconLoader& pIconLoader);
	};

	void LoadThreadFunc(std::stop_token stop_token);

	HMODULE mDll = nullptr;
	ID3D11Device* mD3d11Device = nullptr;
	IDirect3DDevice9* mD3d9Device = nullptr;

	std::vector<Texture> mTextures;
	std::mutex mTexturesMutex;
	std::deque<size_t> mLoadIds;

	std::jthread mLoadThread;
	std::condition_variable_any mThreadCondition;
	std::mutex mThreadMutex;
};

#define GET_TEXTURE(name, source)                                       \
	std::invoke([] {                                                    \
		auto& iconLoader = IconLoader::instance();                      \
		static size_t textureId##name = iconLoader.LoadTexture(source); \
		return iconLoader.GetTexture(textureId##name);                  \
	})
