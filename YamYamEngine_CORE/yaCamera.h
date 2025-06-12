#pragma once
#include "yaComponent.h"
#include "yaEvent.h"
#include "yaGameObject.h"

namespace ya
{
	using namespace ya::math;

	class Camera : public Component
	{
	public:
		enum class eProjectionType
		{
			Perspective,
			Orthographic
		};

		//static Matrix GetGpuViewMatrix() { return ViewMatrix; }
		//static Matrix GetGpuProjectionMatrix() { return ProjectionMatrix; }
		//static void SetGpuViewMatrix(const Matrix& matrix) { ViewMatrix = matrix; }
		//static void SetGpuProjectionMatrix(const Matrix& matrix) { ProjectionMatrix = matrix; }

		Camera();
		virtual ~Camera();

		void Initialize() override;
		void Update() override;
		void LateUpdate() override;
		void Render(const Matrix& view, const Matrix& projection) override;

		void CreateViewMatrix();
		void CreateProjectionMatrix(eProjectionType type);

		const Matrix& GetViewMatrix() { return mViewMatrix; }
		const Matrix& GetProjectionMatrix() { return mProjectionMatrix; }
		void SetProjectionType(const eProjectionType type) { mProjectionType = type; }
		void SetSize(const float size) { mSize = size; }

	private:
		//static Matrix ViewMatrix;
		//static Matrix ProjectionMatrix;

		eProjectionType mProjectionType;

		Matrix mViewMatrix;
		Matrix mProjectionMatrix;
		float mAspectRatio;
		float mNear;
		float mFar;
		float mSize; //
	};
}
