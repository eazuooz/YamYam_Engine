#pragma once
#include "yaGraphicDevice_DX12.h"

namespace ya::graphics
{
	class InputLayout
	{
	public:
		InputLayout();
		~InputLayout();

		//void CreateInputLayout(UINT vertexCount, D3D11_INPUT_ELEMENT_DESC* layout,
		//                       const void* pShaderBytecodeWithInputSignature, SIZE_T BytecodeLength);
		void Bind() const;

	private:
		//Microsoft::WRL::ComPtr<ID3D11InputLayout> mInputLayout;
	};
}
