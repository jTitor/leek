#include "Transform.h"

using namespace LeEK;

const Transform Transform::Identity = Transform();

Transform::~Transform(void)
{
}

Transform& Transform::operator*=(const Transform& other)
{
	scale = other.Scale()*scale;
	//save the orientation change for until after applying translation.
	//we need to transform the other transform's translation,
	//THEN add the new translation.
	//...Why do we have to rotate by the inverse here???
	trans += orien.GetInverse().RotateVector((scale*other.trans));
	orien = other.Orientation()*orien;
	return *this;
}

Matrix4x4 Transform::ToMatrix() const
{
	//TODO
	//optimize this!
	return Matrix4x4::BuildTranslation(trans) * orien.ToMatrix() * Matrix4x4::BuildScale(scale);//Matrix4x4::BuildScale(scale) * orien.ToMatrix() * Matrix4x4::BuildTranslation(trans);
}

Vector3 Transform::ToEulerAngles() const
{
	//convert quart to a matrix M
	Matrix4x4 rotMat = orien.ToMatrix();
	F32 sinY = rotMat(0,2);
	//y = arcsin(M(0,2))
	F32 cosY = Math::Sqrt(1-sinY*sinY);
	//save cosine of y = sqrt(1-M(0,2)^2)
	//x = arccos(M(2,2)/cos(Y))
	//z = arccos(M(0,0)/cos(Y))
	return Vector3(	Math::ACos(rotMat(2,2)/cosY),
					Math::ASin(sinY), 
					Math::ACos(rotMat(0,0)/cosY));
}

String Transform::ToString() const
{
	return String("Translation: ") + trans.ToString() + ", Orientation: " + orien.ToString();
}

Vector3 Transform::TransformVector(const Vector3& v) const
{
	return (orien.RotateVector((scale * v))) + trans;
}