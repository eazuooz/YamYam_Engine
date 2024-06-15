#pragma once

#include "yaEntity.h"
#include "yaComponent.h"

namespace ya
{
	class Transform : public Component
	{
	public:
		Transform();
		virtual ~Transform();

		void Initialize() override;
		void Update() override;
		void LateUpdate() override;
		void Render() override;

		void Bind() const;

		[[nodiscard]] Transform* GetParent() const { return mParent; }
		[[noreturn]] void SetParent(Transform* parent) { mParent = parent; }

		[[nodiscard]] Matrix GetWorldMatrix() const { return mWorldMatrix; }
		[[noreturn]] void SetWorldMatrix(const Matrix& worldMatrix) { mWorldMatrix = worldMatrix; }

		[[nodiscard]] Vector3 GetPosition() const { return mPosition; }
		[[noreturn]] void SetPosition(const Vector3& position) { mPosition = position; }
		[[noreturn]] void SetPosition(const float x, const float y, const float z) { mPosition = Vector3(x, y, z); }

		[[nodiscard]] Vector3 GetRotation() const { return mRotation; }
		[[noreturn]] void SetRotation(const Vector3& rotation) { mRotation = rotation; }
		[[noreturn]] void SetRotation(const float x, const float y, const float z) { mRotation = Vector3(x, y, z); }

		[[nodiscard]] Vector3 GetScale() const { return mScale; }
		[[noreturn]] void SetScale(const Vector3& scale) { mScale = scale; }
		[[noreturn]] void SetScale(const float x, const float y, const float z) { mScale = Vector3(x, y, z); }

		[[nodiscard]] Vector3 Forward() const { return mForward; };
		[[nodiscard]] Vector3 Right() const { return mRight; };
		[[nodiscard]] Vector3 Up() const { return mUp; };

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

	using namespace math;
}
