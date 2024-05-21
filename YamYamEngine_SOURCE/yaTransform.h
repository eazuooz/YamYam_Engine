#pragma once

#include "yaEntity.h"
#include "yaComponent.h"

namespace ya
{
	using namespace math;
	class Transform : public Component
	{
	public:
		Transform();
		~Transform();

		void Initialize() override;
		void Update()  override;
		void LateUpdate()  override;
		void Render()  override;

		void Bind();

		Transform* GetParent() { return mParent; }
		const Matrix GetWorldMatrix() { return mWorldMatrix; }

		const Vector3 GetPosition() { return mPosition; }
		const Vector3 GetRotation() { return mRotation; }
		const Vector3 GetScale() { return mScale; }

		void SetPosition(Vector3 position) { mPosition = position; }
		void SetPosition(float x, float y, float z) { mPosition = Vector3(x, y, z); }
		void SetRotation(Vector3 rotation) { mRotation = rotation; }
		void SetRotation(float x, float y, float z) { mRotation = Vector3(x, y, z); }
		void SetScale(Vector3 scale) { mScale = scale; }
		void SetScale(float x, float y, float z) { mScale = Vector3(x, y, z); }

		const Vector3 Foward() { return mForward; };
		const Vector3 Right() { return mRight; };
		const Vector3 Up() { return mUp; };

	private:
		Transform* mParent;
		Matrix mWorldMatrix;

		Vector3 mPosition;
		Vector3 mRotation;
		Vector3 mScale;	

		Vector3 mForward;
		Vector3 mRight;
		Vector3 mUp;
	};
}


