#include "IconLoader.h"


IconLoader::~IconLoader() {
	mLoadThread.request_stop();
	mLoadThread.join();
	mTextures.clear();
}

void IconLoader::Setup(HMODULE new_dll, IDirect3DDevice9* new_d3d9Device, ID3D11Device* new_d3d11device) {
	mDll = new_dll;
	mD3d11Device = new_d3d11device;
	mD3d9Device = new_d3d9Device;
}

void* IconLoader::GetTexture(size_t pId) {
	assert(mD3d9Device || mD3d11Device);

	return mTextures[pId].GetTexture(*this);
}

size_t IconLoader::LoadTexture(UINT pResourceId) {
	assert(mD3d9Device || mD3d11Device);

	std::lock_guard guard(mTexturesMutex);

	auto& texture = mTextures.emplace_back();

	texture.LoadTexture(*this, pResourceId);

	size_t id = mTextures.size() - 1;

	mLoadIds.push_back(id);

	return id;
}

size_t IconLoader::LoadTexture(const std::filesystem::path& pFilePath) {
	assert(mD3d9Device || mD3d11Device);

	std::lock_guard guard(mTexturesMutex);

	auto& texture = mTextures.emplace_back();

	texture.LoadTexture(*this, pFilePath);

	size_t id = mTextures.size() - 1;

	mLoadIds.push_back(id);

	return id;
}

/*
 * Array of needed GUID conversions.
 * CurrentGUID -> WantedGUID
 */
static const std::vector<std::pair<GUID, GUID>> WIC_CONVERT =
		{
  // Note target GUID in this conversion table must be one of those directly supported formats (above).

				{GUID_WICPixelFormatBlackWhite,           GUID_WICPixelFormat8bppGray        }, // DXGI_FORMAT_R8_UNORM

				{GUID_WICPixelFormat1bppIndexed,          GUID_WICPixelFormat32bppRGBA       }, // DXGI_FORMAT_R8G8B8A8_UNORM
				{GUID_WICPixelFormat2bppIndexed,          GUID_WICPixelFormat32bppRGBA       }, // DXGI_FORMAT_R8G8B8A8_UNORM
				{GUID_WICPixelFormat4bppIndexed,          GUID_WICPixelFormat32bppRGBA       }, // DXGI_FORMAT_R8G8B8A8_UNORM
				{GUID_WICPixelFormat8bppIndexed,          GUID_WICPixelFormat32bppRGBA       }, // DXGI_FORMAT_R8G8B8A8_UNORM

				{GUID_WICPixelFormat2bppGray,             GUID_WICPixelFormat8bppGray        }, // DXGI_FORMAT_R8_UNORM
				{GUID_WICPixelFormat4bppGray,             GUID_WICPixelFormat8bppGray        }, // DXGI_FORMAT_R8_UNORM

				{GUID_WICPixelFormat16bppGrayFixedPoint,  GUID_WICPixelFormat16bppGrayHalf   }, // DXGI_FORMAT_R16_FLOAT
				{GUID_WICPixelFormat32bppGrayFixedPoint,  GUID_WICPixelFormat32bppGrayFloat  }, // DXGI_FORMAT_R32_FLOAT

#ifdef DXGI_1_2_FORMATS

				{GUID_WICPixelFormat16bppBGR555,          GUID_WICPixelFormat16bppBGRA5551   }, // DXGI_FORMAT_B5G5R5A1_UNORM

#else

				{GUID_WICPixelFormat16bppBGR555, GUID_WICPixelFormat32bppRGBA},   // DXGI_FORMAT_R8G8B8A8_UNORM
				{GUID_WICPixelFormat16bppBGRA5551, GUID_WICPixelFormat32bppRGBA}, // DXGI_FORMAT_R8G8B8A8_UNORM
				{GUID_WICPixelFormat16bppBGR565, GUID_WICPixelFormat32bppRGBA},   // DXGI_FORMAT_R8G8B8A8_UNORM

#endif  // DXGI_1_2_FORMATS

				{GUID_WICPixelFormat32bppBGR101010,       GUID_WICPixelFormat32bppRGBA1010102}, // DXGI_FORMAT_R10G10B10A2_UNORM

				{GUID_WICPixelFormat24bppBGR,             GUID_WICPixelFormat32bppRGBA       }, // DXGI_FORMAT_R8G8B8A8_UNORM
				{GUID_WICPixelFormat24bppRGB,             GUID_WICPixelFormat32bppRGBA       }, // DXGI_FORMAT_R8G8B8A8_UNORM
				{GUID_WICPixelFormat32bppPBGRA,           GUID_WICPixelFormat32bppRGBA       }, // DXGI_FORMAT_R8G8B8A8_UNORM
				{GUID_WICPixelFormat32bppPRGBA,           GUID_WICPixelFormat32bppRGBA       }, // DXGI_FORMAT_R8G8B8A8_UNORM

				{GUID_WICPixelFormat48bppRGB,             GUID_WICPixelFormat64bppRGBA       }, // DXGI_FORMAT_R16G16B16A16_UNORM
				{GUID_WICPixelFormat48bppBGR,             GUID_WICPixelFormat64bppRGBA       }, // DXGI_FORMAT_R16G16B16A16_UNORM
				{GUID_WICPixelFormat64bppBGRA,            GUID_WICPixelFormat64bppRGBA       }, // DXGI_FORMAT_R16G16B16A16_UNORM
				{GUID_WICPixelFormat64bppPRGBA,           GUID_WICPixelFormat64bppRGBA       }, // DXGI_FORMAT_R16G16B16A16_UNORM
				{GUID_WICPixelFormat64bppPBGRA,           GUID_WICPixelFormat64bppRGBA       }, // DXGI_FORMAT_R16G16B16A16_UNORM

				{GUID_WICPixelFormat48bppRGBFixedPoint,   GUID_WICPixelFormat64bppRGBAHalf   }, // DXGI_FORMAT_R16G16B16A16_FLOAT
				{GUID_WICPixelFormat48bppBGRFixedPoint,   GUID_WICPixelFormat64bppRGBAHalf   }, // DXGI_FORMAT_R16G16B16A16_FLOAT
				{GUID_WICPixelFormat64bppRGBAFixedPoint,  GUID_WICPixelFormat64bppRGBAHalf   }, // DXGI_FORMAT_R16G16B16A16_FLOAT
				{GUID_WICPixelFormat64bppBGRAFixedPoint,  GUID_WICPixelFormat64bppRGBAHalf   }, // DXGI_FORMAT_R16G16B16A16_FLOAT
				{GUID_WICPixelFormat64bppRGBFixedPoint,   GUID_WICPixelFormat64bppRGBAHalf   }, // DXGI_FORMAT_R16G16B16A16_FLOAT
				{GUID_WICPixelFormat64bppRGBHalf,         GUID_WICPixelFormat64bppRGBAHalf   }, // DXGI_FORMAT_R16G16B16A16_FLOAT
				{GUID_WICPixelFormat48bppRGBHalf,         GUID_WICPixelFormat64bppRGBAHalf   }, // DXGI_FORMAT_R16G16B16A16_FLOAT

				{GUID_WICPixelFormat96bppRGBFixedPoint,   GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT
				{GUID_WICPixelFormat128bppPRGBAFloat,     GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT
				{GUID_WICPixelFormat128bppRGBFloat,       GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT
				{GUID_WICPixelFormat128bppRGBAFixedPoint, GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT
				{GUID_WICPixelFormat128bppRGBFixedPoint,  GUID_WICPixelFormat128bppRGBAFloat }, // DXGI_FORMAT_R32G32B32A32_FLOAT

				{GUID_WICPixelFormat32bppCMYK,            GUID_WICPixelFormat32bppRGBA       }, // DXGI_FORMAT_R8G8B8A8_UNORM
				{GUID_WICPixelFormat64bppCMYK,            GUID_WICPixelFormat64bppRGBA       }, // DXGI_FORMAT_R16G16B16A16_UNORM
				{GUID_WICPixelFormat40bppCMYKAlpha,       GUID_WICPixelFormat64bppRGBA       }, // DXGI_FORMAT_R16G16B16A16_UNORM
				{GUID_WICPixelFormat80bppCMYKAlpha,       GUID_WICPixelFormat64bppRGBA       }, // DXGI_FORMAT_R16G16B16A16_UNORM

#if (_WIN32_WINNT >= 0x0602  /*_WIN32_WINNT_WIN8*/)
				{GUID_WICPixelFormat32bppRGB,             GUID_WICPixelFormat32bppRGBA       }, // DXGI_FORMAT_R8G8B8A8_UNORM
				{GUID_WICPixelFormat64bppRGB,             GUID_WICPixelFormat64bppRGBA       }, // DXGI_FORMAT_R16G16B16A16_UNORM
				{GUID_WICPixelFormat64bppPRGBAHalf,       GUID_WICPixelFormat64bppRGBAHalf   }, // DXGI_FORMAT_R16G16B16A16_FLOAT
#endif

  // We don't support n-channel formats
};

DXGI_FORMAT IconLoader::Texture::GetFormatDx11(WICPixelFormatGUID pPixelFormat) {
	if (pPixelFormat == GUID_WICPixelFormat128bppRGBAFloat) {
		return DXGI_FORMAT_R32G32B32A32_FLOAT;
	}
	if (pPixelFormat == GUID_WICPixelFormat64bppRGBAHalf) {
		return DXGI_FORMAT_R16G16B16A16_FLOAT;
	}
	if (pPixelFormat == GUID_WICPixelFormat64bppRGBA) {
		return DXGI_FORMAT_R16G16B16A16_UNORM;
	}
	if (pPixelFormat == GUID_WICPixelFormat32bppRGBA) {
		return DXGI_FORMAT_R8G8B8A8_UNORM;
	}
	if (pPixelFormat == GUID_WICPixelFormat32bppBGRA) {
		return DXGI_FORMAT_B8G8R8A8_UNORM;
	}
	if (pPixelFormat == GUID_WICPixelFormat32bppBGR) {
		return DXGI_FORMAT_B8G8R8X8_UNORM;
	}
	if (pPixelFormat == GUID_WICPixelFormat32bppRGBA1010102XR) {
		return DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM;
	}
	if (pPixelFormat == GUID_WICPixelFormat32bppRGBA1010102) {
		return DXGI_FORMAT_R10G10B10A2_UNORM;
	}
	if (pPixelFormat == GUID_WICPixelFormat32bppRGBE) {
		return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
	}
	if (pPixelFormat == GUID_WICPixelFormat16bppBGRA5551) {
		return DXGI_FORMAT_B5G5R5A1_UNORM;
	}
	if (pPixelFormat == GUID_WICPixelFormat16bppBGR565) {
		return DXGI_FORMAT_B5G6R5_UNORM;
	}
	if (pPixelFormat == GUID_WICPixelFormat32bppGrayFloat) {
		return DXGI_FORMAT_R32_FLOAT;
	}
	if (pPixelFormat == GUID_WICPixelFormat16bppGrayHalf) {
		return DXGI_FORMAT_R16_FLOAT;
	}
	if (pPixelFormat == GUID_WICPixelFormat16bppGray) {
		return DXGI_FORMAT_R16_UNORM;
	}
	if (pPixelFormat == GUID_WICPixelFormat8bppGray) {
		return DXGI_FORMAT_R8_UNORM;
	}
	if (pPixelFormat == GUID_WICPixelFormat8bppAlpha) {
		return DXGI_FORMAT_A8_UNORM;
	}
	if (pPixelFormat == GUID_WICPixelFormat96bppRGBFloat) {
		return DXGI_FORMAT_R32G32B32_FLOAT;
	}

	throw std::runtime_error("Given DX11 Format is not supported!");
}

static const std::map<DXGI_FORMAT, _D3DFORMAT> DX9_FORMAT = {
		{DXGI_FORMAT_B8G8R8A8_UNORM,      D3DFMT_A8R8G8B8     },
		{DXGI_FORMAT_B8G8R8A8_UNORM_SRGB, D3DFMT_A8R8G8B8     },
		{DXGI_FORMAT_B8G8R8X8_UNORM,      D3DFMT_X8R8G8B8     },
		{DXGI_FORMAT_B8G8R8X8_UNORM_SRGB, D3DFMT_X8R8G8B8     },
		{DXGI_FORMAT_B5G6R5_UNORM,        D3DFMT_R5G6B5       },
 // {DXGI_FORMAT_B5G5R5A1_UNORM, D3DFMT_A1R5G5B5},
  // {DXGI_FORMAT_B4G4R4A4_UNORM, D3DFMT_A4R4G4B4},
		{DXGI_FORMAT_A8_UNORM,            D3DFMT_A8           },
		{DXGI_FORMAT_R8G8B8A8_UNORM,      D3DFMT_A8B8G8R8     },
		{DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, D3DFMT_A8B8G8R8     },
		{DXGI_FORMAT_R16G16_UNORM,        D3DFMT_G16R16       },
		{DXGI_FORMAT_R16G16B16A16_UNORM,  D3DFMT_A16B16G16R16 },
		{DXGI_FORMAT_R8_UNORM,            D3DFMT_L8           },
 // {DXGI_FORMAT_R8G8_UNORM, D3DFMT_A8L8},
  // {DXGI_FORMAT_R8G8_SNORM, D3DFMT_V8U8},
  // {DXGI_FORMAT_R8G8B8A8_SNORM, D3DFMT_Q8W8V8U8},
  // {DXGI_FORMAT_R16G16_SNORM, D3DFMT_V16U16},
  // {DXGI_FORMAT_G8R8_G8B8_UNORM, D3DFMT_R8G8_B8G8},
  // {DXGI_FORMAT_R8G8_B8G8_UNORM, D3DFMT_G8R8_G8B8},
  // {DXGI_FORMAT_BC1_UNORM, D3DFMT_DXT1},
  // {DXGI_FORMAT_BC1_UNORM_SRGB, D3DFMT_DXT1},
  // {DXGI_FORMAT_BC1_UNORM, D3DFMT_DXT2},
  // {DXGI_FORMAT_BC1_UNORM_SRGB, D3DFMT_DXT2},
  // {DXGI_FORMAT_BC2_UNORM, D3DFMT_DXT3},
  // {DXGI_FORMAT_BC2_UNORM_SRGB, D3DFMT_DXT3},
  // {DXGI_FORMAT_BC2_UNORM, D3DFMT_DXT4},
  // {DXGI_FORMAT_BC2_UNORM_SRGB, D3DFMT_DXT4},
  // {DXGI_FORMAT_BC3_UNORM, D3DFMT_DXT5},
  // {DXGI_FORMAT_BC3_UNORM_SRGB, D3DFMT_DXT5},
  // {DXGI_FORMAT_D16_UNORM, D3DFMT_D16},
  // {DXGI_FORMAT_D32_FLOAT, D3DFMT_D32F_LOCKABLE},
		{DXGI_FORMAT_R16_UNORM,           D3DFMT_L16          },
 // {DXGI_FORMAT_R16_UINT, D3DFMT_INDEX16},
  // {DXGI_FORMAT_R32_UINT, D3DFMT_INDEX32},
  // {DXGI_FORMAT_R16G16B16A16_SNORM, D3DFMT_Q16W16V16U16},
		{DXGI_FORMAT_R16_FLOAT,           D3DFMT_R16F         },
		{DXGI_FORMAT_R16G16_FLOAT,        D3DFMT_G16R16F      },
		{DXGI_FORMAT_R16G16B16A16_FLOAT,  D3DFMT_A16B16G16R16F},
		{DXGI_FORMAT_R32_FLOAT,           D3DFMT_R32F         },
		{DXGI_FORMAT_R32G32_FLOAT,        D3DFMT_G32R32F      },
		{DXGI_FORMAT_R32G32B32A32_FLOAT,  D3DFMT_A32B32G32R32F},
};

_D3DFORMAT IconLoader::Texture::GetFormatDx9() {
	const auto& pair = DX9_FORMAT.find(mDxgiFormat);
	if (pair != DX9_FORMAT.end()) {
		return pair->second;
	}
	throw std::runtime_error("Given DX9 Format is not supported!");
}

void IconLoader::Texture::LoadIntoDirectXDevice(IconLoader& pIconLoader) {
	// Create texture
	if (pIconLoader.mD3d11Device) {
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof desc);
		desc.Width = Width;
		desc.Height = Height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = mDxgiFormat;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		D3D11_SUBRESOURCE_DATA subResource;
		ZeroMemory(&subResource, sizeof subResource);
		subResource.pSysMem = static_cast<const void*>(mPixelBuffer.data());
		subResource.SysMemPitch = desc.Width * 4;
		subResource.SysMemSlicePitch = 0;

		CComPtr<ID3D11Texture2D> pTexture;
		HRESULT createTexture2DRes = pIconLoader.mD3d11Device->CreateTexture2D(&desc, &subResource, &pTexture);
		if (!SUCCEEDED(createTexture2DRes)) {
			std::string text = "Error creating 2d texture: ";
			text.append(std::to_string(createTexture2DRes));
			throw std::runtime_error(text);
		}

		// Create texture view
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = mDxgiFormat;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = desc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;

		pIconLoader.mD3d11Device->CreateShaderResourceView(pTexture, &srvDesc, &mD11Texture);
		if (FAILED(createTexture2DRes)) {
			// error copying pixels to buffer
			std::string text = "Error creating shader Resource View: ";
			text.append(std::to_string(createTexture2DRes));
			throw std::runtime_error(text);
		}
	} else if (pIconLoader.mD3d9Device) {
		_D3DFORMAT d3Dformat = GetFormatDx9();

		// This is really dirty, but i don't know how to fix it properly :(
		// Also DX9 support will be dropped in the near future.
		if (d3Dformat == D3DFMT_A8B8G8R8) {
			// convert from abgr to argb
			uint8_t* point = mPixelBuffer.data();
			uint8_t val = 0;
			for (size_t i = 0; i < mPixelBuffer.size(); ++i) {
				val = point[0];
				point[0] = point[2];
				point[2] = val;
				point += 4;
			}

			d3Dformat = D3DFMT_A8R8G8B8;
		}

		HRESULT createTextureRes = pIconLoader.mD3d9Device->CreateTexture(Width, Height, 1, 0, d3Dformat, D3DPOOL_MANAGED, &mD9Texture, NULL);
		if (FAILED(createTextureRes)) {
			// error creating d3d9 texture
			std::string text = "Error creating d3d9 texture: ";
			text.append(std::to_string(createTextureRes));
			throw std::runtime_error(text);
		}

		D3DLOCKED_RECT rect;
		mD9Texture->LockRect(0, &rect, 0, D3DLOCK_DISCARD);
		unsigned char* dest = static_cast<unsigned char*>(rect.pBits);
		memcpy(dest, mPixelBuffer.data(), sizeof(uint8_t) * Width * Height * 4);
		mD9Texture->UnlockRect(0);
	}

	Status = Status::Finished;
}

void* IconLoader::Texture::GetTexture(IconLoader& pIconLoader) {
	if (Status == Status::Finished) {
		if (mD11Texture) {
			return mD11Texture;
		}
		return mD9Texture;
	}

	if (Status == Status::Loaded) {
		LoadIntoDirectXDevice(pIconLoader);
	}

	return nullptr;
}

void IconLoader::Texture::LoadTexture(IconLoader& pIconLoader, UINT pResourceId) {
	if (Status == Status::Pending) {
		mLoadTarget = pResourceId;
		Status = Status::PendingResource;
	}

	if (pIconLoader.mThreadMutex.try_lock()) {
		pIconLoader.mThreadCondition.notify_one();
		pIconLoader.mThreadMutex.unlock();
	}
}

void IconLoader::Texture::LoadTexture(IconLoader& pIconLoader, const std::filesystem::path& pFilePath) {
	if (Status == Status::Pending) {
		mLoadTarget = pFilePath;
		Status = Status::PendingFile;
	}

	if (pIconLoader.mThreadMutex.try_lock()) {
		pIconLoader.mThreadCondition.notify_one();
		pIconLoader.mThreadMutex.unlock();
	}
}

void IconLoader::Texture::Load(IconLoader& pIconLoader) {
	if (Status == Status::PendingResource) {
		LoadFromResource(pIconLoader);
		return;
	}
	if (Status == Status::PendingFile) {
		LoadFromFile();
		return;
	}
#ifndef ARCDPS_EXTENSION_NO_CPR
	if (Status == Status::PendingCpr) {
		LoadFromCpr();
	}
#endif
}

void IconLoader::Texture::LoadFromFile() {
	const auto& path = std::get<std::filesystem::path>(mLoadTarget);
	if (!exists(path)) {
		Status = Status::Error;
		return;
	}

	{
		ULONG_PTR contextToken;
		if (CoGetContextToken(&contextToken) == CO_E_NOTINITIALIZED) {
			HRESULT coInitializeResult = CoInitialize(NULL);
			if (FAILED(coInitializeResult)) {
				return;
			}
		}
	}

	CComPtr<IWICImagingFactory> pIWICFactory;
	// IWICImagingFactory* m_pIWICFactory = NULL;
	HRESULT createInstance = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pIWICFactory));
	if (FAILED(createInstance)) {
		return;
	}

	CComPtr<IWICBitmapDecoder> wicDecoder;
	HRESULT fromFilenameRes = pIWICFactory->CreateDecoderFromFilename(path.c_str(), NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &wicDecoder);
	if (FAILED(fromFilenameRes)) {
		return;
	}

	CComPtr<IWICBitmapFrameDecode> pIDecodeFrame;
	HRESULT getFrameRes = wicDecoder->GetFrame(0, &pIDecodeFrame);
	if (FAILED(getFrameRes)) {
		return;
	}

	LoadFrame(pIDecodeFrame, pIWICFactory);
}

void IconLoader::Texture::LoadFrame(const CComPtr<IWICBitmapFrameDecode>& pIDecodeFrame, const CComPtr<IWICImagingFactory>& pIWICFactory) {
	HRESULT getSizeRes = pIDecodeFrame->GetSize(&Width, &Height);
	if (FAILED(getSizeRes)) {
		return;
	}

	if (Width <= 0 || Height <= 0) {
		return;
	}

	// get pixel format (color-depth)
	WICPixelFormatGUID pixelFormat;
	HRESULT pixelFormatRes = pIDecodeFrame->GetPixelFormat(&pixelFormat);
	if (FAILED(pixelFormatRes)) {
		return;
	}

	// not auto cause intellisense is not able to deduct it...
	const std::ranges::iterator_t<decltype(WIC_CONVERT)>& convertFormat = std::ranges::find_if(WIC_CONVERT, [&pixelFormat](const auto& pPair) { return std::get<0>(pPair) == pixelFormat; });

	WICPixelFormatGUID targetFormat = pixelFormat;

	if (convertFormat != WIC_CONVERT.end()) {
		targetFormat = convertFormat->second;
	}

	CComPtr<IWICComponentInfo> pIComponentInfo;
	HRESULT componentInfoRes = pIWICFactory->CreateComponentInfo(targetFormat, &pIComponentInfo);
	if (FAILED(componentInfoRes)) {
		return;
	}

	WICComponentType componentType;
	HRESULT componentTypeRes = pIComponentInfo->GetComponentType(&componentType);
	if (FAILED(componentTypeRes)) {
		return;
	}

	if (componentType != WICPixelFormat) {
		return;
	}

	CComPtr<IWICPixelFormatInfo> pIPixelFormatInfo;
	HRESULT pixelFormatInfoRes = pIComponentInfo->QueryInterface(__uuidof(IWICPixelFormatInfo), reinterpret_cast<void**>(&pIPixelFormatInfo));
	if (FAILED(pixelFormatInfoRes)) {
		return;
	}

	UINT bitsPerPixel;
	HRESULT bitsPerPixelRes = pIPixelFormatInfo->GetBitsPerPixel(&bitsPerPixel);
	if (FAILED(bitsPerPixelRes)) {
		return;
	}

	// Allocate temporary memory for image
	size_t rowPitch = (Width * bitsPerPixel + 7) / 8;
	size_t imageSize = rowPitch * Height;

	mPixelBuffer.resize(imageSize);

	if (convertFormat == WIC_CONVERT.end()) {
		// no conversion needed, just copy it
		HRESULT copyPixelsRes = pIDecodeFrame->CopyPixels(NULL, static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize), mPixelBuffer.data());
		if (FAILED(copyPixelsRes)) {
			return;
		}
	} else {
		// convert it
		CComPtr<IWICFormatConverter> formatConverter;
		HRESULT formatConverterRes = pIWICFactory->CreateFormatConverter(&formatConverter);
		if (FAILED(formatConverterRes)) {
			return;
		}

		HRESULT initConverterRes = formatConverter->Initialize(pIDecodeFrame, targetFormat, WICBitmapDitherTypeErrorDiffusion, 0, 0, WICBitmapPaletteTypeCustom);
		if (FAILED(initConverterRes)) {
			return;
		}

		HRESULT copyPixelsRes = formatConverter->CopyPixels(NULL, static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize), mPixelBuffer.data());
		if (FAILED(copyPixelsRes)) {
			return;
		}
	}

	mDxgiFormat = GetFormatDx11(targetFormat);

	Status = Status::Loaded;
}

void IconLoader::Texture::LoadFromResource(IconLoader& pIconLoader) {
	HRSRC imageResHandle = FindResource(pIconLoader.mDll, MAKEINTRESOURCE(std::get<UINT>(mLoadTarget)), L"PNG");
	if (!imageResHandle) {
		return;
	}

	// does not need to be freed
	HGLOBAL imageResDataHandle = LoadResource(pIconLoader.mDll, imageResHandle);
	if (!imageResDataHandle) {
		return;
	}

	LPVOID imageFile = LockResource(imageResDataHandle);
	if (!imageFile) {
		return;
	}

	DWORD imageFileSize = SizeofResource(pIconLoader.mDll, imageResHandle);
	if (!imageFileSize) {
		return;
	}

	{
		ULONG_PTR contextToken;
		if (CoGetContextToken(&contextToken) == CO_E_NOTINITIALIZED) {
			HRESULT coInitializeResult = CoInitialize(NULL);
			if (FAILED(coInitializeResult)) {
				return;
			}
		}
	}

	CComPtr<IWICImagingFactory> pIWICFactory;
	// IWICImagingFactory* m_pIWICFactory = NULL;
	HRESULT createInstance = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pIWICFactory));
	if (FAILED(createInstance)) {
		return;
	}

	CComPtr<IWICStream> pIWICStream;
	HRESULT streamRes = pIWICFactory->CreateStream(&pIWICStream);
	if (FAILED(streamRes)) {
		return;
	}

	HRESULT initializeFromMemoryRes = pIWICStream->InitializeFromMemory(reinterpret_cast<BYTE*>(imageFile), imageFileSize);
	if (FAILED(initializeFromMemoryRes)) {
		return;
	}

	CComPtr<IWICBitmapDecoder> pIDecoder;
	HRESULT decoderFromStreamRes = pIWICFactory->CreateDecoderFromStream(pIWICStream, NULL, WICDecodeMetadataCacheOnLoad, &pIDecoder);
	if (FAILED(decoderFromStreamRes)) {
		return;
	}

	CComPtr<IWICBitmapFrameDecode> pIDecodeFrame;
	HRESULT getFrameRes = pIDecoder->GetFrame(0, &pIDecodeFrame);
	if (FAILED(getFrameRes)) {
		return;
	}

	LoadFrame(pIDecodeFrame, pIWICFactory);
}

void IconLoader::LoadThreadFunc(std::stop_token stop_token) {
	// lock the mutex, so `wait` can unlock it.
	mThreadMutex.lock();

	while (true) {
		mThreadCondition.wait(mThreadMutex, stop_token, [this] { return !mLoadIds.empty(); });

		if (stop_token.stop_requested()) break;

		mTexturesMutex.lock();

		size_t id = mLoadIds.front();
		mLoadIds.pop_front();
		// copy the texture object, so we can run things asnyc and don't care when the vector changes.
		Texture texture = mTextures[id];

		mTexturesMutex.unlock();

		texture.Load(*this);

		mTexturesMutex.lock();

		mTextures[id] = std::move(texture);

		mTexturesMutex.unlock();
	}

	// unlock the thread mutex, when the thread is stopped
	mThreadMutex.unlock();
}

#ifndef ARCDPS_EXTENSION_NO_CPR
size_t IconLoader::LoadTexture(const cpr::Url& pUrl) {
	assert(mD3d9Device || mD3d11Device);

	std::lock_guard guard(mTexturesMutex);

	auto& texture = mTextures.emplace_back();

	texture.LoadTexture(*this, pUrl);

	size_t id = mTextures.size() - 1;

	mLoadIds.push_back(id);

	return id;
}

void IconLoader::Texture::LoadTexture(IconLoader& pIconLoader, const cpr::Url& pUrl) {
	if (Status == Status::Pending) {
		mLoadTarget = pUrl;
		Status = Status::PendingCpr;
	}

	if (pIconLoader.mThreadMutex.try_lock()) {
		pIconLoader.mThreadCondition.notify_one();
		pIconLoader.mThreadMutex.unlock();
	}
}

void IconLoader::Texture::LoadFromCpr() {
	auto& url = std::get<cpr::Url>(mLoadTarget);
	std::string urlString = url.str();

	size_t size = urlString.find("//");
	urlString = urlString.replace(0, size + 2, "");
	std::replace(urlString.begin(), urlString.end(), '/', '\\');

	auto filePath = std::filesystem::temp_directory_path();
	filePath.append("GW2-arcdps-extension");

	// create needed subdirs
	filePath.append(urlString);

	if (exists(filePath)) {
		mLoadTarget = filePath;
		LoadFromFile();
		return;
	}

	auto dirPath = filePath;
	dirPath.remove_filename();

	std::error_code err;
	std::filesystem::create_directories(dirPath, err);
	if (err) {
		Status = Status::Error;
		return;
	}

	std::ofstream output(filePath, std::ios::binary);

	cpr::Response response = cpr::Download(output, url);
	if (response.status_code != 200) {
		Status = Status::Error;
		return;
	}

	output.close();
	if (output.fail()) {
		Status = Status::Error;
		return;
	}

	mLoadTarget = filePath;
	LoadFromFile();
}

#endif
