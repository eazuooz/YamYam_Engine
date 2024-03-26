#include "yaUIButton.h"
#include "yaInput.h"

namespace ya
{
	UIButton::UIButton()
		: UIBase(eUIType::Button)
	{

	}

	UIButton::~UIButton()
	{
	}

	void UIButton::OnInit()
	{
		SetPos(Vector2(200.0f, 200.0f));
		SetSize(Vector2(200.0f, 200.0f));

		mOnClick = std::bind(&UIButton::ButtonClick, this);
	}

	void UIButton::OnActive()
	{
	}

	void UIButton::OnInActive()
	{
	}

	void UIButton::OnUpdate()
	{
		Vector2 mousePos = Input::GetMousePosition();
		
		if (mPosition.x <= mousePos.x && mousePos.x <= mPosition.x + mSize.x
			&& mPosition.y <= mousePos.y && mousePos.y <= mPosition.y + mSize.y)
		{
			mbMouseOn = true;
		}
		else
		{
			mbMouseOn = false;
		}

		if (Input::GetKeyDown(eKeyCode::LButton))
		{
			if (mbMouseOn)
			{
				mOnClick();
			}
		}
	}

	void UIButton::OnLateUpdate()
	{
	}

	void UIButton::OnRender()
	{
		
	}

	void UIButton::OnClear()
	{
		
	}

	void UIButton::ButtonClick()
	{
		int a = 0;
	}

}