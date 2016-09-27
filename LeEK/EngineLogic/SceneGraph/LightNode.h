#pragma once
#include "SpatialNode.h"
#include "../../Rendering/Color.h"

namespace LeEK
{
	/**
	Denotes a light in the scene.
	Can be one of four types:
		* Ambient
		* Directional
		* Point
		* Spot
	Note that in the case of ambient and directional lights that
	their position is meaningless.
	*/
	class LightNode : public SpatialNode
	{
	private:
		//virtual void UpdateWorldBound();
		void onFrameChange();
		Vector3 locUp, locRight, locFwd;
		Vector3 worldUp, worldRight, worldFwd;
		bool frameReady;
		//prevent use of base class methods
		//virtual void UpdateState
		//virtual void Draw
	public:
		enum LightType
		{
			LIGHT_AMBIENT,
			LIGHT_DIRECTIONAL,
			LIGHT_POINT,
			LIGHT_SPOT
		};

		LightNode(LightType type = LIGHT_AMBIENT);
		virtual ~LightNode();

		LightType Type;
		Color Ambient;
		Color Diffuse;
		Color Specular;
		F32 Intensity;
		F32 ConstFalloff;
		F32 LinFalloff;
		F32 QuadFalloff;
		bool Attenuate;
		bool Active;

		//spot light params
		F32 Exponent;
		F32 Angle;

		Vector3 GetLocation() const;
		Vector3 GetUp() const;
		Vector3 GetForward() const;
		Vector3 GetRight() const;
		Vector3 GetWorldLocation() const;
		Vector3 GetWorldUp() const;
		Vector3 GetWorldForward() const;
		Vector3 GetWorldRight() const;

		//frame of reference setters
		void SetFrame(	const Vector3& location, 
						const Vector3& upVec, const Vector3& forwardVec, const Vector3& rightVec);
		void SetFrame( const Vector3& location, const Quaternion& orientation);
		void SetLocation(const Vector3& location);
		void SetAxes(const Vector3& upVec, const Vector3& forwardVec, const Vector3& rightVec);
		void SetAxes(const Quaternion& orientation);
		//TODO
		void OnGetVisibleSet(Culler& culler, bool shouldNotCull, bool recalcTrans);
	};
}