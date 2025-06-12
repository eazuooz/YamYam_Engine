#include "yaFloor.h"
#include "yaCat.h"

namespace ya
{
	Floor::Floor()
	{
	}

	Floor::~Floor()
	{
	}

	void Floor::Initialize()
	{
		GameObject::Initialize();
	}

	void Floor::Update()
	{
		GameObject::Update();
	}

	void Floor::LateUpdate()
	{
		GameObject::LateUpdate();
	}

	void Floor::Render(const Matrix& view, const Matrix& projection)
	{
		GameObject::Render(view, projection);
	}
}
