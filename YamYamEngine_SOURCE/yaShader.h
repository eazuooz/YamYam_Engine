#pragma once
#include "yaResource.h"
#include "yaGraphicDevice_DX11.h"

namespace ya::graphics
{
	class Shader : public Resource
	{
	public:
		Shader();
		virtual ~Shader();

		virtual HRESULT Save(const std::wstring& path) override;
		virtual HRESULT Load(const std::wstring& path) override;
		
		bool Create(const eShaderStage stage, const std::wstring& fileName);
		bool CreateVertexShader(const std::wstring& fileName);
		bool CreatePixelShader(const std::wstring& fileName);

		void Bind();

		Microsoft::WRL::ComPtr<ID3DBlob> GetVSBlob() { return mVSBlob; }
		Microsoft::WRL::ComPtr<ID3DBlob> GetHSBlob() { return mHSBlob; }
		Microsoft::WRL::ComPtr<ID3DBlob> GetDSBlob() { return mDSBlob; }
		Microsoft::WRL::ComPtr<ID3DBlob> GetGSBlob() { return mGSBlob; }
		Microsoft::WRL::ComPtr<ID3DBlob> GetPSBlob() { return mPSBlob; }

		void SetRasterizerState(const eRasterizerState state) { mRasterizerState = state; }
		void SetBlendState(const eBlendState state) { mBlendState = state; }
		void SetDepthStencilState(const eDepthStencilState state) { mDepthStencilState = state; }

	private:
		Microsoft::WRL::ComPtr<ID3DBlob> mVSBlob;
		Microsoft::WRL::ComPtr<ID3DBlob> mHSBlob;
		Microsoft::WRL::ComPtr<ID3DBlob> mDSBlob;
		Microsoft::WRL::ComPtr<ID3DBlob> mGSBlob;
		Microsoft::WRL::ComPtr<ID3DBlob> mPSBlob;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> mVS;
		Microsoft::WRL::ComPtr<ID3D11HullShader> mHS;
		Microsoft::WRL::ComPtr<ID3D11DomainShader> mDS;
		Microsoft::WRL::ComPtr<ID3D11GeometryShader> mGS;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> mPS;

		graphics::eRasterizerState mRasterizerState;
		graphics::eBlendState mBlendState;
		graphics::eDepthStencilState mDepthStencilState;
	};
}
