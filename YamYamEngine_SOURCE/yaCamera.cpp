#include "yaCamera.h"
#include "yaGameObject.h"
#include "yaTransform.h"
#include "yaApplication.h"
#include "yaSceneManager.h"

extern ya::Application application;

namespace ya
{
	//Matrix Camera::ViewMatrix = Matrix::Identity;
	//Matrix Camera::ProjectionMatrix = Matrix::Identity;

	Camera::Camera()
		: Component(eComponentType::Camera)
		  , mProjectionType(eProjectionType::Perspective)
		  , mViewMatrix(Matrix::Identity)
		  , mProjectionMatrix(Matrix::Identity)
		  , mAspectRatio(0.0f)
		  , mNear(1.0f)
		  , mFar(1000.0f)
		  , mSize(1.0f)
	{
	}

	Camera::~Camera()
	{
	}

	void Camera::Initialize()
	{
		Scene* activeScene = SceneManager::GetActiveScene();
		activeScene->AddCamera(this);
	}

	void Camera::Update()
	{
	}

	void Camera::LateUpdate()
	{
		CreateViewMatrix();
		CreateProjectionMatrix(mProjectionType);

		//ViewMatrix = mViewMatrix;
		//ProjectionMatrix = mProjectionMatrix;
	}

	void Camera::Render(const Matrix& view, const Matrix& projection)
	{
	}

	void Camera::CreateViewMatrix()
	{
		const Transform* tr = GetOwner()->GetComponent<Transform>();

		const Vector3 pos = tr->GetPosition();
		const Vector3 up = tr->Up();
		const Vector3 forward = tr->Forward();

		mViewMatrix = Matrix::CreateLookToLH(pos, forward, up);


	}

	void Camera::CreateProjectionMatrix(eProjectionType type)
	{
		RECT winRect = {};
		GetClientRect(application.GetWindow().GetHwnd(), &winRect);
		const float width = CAST_FLOAT(winRect.right - winRect.left);
		const float height = CAST_FLOAT(winRect.bottom - winRect.top);
		mAspectRatio = width / height;

		switch (type)
		{
		case eProjectionType::Perspective:
			mProjectionMatrix = Matrix::CreatePerspectiveFieldOfViewLH(XM_2PI / 6.0f, mAspectRatio, mNear, mFar);
			break;
		case eProjectionType::Orthographic:
			mProjectionMatrix = Matrix::CreateOrthographicLH(width / mSize, height / mSize, mNear, mFar);
			break;
		}
	}
}
