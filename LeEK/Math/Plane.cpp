#include "Plane.h"

using namespace LeEK;

void Plane::init(const Vector4& pCoeff)
{
	coeff = pCoeff;
}

Plane::Plane(const Vector3& origin, const Vector3& normal)
{
	//d is the distance to the plane along the normal;
	//project (origin - 0) onto normal and get the length.
	//since normal is... normalized, this simplifies to a dot product
	Vector3 normalizedNorm = normal.GetNormalized();
	init(Vector4(normalizedNorm, -normalizedNorm.Dot(origin)));
}

Plane::Plane(const Vector4& coeff)
{
	//we need to normalize by the normal vector,
	//NOT by all four components.
	Vector3 normal = Vector3(coeff.X(), coeff.Y(), coeff.Z());
	F32 normLen = normal.Length();
	init(Vector4(normal / normLen, coeff.W() / normLen));
}

Plane::~Plane(void)
{
}

F32 Plane::GetDist(const Vector3& point) const
{
	return coeff.Dot(Vector4(point, 1));
}

I32 Plane::GetSide(const Vector3& point) const
{
	F32 dist = GetDist(point);
	//not using Math::Clamp since dist is a floating-point value
	if(Math::ApproxEqual(dist, 0.0f))
	{
		return 0;
	}
	if(dist > 0.0f)
	{
		return 1;
	}
	return -1;
}

Vector3 Plane::GetNormal() const
{
	return coeff.XYZ();
}

Vector3 Plane::GetOrigin() const
{
	//Again, since d is the distance to the plane along the normal,
	//just return d * the xyz components
	return coeff.W() * coeff.XYZ();
}

void Plane::SetNormal(const Vector3& val)
{
	//origin's the same, rederive info from that
	Vector3 origin = GetOrigin();
	Vector3 normalizedNorm = val.GetNormalized();
	init(Vector4(normalizedNorm, -normalizedNorm.Dot(origin)));
}

void Plane::SetOrigin(const Vector3& val)
{
	Vector3 norm = GetNormal();
	init(Vector4(norm, norm.Dot(val)));
}

const Vector4& Plane::GetCoefficients() const
{
	return coeff;
}