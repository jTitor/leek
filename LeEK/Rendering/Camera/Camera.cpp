#include "Camera.h"
#include "Math/MathFunctions.h"
#include "GraphicsWrappers/IGraphicsWrapper.h"
#include "Platforms/IPlatform.h"

using namespace LeEK;

#pragma region CameraBase
CameraBase::CameraBase()
{
	viewMat = Matrix4x4::Identity;
	projMat = Matrix4x4::Identity;
	fovRads = Math::PI_OVER_2;
	nearDist = 0.25f;
	farDist = 50.0f;
	aspectRatio = 1.0f;
	projType = ProjType::PT_PERSPECTIVE;
	viewUpToDate = false;
	projUpToDate = false;
	frustUpToDate = false;
}

void CameraBase::recalcViewMatrix()
{
	onRecalcViewMatrix();
	viewUpToDate = true;
	frustUpToDate = false;
}

void CameraBase::recalcProjMatrix()
{
	switch(projType)
	{
	case ProjType::PT_PERSPECTIVE:
		projMat = Matrix4x4::BuildPerspectiveRH(aspectRatio, fovRads, nearDist, farDist);
		break;
	case ProjType::PT_ORTHOGRAPHIC:
		projMat = Matrix4x4::BuildOrthographicRH(aspectRatio, -aspectRatio, -1, 1, nearDist, farDist);
		break;
	default:
		projMat = Matrix4x4::BuildPerspectiveRH(aspectRatio, fovRads, nearDist, farDist);
		break;
	}
	projUpToDate = true;
	frustUpToDate = false;
}

void CameraBase::recalcFrust()
{
	worldFrust = Frustum::BuildFromMatrix(GetProjMatrix() * GetViewMatrix());
	frustUpToDate = true;
}

Frustum& CameraBase::GetWorldFrustum()
{
	if(projUpToDate && viewUpToDate && frustUpToDate)
	{
		return worldFrust;
	}
	recalcFrust();
	return worldFrust;
}

void CameraBase::SetFOV(F32 valRads)
{
	if(valRads == fovRads)
	{
		return;
	}

	fovRads = Math::Clamp(valRads, 0.0f, Math::TWO_PI);
	projUpToDate = false;
}

void CameraBase::SetNearDist(F32 val)
{
	if(val == nearDist)
	{
		return;
	}

	nearDist = Math::Max(val, 0.001f);
	projUpToDate = false;
}

void CameraBase::SetFarDist(F32 val)
{
	if(val == farDist)
	{
		return;
	}

	farDist = Math::Max(val, 0.01f);
	projUpToDate = false;
}

void CameraBase::SetAspectRatio(F32 val)
{
	if(val == aspectRatio)
	{
		return;
	}

	aspectRatio = Math::Max(val, 0.1f);
	projUpToDate = false;
}

void CameraBase::SetProjType(const ProjType& val)
{
	projType = val;
	projUpToDate = false;
}

#pragma endregion

#pragma region TranslateCamera
TranslateCamera::TranslateCamera()
{
	eyePos = Vector3::Zero;
}

void TranslateCamera::SetPosition(const Vector3& val)
{
	if(eyePos == val)
	{
		return;
	}
	eyePos = val;
	viewUpToDate = false;
}
#pragma endregion

#pragma region FreeLookCamera
void FreeLookCamera::onRecalcViewMatrix()
{
	viewMat = Matrix4x4::BuildViewRH(eyePos, viewDirection, viewUp);
}

void FreeLookCamera::SetUp(const Vector3& val)
{
	viewUp = val;
	viewUpToDate = false;
}

void FreeLookCamera::SetForward(const Vector3& val)
{
	viewDirection = val;
	viewUpToDate = false;
}

void FreeLookCamera::RotateEuler(F32 xRads, F32 yRads, F32 zRads)
{
	//need to rotate from camera's point of view.
	Vector3 right = Vector3::Cross(viewUp, viewDirection);
	Quaternion rot =	Quaternion(viewDirection, zRads) *
						Quaternion(right, xRads) * 
						Quaternion(viewUp, yRads);
	RotateQuaternion(rot);
	//zRads
}

void FreeLookCamera::RotateQuaternion(const Quaternion& q)
{
	Quaternion quat = q.GetNormalized();
	viewDirection = quat.RotateVector(viewDirection);
	viewUp = quat.RotateVector(viewUp);
	viewUpToDate = false;
}
#pragma endregion

#pragma region LookAtCamera
void LookAtCamera::onRecalcViewMatrix()
{
	viewMat = Matrix4x4::BuildViewLookAtRH(eyePos, lookAtPos, Vector3::Up);
}

void LookAtCamera::SetLookAtPos(const Vector3& val)
{
	lookAtPos = val;
	viewUpToDate = false;
}
#pragma endregion

//TODO: implement OrbitCamera
#pragma region OrbitCamera
void OrbitCamera::onRecalcViewMatrix()
{
	Vector3 forward = calcForwardVec();

	viewMat = Matrix4x4::BuildViewLookAtRH(center + forward, -forward, Vector3::Up);
}

Vector3 OrbitCamera::calcForwardVec() const
{
	Vector3 forward = center;
	forward.SetZ(forward.Z() - radius);

	return rotation.RotateVector(forward);
}

void OrbitCamera::SetCenter(const Vector3& val)
{
	center = val;
	viewUpToDate = false;
}

void OrbitCamera::SetRadius(F32 val)
{
	val = Math::Max(val, 0.0f);
	radius = val;
	viewUpToDate = false;
}

void OrbitCamera::SetOrientation(const Quaternion& quat)
{
	rotation = quat;
	viewUpToDate = false;
}

void OrbitCamera::RotateQuaternion(const Quaternion& quat)
{
	rotation *= quat;
	viewUpToDate = false;
}
#pragma endregion