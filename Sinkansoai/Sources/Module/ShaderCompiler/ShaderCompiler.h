#pragma once
#include "../../Module.h"

#include <d3dcompiler.h>

class MShaderCompiler: public MModuleBase
{
	MODULE_CLASS_DECORATOR(MShaderCompiler)
private:
public:

	virtual void Init() override;

	virtual void Teardown() override;

	ID3DBlob* CompileShader(String&& FileName, String&& Entry, EShaderType ShaderType);
	ID3DBlob* CompileShader(String&& FileName, String&& Entry, const vector<D3D_SHADER_MACRO>& Defines, EShaderType ShaderType);
};

