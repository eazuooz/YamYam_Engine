#pragma once
#include "guiEntity.h"


namespace gui
{
	/// <summary>
	/// Editor는 특정 컴포넌트의 Inspector를 커스터마이징하는 데 사용됩니다.
	/// Inspector 창에 표시되는 UI를 재정의하거나 확장합니다.
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





