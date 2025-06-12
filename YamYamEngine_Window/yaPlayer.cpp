#include "yaPlayer.h"
#include <yaInput.h>
#include <yaTransform.h>
#include <yaTime.h>

namespace ya
{
	void Player::Initialize()
	{
		GameObject::Initialize();
	}

	void Player::Update()
	{
		GameObject::Update();
	}

	void Player::LateUpdate()
	{
		GameObject::LateUpdate();
	}

	void Player::Render(const Matrix& view, const Matrix& projection)
	{
		GameObject::Render(view, projection);
	}
}
