#pragma once
#include "yaTexture.h"

namespace ya::graphics
{
	// ����Ÿ���� ������ �����ϴ� ������
	enum class eRenderTragetFormat
	{
		None = 0,

		// Color
		RGBA8,

		// RED ä�ο� ������ ������ ���� (������ ������ ������ �뵵�� ���)
		RED_INTEGER,

		// Depth/stencil
		DEPTH24STENCIL8,

		// Shader resource
		SHADER_RESOURCE,

		// Defaults
		Depth = DEPTH24STENCIL8
	};

	// ����Ÿ���� ��� ������ �����ϴ� ����ü
	struct RenderTargetTextureSpecification
	{
		RenderTargetTextureSpecification() = default;
		RenderTargetTextureSpecification(eRenderTragetFormat format)
			: TextureFormat(format) {}

		eRenderTragetFormat TextureFormat = eRenderTragetFormat::None;
		// TODO: filtering/wrap
	};

	// ����Ÿ�ٵ�(������)�� �������� �����ϴ� ����ü
	struct RenderTargetAttachmentSpecification
	{
		RenderTargetAttachmentSpecification() = default;
		RenderTargetAttachmentSpecification(std::initializer_list<RenderTargetTextureSpecification> attachments)
			: Attachments(attachments) {}

		std::vector<RenderTargetTextureSpecification> Attachments;
	};

	struct RenderTargetSpecification
	{
		UINT Width = 0, Height = 0;
		RenderTargetAttachmentSpecification Attachments;
		UINT Samples = 1;

		bool SwapChainTarget = false;
	};

	class RenderTarget
	{
	public:
		RenderTarget(const RenderTargetSpecification& spec);
		virtual ~RenderTarget();

		static RenderTarget* Create(const RenderTargetSpecification& spec);

		void Invalidate();
		void Bind();
		void Unbind();

		void Resize(UINT width, UINT height);
		int ReadPixel(uint32_t attachmentIndex, int x, int y);
		
		void ClearAttachment(UINT index, const void* value);

		Texture* GetAttachmentTexture(UINT index) { if (index >= mAttachments.size()) assert(NULL && "Rendertarget is not exist"); return mAttachments[index]; }
		Texture* GetDepthAttachment() { return mDepthAttachment; }
		RenderTargetSpecification& GetSpecification() { return mSpecification; }

	private:
		RenderTargetSpecification mSpecification;

		std::vector<RenderTargetTextureSpecification> mSpecifications;
		RenderTargetTextureSpecification mDepthAttachmentSpecification; 

		std::vector<Texture*> mAttachments;
		Texture* mDepthAttachment;
	};
}


