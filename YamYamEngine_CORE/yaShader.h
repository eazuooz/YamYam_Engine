#pragma once
#include "yaResource.h"
#include "yaGraphicDevice_DX12.h"

namespace ya::graphics
{
	class Shader : public Resource
	{
	public:
		Shader();
		virtual ~Shader();

		HRESULT Save(const std::wstring& path) override;
		HRESULT Load(const std::wstring& path) override;

		bool Create(eShaderStage stage, const std::wstring& fileName);
		bool CreateVertexShader(const std::wstring& fileName);
		bool CreatePixelShader(const std::wstring& fileName);

		void Bind();

		//Microsoft::WRL::ComPtr<ID3DBlob> GetVSBlob() const { return mVSBlob; }
		//Microsoft::WRL::ComPtr<ID3DBlob> GetHSBlob() const { return mHSBlob; }
		//Microsoft::WRL::ComPtr<ID3DBlob> GetDSBlob() const { return mDSBlob; }
		//Microsoft::WRL::ComPtr<ID3DBlob> GetGSBlob() const { return mGSBlob; }
		//Microsoft::WRL::ComPtr<ID3DBlob> GetPSBlob() const { return mPSBlob; }
		//
		//Microsoft::WRL::ComPtr<ID3D11VertexShader> GetVS() const { return mVS; }
		//Microsoft::WRL::ComPtr<ID3D11HullShader> GetHS() const { return mHS; }
		//Microsoft::WRL::ComPtr<ID3D11DomainShader> GetDS() const { return mDS; }
		//Microsoft::WRL::ComPtr<ID3D11GeometryShader> GetGS() const { return mGS; }
		//Microsoft::WRL::ComPtr<ID3D11PixelShader> GetPS() const { return mPS; }

		void SetRasterizerState(const eRasterizerState state) { mRasterizerState = state; }
		void SetBlendState(const eBlendState state) { mBlendState = state; }
		void SetDepthStencilState(const eDepthStencilState state) { mDepthStencilState = state; }

	private:
		static bool bWireframe;

		//Microsoft::WRL::ComPtr<ID3DBlob> mVSBlob;
		//Microsoft::WRL::ComPtr<ID3DBlob> mHSBlob;
		//Microsoft::WRL::ComPtr<ID3DBlob> mDSBlob;
		//Microsoft::WRL::ComPtr<ID3DBlob> mGSBlob;
		//Microsoft::WRL::ComPtr<ID3DBlob> mPSBlob;

		//Microsoft::WRL::ComPtr<ID3D11VertexShader> mVS;
		//Microsoft::WRL::ComPtr<ID3D11HullShader> mHS;
		//Microsoft::WRL::ComPtr<ID3D11DomainShader> mDS;
		//Microsoft::WRL::ComPtr<ID3D11GeometryShader> mGS;
		//Microsoft::WRL::ComPtr<ID3D11PixelShader> mPS;

		eRasterizerState mRasterizerState;
		eBlendState mBlendState;
		eDepthStencilState mDepthStencilState;
	};
}
