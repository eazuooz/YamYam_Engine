#include "yaInput.h"
#include "yaApplication.h"

extern ya::Application application;

namespace ya
{
	std::vector<Input::Key> Input::Keys = {};
	Vector2 Input::mMousePosition = Vector2::One;
	bool Input::mBlocked = false;

	void Input::Initialize()
	{
		createKeys();
	}

	void Input::Update()
	{
		if (mBlocked == true)
			return;

		updateKeys();
	}

	void Input::createKeys()
	{
		for (int vk = 0; vk <= 0xFF; ++vk)  // 0x00 ~ 0xFF 범위의 Virtual Key Code 검사
		{
			eKeyCode keyCode = static_cast<eKeyCode>(vk); // VK_* 값을 eKeyCode로 변환

			Key key = {};
			key.bPressed = false;
			key.State = eKeyState::None;
			key.KeyCode = keyCode;
			key.VK_KeyCode = vk;

			Keys.push_back(key);
		}
	}

	void Input::updateKeys()
	{
		std::ranges::for_each(Keys,
		                      [](Key& key) -> void
		                      {
			                      updateKey(key);
		                      });
	}

	void Input::updateKey(Key& key)
	{
		if (GetFocus())
		{
			if (isKeyDown(key.KeyCode))
				updateKeyDown(key);
			else
				updateKeyUp(key);

			getMousePositionByWindow();
		}
		else
		{
			clearKeys();
		}
	}

	bool Input::isKeyDown(eKeyCode code)
	{
		return GetAsyncKeyState(static_cast<int>(code)) & 0x8000;
	}

	void Input::updateKeyDown(Key& key)
	{
		if (key.bPressed == true)
			key.State = eKeyState::Pressed;
		else
			key.State = eKeyState::Down;

		key.bPressed = true;
	}

	void Input::updateKeyUp(Key& key)
	{
		if (key.bPressed == true)
			key.State = eKeyState::Up;
		else
			key.State = eKeyState::None;

		key.bPressed = false;
	}

	void Input::getMousePositionByWindow()
	{
		POINT mousePos = {};
		GetCursorPos(&mousePos);
		ScreenToClient(application.GetWindow().GetHwnd(), &mousePos);

		UINT width = application.GetWindow().GetWidth();
		UINT height = application.GetWindow().GetHeight();

		mMousePosition.x = -1.0f;
		mMousePosition.y = -1.0f;

		if (static_cast<UINT>(mousePos.x) > 0 && static_cast<UINT>(mousePos.x) < width)
			mMousePosition.x = static_cast<float>(mousePos.x);

		if (static_cast<UINT>(mousePos.y) > 0 && static_cast<UINT>(mousePos.y) < height)
			mMousePosition.y = static_cast<float>(mousePos.y);
	}

	void Input::clearKeys()
	{
		for (Key& key : Keys)
		{
			if (key.State == eKeyState::Down || key.State == eKeyState::Pressed)
				key.State = eKeyState::Up;
			else if (key.State == eKeyState::Up)
				key.State = eKeyState::None;

			key.bPressed = false;
		}
	}
}
