#pragma once
#include "yaComponent.h"


namespace ya
{
	class Collider : public Component
	{
	public:
		Collider(eColliderType type);
		virtual ~Collider();

		void Initialize() override;
		void Update() override;
		void LateUpdate() override;
		void Render() override;

		virtual void OnCollisionEnter(Collider* other);
		virtual void OnCollisionStay(Collider* other);
		virtual void OnCollisionExit(Collider* other);

		[[nodiscard]] eColliderType GetColliderType() const { return mType; }
		[[nodiscard]] UINT32 GetID() const { return mID; }

		[[nodiscard]] Vector2 GetOffset() const { return mOffset; }
		[[noreturn]] void SetOffset(const Vector2& offset) { mOffset = offset; }

		[[nodiscard]] Vector2 GetSize() const { return mSize; }
		[[noreturn]] void SetSize(const Vector2& size) { mSize = size; }

	private:
		static UINT CollisionID;

		UINT32 mID;
		Vector2 mOffset;
		Vector2 mSize;
		eColliderType mType;
	};
}
