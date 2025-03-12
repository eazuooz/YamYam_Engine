#pragma once
#include "yaEvent.h"

namespace ya
{
	class EventQueue
	{
	public:
		EventQueue();
		~EventQueue();

		//template <typename T>
		//void RegisterHandler(const EventCallbackFn& callback)
		//{
		//	mCallback = callback;
		//}

		// Ư�� �̺�Ʈ Ÿ�Կ� ���� �ڵ鷯 ���
        template<typename T>
        void RegisterHandler(std::function<bool(T&)> handler)
        {
            mHandlers[T::GetStaticType()] = [handler](Event& e) -> bool {
                return handler(static_cast<T&>(e));
            };
        }

		void Push(Event* event)
		{
			mQueue.push(event);
		}

		void Process()
		{
			while (!mQueue.empty())
			{
				Event* event = mQueue.front();
				mQueue.pop();
				
				EventDispatcher dispatcher(*event);
				
				// ��ϵ� �ڵ鷯 ����
				auto handler = mHandlers.find(event->GetEventType());
				if (handler != mHandlers.end())
				{
					event->Handled != handler->second(*event);
				}

				// �⺻ �ڵ鷯 ���� (ó������ ���� ���)
				if (!event->Handled && mCallback)
				{
					mCallback(*event);
				}
				
				delete event;
				event = nullptr;
			}
		}

		void SetCallback(const EventCallbackFn& callback)
		{
			mCallback = callback;
		}


	private:
		std::queue<Event*> mQueue;
		EventCallbackFn mCallback;
		std::unordered_map<eEventType, HandlerCallbackFn> mHandlers;
	};
}
