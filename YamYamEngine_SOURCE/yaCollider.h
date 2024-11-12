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

		eColliderType GetColliderType() const { return mType; }
		UINT32 GetID() const { return mID; }

		Vector2 GetOffset() const { return mOffset; }
		void SetOffset(const Vector2& offset) { mOffset = offset; }

		Vector2 GetSize() const { return mSize; }
		void SetSize(const Vector2& size) { mSize = size; }

	private:
		static UINT CollisionID;

		UINT32 mID;
		Vector2 mOffset;
		Vector2 mSize;
		eColliderType mType;
	};
}
