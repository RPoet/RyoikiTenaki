#pragma once
#include "RenderBackendD3D12/RenderBackendD3D12.h"

IRenderBackend* GBackend = nullptr;

void InitBackend(const String& BackendName)
{
	if (BackendName.compare(TEXT("D3D12")) == 0)
	{
		GBackend = new RRenderBackendD3D12();
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
		GBackend->Exit();
		delete GBackend;
		cout << "Renderbackend correctly tears down" << endl;
	}
}
