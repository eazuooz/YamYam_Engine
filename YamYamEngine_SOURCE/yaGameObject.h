#pragma once
#include "CommonInclude.h"

namespace ya
{
	class GameObject
	{
	public:
		GameObject();
		~GameObject();

		void Update();
		void LateUpdate();
		void Render(HDC hdc);

		void SetPosition(float x, float y)
		{
			mX = x;
			mY = y;
		}

		float GetPositionX() { return mX; }
		float GetPositionY() { return mY; }


	private:
		// ���ӿ�����Ʈ�� ��ǥ
		float mX;
		float mY;
	};
}
