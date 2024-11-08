#include "../../Windows.h"
#include "../../Misc/Geometry.h"
#include "../../Module/MeshBuilder.h"

#include "RenderBackendD3D12.h"
#include "RenderCommandListD3D12.h"
#include <d3dcompiler.h>

#pragma comment ( lib, "d3d12.lib")
#pragma comment ( lib, "D3DCompiler.lib")
#pragma comment ( lib, "dxgi.lib")

RRenderBackendD3D12::RRenderBackendD3D12()
	: DynamicBuffer(*this, TEXT("Global Buffer"))
	, PositionVertexBuffer(*this, TEXT("Position VertexBuffer"))
	, ColorVertexBuffer(*this, TEXT("Color VertexBuffer"))
	, UVVertexBuffer(*this, TEXT("UV VertexBuffer"))
	, IndexBuffer(*this, TEXT("Global Index buffer"))
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
		CommandLists.emplace_back();

		auto& AllocatedCommandList = CommandLists[0];
		AllocatedCommandList.AllocateCommandLsit(*this);

		// First commandlist is used as a main command list
		MainCommandList = &CommandLists[0];

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

			RTVDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);


			D3D12_DESCRIPTOR_HEAP_DESC DSVHeapDesc = {};
			DSVHeapDesc.NumDescriptors = 1;
			DSVHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			DSVHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			ThrowIfFailed(Device->CreateDescriptorHeap(&DSVHeapDesc, IID_PPV_ARGS(&DSVHeap)));
			cout << "Descriptor dsv heap creation success" << endl;

			DSVDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

			{
				// Create the depth/stencil buffer and view.
				DepthStencilBuffer = CreateTexture2DResource(TEXT("MainDepthStencil"), EResourceFlag::DepthStencilTarget, DepthFormat, MWindow::Get().GetWidth(), MWindow::Get().GetHeight());
			}

			{
				CD3DX12_CPU_DESCRIPTOR_HANDLE DSVHandle(DSVHeap->GetCPUDescriptorHandleForHeapStart());
				Device->CreateDepthStencilView(DepthStencilBuffer.Get(), nullptr, DSVHandle);
			}


			// Describe and create a constant buffer view (CBV) descriptor heap.
			// Flags indicate that this descriptor heap can be bound to the pipeline 
			// and that descriptors contained in it can be referenced by a root table.
			D3D12_DESCRIPTOR_HEAP_DESC CbvHeapDesc = {};
			CbvHeapDesc.NumDescriptors = 1;
			CbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			CbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			ThrowIfFailed(Device->CreateDescriptorHeap(&CbvHeapDesc, IID_PPV_ARGS(&CBVHeap)));
			cout << "Descriptor cbv heap creation success" << endl;

			CBVSRVUAVDescriptorSize = Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


			{
				DynamicBuffer.Allocate(D3D12MaxConstantBufferSize);
				CD3DX12_CPU_DESCRIPTOR_HANDLE CBVHandle(CBVHeap->GetCPUDescriptorHandleForHeapStart());
				auto CBVDesc = DynamicBuffer.GetCBVDesc();

				// Insepct this function
				Device->CreateConstantBufferView(&CBVDesc, CBVHandle);
				
				cout << " Global Dynamic Buffer CBV generation success " << endl;
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

		// Create an empty root signature.
		{
			D3D12_FEATURE_DATA_ROOT_SIGNATURE FeatureData = {};

			// This is the highest version the sample supports. If CheckFeatureSupport succeeds, the HighestVersion returned will not be greater than this.
			FeatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_1;

			if (FAILED(Device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &FeatureData, sizeof(FeatureData))))
			{
				cout << " Root signature 1_1 failed " << endl;
				FeatureData.HighestVersion = D3D_ROOT_SIGNATURE_VERSION_1_0;
			}

			CD3DX12_DESCRIPTOR_RANGE1 Ranges[1];
			Ranges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC);

			CD3DX12_ROOT_PARAMETER1 RootParameters[1];
			RootParameters[0].InitAsDescriptorTable(1, &Ranges[0], D3D12_SHADER_VISIBILITY_VERTEX);

			// Allow input layout and deny uneccessary access to certain pipeline stages.
			D3D12_ROOT_SIGNATURE_FLAGS RootSignatureFlags =
				D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;


			CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC RootSignatureDesc;
			RootSignatureDesc.Init_1_1(_countof(RootParameters), RootParameters, 0, nullptr, RootSignatureFlags);

			ComPtr<ID3DBlob> Signature;
			ComPtr<ID3DBlob> Error;
			ThrowIfFailed(D3DX12SerializeVersionedRootSignature(&RootSignatureDesc, FeatureData.HighestVersion, &Signature, &Error));
			ThrowIfFailed(Device->CreateRootSignature(0, Signature->GetBufferPointer(), Signature->GetBufferSize(), IID_PPV_ARGS(&RootSignature)));

			cout << "Rootsignature creation success" << endl;
		}


		// Create the pipeline state, which includes compiling and loading shaders.
		{
			ComPtr<ID3DBlob> VertexShader;
			ComPtr<ID3DBlob> PixelShader;

#if defined(_DEBUG)
			// Enable better shader debugging with the graphics debugging tools.
			UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
			UINT compileFlags = 0;
#endif


			ComPtr<ID3DBlob> Error;
			D3DCompileFromFile(TEXT("C:/Users/dnjfd/Desktop/Collection/RyoikiTenaki/Sinkansoai/Sources/Render/Shaders/SimpleRendering.hlsl"), nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &VertexShader, &Error);

			if (Error)
			{
				cout << (char*)Error->GetBufferPointer() << endl;
				Error->Release();


				ThrowIfFailed(S_FALSE);
			}


			D3DCompileFromFile(TEXT("C:/Users/dnjfd/Desktop/Collection/RyoikiTenaki/Sinkansoai/Sources/Render/Shaders/SimpleRendering.hlsl"), nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &PixelShader, &Error);
		
			if (Error)
			{
				cout << (char*)Error->GetBufferPointer() << endl;
				Error->Release();


				ThrowIfFailed(S_FALSE);
			}


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
			PSODesc.pRootSignature = RootSignature.Get();
			PSODesc.VS = CD3DX12_SHADER_BYTECODE(VertexShader.Get());
			PSODesc.PS = CD3DX12_SHADER_BYTECODE(PixelShader.Get());
			PSODesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
			PSODesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
			PSODesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
			PSODesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_GREATER_EQUAL;

			PSODesc.DSVFormat = DepthFormat;

			PSODesc.SampleMask = UINT_MAX;
			PSODesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			PSODesc.NumRenderTargets = 1;
			PSODesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			PSODesc.SampleDesc.Count = 1;
			ThrowIfFailed(Device->CreateGraphicsPipelineState(&PSODesc, IID_PPV_ARGS(&PipelineStateObject)));

			cout << " PSO Createion done" << endl;
		}



		// Create testing vertex buffer.
#if 1
		{
			//MMesh Mesh = MGeometryGenerator::Get().GenerateBox(50, 50, 50);
			MMesh Mesh = MMeshBuilder::Get().LoadMesh(TEXT("C:/Users/dnjfd/Desktop/Collection/RyoikiTenaki/Sinkansoai/Resources/sponza.obj"));

			PositionVertexBuffer.SetRawVertexBuffer(Mesh.RenderData.Positions);
			PositionVertexBuffer.AllocateResource();

			UVVertexBuffer.SetRawVertexBuffer(Mesh.RenderData.UV0);
			UVVertexBuffer.AllocateResource();

			IndexBuffer.SetIndexBuffer(Mesh.RenderData.Indices);
			IndexBuffer.AllocateResource();
			
			cout << " Test vertex buffer creation " << endl;
		}
#else
		{
			//MMesh Mesh = MGeometryGenerator::Get().GenerateBox(100, 100, 100);
			MMesh Mesh = MMeshBuilder::Get().LoadMesh(TEXT("C:/Users/dnjfd/Desktop/Collection/RyoikiTenaki/Sinkansoai/Resources/cube_with_uv.obj"));
			//cube_with_uv
			PositionVertexBuffer.SetRawVertexBuffer(Mesh.RenderData.Positions);
			PositionVertexBuffer.AllocateResource();

			UVVertexBuffer.SetRawVertexBuffer(Mesh.RenderData.UV0);
			UVVertexBuffer.AllocateResource();

			//ColorVertexBuffer.SetRawVertexBuffer(Mesh.RenderData.Colors);
			//ColorVertexBuffer.AllocateResource();

			IndexBuffer.SetIndexBuffer(Mesh.RenderData.Indices);
			IndexBuffer.AllocateResource();

			cout << " Test vertex buffer creation " << endl;
		}
#endif 
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
		}


		/// <summary>
		///  Should make below as function of the batched barrier.
		/// </summary>
		auto& CommandAllocator = CommandLists[0].CommandAllocator;
		auto& CommandList = CommandLists[0].CommandList;

		CommandLists[0].Reset();

		// Indicate that the back buffer will now be used to present.
		auto Barrier = CD3DX12_RESOURCE_BARRIER::Transition(DepthStencilBuffer.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_READ);
		CommandLists[0].CommandList-> ResourceBarrier(1, &Barrier);

		CommandLists[0].Close();

		// Execute the command list.
		ID3D12CommandList* ppCommandLists[] = { CommandList };
		CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
		WaitForPreviousFence();
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

	//if (Device)
	//{
	//	Device->Release();
	//}
	Device = nullptr;

	cout << "D3D12 Renderbackend Exit" << endl;
}

void RRenderBackendD3D12::FunctionalityTestRender()
{
	auto& CommandAllocator = CommandLists[0].CommandAllocator;
	auto& CommandList = CommandLists[0].CommandList;

	CommandLists[0].Reset();

	// Set necessary state.
	CommandList->SetGraphicsRootSignature(RootSignature.Get());
	CommandList->SetPipelineState(PipelineStateObject.Get());

	ID3D12DescriptorHeap* Heaps[] = { CBVHeap.Get() };
	CommandList->SetDescriptorHeaps(_countof(Heaps), Heaps);
	CommandList->SetGraphicsRootDescriptorTable(0, CBVHeap->GetGPUDescriptorHandleForHeapStart());

	CommandList->RSSetViewports(1, &Viewport);
	CommandList->RSSetScissorRects(1, &ScissorRect);

	// Indicate that the back buffer will be used as a render target.
	{
		CD3DX12_RESOURCE_BARRIER Barriers[] = 
		{
			CD3DX12_RESOURCE_BARRIER::Transition(RenderTargets[FrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET),
			CD3DX12_RESOURCE_BARRIER::Transition(DepthStencilBuffer.Get(), D3D12_RESOURCE_STATE_DEPTH_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE)
		};
		CommandList->ResourceBarrier(2, Barriers);
	}

	CD3DX12_CPU_DESCRIPTOR_HANDLE RTVHandle(RTVHeap->GetCPUDescriptorHandleForHeapStart(), FrameIndex, RTVDescriptorSize);
	CD3DX12_CPU_DESCRIPTOR_HANDLE DSVHandle(DSVHeap->GetCPUDescriptorHandleForHeapStart());

	CommandList->OMSetRenderTargets(1, &RTVHandle, FALSE, &DSVHandle);

	// Record commands.
	const float ClearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	CommandList->ClearRenderTargetView(RTVHandle, ClearColor, 0, nullptr);
	CommandList->ClearDepthStencilView(DSVHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 0, 0, 0, nullptr);

	CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	CommandList->IASetVertexBuffers(0, 1, &PositionVertexBuffer.GetVertexBufferView());
	CommandList->IASetVertexBuffers(1, 1, &UVVertexBuffer.GetVertexBufferView());

	CommandList->IASetIndexBuffer(&IndexBuffer.GetIndexBufferView());
	CommandList->DrawIndexedInstanced(IndexBuffer.GetNumIndices(), 1, 0, 0, 0);


	{
		CD3DX12_RESOURCE_BARRIER Barriers[] =
		{
			CD3DX12_RESOURCE_BARRIER::Transition(RenderTargets[FrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT),
			CD3DX12_RESOURCE_BARRIER::Transition(DepthStencilBuffer.Get(), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_DEPTH_READ)
		};
		CommandList->ResourceBarrier(2, Barriers);
	}

	CommandLists[0].Close();

	// Execute the command list.
	ID3D12CommandList* ppCommandLists[] = { CommandList };
	CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Present the frame.
	ThrowIfFailed(SwapChain->Present(1, 0));

	WaitForPreviousFence();
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

	TRefCountPtr<ID3D12Resource> OutResource;
	ThrowIfFailed(Device->CreateCommittedResource(
		&HeapProperty,
		D3D12_HEAP_FLAG_NONE,
		&Desc,
		D3D12_RESOURCE_STATE_COMMON,
		&ClearValue,
		IID_PPV_ARGS(&OutResource)));

	return OutResource;
}

TRefCountPtr<ID3D12Resource> RRenderBackendD3D12::CreateTexture2DResource(String&& Name, EResourceFlag ResourceFlag, DXGI_FORMAT Format, uint32 Width, uint32 Height)
{
	auto Resource = CreateUnderlyingResource(RenderTexture2D, ResourceFlag, Format, Width, Height, 1, 1);
	Resource->SetName(Name.c_str());
	return Resource;
}