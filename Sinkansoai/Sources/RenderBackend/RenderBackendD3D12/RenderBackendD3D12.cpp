#include "../../Windows.h"
#include "../../Misc/Geometry.h"
#include "../../Windows.h"
#include "../../Module/Mesh/MeshBuilder.h"
#include "../../Module/Texture/TextureBuilder.h"
#include "../../Module/ShaderCompiler/ShaderCompiler.h"
#include "../../SceneSelection.h"

#include "RenderBackendD3D12.h"
#include "../../Render/SceneTextures.h"
#include "../../Render/MaterialRegistry.h"

#include <string>

#pragma comment ( lib, "d3d12.lib")
#pragma comment ( lib, "dxgi.lib")

struct GPUQueueTimer
{
	std::chrono::time_point<std::chrono::high_resolution_clock> CurrentTime{};
	std::chrono::time_point<std::chrono::high_resolution_clock> PreviousTime{};
	std::chrono::duration<float> DeltaTime{};
};

GPUQueueTimer QueueTimer;

static String GetLatestWinPixGpuCapturerPath()
{
	LPWSTR programFilesPath = nullptr;
	SHGetKnownFolderPath(FOLDERID_ProgramFiles, KF_FLAG_DEFAULT, NULL, &programFilesPath);

	String pixSearchPath = programFilesPath + std::wstring(L"\\Microsoft PIX\\*");

	WIN32_FIND_DATA findData;
	bool foundPixInstallation = false;
	wchar_t newestVersionFound[MAX_PATH];

	HANDLE hFind = FindFirstFile(pixSearchPath.c_str(), &findData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) &&
				(findData.cFileName[0] != '.'))
			{
				if (!foundPixInstallation || wcscmp(newestVersionFound, findData.cFileName) <= 0)
				{
					foundPixInstallation = true;
					StringCchCopy(newestVersionFound, _countof(newestVersionFound), findData.cFileName);
				}
			}
		} while (FindNextFile(hFind, &findData) != 0);
	}

	FindClose(hFind);

	if (!foundPixInstallation)
	{
		// TODO: Error, no PIX installation found
	}

	wchar_t output[MAX_PATH];
	StringCchCopy(output, pixSearchPath.length(), pixSearchPath.data());
	StringCchCat(output, MAX_PATH, &newestVersionFound[0]);
	StringCchCat(output, MAX_PATH, L"\\WinPixGpuCapturer.dll");

	return &output[0];
}


void RRenderBackendD3D12::InitSceneTextures(RSceneTextures& SceneTextures)
{
	if (bSceneTexturesInitialized)
	{
		return;
	}

	if (!Device || !SceneTextureRTVHeap || !DSVHeap || !CBVSRVHeap)
	{
		return;
	}
	if (SceneTextureCPUHandle.ptr == 0)
	{
		return;
	}

	const uint32 Width = MWindow::Get().GetWidth();
	const uint32 Height = MWindow::Get().GetHeight();

	auto MakeSceneTexture = [&](const wchar_t* Name, DXGI_FORMAT Format, EResourceFlag Flag)
	{
		return SharedPtr<RTextureD3D12>(new RTextureD3D12(*this, Name, Width, Height, 1, Format, Flag, EResourceType::RenderTexture2D));
	};

	SceneTextures.SceneDepth = MakeSceneTexture(TEXT("MainDepthStencil"), DXGI_FORMAT_R32G8X24_TYPELESS, EResourceFlag::DepthStencilTarget);
	SceneTextures.SceneDepth->AllocateResource();
	CastAsD3D12<RTextureD3D12>(SceneTextures.SceneDepth.get())->SetSRVFormat(DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS);

	SceneTextures.SceneColor = MakeSceneTexture(TEXT("SceneColor"), DXGI_FORMAT_R8G8B8A8_UNORM, EResourceFlag::RenderTarget);
	SceneTextures.SceneColor->AllocateResource();

	SceneTextures.BaseColor = MakeSceneTexture(TEXT("BaseColor"), DXGI_FORMAT_R8G8B8A8_UNORM, EResourceFlag::RenderTarget);
	SceneTextures.BaseColor->AllocateResource();

	SceneTextures.WorldNormal = MakeSceneTexture(TEXT("WorldNormal"), DXGI_FORMAT_R8G8B8A8_UNORM, EResourceFlag::RenderTarget);
	SceneTextures.WorldNormal->AllocateResource();

	SceneTextures.Material = MakeSceneTexture(TEXT("Material"), DXGI_FORMAT_R32G32B32A32_FLOAT, EResourceFlag::RenderTarget);
	SceneTextures.Material->AllocateResource();

	SceneTextures.DebugTexture = MakeSceneTexture(TEXT("DebugTexture"), DXGI_FORMAT_R32G32B32A32_FLOAT, EResourceFlag::RenderTarget);
	SceneTextures.DebugTexture->AllocateResource();

	// DSV
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC DepthStencilViewDesc{};
		DepthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		DepthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;
		DepthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		Device->CreateDepthStencilView(ToD3D12Resource(SceneTextures.SceneDepth.get()), &DepthStencilViewDesc, DSVHeap->GetCPUDescriptorHandleForHeapStart());
	}

	// RTVs
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE RTVHandle(SceneTextureRTVHeap->GetCPUDescriptorHandleForHeapStart());
		CastAsD3D12<RTextureD3D12>(SceneTextures.SceneColor.get())->CreateRTV(RTVHandle);
		RTVHandle.Offset(1, RTVDescriptorSize);

		CastAsD3D12<RTextureD3D12>(SceneTextures.BaseColor.get())->CreateRTV(RTVHandle);
		RTVHandle.Offset(1, RTVDescriptorSize);

		CastAsD3D12<RTextureD3D12>(SceneTextures.WorldNormal.get())->CreateRTV(RTVHandle);
		RTVHandle.Offset(1, RTVDescriptorSize);

		CastAsD3D12<RTextureD3D12>(SceneTextures.Material.get())->CreateRTV(RTVHandle);
		RTVHandle.Offset(1, RTVDescriptorSize);

		CastAsD3D12<RTextureD3D12>(SceneTextures.DebugTexture.get())->CreateRTV(RTVHandle);
	}

	// SRVs
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE SRVHandle(SceneTextureCPUHandle);

		auto CreateSceneSRV = [&](RTexture* Texture)
		{
			auto* D3D12Texture = CastAsD3D12<RTextureD3D12>(Texture);
			Device->CreateShaderResourceView(ToD3D12Resource(Texture), &D3D12Texture->GetSRVDesc(), SRVHandle);
			SRVHandle.Offset(1, CBVSRVUAVDescriptorSize);
		};

		CreateSceneSRV(SceneTextures.SceneDepth.get());
		CreateSceneSRV(SceneTextures.SceneColor.get());
		CreateSceneSRV(SceneTextures.BaseColor.get());
		CreateSceneSRV(SceneTextures.WorldNormal.get());
		CreateSceneSRV(SceneTextures.Material.get());
	}

	// Initialize resource states to match renderer expectations.
	{
		ResourceBarrier Barriers[] =
		{
			MakeTransitionBarrier(SceneTextures.SceneDepth.get(), ResourceState::Common, ResourceState::DepthRead),
			MakeTransitionBarrier(SceneTextures.SceneColor.get(), ResourceState::Common, ResourceState::GenericRead),
			MakeTransitionBarrier(SceneTextures.BaseColor.get(), ResourceState::Common, ResourceState::GenericRead),
			MakeTransitionBarrier(SceneTextures.WorldNormal.get(), ResourceState::Common, ResourceState::GenericRead),
			MakeTransitionBarrier(SceneTextures.Material.get(), ResourceState::Common, ResourceState::GenericRead),
			MakeTransitionBarrier(SceneTextures.DebugTexture.get(), ResourceState::Common, ResourceState::RenderTarget),
		};
		GraphicsCommandList.SumbitResourceBarriers(_countof(Barriers), Barriers);
	}

	bSceneTexturesInitialized = true;
}

RRenderBackendD3D12::RRenderBackendD3D12()
	: DynamicBuffers{ RDynamicBufferD3D12(*this, TEXT("Global Buffer0")) , RDynamicBufferD3D12(*this, TEXT("Global Buffer1")) }
{
	SetBackendName(TEXT("D3D12"));
}

void RRenderBackendD3D12::Init()
{
	QueueTimer.CurrentTime = std::chrono::high_resolution_clock::now();
	QueueTimer.PreviousTime = std::chrono::high_resolution_clock::now();

#if USE_PIX
	// Check to see if a copy of WinPixGpuCapturer.dll has already been injected into the application.
	// This may happen if the application is launched through the PIX UI. 
	if (GetModuleHandle(L"WinPixGpuCapturer.dll") == 0)
	{
		LoadLibrary(GetLatestWinPixGpuCapturerPath().c_str());
	}
#endif

	UINT DxgiFactoryFlags = 0;
#if defined(_DEBUG)
	TRefCountPtr<ID3D12Debug> DebugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController))))
	{
		DebugController->EnableDebugLayer();
		DxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}
#endif

	TRefCountPtr<IDXGIFactory4> Factory{};
	ThrowIfFailed(CreateDXGIFactory2(DxgiFactoryFlags, IID_PPV_ARGS(&Factory)));


	HRESULT HR = D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), (void**)&Device);
	if (HR != S_OK)
	{
		ThrowIfFailed(HR);
		cout << "D3D12 Renderbackend Device Init false" << endl;
	}
	else
	{
		// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
		D3D12_FEATURE_DATA_ROOT_SIGNATURE FeatureData;
		bSupportsRootSignatureVersion1_1 = true;
		if (FAILED(Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &FeatureData, sizeof(FeatureData))))
		{
			bSupportsRootSignatureVersion1_1 = false;
		}


		RTVDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		DSVDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		CBVSRVUAVDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		GraphicsCommandList.AllocateCommandList(*this);
		ComputeCommandList.AllocateCommandList(*this);
		CopyCommandList.AllocateCommandList(*this);

		MainGraphicsCommandList = &GraphicsCommandList;
		MainComputeCommandList = &ComputeCommandList;
		MainCopyCommandList = &CopyCommandList;

		GraphicsCommandList.Reset();

		cout << "D3D12 Renderbackend Device Init Success" << endl;

		{
			// Describe and create the command queue.
			D3D12_COMMAND_QUEUE_DESC QueueDesc = {};
			QueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
			QueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

			ThrowIfFailed(Device->CreateCommandQueue(&QueueDesc, IID_PPV_ARGS(&CommandQueue)));
			cout << "D3D12 Queue Init Success" << endl;
		}

		{   

			Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, static_cast<float>(MWindow::Get().GetWidth()), static_cast<float>(MWindow::Get().GetHeight()));
			ScissorRect = CD3DX12_RECT(0, 0, static_cast<LONG>(MWindow::Get().GetWidth()), static_cast<LONG>(MWindow::Get().GetHeight()));

			// Describe and create the swap chain.
			DXGI_SWAP_CHAIN_DESC1 SwapChainDesc = {};
			SwapChainDesc.BufferCount = NumBackBuffers;
			SwapChainDesc.Width = MWindow::Get().GetWidth();
			SwapChainDesc.Height = MWindow::Get().GetHeight();

			SwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
			SwapChainDesc.SampleDesc.Count = 1;

			ComPtr<IDXGISwapChain1> SwapChain1;
			ThrowIfFailed(Factory->CreateSwapChainForHwnd(
				CommandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
				MWindow::Get().GetHWND(),
				&SwapChainDesc,
				nullptr,
				nullptr,
				&SwapChain1
			));


			// No alt enter
			Factory->MakeWindowAssociation(MWindow::Get().GetHWND(), DXGI_MWA_NO_ALT_ENTER);

			SwapChain1.As(&SwapChain);

			FrameIndex = SwapChain->GetCurrentBackBufferIndex();

			cout << "Swap chain init success" << " SwapChain First Backbuffer Index : " << FrameIndex << endl;
		}

		{
			vector<Material> LightVolumeMaterials;
			auto LightVolumeMeshes = MMeshBuilder::Get().LoadMesh(TEXT("C:/Users/dnjfd/Desktop/Collection/RyoikiTenaki/Sinkansoai/Resources/LightVolume/"), TEXT("Sphere.obj"), LightVolumeMaterials);
			if (!LightVolumeMeshes.empty())
			{
				LightVolumeMesh.InitResources(LightVolumeMeshes[0],
					[&](RVertexBuffer*& PositionVB, vector<RVertexBuffer*>& UVVBs, RVertexBuffer*& NormalVB, RVertexBuffer*& TangetVB, RVertexBuffer*& BitangetVB, RIndexBuffer*& IB, uint32 NumUVChannels)
					{
						PositionVB = new RVertexBufferD3D12(*this, TEXT("PositionVertexBuffer"));
						UVVBs.resize(NumUVChannels);
						for (uint32 ChannelIndex = 0; ChannelIndex < NumUVChannels; ++ChannelIndex)
						{
							UVVBs[ChannelIndex] = new RVertexBufferD3D12(*this, TEXT("UVVertexBuffer"));
						}
						NormalVB = new RVertexBufferD3D12(*this, TEXT("NormalVertexBuffer"));
						TangetVB = new RVertexBufferD3D12(*this, TEXT("TangentVertexBuffer"));
						BitangetVB = new RVertexBufferD3D12(*this, TEXT("BitangentVertexBuffer"));
						IB = new RIndexBufferD3D12(*this, TEXT("IndexBuffer"));
					});
			}

		}

		auto DepthFormat = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

		// Create testing resources
		{
			const auto& SelectedScene = GetSelectedSceneAsset();
			vector<MMesh> LoadedMeshes;
			vector<Material> LoadedMaterials;
			if (SelectedScene.Type == ESceneAssetType::Gltf)
			{
				LoadedMeshes = MMeshBuilder::Get().LoadMeshGLTF(SelectedScene.RootPath, SelectedScene.FileName, LoadedMaterials);
			}
			else if (SelectedScene.Type == ESceneAssetType::Fbx)
			{
				LoadedMeshes = MMeshBuilder::Get().LoadMeshFBX(SelectedScene.RootPath, SelectedScene.FileName, LoadedMaterials);
			}
			else
			{
				LoadedMeshes = MMeshBuilder::Get().LoadMesh(SelectedScene.RootPath, SelectedScene.FileName, LoadedMaterials);
			}


			for (auto& DynamicBuffer : DynamicBuffers)
			{
				DynamicBuffer.Allocate(D3D12MaxConstantBufferSize);
			}

			// Default Texture
			{
				DefaultTexture = SharedPtr<RTextureD3D12>(new RTextureD3D12(*this, TEXT("CheckerTexture"), 256, 256, 1, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, EResourceFlag::None));
				DefaultTexture->AllocateResource();

				auto RawTexture = MTextureBuilder::Get().GenerateDefaultTexture(256, 256, 4);
				DefaultTexture->StreamTexture(RawTexture.data());
			}

			// Default Black
			{
				DefaultBlackTexture = SharedPtr<RTextureD3D12>(new RTextureD3D12(*this, TEXT("BlackDummy"), 1, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, EResourceFlag::None));
				DefaultBlackTexture->AllocateResource();

				vector<uint8> BlackDummy;
				BlackDummy.resize(4);
				DefaultBlackTexture->StreamTexture(BlackDummy.data());
			}
			// Default White
			{
				DefaultWhiteTexture = SharedPtr<RTextureD3D12>(new RTextureD3D12(*this, TEXT("WhiteDummy"), 1, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, EResourceFlag::None));
				DefaultWhiteTexture->AllocateResource();

				vector<uint8> WhiteDummy = { 255, 255, 255, 255 };
				DefaultWhiteTexture->StreamTexture(WhiteDummy.data());
			}
			// Default MetallicRoughness (Metallic=0, Roughness=1)
			{
				DefaultMetalRoughTexture = SharedPtr<RTextureD3D12>(new RTextureD3D12(*this, TEXT("DefaultMetalRough"), 1, 1, 1, DXGI_FORMAT_R8G8B8A8_UNORM, EResourceFlag::None));
				DefaultMetalRoughTexture->AllocateResource();

				vector<uint8> MetalRoughDummy = { 0, 255, 0, 255 };
				DefaultMetalRoughTexture->StreamTexture(MetalRoughDummy.data());
			}
			cout << " Test resource creation done " << endl;


			// TO DO : Move MaterialRegistry to the world and separate texture generation by using texture handle
			// MaterialRegistry must have texture handle and the actual creation of texture should be done other place not in here.
			auto& Registry = MaterialRegistry::Get();
			Registry.Reset();

			if (LoadedMaterials.empty())
			{
				LoadedMaterials.emplace_back();
			}

			auto CreateTextureResource = [&](TextureAsset& RawTexture) -> SharedPtr<TextureResource>
			{
				const DXGI_FORMAT Format = RawTexture.bSRGB ? DXGI_FORMAT_B8G8R8A8_UNORM_SRGB : DXGI_FORMAT_B8G8R8A8_UNORM;
				auto Resource = SharedPtr<RTextureD3D12>(new RTextureD3D12(*this, RawTexture.Name, RawTexture.Width, RawTexture.Height, 1, Format, EResourceFlag::None));
				Resource->AllocateResource();
				if (!RawTexture.Pixels.empty())
				{
					Resource->StreamTexture(reinterpret_cast<void*>(RawTexture.Pixels.data()));
				}
				return Resource;
			};

			const auto MaterialIds = Registry.RegisterMaterials(LoadedMaterials, CreateTextureResource, DefaultTexture, DefaultBlackTexture, DefaultWhiteTexture, DefaultMetalRoughTexture);
			auto MapMaterialId = [&](uint32 LocalId) -> MaterialId
			{
				if (LocalId < MaterialIds.size())
				{
					return MaterialIds[LocalId];
				}
				return MaterialIds.empty() ? 0u : MaterialIds[0];
			};

			for (auto& Mesh : LoadedMeshes)
			{
				for (auto& Section : Mesh.Sections)
				{
					Section.MaterialId = MapMaterialId(Section.MaterialId);
				}
			}

			RenderMeshes.clear();
			RenderMeshes.resize(LoadedMeshes.size());
			for (size_t MeshIndex = 0; MeshIndex < LoadedMeshes.size(); ++MeshIndex)
			{
				auto& Mesh = LoadedMeshes[MeshIndex];
				auto& RenderMesh = RenderMeshes[MeshIndex];
				RenderMesh.InitResources(Mesh,
					[&](RVertexBuffer*& PositionVB, vector<RVertexBuffer*>& UVVBs, RVertexBuffer*& NormalVB, RVertexBuffer*& TangetVB, RVertexBuffer*& BitangetVB, RIndexBuffer*& IB, uint32 NumUVChannels)
					{
						PositionVB = new RVertexBufferD3D12(*this, TEXT("PositionVertexBuffer"));
						UVVBs.resize(NumUVChannels);
						for (uint32 ChannelIndex = 0; ChannelIndex < NumUVChannels; ++ChannelIndex)
						{
							UVVBs[ChannelIndex] = new RVertexBufferD3D12(*this, TEXT("UVVertexBuffer"));
						}
						NormalVB = new RVertexBufferD3D12(*this, TEXT("NormalVertexBuffer"));
						TangetVB = new RVertexBufferD3D12(*this, TEXT("TangentVertexBuffer"));
						BitangetVB = new RVertexBufferD3D12(*this, TEXT("BitangentVertexBuffer"));
						IB = new RIndexBufferD3D12(*this, TEXT("IndexBuffer"));
					});
			}
		}


		// Create descriptor heaps.
		{
			// For swap chain RTV
			D3D12_DESCRIPTOR_HEAP_DESC RTVHeapDesc = {};
			RTVHeapDesc.NumDescriptors = NumBackBuffers;
			RTVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			RTVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			ThrowIfFailed(Device->CreateDescriptorHeap(&RTVHeapDesc, IID_PPV_ARGS(&RTVHeap)));
			cout << "Descriptor rtv heap creation success" << endl;


			D3D12_DESCRIPTOR_HEAP_DESC DSVHeapDesc = {};
			DSVHeapDesc.NumDescriptors = 1;
			DSVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			DSVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			ThrowIfFailed(Device->CreateDescriptorHeap(&DSVHeapDesc, IID_PPV_ARGS(&DSVHeap)));
			cout << "Descriptor dsv heap creation success" << endl;

			{
				D3D12_DESCRIPTOR_HEAP_DESC SceneTextureRTVHeapDesc = {};
				SceneTextureRTVHeapDesc.NumDescriptors = 5;
				SceneTextureRTVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
				SceneTextureRTVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
				ThrowIfFailed(Device->CreateDescriptorHeap(&SceneTextureRTVHeapDesc, IID_PPV_ARGS(&SceneTextureRTVHeap)));
				cout << "Descriptor rtv heap creation success" << endl;
			}


			const uint32 NumConstantBuffer = _countof(DynamicBuffers);
			const uint32 NumSceneTextures = 5;
			const auto& Registry = MaterialRegistry::Get();
			const uint32 NumMaterialTextures = (std::max)(Registry.GetTotalTextureCount(), kMaterialTextureSlotCount);
			const uint32 NumTextures = NumMaterialTextures + 1 + NumSceneTextures; // + 1 ( Default Texture )

			D3D12_DESCRIPTOR_HEAP_DESC CbvHeapDesc = {};
			CbvHeapDesc.NumDescriptors = NumConstantBuffer + NumTextures;
			CbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			CbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			ThrowIfFailed(Device->CreateDescriptorHeap(&CbvHeapDesc, IID_PPV_ARGS(&CBVSRVHeap)));
			cout << "Descriptor cbv heap creation success" << endl;


			{
				CD3DX12_CPU_DESCRIPTOR_HANDLE CBVSRVUAVCPUHandle(CBVSRVHeap->GetCPUDescriptorHandleForHeapStart());
				CD3DX12_GPU_DESCRIPTOR_HANDLE CBVSRVUAVGPUHandle(CBVSRVHeap->GetGPUDescriptorHandleForHeapStart());

				// Refactor belows terrible code boiler plate.

				AddressCacheForDescriptorHeapStart[ToDescriptorIndex(EDescriptorHeapAddressSpace::ConstantBufferView)] = CBVSRVUAVGPUHandle;

				for (auto& DynamicBuffer : DynamicBuffers)
				{
					auto CBVDesc = DynamicBuffer.GetCBVDesc();
					Device->CreateConstantBufferView(&CBVDesc, CBVSRVUAVCPUHandle);
					CBVSRVUAVCPUHandle.Offset(CBVSRVUAVDescriptorSize);
					CBVSRVUAVGPUHandle.Offset(CBVSRVUAVDescriptorSize);

					++NumRegisteredHeaps[ToDescriptorIndex(EDescriptorHeapAddressSpace::ConstantBufferView)];
				}
				
				AddressCacheForDescriptorHeapStart[ToDescriptorIndex(EDescriptorHeapAddressSpace::ShaderResourceView)] = CBVSRVUAVGPUHandle;
				{
					Device->CreateShaderResourceView(ToD3D12Resource(DefaultTexture.get()), &DefaultTexture->GetSRVDesc(), CBVSRVUAVCPUHandle);
					CBVSRVUAVCPUHandle.Offset(CBVSRVUAVDescriptorSize);
					CBVSRVUAVGPUHandle.Offset(CBVSRVUAVDescriptorSize);

					++NumRegisteredHeaps[ToDescriptorIndex(EDescriptorHeapAddressSpace::ShaderResourceView)];
				}
				
				for (const auto& Material : Registry.GetMaterials())
				{
					for (const auto& Texture : Material.TexturesGPU)
					{
						auto* D3D12Texture = CastAsD3D12<RTextureD3D12>(Texture.get());
						Device->CreateShaderResourceView(ToD3D12Resource(D3D12Texture), &D3D12Texture->GetSRVDesc(), CBVSRVUAVCPUHandle);
						CBVSRVUAVCPUHandle.Offset(CBVSRVUAVDescriptorSize);
						CBVSRVUAVGPUHandle.Offset(CBVSRVUAVDescriptorSize);

						++NumRegisteredHeaps[ToDescriptorIndex(EDescriptorHeapAddressSpace::ShaderResourceView)];
					}
				}

				// Scene textures (reserved; descriptors filled on InitSceneTextures)
				{
					SceneTextureGPUHandle = CBVSRVUAVGPUHandle;
					SceneTextureCPUHandle = CBVSRVUAVCPUHandle;
					CBVSRVUAVCPUHandle.Offset(NumSceneTextures, CBVSRVUAVDescriptorSize);
					CBVSRVUAVGPUHandle.Offset(NumSceneTextures, CBVSRVUAVDescriptorSize);

					NumRegisteredHeaps[ToDescriptorIndex(EDescriptorHeapAddressSpace::ShaderResourceView)] += NumSceneTextures;
				}

				AddressCacheForDescriptorHeapStart[ToDescriptorIndex(EDescriptorHeapAddressSpace::UnorderedAccessView)] = CBVSRVUAVGPUHandle;
				{
					// No UAV to register
				}

				// Refactor end

				cout << " Global Heap Cache Generation Done " << endl;
			}
		}

		// Create frame resources.
		{
			CD3DX12_CPU_DESCRIPTOR_HANDLE RTVHandle(RTVHeap->GetCPUDescriptorHandleForHeapStart());

			// Create a RTV for each frame.
			for (UINT i = 0; i < NumBackBuffers; i++)
			{
				ThrowIfFailed(SwapChain->GetBuffer(i, IID_PPV_ARGS(&RenderTargets[i])));
				Device->CreateRenderTargetView(RenderTargets[i].Get(), nullptr, RTVHandle);
				RTVHandle.Offset(1, RTVDescriptorSize);

				if (!BackBufferTextures[i])
				{
					String Name = TEXT("BackBuffer") + std::to_wstring(i);
					BackBufferTextures[i] = SharedPtr<RTextureD3D12>(new RTextureD3D12(*this, Name, MWindow::Get().GetWidth(), MWindow::Get().GetHeight(), 1, DXGI_FORMAT_R8G8B8A8_UNORM, EResourceFlag::RenderTarget, EResourceType::RenderTexture2D));
				}
				BackBufferTextures[i]->SetUnderlyingResource(RenderTargets[i].Get());
			}

			cout << "Descriptor heap creation success" << endl;
		}

		GraphicsPipelines.emplace_back();
		GraphicsPipelines.emplace_back();
		GraphicsPipelines.emplace_back();
		GraphicsPipelines.emplace_back();
		GraphicsPipelines.emplace_back();
		GraphicsPipelines.emplace_back();

		D3D12_STATIC_SAMPLER_DESC Sampler = {};
		Sampler.Filter = D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
		Sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		Sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		Sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		Sampler.MipLODBias = 0;
		Sampler.MaxAnisotropy = 0;
		Sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		Sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		Sampler.MinLOD = 0.0f;
		Sampler.MaxLOD = D3D12_FLOAT32_MAX;
		Sampler.ShaderRegister = 0;
		Sampler.RegisterSpace = 0;
		Sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// TO DO : Make those pipelines inserted into hash map.
		// Create mesh passes signature.
		{
			CD3DX12_DESCRIPTOR_RANGE1 Ranges[3]{};
			Ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, NumRegisteredHeaps[ToDescriptorIndex(EDescriptorHeapAddressSpace::ConstantBufferView)], 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
			Ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, NumRegisteredHeaps[ToDescriptorIndex(EDescriptorHeapAddressSpace::ShaderResourceView)], 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
			//Ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, NumRegisteredHeaps[ToDescriptorIndex(EDescriptorHeapAddressSpace::UnorderedAccessView)], 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

			CD3DX12_ROOT_PARAMETER1 RootParameters[4]{};
			RootParameters[0].InitAsDescriptorTable(1, &Ranges[0], D3D12_SHADER_VISIBILITY_ALL);
			RootParameters[1].InitAsDescriptorTable(1, &Ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);
			RootParameters[2].InitAsConstants(3, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);
			RootParameters[3].InitAsConstants(kMaterialConstantCount, 0, 2, D3D12_SHADER_VISIBILITY_PIXEL);


			// Allow input layout and deny uneccessary access to certain pipeline stages.
			D3D12_ROOT_SIGNATURE_FLAGS RootSignatureFlags =
				D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

			CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC RootSignatureDesc;
			RootSignatureDesc.Init_1_1(_countof(RootParameters), RootParameters, 1, &Sampler, RootSignatureFlags);

			CreateRootSignature(RootSignatureDesc, GraphicsPipelines[ToPipelineIndex(EGraphicsPipeline::Prepass)]);
			GraphicsPipelines[ToPipelineIndex(EGraphicsPipeline::Prepass)].GetRootSignature()->SetName(TEXT("PrepassRS"));

			CreateRootSignature(RootSignatureDesc, GraphicsPipelines[ToPipelineIndex(EGraphicsPipeline::Basepass)]);
			GraphicsPipelines[ToPipelineIndex(EGraphicsPipeline::Basepass)].GetRootSignature()->SetName(TEXT("BasepassRS"));

			CreateRootSignature(RootSignatureDesc, GraphicsPipelines[ToPipelineIndex(EGraphicsPipeline::ForwardLighting)]);
			GraphicsPipelines[ToPipelineIndex(EGraphicsPipeline::ForwardLighting)].GetRootSignature()->SetName(TEXT("ForwardLightingRS"));

			cout << "Rootsignature creation success" << endl;
		}

		// Create deferred lighting signature.
		{
			CD3DX12_DESCRIPTOR_RANGE1 Ranges[2]{};
			Ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, NumRegisteredHeaps[ToDescriptorIndex(EDescriptorHeapAddressSpace::ConstantBufferView)], 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
			Ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 5, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

			CD3DX12_ROOT_PARAMETER1 RootParameters[3]{};
			RootParameters[0].InitAsDescriptorTable(1, &Ranges[0], D3D12_SHADER_VISIBILITY_ALL);
			RootParameters[1].InitAsDescriptorTable(1, &Ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);
			RootParameters[2].InitAsConstants(1, 0, 3, D3D12_SHADER_VISIBILITY_ALL);

			D3D12_ROOT_SIGNATURE_FLAGS RootSignatureFlags =
				D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;


			CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC RootSignatureDesc;
			RootSignatureDesc.Init_1_1(_countof(RootParameters), RootParameters, 1, &Sampler, RootSignatureFlags);

			CreateRootSignature(RootSignatureDesc, GraphicsPipelines[ToPipelineIndex(EGraphicsPipeline::DeferredLighting)]);
			GraphicsPipelines[ToPipelineIndex(EGraphicsPipeline::DeferredLighting)].GetRootSignature()->SetName(TEXT("DeferredLightingRS"));

			CreateRootSignature(RootSignatureDesc, GraphicsPipelines[ToPipelineIndex(EGraphicsPipeline::DeferredLocalLighting)]);
			GraphicsPipelines[ToPipelineIndex(EGraphicsPipeline::DeferredLighting)].GetRootSignature()->SetName(TEXT("DeferredLightingRS"));

			CreateRootSignature(RootSignatureDesc, GraphicsPipelines[ToPipelineIndex(EGraphicsPipeline::Postprocess)]);
			GraphicsPipelines[ToPipelineIndex(EGraphicsPipeline::Postprocess)].GetRootSignature()->SetName(TEXT("PostprocessRS"));
			cout << "Rootsignature creation success" << endl;
		}

		// Prepass PSO
		{
			const uint32 NumMaterialTextures = (std::max)(MaterialRegistry::Get().GetTotalTextureCount(), kMaterialTextureSlotCount);
			const std::string NumMaterialTexturesStr = std::to_string(NumMaterialTextures);
			const uint32 MaterialTextureStride = kMaterialTextureSlotCount;
			const std::string MaterialTextureStrideStr = std::to_string(MaterialTextureStride);
			uint32 NumUVChannels = 1;
			for (const auto& Mesh : RenderMeshes)
			{
				NumUVChannels = (std::max)(NumUVChannels, Mesh.GetNumUVChannels());
			}
			const std::string NumUVChannelsStr = std::to_string(NumUVChannels);
			vector<D3D_SHADER_MACRO> Defines
			{
				D3D_SHADER_MACRO{ "NUM_MATERIAL_TEXTURES", NumMaterialTexturesStr.c_str() },
				D3D_SHADER_MACRO{ "MATERIAL_TEXTURE_STRIDE", MaterialTextureStrideStr.c_str() },
				D3D_SHADER_MACRO{ "NUM_UV_CHANNELS", NumUVChannelsStr.c_str() },
				D3D_SHADER_MACRO{ nullptr, nullptr }
			};

			TRefCountPtr<ID3DBlob> VertexShader = MShaderCompiler::Get().CompileShader(TEXT("C:/Users/dnjfd/Desktop/Collection/RyoikiTenaki/Sinkansoai/Sources/Render/Shaders/PrepassRendering.hlsl"), TEXT("VSMain"), Defines, EShaderType::VS);

			TRefCountPtr<ID3DBlob> PixelShader = MShaderCompiler::Get().CompileShader(TEXT("C:/Users/dnjfd/Desktop/Collection/RyoikiTenaki/Sinkansoai/Sources/Render/Shaders/PrepassRendering.hlsl"), TEXT("PSMain"), Defines, EShaderType::PS);

			D3D12_INPUT_ELEMENT_DESC InputElementDescsUV1[] =
			{
				{ "POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD",		0, DXGI_FORMAT_R32G32_FLOAT,		1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD",		1, DXGI_FORMAT_R32G32_FLOAT,		2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			};
			D3D12_INPUT_ELEMENT_DESC InputElementDescsUV0[] =
			{
				{ "POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD",		0, DXGI_FORMAT_R32G32_FLOAT,		1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			};

			D3D12_GRAPHICS_PIPELINE_STATE_DESC PSODesc = {};
			if (NumUVChannels > 1)
			{
				PSODesc.InputLayout = { InputElementDescsUV1, _countof(InputElementDescsUV1) };
			}
			else
			{
				PSODesc.InputLayout = { InputElementDescsUV0, _countof(InputElementDescsUV0) };
			}
			PSODesc.pRootSignature = GraphicsPipelines[ToPipelineIndex(EGraphicsPipeline::Prepass)].GetRootSignature().Get();
			PSODesc.VS = CD3DX12_SHADER_BYTECODE{ VertexShader.Get() };
			PSODesc.PS = CD3DX12_SHADER_BYTECODE{ PixelShader.Get() };
			PSODesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			PSODesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			PSODesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			PSODesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
			PSODesc.DSVFormat = DepthFormat;
			PSODesc.SampleDesc.Count = 1;
			PSODesc.SampleMask = UINT_MAX;
			PSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			PSODesc.NumRenderTargets = 0;
			ThrowIfFailed(Device->CreateGraphicsPipelineState(&PSODesc, IID_PPV_ARGS(&GraphicsPipelines[ToPipelineIndex(EGraphicsPipeline::Prepass)].GetPipelineStateObject())));

			cout << " Prepass done " << endl;
		}

		// ForwardLighting PSO
		{
			const uint32 NumMaterialTextures = (std::max)(MaterialRegistry::Get().GetTotalTextureCount(), kMaterialTextureSlotCount);
			const std::string NumMaterialTexturesStr = std::to_string(NumMaterialTextures);
			const uint32 MaterialTextureStride = kMaterialTextureSlotCount;
			const std::string MaterialTextureStrideStr = std::to_string(MaterialTextureStride);
			uint32 NumUVChannels = 1;
			for (const auto& Mesh : RenderMeshes)
			{
				NumUVChannels = (std::max)(NumUVChannels, Mesh.GetNumUVChannels());
			}
			const std::string NumUVChannelsStr = std::to_string(NumUVChannels);
			vector<D3D_SHADER_MACRO> Defines
			{
				D3D_SHADER_MACRO{ "USE_GBUFFER", "0" },
				D3D_SHADER_MACRO{ "BASE_PASS", "1" },
				D3D_SHADER_MACRO{ "NUM_MATERIAL_TEXTURES", NumMaterialTexturesStr.c_str() },
				D3D_SHADER_MACRO{ "MATERIAL_TEXTURE_STRIDE", MaterialTextureStrideStr.c_str() },
				D3D_SHADER_MACRO{ "NUM_UV_CHANNELS", NumUVChannelsStr.c_str() },
				D3D_SHADER_MACRO{ nullptr, nullptr }
			};

			TRefCountPtr<ID3DBlob> VertexShader = MShaderCompiler::Get().CompileShader(TEXT("C:/Users/dnjfd/Desktop/Collection/RyoikiTenaki/Sinkansoai/Sources/Render/Shaders/SimpleRendering.hlsl"), TEXT("VSMain"), Defines, EShaderType::VS);
			TRefCountPtr<ID3DBlob> PixelShader = MShaderCompiler::Get().CompileShader(TEXT("C:/Users/dnjfd/Desktop/Collection/RyoikiTenaki/Sinkansoai/Sources/Render/Shaders/SimpleRendering.hlsl"), TEXT("PSMain"), Defines, EShaderType::PS);

			// Define the vertex input layout.
			D3D12_INPUT_ELEMENT_DESC InputElementDescsUV1[] =
			{
				{ "POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD",		0, DXGI_FORMAT_R32G32_FLOAT,		1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD",		1, DXGI_FORMAT_R32G32_FLOAT,		2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "NORMAL",			0, DXGI_FORMAT_R32G32B32_FLOAT,		3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TANGENT",		0, DXGI_FORMAT_R32G32B32_FLOAT,		4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "BITANGENT",		0, DXGI_FORMAT_R32G32B32_FLOAT,		5, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			};
			D3D12_INPUT_ELEMENT_DESC InputElementDescsUV0[] =
			{
				{ "POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD",		0, DXGI_FORMAT_R32G32_FLOAT,		1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "NORMAL",			0, DXGI_FORMAT_R32G32B32_FLOAT,		2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TANGENT",		0, DXGI_FORMAT_R32G32B32_FLOAT,		3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "BITANGENT",		0, DXGI_FORMAT_R32G32B32_FLOAT,		4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			};

			D3D12_GRAPHICS_PIPELINE_STATE_DESC PSODesc = {};
			if (NumUVChannels > 1)
			{
				PSODesc.InputLayout = { InputElementDescsUV1, _countof(InputElementDescsUV1) };
			}
			else
			{
				PSODesc.InputLayout = { InputElementDescsUV0, _countof(InputElementDescsUV0) };
			}
			PSODesc.pRootSignature = GraphicsPipelines[ToPipelineIndex(EGraphicsPipeline::ForwardLighting)].GetRootSignature().Get();
			PSODesc.VS = CD3DX12_SHADER_BYTECODE{ VertexShader.Get() };
			PSODesc.PS = CD3DX12_SHADER_BYTECODE{ PixelShader.Get() };
			PSODesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			PSODesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			PSODesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			PSODesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
			PSODesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_EQUAL;
			PSODesc.DSVFormat = DepthFormat;
			PSODesc.SampleMask = UINT_MAX;
			PSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

			PSODesc.NumRenderTargets = 1;
			PSODesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;

			PSODesc.SampleDesc.Count = 1;
			ThrowIfFailed(Device->CreateGraphicsPipelineState(&PSODesc, IID_PPV_ARGS(&GraphicsPipelines[ToPipelineIndex(EGraphicsPipeline::ForwardLighting)].GetPipelineStateObject())));

			cout << " Basepass done " << endl;
		}

		// Basepass PSO
		{
			const uint32 NumMaterialTextures = (std::max)(MaterialRegistry::Get().GetTotalTextureCount(), kMaterialTextureSlotCount);
			const std::string NumMaterialTexturesStr = std::to_string(NumMaterialTextures);
			const uint32 MaterialTextureStride = kMaterialTextureSlotCount;
			const std::string MaterialTextureStrideStr = std::to_string(MaterialTextureStride);
			uint32 NumUVChannels = 1;
			for (const auto& Mesh : RenderMeshes)
			{
				NumUVChannels = (std::max)(NumUVChannels, Mesh.GetNumUVChannels());
			}
			const std::string NumUVChannelsStr = std::to_string(NumUVChannels);
			vector<D3D_SHADER_MACRO> Defines
			{
				D3D_SHADER_MACRO{ "USE_GBUFFER", "1" },
				D3D_SHADER_MACRO{ "BASE_PASS", "1" },
				D3D_SHADER_MACRO{ "NUM_MATERIAL_TEXTURES", NumMaterialTexturesStr.c_str() },
				D3D_SHADER_MACRO{ "MATERIAL_TEXTURE_STRIDE", MaterialTextureStrideStr.c_str() },
				D3D_SHADER_MACRO{ "NUM_UV_CHANNELS", NumUVChannelsStr.c_str() },
				D3D_SHADER_MACRO{ nullptr, nullptr }
			};

			TRefCountPtr<ID3DBlob> VertexShader = MShaderCompiler::Get().CompileShader(TEXT("C:/Users/dnjfd/Desktop/Collection/RyoikiTenaki/Sinkansoai/Sources/Render/Shaders/SimpleRendering.hlsl"), TEXT("VSMain"), Defines, EShaderType::VS);
			TRefCountPtr<ID3DBlob> PixelShader = MShaderCompiler::Get().CompileShader(TEXT("C:/Users/dnjfd/Desktop/Collection/RyoikiTenaki/Sinkansoai/Sources/Render/Shaders/SimpleRendering.hlsl"), TEXT("PSMain"), Defines, EShaderType::PS);

			// Define the vertex input layout.
			D3D12_INPUT_ELEMENT_DESC InputElementDescsUV1[] =
			{
				{ "POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD",		0, DXGI_FORMAT_R32G32_FLOAT,		1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD",		1, DXGI_FORMAT_R32G32_FLOAT,		2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "NORMAL",			0, DXGI_FORMAT_R32G32B32_FLOAT,		3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TANGENT",		0, DXGI_FORMAT_R32G32B32_FLOAT,		4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "BITANGENT",		0, DXGI_FORMAT_R32G32B32_FLOAT,		5, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			};
			D3D12_INPUT_ELEMENT_DESC InputElementDescsUV0[] =
			{
				{ "POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD",		0, DXGI_FORMAT_R32G32_FLOAT,		1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "NORMAL",			0, DXGI_FORMAT_R32G32B32_FLOAT,		2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TANGENT",		0, DXGI_FORMAT_R32G32B32_FLOAT,		3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "BITANGENT",		0, DXGI_FORMAT_R32G32B32_FLOAT,		4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			};

			D3D12_GRAPHICS_PIPELINE_STATE_DESC PSODesc = {};
			if (NumUVChannels > 1)
			{
				PSODesc.InputLayout = { InputElementDescsUV1, _countof(InputElementDescsUV1) };
			}
			else
			{
				PSODesc.InputLayout = { InputElementDescsUV0, _countof(InputElementDescsUV0) };
			}
			PSODesc.pRootSignature = GraphicsPipelines[ToPipelineIndex(EGraphicsPipeline::Basepass)].GetRootSignature().Get();
			PSODesc.VS = CD3DX12_SHADER_BYTECODE{ VertexShader.Get() };
			PSODesc.PS = CD3DX12_SHADER_BYTECODE{ PixelShader.Get() };
			PSODesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			PSODesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			PSODesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			PSODesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
			PSODesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_EQUAL;
			PSODesc.DSVFormat = DepthFormat;
			PSODesc.SampleMask = UINT_MAX;
			PSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

			PSODesc.NumRenderTargets = 4;
			PSODesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			PSODesc.RTVFormats[1] = DXGI_FORMAT_R8G8B8A8_UNORM;
			PSODesc.RTVFormats[2] = DXGI_FORMAT_R8G8B8A8_UNORM;
			PSODesc.RTVFormats[3] = DXGI_FORMAT_R32G32B32A32_FLOAT;

			PSODesc.SampleDesc.Count = 1;
			ThrowIfFailed(Device->CreateGraphicsPipelineState(&PSODesc, IID_PPV_ARGS(&GraphicsPipelines[ToPipelineIndex(EGraphicsPipeline::Basepass)].GetPipelineStateObject())));

			cout << " Basepass done " << endl;
		}

		// Lighting PSO
		{
			TRefCountPtr<ID3DBlob> VertexShader = MShaderCompiler::Get().CompileShader(TEXT("C:/Users/dnjfd/Desktop/Collection/RyoikiTenaki/Sinkansoai/Sources/Render/Shaders/ScreenPass.hlsl"), TEXT("VSMain"), EShaderType::VS);
			TRefCountPtr<ID3DBlob> PixelShader = MShaderCompiler::Get().CompileShader(TEXT("C:/Users/dnjfd/Desktop/Collection/RyoikiTenaki/Sinkansoai/Sources/Render/Shaders/Lighting.hlsl"), TEXT("PSMain"), EShaderType::PS);

			D3D12_GRAPHICS_PIPELINE_STATE_DESC PSODesc = {};
			PSODesc.InputLayout = { nullptr, 0 };
			PSODesc.pRootSignature = GraphicsPipelines[ToPipelineIndex(EGraphicsPipeline::DeferredLighting)].GetRootSignature().Get();
			PSODesc.VS = CD3DX12_SHADER_BYTECODE{ VertexShader.Get() };
			PSODesc.PS = CD3DX12_SHADER_BYTECODE{ PixelShader.Get() };
			PSODesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			PSODesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
			PSODesc.RasterizerState.FrontCounterClockwise = TRUE;
			PSODesc.RasterizerState.DepthClipEnable = false;

			PSODesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

			PSODesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			PSODesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
			PSODesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;

			PSODesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
			PSODesc.SampleMask = UINT_MAX;
			PSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			PSODesc.NumRenderTargets = 2;
			PSODesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // Scene Color
			PSODesc.RTVFormats[1] = DXGI_FORMAT_R32G32B32A32_FLOAT; // Debug Texture
			PSODesc.SampleDesc.Count = 1;
			ThrowIfFailed(Device->CreateGraphicsPipelineState(&PSODesc, IID_PPV_ARGS(&GraphicsPipelines[ToPipelineIndex(EGraphicsPipeline::DeferredLighting)].GetPipelineStateObject())));

			cout << " Lighting Pass done " << endl;
		}

		// Local Lighting PSO
		{
			vector<D3D_SHADER_MACRO> Defines{ D3D_SHADER_MACRO{ "LOCAL_LIGHT", "1" }, D3D_SHADER_MACRO{ nullptr, nullptr } };

			TRefCountPtr<ID3DBlob> VertexShader = MShaderCompiler::Get().CompileShader(TEXT("C:/Users/dnjfd/Desktop/Collection/RyoikiTenaki/Sinkansoai/Sources/Render/Shaders/Lighting.hlsl"), TEXT("VSMain"), Defines, EShaderType::VS);
			TRefCountPtr<ID3DBlob> PixelShader = MShaderCompiler::Get().CompileShader(TEXT("C:/Users/dnjfd/Desktop/Collection/RyoikiTenaki/Sinkansoai/Sources/Render/Shaders/Lighting.hlsl"), TEXT("PSMain"), Defines, EShaderType::PS);

			D3D12_INPUT_ELEMENT_DESC InputElementDescs[] =
			{
				{ "POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			};

			D3D12_GRAPHICS_PIPELINE_STATE_DESC PSODesc = {};
			PSODesc.InputLayout = { InputElementDescs, _countof(InputElementDescs) };
			PSODesc.pRootSignature = GraphicsPipelines[ToPipelineIndex(EGraphicsPipeline::DeferredLocalLighting)].GetRootSignature().Get();
			PSODesc.VS = CD3DX12_SHADER_BYTECODE{ VertexShader.Get() };
			PSODesc.PS = CD3DX12_SHADER_BYTECODE{ PixelShader.Get() };
			PSODesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			PSODesc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;

			const D3D12_RENDER_TARGET_BLEND_DESC AddBlendState =
			{
				true, false,
				D3D12_BLEND_ONE, D3D12_BLEND_ONE, D3D12_BLEND_OP_ADD,
				D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
				D3D12_LOGIC_OP_NOOP,
				D3D12_COLOR_WRITE_ENABLE_ALL,
			};

			PSODesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			PSODesc.BlendState.RenderTarget[0] = AddBlendState;
			PSODesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			PSODesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
			PSODesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
			PSODesc.DSVFormat = DepthFormat;
			PSODesc.SampleMask = UINT_MAX;
			PSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			PSODesc.NumRenderTargets = 2;
			PSODesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM; // Scene Color
			PSODesc.RTVFormats[1] = DXGI_FORMAT_R32G32B32A32_FLOAT; // Debug Texture
			PSODesc.SampleDesc.Count = 1;

			ThrowIfFailed(Device->CreateGraphicsPipelineState(&PSODesc, IID_PPV_ARGS(&GraphicsPipelines[ToPipelineIndex(EGraphicsPipeline::DeferredLocalLighting)].GetPipelineStateObject())));

			cout << " Local Lighting Pass done " << endl;
		}


		// Postprocess PSO
		{
			TRefCountPtr<ID3DBlob> VertexShader = MShaderCompiler::Get().CompileShader(TEXT("C:/Users/dnjfd/Desktop/Collection/RyoikiTenaki/Sinkansoai/Sources/Render/Shaders/ScreenPass.hlsl"), TEXT("VSMain"), EShaderType::VS);
			TRefCountPtr<ID3DBlob> PixelShader = MShaderCompiler::Get().CompileShader(TEXT("C:/Users/dnjfd/Desktop/Collection/RyoikiTenaki/Sinkansoai/Sources/Render/Shaders/Postprocess.hlsl"), TEXT("PSMain"), EShaderType::PS);

			D3D12_GRAPHICS_PIPELINE_STATE_DESC PSODesc = {};
			PSODesc.InputLayout = { nullptr, 0 };
			PSODesc.pRootSignature = GraphicsPipelines[ToPipelineIndex(EGraphicsPipeline::Postprocess)].GetRootSignature().Get();
			PSODesc.VS = CD3DX12_SHADER_BYTECODE{ VertexShader.Get() };
			PSODesc.PS = CD3DX12_SHADER_BYTECODE{ PixelShader.Get() };
			PSODesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			PSODesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
			PSODesc.RasterizerState.FrontCounterClockwise = TRUE;
			PSODesc.RasterizerState.DepthClipEnable = false;

			PSODesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);

			PSODesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			PSODesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
			PSODesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;

			PSODesc.DSVFormat = DXGI_FORMAT_UNKNOWN;
			PSODesc.SampleMask = UINT_MAX;
			PSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			PSODesc.NumRenderTargets = 1;
			PSODesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			PSODesc.SampleDesc.Count = 1;
			ThrowIfFailed(Device->CreateGraphicsPipelineState(&PSODesc, IID_PPV_ARGS(&GraphicsPipelines[ToPipelineIndex(EGraphicsPipeline::Postprocess)].GetPipelineStateObject())));

			cout << " Lighting Pass done " << endl;
		}


		// Create synchronization objects and wait until assets have been uploaded to the GPU.
		{
			ThrowIfFailed(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)));
			FenceValue = 1;

			// Create an event handle to use for frame synchronization.
			FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
			if (FenceEvent == nullptr)
			{
				ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
			}

			WaitForPreviousFence();
			cout << " Test fence creation " << endl;
			GraphicsCommandList.Close();

			// Execute the command list.
			ID3D12CommandList* ppCommandLists[] = { GraphicsCommandList.GetRawCommandListBase() };
			CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
			WaitForPreviousFence();
		}
	}
}

void RRenderBackendD3D12::Teardown()
{
	WaitForPreviousFence();

	CloseHandle(FenceEvent);

	for (UINT i = 0; i < NumBackBuffers; i++)
	{
		RenderTargets[i] = nullptr;
	}

	//RTVHeap->Release();
	RTVHeap = nullptr;
	SwapChain = nullptr;

	// Except below object, Skip all other object tearing down because everything else should be separated soon.
	Device = nullptr;

	cout << "D3D12 Renderbackend Exit" << endl;
}


void RRenderBackendD3D12::Execute()
{
	GraphicsCommandList.Close();

	QueueTimer.CurrentTime = std::chrono::high_resolution_clock::now();
	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { GraphicsCommandList.GetRawCommandListBase() };
	CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

}

void RRenderBackendD3D12::WaitForPreviousFence()
{
	const UINT64 WaitingFenceValue = FenceValue;
	ThrowIfFailed(CommandQueue->Signal(Fence.Get(), WaitingFenceValue));
	FenceValue++;

	// Wait until the previous frame is finished.
	if (Fence->GetCompletedValue() < WaitingFenceValue)
	{
		ThrowIfFailed(Fence->SetEventOnCompletion(WaitingFenceValue, FenceEvent));
		WaitForSingleObject(FenceEvent, INFINITE);
	}
	QueueTimer.CurrentTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration<float> DeltaTime = (QueueTimer.CurrentTime - QueueTimer.PreviousTime);
	QueueTimer.PreviousTime = QueueTimer.CurrentTime;

	MWindow::Get().SetFrame(1.0f / DeltaTime.count());

	FrameIndex = SwapChain->GetCurrentBackBufferIndex();
}

void RRenderBackendD3D12::RenderBegin()
{
	GraphicsCommandList.Reset();
	GraphicsCommandList.BeginEvent(0xFFFFFFFF, TEXT("RenderFrame"));
}

void RRenderBackendD3D12::RenderFinish()
{
	// Flush temp buffer done its purposes.
	// After implementing asynchronous GPU flow, this should be changed as following the GPU lifetime.
	UploadHeapReferences.clear();

	GraphicsCommandList.EndEvent();

	Execute();
	SwapChain->Present(1, 0);
	WaitForPreviousFence();
}

vector<RMesh*> RRenderBackendD3D12::GetRenderMesh()
{
	vector<RMesh*> Temp;
	Temp.reserve(RenderMeshes.size());
	for (auto& Mesh : RenderMeshes)
	{
		Temp.push_back(&Mesh);
	}
	return Temp;
}

RMesh* RRenderBackendD3D12::GetLightVolumeMesh()
{
	return &LightVolumeMesh;
}

// TO DO Remove this and move to scene texture.
uint32 RRenderBackendD3D12::GetSceneRTVCount() const
{
	return 5;
}

D3D12_CPU_DESCRIPTOR_HANDLE RRenderBackendD3D12::GetSceneRTVHandle(uint32 Index)
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE Handle(SceneTextureRTVHeap->GetCPUDescriptorHandleForHeapStart(), Index, RTVDescriptorSize);
	return Handle;
}

D3D12_CPU_DESCRIPTOR_HANDLE RRenderBackendD3D12::GetSceneDepthHandle()
{
	return DSVHeap->GetCPUDescriptorHandleForHeapStart();
}

D3D12_GPU_DESCRIPTOR_HANDLE RRenderBackendD3D12::GetSceneTextureGPUHandle()
{
	return SceneTextureGPUHandle;
}

ID3D12DescriptorHeap* RRenderBackendD3D12::GetCBVSRVHeap()
{
	return CBVSRVHeap.Get();
}

D3D12_GPU_DESCRIPTOR_HANDLE RRenderBackendD3D12::GetDescriptorHandle(EDescriptorHeapAddressSpace AddressSpace)
{
	return AddressCacheForDescriptorHeapStart[ToDescriptorIndex(AddressSpace)];
}

const D3D12_VIEWPORT* RRenderBackendD3D12::GetViewport()
{
	return &Viewport;
}

const D3D12_RECT* RRenderBackendD3D12::GetScissorRect()
{
	return &ScissorRect;
}

RGraphicsPipeline* RRenderBackendD3D12::GetGraphicsPipeline(EGraphicsPipeline Pipeline)
{
	return &GraphicsPipelines[static_cast<uint32>(Pipeline)];
}

D3D12_CPU_DESCRIPTOR_HANDLE RRenderBackendD3D12::GetBackBufferRTVHandle()
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE Handle(RTVHeap->GetCPUDescriptorHandleForHeapStart(), FrameIndex, RTVDescriptorSize);
	return Handle;
}

RTexture* RRenderBackendD3D12::GetBackBufferResource()
{
	return BackBufferTextures[FrameIndex].get();
}

D3D12_RESOURCE_DESC CreateResourceDescD3D12(EResourceType ResourceType, EResourceFlag ResourceFlag, DXGI_FORMAT Format, uint32 Width, uint32 Height, uint32 NumMips, uint32 ArraySize)
{
	// Create the depth/stencil buffer and view.
	D3D12_RESOURCE_DESC Desc;
	Desc.Dimension = D3D12_RESOURCE_DIMENSION_UNKNOWN;
	Desc.Alignment = 0;
	Desc.Width = Width;
	Desc.Height = Height;
	Desc.DepthOrArraySize = NumMips;
	Desc.MipLevels = ArraySize;
	Desc.Format = Format;
	Desc.SampleDesc.Count = 1;
	Desc.SampleDesc.Quality = 0;
	Desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	Desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	switch (ResourceType)
	{
	case RenderBuffer:
		Desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		break;
	case RenderTexture1D:
		Desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
		break;
	case RenderTexture2D:
		Desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		break;
	case RenderTexture3D:
		Desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
		break;
	case Texture2D:
		Desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		break;
	default:
		break;
	}

	switch (ResourceFlag)
	{
	case RenderTarget:
		Desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		break;
	case DepthStencilTarget:
		Desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
		break;
	case UnorderedAccess:
		Desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		break;
	case DenyShaderResource:
		Desc.Flags = D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE;
		break;
	case AllowSimultanenousAccess:
		Desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_SIMULTANEOUS_ACCESS;
		break;
	default:
		break;
	}

	return Desc;
}

TRefCountPtr<ID3D12Resource> RRenderBackendD3D12::CreateUnderlyingResource(EResourceType ResourceType, EResourceFlag ResourceFlag, DXGI_FORMAT Format, uint32 Width, uint32 Height, uint32 NumMips, uint32 Depths)
{		
	D3D12_RESOURCE_DESC Desc = CreateResourceDescD3D12(ResourceType, ResourceFlag, Format, Width, Height, NumMips, Depths);

	auto HeapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_CLEAR_VALUE ClearValue;
	ClearValue.Format = Format;

	const bool bNeedsClearValue = (ResourceFlag == EResourceFlag::DepthStencilTarget || ResourceFlag == EResourceFlag::RenderTarget);

	if (ResourceFlag == EResourceFlag::DepthStencilTarget)
	{
		ClearValue.Format = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
		ClearValue.DepthStencil.Depth = 0.0f;
		ClearValue.DepthStencil.Stencil = 0;
	}
	else
	{
		ClearValue.Color[0] = 0;
		ClearValue.Color[1] = 0;
		ClearValue.Color[2] = 0;
		ClearValue.Color[3] = 0;
	}

	const bool bTextureResource = (ResourceType == EResourceType::Texture2D);

	TRefCountPtr<ID3D12Resource> OutResource;
	ThrowIfFailed(Device->CreateCommittedResource(
		&HeapProperty,
		D3D12_HEAP_FLAG_NONE,
		&Desc,
		bTextureResource ? D3D12_RESOURCE_STATE_COPY_DEST : D3D12_RESOURCE_STATE_COMMON,
		bNeedsClearValue ? &ClearValue : nullptr,
		IID_PPV_ARGS(&OutResource)));

	return OutResource;
}

TRefCountPtr<ID3D12Resource> RRenderBackendD3D12::CreateRenderTargetResource(String&& Name, EResourceFlag ResourceFlag, DXGI_FORMAT Format, uint32 Width, uint32 Height)
{
	auto Resource = CreateUnderlyingResource(RenderTexture2D, ResourceFlag, Format, Width, Height, 1, 1);
	Resource->SetName(Name.c_str());
	return Resource;
}

TRefCountPtr<ID3D12Resource> RRenderBackendD3D12::CreateTexture2DResource(String&& Name, EResourceFlag ResourceFlag, DXGI_FORMAT Format, uint32 Width, uint32 Height)
{
	auto Resource = CreateUnderlyingResource(Texture2D, ResourceFlag, Format, Width, Height, 1, 1);
	Resource->SetName(Name.c_str());
	return Resource;
}

TRefCountPtr<ID3D12Resource> RRenderBackendD3D12::CreateUploadHeap(uint32 UploadBufferSize)
{
	auto HeapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto ResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(UploadBufferSize);

	ComPtr<ID3D12Resource> Resource;
	// Create the GPU upload buffer.
	ThrowIfFailed(Device->CreateCommittedResource(
		&HeapProperty,
		D3D12_HEAP_FLAG_NONE,
		&ResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&Resource)));

	Resource->SetName(TEXT("Upload heap"));

	UploadHeapReferences.push_back(Resource);
	return Resource;
}

void RRenderBackendD3D12::CreateRootSignature(const CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC& RootSignatureDesc, RGraphicsPipeline& Pipepline)
{
	ComPtr<ID3DBlob> Signature;
	ComPtr<ID3DBlob> Error;

	D3DX12SerializeVersionedRootSignature(&RootSignatureDesc, bSupportsRootSignatureVersion1_1 ? D3D_ROOT_SIGNATURE_VERSION_1_1 : D3D_ROOT_SIGNATURE_VERSION_1_0, &Signature, &Error);
	if (Error)
	{
		cout << (char*)Error->GetBufferPointer() << endl;
		Error->Release();
		ThrowIfFailed(S_FALSE);
	}

	ThrowIfFailed(Device->CreateRootSignature(0, Signature->GetBufferPointer(), Signature->GetBufferSize(), IID_PPV_ARGS(&Pipepline.GetRootSignature())));
}
