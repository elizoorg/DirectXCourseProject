#pragma once
#include <string>
#include <windows.h>
#include <d3d12.h>
#include <wrl.h>
#include <d3dcompiler.h>
#include "com_exception.h"


using Microsoft::WRL::ComPtr;

ComPtr<ID3DBlob> CompileShader(const std::wstring& filename, const std::string& entryPoint, const std::string& target)
{
    UINT compileFlags = 0;
#if defined(_DEBUG)
    compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ComPtr<ID3DBlob> byteCode = nullptr;
    ComPtr<ID3DBlob> errors;

    HRESULT hr = D3DCompileFromFile(
        filename.c_str(),
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        entryPoint.c_str(),
        target.c_str(),
        compileFlags,
        0,
        &byteCode,
        &errors);

    if (errors != nullptr)
    {
        OutputDebugStringA((char*)errors->GetBufferPointer());
    }

    DX::ThrowIfFailed(hr);
    return byteCode;
}