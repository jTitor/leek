#include "LightNode.h"
#include "Rendering/Color.h"

using namespace LeEK;

LightNode::LightNode(LightType type)
{
	Type = type;
	Ambient = Colors::White;
	Diffuse = Colors::White;
	Specular = Colors::White;
	Intensity = 0;
	ConstFalloff = 1;
	LinFalloff = 1;
	QuadFalloff = 1;

	Exponent = 1;
	Angle = 45;

	Attenuate = false;
	Active = false;

	locUp = Vector3::Up;
	worldUp = Vector3::Up;
	locRight = Vector3::Right;
	worldRight = Vector3::Right;
	locFwd = Vector3::Forward;
	worldFwd = Vector3::Forward;

	frameReady = false;
}

LightNode::~LightNode()
{
}

Vector3 LightNode::GetLocation() const
{
	return localTrans.Position();
}

Vector3 LightNode::GetUp() const
{
	return locUp;
}

Vector3 LightNode::GetForward() const
{
	return locFwd;
}

Vector3 LightNode::GetRight() const
{
	return locRight;
}

Vector3 LightNode::GetWorldLocation() const
{
	return GetWorldTransform().Position();
}

Vector3 LightNode::GetWorldUp() const
{
	return worldUp;
}

Vector3 LightNode::GetWorldForward() const
{
	return worldFwd;
}

Vector3 LightNode::GetWorldRight() const
{
	return worldRight;
}

//frame of reference setters
void LightNode::SetFrame(	const Vector3& location, 
				const Vector3& upVec, const Vector3& forwardVec, const Vector3& rightVec)
{
	//unused
}

void LightNode::SetFrame( const Vector3& location, const Quaternion& orientation)
{
	localTrans.SetPosition(location);
	localTrans.SetOrientation(orientation);
	
	onFrameChange();
}

void LightNode::SetLocation(const Vector3& location)
{
	localTrans.SetPosition(location);
}

void LightNode::SetAxes(const Quaternion& orientation)
{
	localTrans.SetOrientation(orientation);
	
	onFrameChange();
}

void LightNode::SetAxes(const Vector3& upVec, const Vector3& forwardVec, const Vector3& rightVec)
{
	//unused
}

//Interface implementation.
void LightNode::OnGetVisibleSet(Culler& culler, bool shouldNotCull, bool recalcTrans)
{
	//TODO
}

void LightNode::onFrameChange()
{
	//recalculate cached axii
	locUp = localTrans.Orientation() * Vector3::Up;
	locRight = localTrans.Orientation() * Vector3::Right;
	locFwd = localTrans.Orientation() * Vector3::Forward;

	Transform world = GetWorldTransform();

	worldUp = world.Orientation() * Vector3::Up;
	worldRight = world.Orientation() * Vector3::Right;
	worldFwd = world.Orientation() * Vector3::Forward;

	frameReady = true;
}