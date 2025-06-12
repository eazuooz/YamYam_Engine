#include "yaEditorCamera.h"

namespace ya
{
	EditorCamera::EditorCamera()
		: Camera()
		, mRenderTarget(nullptr)
	{

	}
	EditorCamera::~EditorCamera()
	{
		SAFE_DELETE(mRenderTarget);
	}

	void EditorCamera::Initialize()
	{
		Camera::Initialize();
	}

	void EditorCamera::Update()
	{
		Camera::Update();
	}

	void EditorCamera::LateUpdate()
	{
		Camera::LateUpdate();
	}

	void EditorCamera::Render(const Matrix& view, const Matrix& projection)
	{
		Camera::Render(view, projection);
	}

	void EditorCamera::OnEvent(Event& e)
	{

	}

	void EditorCamera::CreateRenderTarget(UINT width, UINT height)
	{
		if (mRenderTarget != nullptr)
			delete mRenderTarget;

		graphics::RenderTargetSpecification spec;
		spec.Width = width;
		spec.Height = height;

		spec.Attachments = { graphics::eRenderTragetFormat::RGBA8, graphics::eRenderTragetFormat::Depth };

		mRenderTarget = new graphics::RenderTarget(spec);
	}
}
