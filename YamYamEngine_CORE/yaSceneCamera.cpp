#include "yaSceneCamera.h"

namespace ya
{
	SceneCamera::SceneCamera()
		: Camera()
	{

	}
	SceneCamera::~SceneCamera()
	{

	}

	void SceneCamera::Initialize()
	{
		Camera::Initialize();
	}

	void SceneCamera::Update()
	{
		Camera::Update();
	}

	void SceneCamera::LateUpdate()
	{
		Camera::LateUpdate();
	}

	void SceneCamera::Render(const Matrix& view, const Matrix& projection)
	{
		Camera::Render(view, projection);
	}

	void SceneCamera::OnEvent(Event& e)
	{

	}

}
