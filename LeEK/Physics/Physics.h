#pragma once
#include "EngineLogic/IEngineModule.h"
#include "Memory/Allocator.h"
#include "Math/Vector3.h"
#include "Math/Quaternion.h"
#include "EngineLogic/Transform.h"
#include "Time/GameTime.h"
#include "GraphicsWrappers/IGraphicsWrapper.h"
#include <Libraries/Bullet_2_81/src/btBulletDynamicsCommon.h>
#include <Libraries/Bullet_2_81/src/LinearMath/btIDebugDraw.h>

namespace LeEK
{
	//converts between LeEK and Bullet implementations of vectors and quaternions.
	namespace BulletHelpers
	{
		inline btVector3 BuildBtVec3(const Vector3& v)
		{
			return btVector3(v.X(), v.Y(), v.Z());
		}

		inline btQuaternion BuildBtQuat(const Quaternion& rot)
		{
			return btQuaternion(rot.X(), rot.Y(), rot.Z(), rot.W());
		}

		inline btTransform BuildBtTrans(const Vector3& trans, const Quaternion& rot)
		{
			return btTransform(BuildBtQuat(rot), BuildBtVec3(trans));
		}

		inline btTransform BuildBtTrans(const Transform& transform)
		{
			return BuildBtTrans(transform.Position(), transform.Orientation());
		}

		inline Vector3 GetVec3(const btVector3& v)
		{
			return Vector3(v.x(), v.y(), v.z());
		}

		inline Quaternion GetQuat(const btQuaternion& rot)
		{
			return Quaternion(rot.x(), rot.y(), rot.z(), rot.w());
		}

		inline Vector3 GetTranslation(const btTransform& trans)
		{
			return GetVec3(trans.getOrigin());
		}

		inline Quaternion GetRotation(const btTransform& trans)
		{
			return GetQuat(trans.getRotation());
		}
		inline Transform GetTransform(const btTransform& trans)
		{
			return Transform(GetTranslation(trans), GetRotation(trans));
		}
	}
	//TODO: Turn pointers here into handles?
	//class for volumes used to test collision queries.
	class PhantomBase
	{
		friend class PhysicsWorld;
	private:
		btCollisionObject phantom;
		//btGhostObject phantom;
	};

	//class for volumes used to give RigidBodies collision detection.
	class CollisionShapeBase
	{
		friend class RigidBody;
	protected:
		CollisionShapeBase() {}
	public:
		btCollisionShape* cs;
		CollisionShapeBase(btCollisionShape* original) { cs = original; }
		~CollisionShapeBase() { CustomDelete(cs); }
		const Vector3 FindInertia(F32 mass) const;
	};

	//how should we handle compound shapes?
	//leave it up to the scene graph?

	class CompoundCollShape : public CollisionShapeBase
	{
	public:
		CompoundCollShape()
		{
			//what happens if called w/ false?
			cs = CustomNew<btCompoundShape>(PHYS_ALLOC, "PhysShapeAlloc");//new btCompoundShape();
		}
		bool AddShape(const CollisionShapeBase* s, Vector3 localPos, Quaternion localRot);
		bool RemoveShape(const CollisionShapeBase* s);
		inline U32 ChildShapeCount() { return ((btCompoundShape*)cs)->getNumChildShapes(); }
		CollisionShapeBase GetChildShape(U32 index);
	};

	class BoxCollShape : public CollisionShapeBase
	{
	public:
		BoxCollShape(Vector3 halfExtents)
		{
			BoxCollShape(halfExtents.X(), halfExtents.Y(), halfExtents.Z());
		}
		BoxCollShape(F32 halfWidth, F32 halfHeight, F32 halfDepth)
		{
			cs = CustomNew<btBoxShape>(PHYS_ALLOC, "PhysShapeAlloc", btVector3(halfWidth, halfHeight, halfDepth));//new btBoxShape(btVector3(halfWidth, halfHeight, halfDepth));
		}
	};

	class SphereCollShape : public CollisionShapeBase
	{
	public:
		SphereCollShape(F32 radius)
		{
			cs = CustomNew<btSphereShape>(PHYS_ALLOC, "PhysShapeAlloc", radius);//new btSphereShape(radius);
		}
	};

	class ConeCollShape : public CollisionShapeBase
	{
	public:
		ConeCollShape(F32 height, F32 radius)
		{
			cs = CustomNew<btConeShape>(PHYS_ALLOC, "PhysShapeAlloc", radius, height);//new btConeShape(radius, height);
		}
	};

	class CapsuleCollShape : public CollisionShapeBase
	{
	public:
		CapsuleCollShape(F32 height, F32 radius)
		{
			cs = CustomNew<btCapsuleShape>(PHYS_ALLOC, "PhysShapeAlloc", radius, height);//new btCapsuleShape(radius, height);
		}
	};

	class CylinderCollShape : public CollisionShapeBase
	{
	public:
		CylinderCollShape(Vector3 halfExtents)
		{
			CylinderCollShape(halfExtents.X(), halfExtents.Y(), halfExtents.Z());
		}
		CylinderCollShape(F32 halfWidth, F32 halfHeight, F32 halfDepth)
		{
			cs = CustomNew<btCylinderShape>(PHYS_ALLOC, "PhysShapeAlloc", btVector3(halfWidth, halfHeight, halfDepth));//new btCylinderShape(btVector3(halfWidth, halfHeight, halfDepth));
		}
	};

	class CollisionBodyBase
	{
		friend class PhysicsWorld;
	private:
		btCollisionObject co;
	};

	/**
	* Interface between Bullet and the engine.
	*/
	class BulletMotionState : public btMotionState
	{
	protected:
		btTransform trans;
	public:
		BulletMotionState(const btTransform& initialTrans);
		virtual ~BulletMotionState();

		virtual void getWorldTransform(btTransform &worldTrans) const;
		virtual void setWorldTransform(const btTransform &worldTrans);
	};

	class RigidBody
	{
		friend class PhysicsWorld;
	protected:
		btRigidBody* rb;
		CompoundCollShape collShape;
	public:
		//on init, build a MotionState and attach to physics world
		//have to also calculate inertia, which requires having a shape of some kind
		RigidBody(F32 mass, const Vector3& pos, const Quaternion& rot, const CollisionShapeBase& colShape);
		//bool AddShape(const CollisionShapeBase& s);// { return collShape.AddShape(s); }
		//bool RemoveShape(const CollisionShapeBase& s);// { return collShape.RemoveShape(s); }
		Vector3 Position();
		Quaternion Rotation();
		Vector3 Up();
		Vector3 Forward();
		Vector3 Right();
		F32 LinearDrag();
		F32 AngularDrag();
		//sets global position and transform.
		void SetPosition(const Vector3& pos);
		void SetRotation(const Quaternion& rot);
		void SetLinearDrag(F32 drag);
		void SetAngularDrag(F32 drag);
		void ApplyForce(const Vector3& force);
		void ApplyLocalForce(const Vector3& force) { ApplyForce(Rotation() * force); }
		void ApplyTorque(const Vector3& torque);
		void ApplyLocalTorque(const Vector3& torque) { ApplyTorque(Rotation() * torque); }
		//add SetPosition(), SetRotation()?
	};

	//debug drawer for PhysicsWorld
	class DebugDrawer : public btIDebugDraw
	{
	private:
		U32 debugLevel; //uses values from btIDebugDraw's DebugDrawModes
		GfxWrapperHandle grpWrapper;
	public:
		DebugDrawer(GfxWrapperHandle r) : grpWrapper(r)
		{
		}
		void 	drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color);
		//void 	drawLine (const btVector3 &from, const btVector3 &to, const btVector3 &fromColor, const btVector3 &toColor) //don't have line color interpolation yet
		void 	drawSphere (btScalar radius, const btTransform &transform, const btVector3 &color); //sphere's busted
		void 	drawSphere (const btVector3 &p, btScalar radius, const btVector3 &color);
		//void 	drawTriangle (const btVector3 &v0, const btVector3 &v1, const btVector3 &v2, const btVector3 &, const btVector3 &, const btVector3 &, const btVector3 &color, btScalar alpha) //triangle's not implemented
		//void 	drawTriangle (const btVector3 &v0, const btVector3 &v1, const btVector3 &v2, const btVector3 &color, btScalar)
		void 	drawContactPoint(const btVector3 &PointOnB, const btVector3 &normalOnB, btScalar distance, int lifeTime, const btVector3 &color);
		void 	reportErrorWarning(const char *warningString);
		void 	draw3dText(const btVector3 &location, const char *textString) {} //don't have text display yet
		void 	setDebugMode(int debugMode) { debugLevel = debugMode; }
		int 	getDebugMode() const { return debugLevel; }
		void 	drawAabb(const btVector3 &from, const btVector3 &to, const btVector3 &color); //calc extents via the from and to, get center from average of sum of from and to, then make call to draw a box w/ identity transform
		//void 	drawTransform (const btTransform &transform, btScalar orthoLen) //how should transform be drawn?
		//void 	drawArc (const btVector3 &center, const btVector3 &normal, const btVector3 &axis, btScalar radiusA, btScalar radiusB, btScalar minAngle, btScalar maxAngle, const btVector3 &color, bool drawSect, btScalar stepDegrees=btScalar(10.f)) //arc's not implemented
		//void 	drawSpherePatch (const btVector3 &center, const btVector3 &up, const btVector3 &axis, btScalar radius, btScalar minTh, btScalar maxTh, btScalar minPs, btScalar maxPs, const btVector3 &color, btScalar stepDegrees=btScalar(10.f))
		void 	drawBox(const btVector3 &bbMin, const btVector3 &bbMax, const btVector3 &color);
		void 	drawBox(const btVector3 &bbMin, const btVector3 &bbMax, const btTransform &trans, const btVector3 &color);
		//void 	drawCapsule (btScalar radius, btScalar halfHeight, int upAxis, const btTransform &transform, const btVector3 &color) //none of these are implemented
		//void 	drawCylinder (btScalar radius, btScalar halfHeight, int upAxis, const btTransform &transform, const btVector3 &color)
		//void 	drawCone (btScalar radius, btScalar height, int upAxis, const btTransform &transform, const btVector3 &color)
		//void 	drawPlane (const btVector3 &planeNormal, btScalar planeConst, const btTransform &transform, const btVector3 &color)
	};

	//singleton class?
	//refactor into interfaces - this is the bullet implementation
	/**
	*	Implementation of physics system using Bullet.
	*	Should use MKS system.
	*/
	class PhysicsWorld : public IEngineModule
	{
		//seems like a terrible idea to have friend classes
		//but at least you don't expose implentation outside of this file

	private:
		btBroadphaseInterface* bphaseAlgo;
		btDefaultCollisionConfiguration* collConfig;
		btCollisionDispatcher* collDispatcher;
		btSequentialImpulseConstraintSolver* solver;
		btDiscreteDynamicsWorld* physWorld;
		DebugDrawer* debugDrawer;
		//static PhysicsWorld* instance = NULL;
		//btCollisionWorld collisionWorld;
	public:
		bool Startup();
		void Shutdown();
		void AttachRigidBody(RigidBody* rb);
		void AttachCollisionBody(CollisionBodyBase* cb);
		void AttachPhantom(PhantomBase* p);
		void RemoveRigidBody(RigidBody* rb);
		void RemoveCollisionBody(CollisionBodyBase* cb);
		void RemovePhantom(PhantomBase* p);
		void AttachDebugDrawer(DebugDrawer* dd);
		void SetGravityVector(const Vector3& g);
		void Update(const GameTime& time);
		void DebugDraw();
	};

	void CollisionCallback(btDynamicsWorld *world, btScalar timeStep);
}
