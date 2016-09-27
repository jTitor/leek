#include "StdAfx.h"
#include "Physics.h"
#include "Constants/AllocTypes.h"
//include btBulletDynamicsCommon.h

using namespace LeEK;

//RigidBody
const BoxCollShape DEFAULT_COLBODY = BoxCollShape(0, 0, 0);

bool PhysicsWorld::Startup()
{
	//initialization requires you to:
	//1. set broadphase algorithm (just use btDbvtBroadphase)
	//2. set collision configuration and collision dispatcher 
	//	 (we can use defaults btDefaultCollisionConfiguration and btCollisionDispatcher)
	//3. set solver (for this project use btSequentialImpulseConstraintSolver)
	//4. instantiate physics world
	//5. !!!set gravity!!!
	btAlignedAllocSetCustom(Allocator::BulletMalloc, Allocator::BulletFree);
	LogD("Setting Bullet allocation functions");
	bphaseAlgo = CustomNew<btDbvtBroadphase>(PHYS_ALLOC, "PhysBphaseAlloc");//new btDbvtBroadphase();

	collConfig = CustomNew<btDefaultCollisionConfiguration>(PHYS_ALLOC, "PhysCollConfigAlloc");//new btDefaultCollisionConfiguration();
	collDispatcher = CustomNew<btCollisionDispatcher>(PHYS_ALLOC, "PhysCollDspchrAlloc", collConfig);//new btCollisionDispatcher(collConfig);

	solver = CustomNew<btSequentialImpulseConstraintSolver>(PHYS_ALLOC, "PhysSolverAlloc");//new btSequentialImpulseConstraintSolver();

	physWorld = CustomNew<btDiscreteDynamicsWorld>(PHYS_ALLOC, "PhysWorldAlloc", collDispatcher, bphaseAlgo, solver, collConfig);//new btDiscreteDynamicsWorld(collDispatcher, bphaseAlgo, solver, collConfig);
	physWorld->setInternalTickCallback(CollisionCallback, static_cast<void*>(this));

	physWorld->setGravity(btVector3(0, 0, 0));

	return true;
}

void PhysicsWorld::Shutdown()
{
	//pop off all objects in the world first; each object is itself responsible for deleting its data
	CustomDelete(physWorld);
	CustomDelete(solver);
	CustomDelete(collDispatcher);
	CustomDelete(collConfig);
	CustomDelete(bphaseAlgo);
	LogD("Deleted physics world");
}

void PhysicsWorld::AttachRigidBody(RigidBody*rb)
{
	physWorld->addRigidBody(rb->rb);
}

void PhysicsWorld::AttachCollisionBody(CollisionBodyBase* cb)
{
	physWorld->addCollisionObject(&(cb->co));
}

void PhysicsWorld::AttachPhantom(PhantomBase* p)
{
	physWorld->addCollisionObject(&(p->phantom));
}

void PhysicsWorld::RemoveRigidBody(RigidBody*rb)
{
	if (!rb)
	{
		return;
	}
	physWorld->removeRigidBody(rb->rb);
}

void PhysicsWorld::RemoveCollisionBody(CollisionBodyBase* cb)
{
	if (!cb)
	{
		return;
	}
	physWorld->removeCollisionObject(&(cb->co));
}

void PhysicsWorld::RemovePhantom(PhantomBase* p)
{
	if (!p)
	{
		return;
	}
	physWorld->removeCollisionObject(&(p->phantom));
}

void PhysicsWorld::AttachDebugDrawer(DebugDrawer* dd)
{
	debugDrawer = dd;
	physWorld->setDebugDrawer(dd);
}

void PhysicsWorld::SetGravityVector(const Vector3& g)
{
	physWorld->setGravity(btVector3(g.X(), g.Y(), g.Z()));
}

void PhysicsWorld::Update(const GameTime& time)
{
	//needs a fixed timestep
	physWorld->stepSimulation(time.ElapsedGameTime().ToSeconds(), 8);
	//if there's any collisions, the collision callback should handle it.
}

void PhysicsWorld::DebugDraw()
{
	physWorld->debugDrawWorld();
}

void LeEK::CollisionCallback(btDynamicsWorld *world, btScalar timeStep)
{
	//iterate over collision events
	I32 numEvents = world->getDispatcher()->getNumManifolds();
	if(numEvents > 0)
	{
		//Log::D(numEvents + String(" collisions detected"));
	}
	for(I32 i = 0; i < numEvents; ++i)
	{
		//get the pair of objects
		//and notify our internal stuff about what happened
	}
}

//CompoundCollShape
bool CompoundCollShape::AddShape(const CollisionShapeBase* s, Vector3 localPos, Quaternion localRot)
{
	((btCompoundShape*)cs)->addChildShape(BulletHelpers::BuildBtTrans(localPos, localRot), s->cs);
	return true;
}
bool CompoundCollShape::RemoveShape(const CollisionShapeBase* s)
{
	((btCompoundShape*)cs)->removeChildShape(s->cs);
	return true;
}

const Vector3 CollisionShapeBase::FindInertia(F32 mass)
const {
	btVector3 inertia(0, 0, 0);
	cs->calculateLocalInertia(mass, inertia);
	return Vector3(inertia.x(), inertia.y(), inertia.z());
}
CollisionShapeBase CompoundCollShape::GetChildShape(U32 index)
{
	if (index < ChildShapeCount())
	{
		return CollisionShapeBase(((btCompoundShape*)cs)->getChildShape(index));
	}
	//return some default shape?
	return DEFAULT_COLBODY;
}

//BulletMotionState
BulletMotionState::BulletMotionState(const btTransform& initialTrans)
{
	trans = initialTrans;
}

BulletMotionState::~BulletMotionState()
{
}

void BulletMotionState::getWorldTransform(btTransform &worldTrans) const
{
	worldTrans = trans;
}

void BulletMotionState::setWorldTransform(const btTransform &worldTrans)
{
	//put in things to update scene graph of connected object here
	//btQuaternion rot = worldTrans.getRotation();
	//mVisibleobj->setOrientation(rot.w(), rot.x(), rot.y(), rot.z());
	//btVector3 pos = worldTrans.getOrigin();
	//mVisibleobj->setPosition(pos.x(), pos.y(), pos.z());
	trans = worldTrans;
}

//RigidBody
RigidBody::RigidBody(F32 mass = 0.0f, const Vector3& pos = Vector3::Zero, const Quaternion& rot = Quaternion::Identity, const CollisionShapeBase& colShape = DEFAULT_COLBODY)
{
	btTransform rbTransform;
	rbTransform.setIdentity();
	rbTransform.setOrigin(btVector3(pos.X(), pos.Y(), pos.Z()));
	rbTransform.setRotation(BulletHelpers::BuildBtQuat(rot));
	//must also calculate inertia if object has mass
	//so dynamics works properly
	btVector3 rbInertia(0, 0, 0);
	if (mass != 0.0f)
	{
		Vector3 inertia = colShape.FindInertia(mass);
		LogD(String("Calculated inertia = ") + inertia.ToString());
		rbInertia.setX(inertia.X());
		rbInertia.setY(inertia.Y());
		rbInertia.setZ(inertia.Z());
	}
	BulletMotionState* motionState = CustomNew<BulletMotionState>(PHYS_ALLOC, "PhysMotStateAlloc", rbTransform);//new btDefaultMotionState(rbTransform);
	btRigidBody::btRigidBodyConstructionInfo info(mass, motionState, colShape.cs, rbInertia);
	rb = CustomNew<btRigidBody>(PHYS_ALLOC, "PhysRigidBodyAlloc", info);//new btRigidBody(info);
	rb->setDamping(0, 0.5f);
	rb->setRestitution(0.25f);
	rb->setSleepingThresholds(0.0f, 0.0f);
	rb->activate(true);
}

Vector3 RigidBody::Position()
{
	btTransform res = btTransform();
	rb->getMotionState()->getWorldTransform(res);
	return BulletHelpers::GetVec3(res.getOrigin());//rb->getWorldTransform().getOrigin());//>getCenterOfMassTransform().getOrigin());
}

Quaternion RigidBody::Rotation()
{
	btTransform res = btTransform();
	rb->getMotionState()->getWorldTransform(res);
	return BulletHelpers::GetQuat(res.getRotation());//res);
}

F32 RigidBody::LinearDrag()
{
	return rb->getLinearDamping();
}

F32 RigidBody::AngularDrag()
{
	return rb->getAngularDamping();
}

Vector3 RigidBody::Up()
{
	return Rotation() * Vector3::Up;
}

Vector3 RigidBody::Forward()
{
	return Rotation() * Vector3::Forward;
}

Vector3 RigidBody::Right()
{
	return Rotation() * Vector3::Right;
}

void RigidBody::SetPosition(const Vector3& pos)
{
	btTransform newTrans(rb->getWorldTransform());
	newTrans.setOrigin(BulletHelpers::BuildBtVec3(pos));
	rb->setWorldTransform(newTrans);
}

void RigidBody::SetRotation(const Quaternion& rot)
{
	btTransform newTrans(rb->getWorldTransform());
	newTrans.setRotation(BulletHelpers::BuildBtQuat(rot));
	rb->setWorldTransform(newTrans);
}

void RigidBody::ApplyForce(const Vector3& force)
{
	rb->activate(true);
	rb->applyCentralForce(BulletHelpers::BuildBtVec3(force));
}

void RigidBody::ApplyTorque(const Vector3& torque)
{
	rb->applyTorque(BulletHelpers::BuildBtVec3(Rotation() * torque));
}

//DebugDrawer
void DebugDrawer::drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color)
{
	//only draw if render verbosity is high enough
	if (debugLevel < btIDebugDraw::DBG_DrawWireframe)
	{
		return;
	}

	grpWrapper->DebugDrawLine(BulletHelpers::GetVec3(from),
		BulletHelpers::GetVec3(to),
		Color(BulletHelpers::GetVec3(color)));
}
void DebugDrawer::drawContactPoint(const btVector3 &PointOnB, const btVector3 &normalOnB, btScalar distance, int lifeTime, const btVector3 &color)
{
	//don't think we have points yet
}
void DebugDrawer::reportErrorWarning(const char *warningString)
{
	LogW(String("BulletWarn: ") + warningString);
}
void DebugDrawer::drawAabb(const btVector3 &from, const btVector3 &to, const btVector3 &color)
{
	//only draw if render verbosity is high enough
	if (debugLevel < btIDebugDraw::DBG_DrawAabb)
	{
		return;
	}

	//calc extents via the from and to, get center from average of sum of from and to, then make call to draw a box w/ identity transform
	Vector3 halfExtents = BulletHelpers::GetVec3(to - from) / 2.0f;
	Vector3 center = BulletHelpers::GetVec3(from) + halfExtents;//BulletHelpers::GetVec3((to + from) / 2.0f);
	grpWrapper->DebugDrawBox(center, Quaternion::Identity, halfExtents.Y(), halfExtents.X(), halfExtents.Z(), Color(BulletHelpers::GetVec3(color)));
}
void DebugDrawer::drawBox(const btVector3 &bbMin, const btVector3 &bbMax, const btVector3 &color)
{
	drawBox(bbMin, bbMax, btTransform::getIdentity(), color);
}
void DebugDrawer::drawBox(const btVector3 &bbMin, const btVector3 &bbMax, const btTransform &trans, const btVector3 &color)
{
	//only draw if render verbosity is high enough
	/*if(debugLevel < btIDebugDraw::DBG_DrawAabb)
	{
	return;
	}*/

	//calc extents via the from and to, get center from average of sum of from and to, then make call to draw a box w/ identity transform
	Vector3 halfExtents = BulletHelpers::GetVec3(bbMax - bbMin) / 2.0f;
	Vector3 center = BulletHelpers::GetVec3(bbMin) + halfExtents;//BulletHelpers::GetVec3((bbMax + bbMin) / 2.0f); //don't think that the translation component of trans needs to be factored in
	grpWrapper->DebugDrawBox(BulletHelpers::GetVec3(trans.getOrigin()), BulletHelpers::GetRotation(trans), halfExtents.Y(), halfExtents.X(), halfExtents.Z(), Color(BulletHelpers::GetVec3(color)));
}

void DebugDrawer::drawSphere(const btVector3 &p, btScalar radius, const btVector3 &color)
{
	Vector3 center = BulletHelpers::GetVec3(p);
	Vector3 convertedColor = BulletHelpers::GetVec3(color);
	grpWrapper->DebugDrawSphere(center, radius, Color(convertedColor));
}

void DebugDrawer::drawSphere(btScalar radius, const btTransform &transform, const btVector3 &color)
{
	drawSphere(transform.getOrigin(), radius, color);
}