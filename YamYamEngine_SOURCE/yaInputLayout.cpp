#include "yaInputLayout.h"


namespace ya::graphics
{
	InputLayout::InputLayout()
		: mInputLayout(nullptr)

	{
	}

	InputLayout::~InputLayout()
	{
	}

	void InputLayout::CreateInputLayout(UINT vertexCount, D3D11_INPUT_ELEMENT_DESC* layout
	                                    , const void* pShaderBytecodeWithInputSignature, SIZE_T BytecodeLength)
	{
		if (!(GetDevice<GraphicDevice_DX11>()->CreateInputLayout(layout, vertexCount
		                                     , pShaderBytecodeWithInputSignature
		                                     , BytecodeLength
		                                     , mInputLayout.GetAddressOf())))
			assert(NULL && "Create input layout failed!");
	}

	void InputLayout::Bind() const
	{
		if (mInputLayout)
			GetDevice<GraphicDevice_DX11>()->BindInputLayout(mInputLayout.Get());
	}
}
