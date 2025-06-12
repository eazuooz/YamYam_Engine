#pragma once
#include "yaCamera.h"
#include "yaRenderTarget.h"


namespace ya
{
	class EditorCamera : public Camera
	{
	public:
		EditorCamera();
		virtual ~EditorCamera();

		void Initialize() override;
		void Update() override;
		void LateUpdate() override;
		void Render(const Matrix& view, const Matrix& projection) override;
		void OnEvent(Event& e);

		void CreateRenderTarget(UINT width, UINT height);
		graphics::RenderTarget* GetRenderTarget() { return mRenderTarget; }
		

	private:
		graphics::RenderTarget* mRenderTarget;
	};
}
