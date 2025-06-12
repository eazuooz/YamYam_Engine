#pragma once
#include "yaEvent.h"
#include "yaInput.h"



namespace ya
{
	class KeyEvent : public Event
	{
	public:
		eKeyCode GetKeyCode() const { return mKeyCode; }

		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryInput)
	protected:
		KeyEvent(const eKeyCode keycode)
			: mKeyCode(keycode) {}

		eKeyCode mKeyCode;
	};

	class KeyPressedEvent : public KeyEvent
	{
	public:
		KeyPressedEvent(const eKeyCode keycode, bool isRepeat = false)
			: KeyEvent(keycode), mIsRepeat(isRepeat) {}

		bool IsRepeat() const { return mIsRepeat; }

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyPressedEvent: " << static_cast<int>(mKeyCode) << " (repeat = " << mIsRepeat << ")";
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyPressed)
	private:
		bool mIsRepeat;
	};

	class KeyReleasedEvent : public KeyEvent
	{
	public:
		KeyReleasedEvent(const eKeyCode keycode)
			: KeyEvent(keycode) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << static_cast<int>(mKeyCode);
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyReleased)
	};

	class KeyTypedEvent : public KeyEvent
	{
	public:
		KeyTypedEvent(const eKeyCode keycode)
			: KeyEvent(keycode) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "KeyTypedEvent: " << static_cast<int>(mKeyCode);
			return ss.str();
		}

		EVENT_CLASS_TYPE(KeyTyped)
	};
}