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

		Transform* tr = GetComponent<Transform>();
		 
		//Translate
		Vector3 pos = tr->GetPosition();
		 if (Input::GetKey(eKeyCode::W))
			 pos.y += 30.0f * Time::DeltaTime();
		 if (Input::GetKey(eKeyCode::S))
			 pos.y -= 30.0f * Time::DeltaTime();
		 if (Input::GetKey(eKeyCode::A))
			 pos.x -= 30.0f * Time::DeltaTime();
		 if (Input::GetKey(eKeyCode::D))
			 pos.x += 30.0f * Time::DeltaTime();
		 
		 tr->SetPosition(pos);

		 //Rotate
		 Vector3 rot = tr->GetRotation();
		 if (Input::GetKey(eKeyCode::Q))
			 rot.z += 90.0f * Time::DeltaTime();
		 if (Input::GetKey(eKeyCode::E))
			 rot.z -= 90.0f * Time::DeltaTime();

		 tr->SetRotation(rot);
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
