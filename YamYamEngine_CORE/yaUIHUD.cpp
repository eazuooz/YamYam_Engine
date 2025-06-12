#include "yaUIHUD.h"
#include "yaResources.h"


namespace ya
{
	UIHUD::UIHUD()
		: UIBase(eUIType::HpBar)
	{
	}

	UIHUD::~UIHUD()
	{
	}

	void UIHUD::OnInit()
	{
		mTexture = Resources::Find<graphics::Texture>(L"HPBAR");
	}

	void UIHUD::OnActive()
	{
		//int* p = new int;
	}

	void UIHUD::OnInActive()
	{
	}

	void UIHUD::OnUpdate()
	{
	}

	void UIHUD::OnLateUpdate()
	{
	}

	void UIHUD::OnRender()
	{
		//TransparentBlt(hdc
		//	, 0, 0
		//	, mTexture->GetWidth()
		//	, mTexture->GetHeight()
		//	, mTexture->GetHdc()
		//	, 0, 0
		//	, (float)mTexture->GetWidth() / 2.0f
		//	, (float)mTexture->GetHeight()
		//	, RGB(255, 0, 255));
	}

	void UIHUD::OnClear()
	{
		//delete p;
	}

}
