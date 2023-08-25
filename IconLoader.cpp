#include "IconLoader.h"
#include "SimpleNetworkStack.h"

#include <utility>

namespace {
	/*
 * Array of needed GUID conversions.
 * CurrentGUID -> WantedGUID
 */
	const std::vector<std::pair<GUID, GUID>> WIC_CONVERT = {
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
} // namespace

IconLoader::IconLoader() {
	mThread = std::move(std::jthread([this](std::stop_token pToken) {
		runner(std::move(pToken));
	}));
}

IconLoader::~IconLoader() {
	mThread.request_stop();
	mThread.join();
}

void IconLoader::Setup(HMODULE pDll, ID3D11Device* pD11Device) {
	mDll = pDll;
	mD11Device = pD11Device;
}

void IconLoader::runner(std::stop_token pToken) {
	while (true) {
		std::unique_lock lock(mThreadMutex);
		mThreadVariable.wait(lock, pToken, [this] {
			return !mThreadQueue.empty();
		});

		if (pToken.stop_requested()) {
			break;
		}

		auto element = std::move(mThreadQueue.front());
		mThreadQueue.pop();

		lock.unlock();

		// do things with the Icon.
		element.Load();
	}
}

void IconLoader::queueLoad(const QueueIcon& pIcon) {
	std::lock_guard guard(mLoadQueueMutex);

	mLoadQueue.try_emplace(pIcon.mName, pIcon);
}

void IconLoader::loadDone(const IconLoader::QueueIcon& pIcon, ID3D11ShaderResourceView* pTexture) {
	mIcons.try_emplace(pIcon.mName, pIcon.mWidth, pIcon.mHeight, pTexture);
}

void IconLoader::QueueIcon::Load() {
	switch (mWay) {
		case LoadWay::File:
			LoadFile(std::get<std::filesystem::path>(mResource));
			break;
		case LoadWay::Url:
			LoadUrl(std::get<std::string>(mResource));
			break;
		case LoadWay::Gw2Dat:
			LoadGw2Dat(std::get<std::string>(mResource));
			break;
		case LoadWay::Resource:
			LoadResource(std::get<UINT>(mResource));
			break;
	}
}

void IconLoader::QueueIcon::LoadFile(const std::filesystem::path& pFilepath) {
	if (!exists(pFilepath)) {
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
	HRESULT createInstance = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pIWICFactory));
	if (FAILED(createInstance)) {
		return;
	}

	CComPtr<IWICBitmapDecoder> wicDecoder;
	HRESULT fromFilenameRes = pIWICFactory->CreateDecoderFromFilename(pFilepath.c_str(), NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &wicDecoder);
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

void IconLoader::QueueIcon::LoadUrl(const std::string& pUrl) {
	// build local path to cache
	std::filesystem::path filepath = pUrl;
	size_t size = pUrl.find("//");
	if (size > 0) {
		filepath = pUrl.substr(size + 2);
	}

	auto fullPath = std::filesystem::temp_directory_path();
	fullPath /= "GW2-arcdps-extension";
	fullPath /= filepath;

	// if file is already downloaded -> load it.
	if (exists(fullPath)) {
		LoadFile(fullPath);
		return;
	}

	// create all folders before starting the download
	auto folderPath = fullPath;
	folderPath.remove_filename();
	std::filesystem::create_directories(folderPath);

	// file doesn't exist -> download it
	auto& networkStack = SimpleNetworkStack::instance();

	networkStack.QueueGet(
			pUrl,
			[*this, fullPath](const SimpleNetworkStack::Result& pResult) mutable {
				if (pResult.has_value() && pResult.value().Code == 200) {
					// this is run in the NetworkStack thread, we want to further load in the IconLoader thread, so we register the file as new again.
					mIconLoader.RegisterFile(mName, fullPath);
					return;
				}
			},
			fullPath
	);
}

void IconLoader::QueueIcon::LoadGw2Dat(const std::string& pId) {
	// build http link to download from gw2dat
	std::string url = std::format("https://assets.gw2dat.com/{}.png", pId);

	LoadUrl(url);
}

void IconLoader::QueueIcon::LoadResource(UINT pId) {
	HRSRC imageResHandle = FindResource(mIconLoader.mDll, MAKEINTRESOURCE(pId), L"PNG");
	if (!imageResHandle) {
		return;
	}

	// does not need to be freed
	HGLOBAL imageResDataHandle = ::LoadResource(mIconLoader.mDll, imageResHandle);
	if (!imageResDataHandle) {
		return;
	}

	LPVOID imageFile = LockResource(imageResDataHandle);
	if (!imageFile) {
		return;
	}

	DWORD imageFileSize = SizeofResource(mIconLoader.mDll, imageResHandle);
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

void IconLoader::QueueIcon::LoadFrame(const CComPtr<IWICBitmapFrameDecode>& pIDecodeFrame, const CComPtr<IWICImagingFactory>& pIWICFactory) {
	HRESULT getSizeRes = pIDecodeFrame->GetSize(&mWidth, &mHeight);
	if (FAILED(getSizeRes)) {
		return;
	}

	if (mWidth <= 0 || mHeight <= 0) {
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
	size_t rowPitch = (mWidth * bitsPerPixel + 7) / 8;
	size_t imageSize = rowPitch * mHeight;

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

	mIconLoader.queueLoad(*this);
}

void IconLoader::QueueIcon::DeviceLoad() {
	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof desc);
	desc.Width = mWidth;
	desc.Height = mHeight;
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
	HRESULT createTexture2DRes = mIconLoader.mD11Device->CreateTexture2D(&desc, &subResource, &pTexture);
	if (!SUCCEEDED(createTexture2DRes)) {
		//		std::string text = "Error creating 2d texture: ";
		//		text.append(std::to_string(createTexture2DRes));
		//		throw std::runtime_error(text);
		return;
	}

	// Create texture view
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = mDxgiFormat;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = desc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;

	ID3D11ShaderResourceView* d11Texture;
	mIconLoader.mD11Device->CreateShaderResourceView(pTexture, &srvDesc, &d11Texture);
	if (FAILED(createTexture2DRes)) {
		// error copying pixels to buffer
		//		std::string text = "Error creating shader mResource View: ";
		//		text.append(std::to_string(createTexture2DRes));
		//		throw std::runtime_error(text);
		return;
	}

	mIconLoader.loadDone(*this, d11Texture);
}

DXGI_FORMAT IconLoader::QueueIcon::GetFormatDx11(WICPixelFormatGUID pPixelFormat) {
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
