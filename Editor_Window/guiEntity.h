#pragma once
#include "..\\YamYamEngine_CORE\\CommonInclude.h"
#include "..\\YamYamEngine_CORE\\yaEvent.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

#include "ImGuizmo.h"
#include "ImSequencer.h"
#include "ImZoomSlider.h"
#include "ImCurveEdit.h"
#include "GraphEditor.h"

namespace gui
{
	class Labelled
	{
	public:
		Labelled();
		virtual ~Labelled();

		void SetName(const std::string& name) { mName = name; }
		std::string& GetName() { return mName; }

	private:
		std::string mName;
	};
}