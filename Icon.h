#pragma once

#include <Windows.h>
#include <d3d9.h>
#include <d3d11.h>
#include <map>

class IconLoader;

class Icon {
public:
	Icon(UINT name, HMODULE dll, IDirect3DDevice9* d3d9Device, ID3D11Device* d3d11Device);
	Icon() = delete;
	~Icon();

	// delete copy/move
	Icon(const Icon& other) = delete;
	Icon(Icon&& other) noexcept = delete;
	Icon& operator=(const Icon& other) = delete;
	Icon& operator=(Icon&& other) noexcept = delete;

	void* getTexture() const;
	
private:
	UINT width;
	UINT height;
	IDirect3DTexture9* d9texture = nullptr;
	ID3D11ShaderResourceView* d11texture = nullptr;
};

/**
 * Call `Setup()` in `mod_init()`. This is needed, so this class knows about the dll and the directx device!
 * Call `Shutdown()` in `mod_release()`. This is needed, so gw2 does not crash while closing.
 */
class IconLoader {
public:
	void Setup(HMODULE new_dll, IDirect3DDevice9* d3d9Device, ID3D11Device* new_d3d11device);
	void* getTexture(UINT name);
	void Shutdown();

private:
	HMODULE dll = nullptr;
	ID3D11Device* d3d11device = nullptr;
	IDirect3DDevice9* d3d9Device = nullptr;
	std::map<UINT, Icon> textures;
};

extern IconLoader iconLoader;
