#include "yaInput.h"
#include "yaApplication.h"

extern ya::Application application;

namespace ya
{
	std::vector<Input::Key> Input::Keys = {};
	Vector2 Input::mMousePosition = Vector2::One;

	int ASCII[static_cast<UINT>(eKeyCode::End)] =
	{
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',
		'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L',
		'Z', 'X', 'C', 'V', 'B', 'N', 'M',
		VK_LEFT, VK_RIGHT, VK_DOWN, VK_UP,
		VK_LBUTTON, VK_MBUTTON, VK_RBUTTON,
		VK_LCONTROL,
	};

	void Input::Initialize()
	{
		createKeys();
	}

	void Input::Update()
	{
		updateKeys();
	}

	void Input::createKeys()
	{
		for (size_t i = 0; i < static_cast<UINT>(eKeyCode::End); i++)
		{
			Key key = {};
			key.bPressed = false;
			key.State = eKeyState::None;
			key.KeyCode = static_cast<eKeyCode>(i);

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
		return GetAsyncKeyState(ASCII[static_cast<UINT>(code)]) & 0x8000;
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
		ScreenToClient(application.GetHwnd(), &mousePos);

		UINT width = application.GetWidth();
		UINT height = application.GetHeight();

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
