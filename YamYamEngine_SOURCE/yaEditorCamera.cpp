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

}
