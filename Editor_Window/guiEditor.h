#pragma once
#include "guiEntity.h"


namespace gui
{
	/// <summary>
	/// Editor�� Ư�� ������Ʈ�� Inspector�� Ŀ���͸���¡�ϴ� �� ���˴ϴ�.
	/// Inspector â�� ǥ�õǴ� UI�� �������ϰų� Ȯ���մϴ�.
	/// </summary>
	class Editor : public Labelled
	{
	public:
		enum class eState
		{
			Paused,
			Active,
			Disabled,
			Destroyed
		};

		Editor();
		virtual ~Editor();
		
		virtual void Initialize();
		virtual void Update();
		virtual void OnGUI(); 
		virtual void OnEnable();
		virtual void OnDisable();
		virtual void OnDestroy();

	private:
		eState State;
	};
}





