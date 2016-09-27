#pragma once
#include "Math/Vector3.h"
#include "Math/Matrix4x4.h"
#include "EngineLogic/Transform.h"
#include "Memory/Handle.h"
#include "Rendering/Bounds/Frustum.h"

namespace LeEK
{
	enum ProjType { PT_PERSPECTIVE, PT_ORTHOGRAPHIC };

	//base class for cameras.
	class CameraBase
	{
	protected:
		Frustum worldFrust;
		Matrix4x4 viewMat;
		Matrix4x4 projMat;
		F32 fovRads;
		F32 nearDist, farDist;
		F32 aspectRatio;
		ProjType projType;
		bool viewUpToDate, projUpToDate, frustUpToDate;
		void recalcViewMatrix();
		/**
		Allows cameras to take class-specific action when a view matrix
		recalc is requested.
		*/
		virtual void onRecalcViewMatrix() = 0;
		void recalcProjMatrix();
		void recalcFrust();
	public:
		CameraBase();
		virtual ~CameraBase() {}
		virtual Vector3 Position() const = 0;//{ return eyePos; }
		virtual Vector3 Up() const = 0;//{ return viewUp; }
		virtual Vector3 Forward() const = 0;//{ return viewDirection; }
		Vector3 Right() const { return Vector3::Cross(Up(), Forward()); }
		virtual void SetUp(const Vector3& val) = 0;
		virtual void SetForward(const Vector3& val) = 0;
		inline F32 FOV() const { return fovRads; }
		inline F32 NearDist() const { return nearDist; }
		inline F32 FarDist() const { return farDist; }
		inline F32 AspectRatio() const { return aspectRatio; }
		void SetFOV(F32 valRads);
		void SetNearDist(F32 val);
		void SetFarDist(F32 val);
		void SetAspectRatio(F32 val);

		Matrix4x4& GetViewMatrix()
		{ 
			if(viewUpToDate)
			{
				return viewMat;
			}
			recalcViewMatrix();
			return viewMat;
		}
		Matrix4x4 GetViewToWorldMatrix() { return GetViewMatrix().GetTransformInverse(); }
		Matrix4x4& GetProjMatrix()
		{
			if(projUpToDate)
			{
				return projMat;
			}
			recalcProjMatrix();
			return projMat;
		}
		const ProjType& GetProjType() { return projType; }
		void SetProjType(const ProjType& val);

		Frustum& GetWorldFrustum();

		Quaternion GetRotation()
		{
			//want the angle first
			F32 cos = Vector3::Dot(Vector3::Forward, Forward());
			F32 angle = Math::ACos(cos);
			Vector3 axis = Vector3::Cross(Vector3::Forward, Forward());
			return Quaternion(axis, angle);
		}
		virtual void RotateEuler(F32 xRads, F32 yRads, F32 zRads) = 0;
		inline void RotateX(F32 xRads) { RotateEuler(xRads, 0, 0); }
		inline void RotateY(F32 yRads) { RotateEuler(0, yRads, 0); }
		inline void RotateZ(F32 zRads) { RotateEuler(0, 0, zRads); }

		virtual void SetPosition(const Vector3& val) = 0;
		virtual void Translate(const Vector3& trans) = 0;
	};

	class TranslateCamera : public CameraBase
	{
	protected:
		Vector3 eyePos;
		//void onRecalcViewMatrix();
	public:
		TranslateCamera();
		~TranslateCamera()
		{
			eyePos = Vector3::Zero;
		}

		Vector3 Position() const { return eyePos; }
		

		void SetPosition(const Vector3& val);
		void Translate(const Vector3& trans) { SetPosition(eyePos + trans); }
	};

	class FreeLookCamera : public TranslateCamera
	{
	protected:
		Vector3 viewDirection;
		Vector3 viewUp;
		void onRecalcViewMatrix();
	public:
		FreeLookCamera()
		{
			viewDirection = Vector3::Forward;
			viewUp = Vector3::Up;
		}
		~FreeLookCamera() {}

		Vector3 Up() const { return viewUp; }
		Vector3 Forward() const { return viewDirection; }
		void SetUp(const Vector3& val);
		void SetForward(const Vector3& val);

		void RotateQuaternion(const Quaternion& quat);
		void RotateEuler(F32 xRads, F32 yRads, F32 zRads);
	};

	class LookAtCamera : public TranslateCamera
	{
	protected:
		Vector3 lookAtPos;
		void onRecalcViewMatrix();
	public:
		LookAtCamera()
		{
			lookAtPos = Vector3::Zero;
			//viewDirection = Vector3::Forward; //change to specify actual direction
		}
		~LookAtCamera() {}

		void RotateEuler(F32 xRads, F32 yRads, F32 zRads) {}
		
		Vector3 Up() const
		{ 
			Vector3 forward = Forward();
			Vector3 up = Vector3::Up;
			return up + Vector3::Dot(forward, up)*forward;
		}
		Vector3 Forward() const
		{ 
			return lookAtPos - eyePos;
		}
		void SetUp(const Vector3& val) {}
		void SetForward(const Vector3& val) {}

		inline const Vector3& LookAtPos() const { return lookAtPos; }
		void SetLookAtPos(const Vector3& val);
	};

	//kind of an exception.
	//can't translate it, that'd violate orbit behavior.
	//TODO: implement!
	class OrbitCamera : public CameraBase
	{
	protected:
		Quaternion rotation;
		Vector3 center;
		F32 radius;
		void onRecalcViewMatrix();
		Vector3 calcForwardVec() const;
	public:
		OrbitCamera()
		{
			rotation = Quaternion::Identity;
			center = Vector3::Zero;
			radius = 0.0f;
		}
		~OrbitCamera() {}

		inline const Vector3& Center() const { return center; }
		void SetCenter(const Vector3& val);
		inline F32 Radius() const { return radius; }
		void SetRadius(F32 val);

		Vector3 Position() const { return center - calcForwardVec(); }
		Vector3 Up() const 
		{
			Vector3 forward = Forward();
			Vector3 up = Vector3::Up;
			return up + Vector3::Dot(forward, up)*forward;
		}
		Vector3 Forward() const { return calcForwardVec(); }
		void SetUp(const Vector3& val) {}
		void SetForward(const Vector3& val) {}

		void SetOrientation(const Quaternion& quat);
		void RotateQuaternion(const Quaternion& quat);
		void RotateEuler(F32 xRads, F32 yRads, F32 zRads)
		{
			RotateQuaternion(Quaternion::FromEulerAngles(xRads, yRads, zRads));
		}

		void SetPosition(const Vector3& val) { SetRadius(val.Z()); }
		void Translate(const Vector3& trans) { SetRadius(radius + trans.Z()); }
	};

	typedef TypedHandle<CameraBase> CameraHandle;
}
