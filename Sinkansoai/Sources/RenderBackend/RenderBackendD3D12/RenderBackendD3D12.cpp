#include "../../Windows.h"
#include "../../Misc/Geometry.h"
#include "../../Module/Mesh/MeshBuilder.h"
#include "../../Module/Texture/TextureBuilder.h"
#include "../../Module/ShaderCompiler/ShaderCompiler.h"

#include "RenderBackendD3D12.h"
#include "RenderCommandListD3D12.h"

#pragma comment ( lib, "d3d12.lib")
#pragma comment ( lib, "dxgi.lib")

RMesh RenderMesh;
SharedPtr<RTexture2DD3D12> DefaultTexture;



RRenderBackendD3D12::RRenderBackendD3D12()
	: DynamicBuffer(*this, TEXT("Global Buffer"))
{
	SetBackendName(TEXT("D3D12"));
}

void RRenderBackendD3D12::Init()
{
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
		RTVDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		DSVDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		CBVSRVUAVDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		CommandLists.emplace_back();

		auto& AllocatedCommandList = CommandLists[0];
		AllocatedCommandList.AllocateCommandLsit(*this);

		// First commandlist is used as a main command list
		MainCommandList = &CommandLists[0];

		auto& CommandAllocator = CommandLists[0].CommandAllocator;
		auto& CommandList = CommandLists[0].CommandList;

		CommandLists[0].Reset();

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
			DynamicBuffer.Allocate(D3D12MaxConstantBufferSize);

			// Default Texture
			{
				DefaultTexture = SharedPtr<RTexture2DD3D12>(new RTexture2DD3D12(*this, TEXT("CheckerTexture"), 256, 256, 1, DXGI_FORMAT_R8G8B8A8_UNORM, EResourceFlag::None));
				DefaultTexture->AllocateResource();

				auto RawTexture = MTextureBuilder::Get().GenerateDefaultTexture(256, 256, 4);
				DefaultTexture->StreamTexture(RawTexture.data());
			}
		}

		// Create testing resources
		{
			MMesh Mesh = MMeshBuilder::Get().LoadMesh(TEXT("C:/Users/dnjfd/Desktop/Collection/RyoikiTenaki/Sinkansoai/Resources/sponza/"), TEXT("sponza.obj"));

			RenderMesh.InitResources(Mesh,
				[&](RVertexBuffer*& PositionVB, RVertexBuffer*& UVVB, RVertexBuffer*& NormalVB, RVertexBuffer*& TangetVB, RIndexBuffer*& IB)
				{
					PositionVB = new RVertexBufferD3D12(*this, TEXT("PositionVertexBuffer"));
					UVVB = new RVertexBufferD3D12(*this, TEXT("UVVertexBuffer"));
					NormalVB = new RVertexBufferD3D12(*this, TEXT("NormalVertexBuffer"));
					TangetVB = new RVertexBufferD3D12(*this, TEXT("TangentVertexBuffer"));
					IB = new RIndexBufferD3D12(*this, TEXT("IndexBuffer"));
				});

			RenderMesh.InitMaterials(Mesh.Materials,
				[&](SharedPtr< RTexture >& Resource, MTexture& RawTexture)
				{
					Resource = SharedPtr<RTexture2DD3D12>(new RTexture2DD3D12(*this, TEXT("CheckerTexture"), RawTexture.Width, RawTexture.Height, 1, DXGI_FORMAT_B8G8R8A8_UNORM, EResourceFlag::None));

					Resource->AllocateResource();
					Resource->StreamTexture(RawTexture.Pixels.data());
				});



			cout << " Test resource creation done " << endl;
		}

		auto DepthFormat = DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

		// Create descriptor heaps.
		{
			// Describe and create a render target view (RTV) descriptor heap.

			// Decriptor Heap -> Heap space for the views. not actual memory
			// D3D12 uses view system which separate actual memory allocation and the view of it.
			// So doesn't require change of the actual memory allocation but just change view to reuse allocated space.

			// So finally decriptor heap means Heap space for the stored view.
			// View is pointing actual data heap ( texture or buffer heap ).
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
				// Create the depth/stencil buffer and view.
				DepthStencilBuffer = CreateRenderTargetResource(TEXT("MainDepthStencil"), EResourceFlag::DepthStencilTarget, DepthFormat, MWindow::Get().GetWidth(), MWindow::Get().GetHeight());

				Device->CreateDepthStencilView(DepthStencilBuffer.Get(), nullptr, DSVHeap->GetCPUDescriptorHandleForHeapStart());
			}

			
			const uint32 NumConstantBuffer = 1; // Global Dynamic Buffer ( View Buffer )
			const uint32 NumTextures = RenderMesh.GetNumRegisteredTextures() + 1; // + 1 ( Default Texture )

			D3D12_DESCRIPTOR_HEAP_DESC CbvHeapDesc = {};
			CbvHeapDesc.NumDescriptors = NumConstantBuffer + NumTextures;
			CbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			CbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			ThrowIfFailed(Device->CreateDescriptorHeap(&CbvHeapDesc, IID_PPV_ARGS(&CBVSRVHeap)));
			cout << "Descriptor cbv heap creation success" << endl;


			{
				CD3DX12_CPU_DESCRIPTOR_HANDLE CBVSRVUAVCPUHandle(CBVSRVHeap->GetCPUDescriptorHandleForHeapStart());
				CD3DX12_GPU_DESCRIPTOR_HANDLE CBVSRVUAVGPUHandle(CBVSRVHeap->GetGPUDescriptorHandleForHeapStart());

				AddressCacheForDescriptorHeapStart[EDescriptorHeapAddressSpace::ConstantBufferView] = CBVSRVUAVGPUHandle;
				{
					auto CBVDesc = DynamicBuffer.GetCBVDesc();
					Device->CreateConstantBufferView(&CBVDesc, CBVSRVUAVCPUHandle);
					CBVSRVUAVCPUHandle.Offset(CBVSRVUAVDescriptorSize);
					CBVSRVUAVGPUHandle.Offset(CBVSRVUAVDescriptorSize);

					++NumRegisteredHeaps[EDescriptorHeapAddressSpace::ConstantBufferView];
				}
				
				AddressCacheForDescriptorHeapStart[EDescriptorHeapAddressSpace::ShaderResourceView] = CBVSRVUAVGPUHandle;
				{
					Device->CreateShaderResourceView(DefaultTexture->GetUnderlyingResource(), &DefaultTexture->GetSRVDesc(), CBVSRVUAVCPUHandle);
					CBVSRVUAVCPUHandle.Offset(CBVSRVUAVDescriptorSize);

					++NumRegisteredHeaps[EDescriptorHeapAddressSpace::ShaderResourceView];
				}
				
				for (auto& Material : RenderMesh.Materials)
				{
					if (Material.Textures.size() == 0)
					{
						Device->CreateShaderResourceView(DefaultTexture->GetUnderlyingResource(), &DefaultTexture->GetSRVDesc(), CBVSRVUAVCPUHandle);
						CBVSRVUAVCPUHandle.Offset(CBVSRVUAVDescriptorSize);
						CBVSRVUAVGPUHandle.Offset(CBVSRVUAVDescriptorSize);

						++NumRegisteredHeaps[EDescriptorHeapAddressSpace::ShaderResourceView];
					}
					else
					{
						for (auto& Texture : Material.Textures)
						{
							RTexture2DD3D12* D3D12Texture = CastAsD3D12<RTexture2DD3D12>(Texture.get());
							Device->CreateShaderResourceView(D3D12Texture->GetUnderlyingResource(), &D3D12Texture->GetSRVDesc(), CBVSRVUAVCPUHandle);
							CBVSRVUAVCPUHandle.Offset(CBVSRVUAVDescriptorSize);
							CBVSRVUAVGPUHandle.Offset(CBVSRVUAVDescriptorSize);

							++NumRegisteredHeaps[EDescriptorHeapAddressSpace::ShaderResourceView];
						}
					}
				}

				AddressCacheForDescriptorHeapStart[EDescriptorHeapAddressSpace::UnorderedAccessView] = CBVSRVUAVGPUHandle;
				{
					// No UAV to register
				}

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
			}

			cout << "Descriptor heap creation success" << endl;
		}



		GraphicsPipelines.emplace_back();
		GraphicsPipelines.emplace_back();

		// Create signature.
		{
			D3D12_FEATURE_DATA_ROOT_SIGNATURE FeatureData = {};

			// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
			FeatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

			if (FAILED(Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &FeatureData, sizeof(FeatureData))))
			{
				cout << " Root signature 1_1 failed " << endl;
				FeatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
			}

			CD3DX12_DESCRIPTOR_RANGE1 Ranges[3]{};
			Ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, NumRegisteredHeaps[EDescriptorHeapAddressSpace::ConstantBufferView], 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);
			Ranges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, NumRegisteredHeaps[EDescriptorHeapAddressSpace::ShaderResourceView], 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

			//Ranges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, NumRegisteredHeaps[EDescriptorHeapAddressSpace::UnorderedAccessView], 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);


			CD3DX12_ROOT_PARAMETER1 RootParameters[5]{};
			RootParameters[0].InitAsDescriptorTable(1, &Ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);
			RootParameters[1].InitAsDescriptorTable(1, &Ranges[1], D3D12_SHADER_VISIBILITY_PIXEL);

			//RootParameters[2].InitAsConstantBufferView(1, 0, D3D12_ROOT_DESCRIPTOR_FLAG_NONE,D3D12_SHADER_VISIBILITY_PIXEL);

			RootParameters[3].InitAsConstants(3, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);
			RootParameters[4].InitAsConstants(1, 0, 2, D3D12_SHADER_VISIBILITY_PIXEL);

			// Allow input layout and deny uneccessary access to certain pipeline stages.
			D3D12_ROOT_SIGNATURE_FLAGS RootSignatureFlags =
				D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

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

			CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC RootSignatureDesc;
			RootSignatureDesc.Init_1_1(_countof(RootParameters), RootParameters, 1, &Sampler, RootSignatureFlags);

			ComPtr<ID3DBlob> Signature;
			ComPtr<ID3DBlob> Error;
			D3DX12SerializeVersionedRootSignature(&RootSignatureDesc, FeatureData.HighestVersion, &Signature, &Error);
			if (Error)
			{
				cout << (char*)Error->GetBufferPointer() << endl;
				Error->Release();
				ThrowIfFailed(S_FALSE);
			}

			ThrowIfFailed(Device->CreateRootSignature(0, Signature->GetBufferPointer(), Signature->GetBufferSize(), IID_PPV_ARGS(&GraphicsPipelines[EGraphicsPipeline::Prepass].GetRootSignature())));
			ThrowIfFailed(Device->CreateRootSignature(0, Signature->GetBufferPointer(), Signature->GetBufferSize(), IID_PPV_ARGS(&GraphicsPipelines[EGraphicsPipeline::Basepass].GetRootSignature())));

			cout << "Rootsignature creation success" << endl;
		}


		// Prepass PSO
		{
			TRefCountPtr<ID3DBlob> VertexShader = MShaderCompiler::Get().CompileShader(TEXT("C:/Users/dnjfd/Desktop/Collection/RyoikiTenaki/Sinkansoai/Sources/Render/Shaders/PrepassRendering.hlsl"), TEXT("VSMain"), EShaderType::VS);

			D3D12_INPUT_ELEMENT_DESC InputElementDescs[] =
			{
				{ "POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			};

			// Describe and create the graphics pipeline state object (PSO).
			D3D12_GRAPHICS_PIPELINE_STATE_DESC PSODesc = {};
			PSODesc.InputLayout = { InputElementDescs, _countof(InputElementDescs) };
			PSODesc.pRootSignature = GraphicsPipelines[EGraphicsPipeline::Prepass].GetRootSignature().Get();
			PSODesc.VS = CD3DX12_SHADER_BYTECODE(VertexShader.Get());
			PSODesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			PSODesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			PSODesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			PSODesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
			PSODesc.DSVFormat = DepthFormat;
			PSODesc.SampleDesc.Count = 1;
			PSODesc.SampleMask = UINT_MAX;
			PSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			PSODesc.NumRenderTargets = 0;
			ThrowIfFailed(Device->CreateGraphicsPipelineState(&PSODesc, IID_PPV_ARGS(&GraphicsPipelines[EGraphicsPipeline::Prepass].GetPipelineStateObject())));

			cout << " Basepass done " << endl;
		}

		// Basepass PSO
		{
			TRefCountPtr<ID3DBlob> VertexShader = MShaderCompiler::Get().CompileShader(TEXT("C:/Users/dnjfd/Desktop/Collection/RyoikiTenaki/Sinkansoai/Sources/Render/Shaders/SimpleRendering.hlsl"), TEXT("VSMain"), EShaderType::VS);
			TRefCountPtr<ID3DBlob> PixelShader = MShaderCompiler::Get().CompileShader(TEXT("C:/Users/dnjfd/Desktop/Collection/RyoikiTenaki/Sinkansoai/Sources/Render/Shaders/SimpleRendering.hlsl"), TEXT("PSMain"), EShaderType::PS);

			// Define the vertex input layout.
			D3D12_INPUT_ELEMENT_DESC InputElementDescs[] =
			{
				{ "POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT,		0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD",		0, DXGI_FORMAT_R32G32_FLOAT,		1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
				//{ "COLOR",		0, DXGI_FORMAT_R32G32B32A32_FLOAT,	1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
			};

			// Describe and create the graphics pipeline state object (PSO).
			D3D12_GRAPHICS_PIPELINE_STATE_DESC PSODesc = {};
			PSODesc.InputLayout = { InputElementDescs, _countof(InputElementDescs) };
			PSODesc.pRootSignature = GraphicsPipelines[EGraphicsPipeline::Prepass].GetRootSignature().Get();
			PSODesc.VS = CD3DX12_SHADER_BYTECODE(VertexShader.Get());
			PSODesc.PS = CD3DX12_SHADER_BYTECODE(PixelShader.Get());
			PSODesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			PSODesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			PSODesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			PSODesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
			PSODesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;
			PSODesc.DSVFormat = DepthFormat;
			PSODesc.SampleMask = UINT_MAX;
			PSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			PSODesc.NumRenderTargets = 1;
			PSODesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			PSODesc.SampleDesc.Count = 1;
			ThrowIfFailed(Device->CreateGraphicsPipelineState(&PSODesc, IID_PPV_ARGS(&GraphicsPipelines[EGraphicsPipeline::Basepass].GetPipelineStateObject())));

			cout << " Basepass done " << endl;
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

			// Indicate that the back buffer will now be used to present.
			auto Barrier = CD3DX12_RESOURCE_BARRIER::Transition(DepthStencilBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_READ);
			CommandLists[0].CommandList->ResourceBarrier(1, &Barrier);
			CommandLists[0].Close();

			// Execute the command list.
			ID3D12CommandList* ppCommandLists[] = { CommandList };
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
	CommandLists.clear();

	Device = nullptr;

	cout << "D3D12 Renderbackend Exit" << endl;
}

void RRenderBackendD3D12::Prepass()
{
	auto& D3D12CommandList = CommandLists[0];
	auto& CommandList = CommandLists[0].CommandList;

	CD3DX12_CPU_DESCRIPTOR_HANDLE DSVHandle(DSVHeap->GetCPUDescriptorHandleForHeapStart());
	CommandList->ClearDepthStencilView(DSVHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 0, 0, 0, nullptr);
	CommandList->OMSetRenderTargets(0, nullptr, false, &DSVHandle);

	D3D12CommandList.SetGraphicsPipeline(GraphicsPipelines[EGraphicsPipeline::Prepass]);

	ID3D12DescriptorHeap* Heaps[] = { CBVSRVHeap.Get() };
	CommandList->SetDescriptorHeaps(_countof(Heaps), Heaps);
	CommandList->SetGraphicsRootDescriptorTable(0, AddressCacheForDescriptorHeapStart[EDescriptorHeapAddressSpace::ConstantBufferView]);

	auto& Mesh = RenderMesh;
	D3D12CommandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	D3D12CommandList.SetVertexBuffer(0, Mesh.PositionVertexBuffer);
	D3D12CommandList.SetIndexBuffer(Mesh.IndexBuffer);

	for (int32 iSection = 0; iSection < Mesh.Sections.size(); ++iSection)
	{
		const auto& Section = Mesh.Sections[iSection];
		const int32 NumIndices = Section.End - Section.Start;
		const int32 StartIndex = Section.Start;	
		CommandList->DrawIndexedInstanced(NumIndices, 1, StartIndex, 0, 0);
	}

	{
		CD3DX12_RESOURCE_BARRIER Barriers[] =
		{
			CD3DX12_RESOURCE_BARRIER::Transition(DepthStencilBuffer.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ)
		};
		CommandList->ResourceBarrier(_countof(Barriers), Barriers);
	}
}


void RRenderBackendD3D12::Basepass()
{
	auto& D3D12CommandList = CommandLists[0];
	auto& CommandList = CommandLists[0].CommandList;

	CD3DX12_CPU_DESCRIPTOR_HANDLE RTVHandle(RTVHeap->GetCPUDescriptorHandleForHeapStart(), FrameIndex, RTVDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE DSVHandle(DSVHeap->GetCPUDescriptorHandleForHeapStart());

	// Record commands.
	const float ClearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };

	CommandList->ClearRenderTargetView(RTVHandle, ClearColor, 0, nullptr);
	CommandList->OMSetRenderTargets(1, &RTVHandle, false, &DSVHandle);

	ID3D12DescriptorHeap* Heaps[] = { CBVSRVHeap.Get() };
	CommandList->SetDescriptorHeaps(_countof(Heaps), Heaps);
	CommandList->SetGraphicsRootDescriptorTable(0, AddressCacheForDescriptorHeapStart[EDescriptorHeapAddressSpace::ConstantBufferView]);
	CommandList->SetGraphicsRootDescriptorTable(1, AddressCacheForDescriptorHeapStart[EDescriptorHeapAddressSpace::ShaderResourceView]);

	D3D12CommandList.SetGraphicsPipeline(GraphicsPipelines[EGraphicsPipeline::Basepass]);

	auto& Mesh = RenderMesh;
	D3D12CommandList.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	D3D12CommandList.SetVertexBuffer(0, Mesh.PositionVertexBuffer);
	D3D12CommandList.SetVertexBuffer(1, Mesh.UVVertexBuffer);
	D3D12CommandList.SetIndexBuffer(Mesh.IndexBuffer);

	for (int32 iSection = 0; iSection < Mesh.Sections.size(); ++iSection)
	{
		const auto& Section = Mesh.Sections[iSection];
		const auto& Color = Mesh.Materials[Section.MaterialId].Colors.size() > 0 ? Mesh.Materials[Section.MaterialId].Colors[0] : float3(1, 1, 1);
		const int32 NumIndices = Section.End - Section.Start;
		const int32 StartIndex = Section.Start;

		CommandList->SetGraphicsRoot32BitConstants(3, sizeof(Color) / 4, &Color, 0);
		CommandList->SetGraphicsRoot32BitConstant(4, Section.MaterialId, 0);
		CommandList->DrawIndexedInstanced(NumIndices, 1, StartIndex, 0, 0);
	}

	{
		CD3DX12_RESOURCE_BARRIER Barriers[] =
		{
			CD3DX12_RESOURCE_BARRIER::Transition(RenderTargets[FrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT),
		};
		CommandList->ResourceBarrier(_countof(Barriers), Barriers);
	}
}

void RRenderBackendD3D12::FunctionalityTestRender()
{
	CommandLists[0].Reset();

	auto& D3D12CommandList = CommandLists[0];
	auto& CommandList = CommandLists[0].CommandList;
	{
		CD3DX12_RESOURCE_BARRIER Barriers[] =
		{
			CD3DX12_RESOURCE_BARRIER::Transition(RenderTargets[FrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET),
			CD3DX12_RESOURCE_BARRIER::Transition(DepthStencilBuffer.Get(), D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE)
		};
		CommandList->ResourceBarrier(_countof(Barriers), Barriers);
	}
	CommandList->RSSetViewports(1, &Viewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);

	Prepass();
	Basepass();



	CommandLists[0].Close();
	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { CommandList };
	CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Present the frame.
	ThrowIfFailed(SwapChain->Present(1, 0));

	WaitForPreviousFence();

	RenderFinish();
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

	FrameIndex = SwapChain->GetCurrentBackBufferIndex();

}

void RRenderBackendD3D12::RenderFinish()
{
	// Flush temp buffer done its purposes.
	// After implementing asynchronous GPU flow, this should be changed as following the GPU lifetime.
	UploadHeapReferences.clear();

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
	// Create the depth/stencil buffer and view.
	D3D12_RESOURCE_DESC Desc = CreateResourceDescD3D12(ResourceType, ResourceFlag, Format, Width, Height, NumMips, Depths);

	auto HeapProperty = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	D3D12_CLEAR_VALUE ClearValue;
	ClearValue.Format = Format;

	if (ResourceFlag == EResourceFlag::DepthStencilTarget)
	{
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
		bTextureResource ? nullptr : &ClearValue,
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
