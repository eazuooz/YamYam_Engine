#pragma once

#include "yaLabelled.h"
#include "yaComponent.h"

namespace ya
{
	using namespace math;

	class Transform : public Component
	{
	public:
		Transform();
		virtual ~Transform();

		void Initialize() override;
		void Update() override;
		void LateUpdate() override;
		void Render(const Matrix& view, const Matrix& projection) override;

		void Bind(const Matrix& view, const Matrix& projection) const;

		Transform* GetParent() const { return mParent; }
		void SetParent(Transform* parent) { mParent = parent; }

		Matrix GetWorldMatrix() const { return mWorldMatrix; }
		void SetWorldMatrix(const Matrix& worldMatrix) { mWorldMatrix = worldMatrix; }

		Vector3 GetPosition() const { return mPosition; }
		void SetPosition(const Vector3& position) { mPosition = position; }
		void SetPosition(const float x, const float y, const float z) { mPosition = Vector3(x, y, z); }

		Vector3 GetRotation() const { return mRotation; }
		void SetRotation(const Vector3& rotation) { mRotation = rotation; }
		void SetRotation(const float x, const float y, const float z) { mRotation = Vector3(x, y, z); }

		Vector3 GetScale() const { return mScale; }
		void SetScale(const Vector3& scale) { mScale = scale; }
		void SetScale(const float x, const float y, const float z) { mScale = Vector3(x, y, z); }

		Vector3 Forward() const { return mForward; };
		Vector3 Right() const { return mRight; };
		Vector3 Up() const { return mUp; };

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
