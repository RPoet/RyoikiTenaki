#include "RenderBackend.h"
#include "RenderBackendD3D12/RenderBackendD3D12.h"

RRenderBackend* GBackend = nullptr;

void InitBackend(const String& BackendName)
{
	if (BackendName.compare(TEXT("D3D12")) == 0)
	{
		GBackend = &RRenderBackendD3D12::Get();
		cout << "D3D12 backend selected" << endl;
	}

	if (GBackend)
	{
		GBackend->Init();
	}
}

void TeardownBackend()
{
	if (GBackend)
	{
		GBackend->Teardown();
		cout << "Renderbackend correctly tears down" << endl;
	}
	GBackend = nullptr;
}
