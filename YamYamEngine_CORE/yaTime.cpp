#include "yaTime.h"


namespace ya
{
	LARGE_INTEGER Time::CpuFrequency = {};
	LARGE_INTEGER Time::PrevFrequency = {};
	LARGE_INTEGER Time::CurrentFrequency = {};
	float Time::DeltaTimeValue = 0.0f;

	void Time::Initialize()
	{
		// Cpu ���� ������
		QueryPerformanceFrequency(&CpuFrequency);

		// ���α׷��� ���� ���� �� ���� ������
		QueryPerformanceCounter(&PrevFrequency);
	}

	void Time::Update()
	{
		QueryPerformanceCounter(&CurrentFrequency);

		float differenceFrequency
			= static_cast<float>(CurrentFrequency.QuadPart - PrevFrequency.QuadPart);
		DeltaTimeValue = differenceFrequency / static_cast<float>(CpuFrequency.QuadPart);
		PrevFrequency.QuadPart = CurrentFrequency.QuadPart;
	}

	void Time::Render()
	{
		// Do nothing
	}
}
