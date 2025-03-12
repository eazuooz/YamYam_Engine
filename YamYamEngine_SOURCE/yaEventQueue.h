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

		// 특정 이벤트 타입에 대한 핸들러 등록
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
				
				// 등록된 핸들러 실행
				auto handler = mHandlers.find(event->GetEventType());
				if (handler != mHandlers.end())
				{
					event->Handled != handler->second(*event);
				}

				// 기본 핸들러 실행 (처리되지 않은 경우)
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
