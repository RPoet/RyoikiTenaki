#include "ShaderCompiler.h"
#include "../../Engine.h"

#pragma comment ( lib, "D3DCompiler.lib")

IMPLEMENT_MODULE(MShaderCompiler)

void MShaderCompiler::Init()
{
	Super::Init();
	
	cout << "Shader Compiler Init" << endl;

}

void MShaderCompiler::Teardown()
{
	Super::Teardown();
}


ID3DBlob* MShaderCompiler::CompileShader(String&& FileName, String&& Entry, const vector<D3D_SHADER_MACRO>& Defines, EShaderType ShaderType)
{
#if defined(_DEBUG)
	// Enable better shader debugging with the graphics debugging tools.
	UINT CompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	LPCSTR Target{};
	switch (ShaderType)
	{
	case VS: //VS 
		Target = "vs_5_1";
		break;
	case PS: //PS 
		Target = "ps_5_1";
		break;
	default:
		assert(false && "No entry");
		break;
	}

	ID3DBlob* Shader{};
	ID3DBlob* Error{};
	D3DCompileFromFile(FileName.c_str(), Defines.size() > 0 ? &Defines[0] : nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, CharString(Entry.begin(), Entry.end()).c_str(), Target, CompileFlags, 0, &Shader, &Error);

	if (Error)
	{
		cout << (char*)Error->GetBufferPointer() << endl;
		Error->Release();
		Error = nullptr;

		assert(false && "Shader compile error");
	}

	return Shader;
}

ID3DBlob* MShaderCompiler::CompileShader(String&& FileName, String&& Entry, EShaderType ShaderType)
{
	const vector<D3D_SHADER_MACRO> Defines{};
	return CompileShader(move(FileName), move(Entry), Defines, ShaderType);
}
