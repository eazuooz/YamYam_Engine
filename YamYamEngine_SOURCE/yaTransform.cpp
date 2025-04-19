#include "yaTransform.h"
#include "yaGraphics.h"
#include "yaCamera.h"
#include "yaConstantBuffer.h"
#include "yaRenderer.h"

namespace ya
{
	Transform::Transform()
		: Component(enums::eComponentType::Transform)
		, mWorldMatrix(Matrix::Identity)
		, mScale(Vector3::One)
		, mRotation(Vector3::Zero)
		, mPosition(Vector3::Zero)
	{
	}

	Transform::~Transform()
	{
	}

	void Transform::Initialize()
	{
	}

	void Transform::Update()
	{
		
	}

	void Transform::LateUpdate()
	{
		Matrix scale = Matrix::CreateScale(mScale.x, mScale.y, mScale.z);
		Matrix rotation = Matrix::CreateRotationX(math::Radian(mRotation.x));
		rotation *= Matrix::CreateRotationY(math::Radian(mRotation.y));
		rotation *= Matrix::CreateRotationZ(math::Radian(mRotation.z));
		Matrix translation = Matrix::CreateTranslation(mPosition);

		mWorldMatrix = scale * rotation * translation;

		mForward = Vector3::TransformNormal(Vector3::Forward, rotation);
		mRight = Vector3::TransformNormal(Vector3::Right, rotation);
		mUp = Vector3::TransformNormal(Vector3::Up, rotation);
	}

	void Transform::Render(const Matrix& view, const Matrix& projection)
	{

	}

	void Transform::Bind(const Matrix& view, const Matrix& projection) const
	{
		graphics::TransformCB cbData = {};
		cbData.World = mWorldMatrix;
		cbData.View = view;
		cbData.Projection = projection;

		graphics::ConstantBuffer* cb = renderer::constantBuffers[CBSLOT_TRANSFORM];

		cb->SetData(&cbData);
		cb->Bind(eShaderStage::All);
	}
}
