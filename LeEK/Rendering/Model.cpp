#include "Model.h"
#include "../Logging/Log.h"

using namespace LeEK;

Model::Model(U32 meshCount)
{
	boundsCenter = Vector3::Zero;
	aabbHalfBounds = Vector3::Zero;
	boundingRadius = 0.0f;
	//resize the mesh list
	meshes.reserve(meshCount);
}

Model::Model(const Model& other)
{
	boundsCenter = other.boundsCenter;
	aabbHalfBounds = other.aabbHalfBounds;
	boundingRadius = other.boundingRadius;
	meshes = other.meshes;
}

Model::~Model(void)
{
}

void Model::SetAABBBounds(const Vector3& val)
{
	SetAABBHalfBounds(val / 2.0f);
}

void Model::SetAABBHalfBounds(const Vector3& halfVal)
{
	//ensure new bounds are non negative
	aabbHalfBounds = halfVal.GetAbs();
	//implies sphere radius must be recalculated...
	boundingRadius = aabbHalfBounds.Length();
}

void Model::SetBoundsCenter(const Vector3& val)
{
	boundsCenter = val;
}

/*
void Model::SetBoundingRadius(F32 val)
{
	boundingRadius = val;
}
*/

void Model::AddMesh(const Mesh& mesh)
{
	meshes.push_back(mesh);
}

void Model::RecalcBounds()
{
	LogV("Recalculating model bounds...");
	Vector3 aabbMax = Vector3::Zero;
	Vector3 aabbMin = Vector3::Zero;
	F32 newRadius = 0;

	U32 numVerts = 0;

	//iterate through all meshes
	for(int i = 0; i < meshes.size(); ++i)
	{
		const Geometry& geom = meshes[i].GetGeometry();
		//then each vertex in the mesh's geometry
		for(int j = 0; j < geom.VertexCount(); ++j)
		{
			const Vector3& vert = geom.Vertices()[j].Position;
			//expand AABB bounds to fit this vertex if needed
			aabbMax.SetX(Math::Max(aabbMax.X(), vert.X()));
			aabbMax.SetY(Math::Max(aabbMax.Y(), vert.Y()));
			aabbMax.SetZ(Math::Max(aabbMax.Z(), vert.Z()));

			//we ALSO need the minimum bounds to get the AABB's center.
			aabbMin.SetX(Math::Min(aabbMin.X(), vert.X()));
			aabbMin.SetY(Math::Min(aabbMin.Y(), vert.Y()));
			aabbMin.SetZ(Math::Min(aabbMin.Z(), vert.Z()));
		}
		numVerts += geom.VertexCount();
	}
	LogV(String("\tProcessed ") + numVerts + " vertices.");
	Vector3 finalMax = Vector3(	Math::Max(aabbMax.X(), aabbMin.X()),
								Math::Max(aabbMax.Y(), aabbMin.Y()),
								Math::Max(aabbMax.Z(), aabbMin.Z()));
	Vector3 finalMin = Vector3(	Math::Min(aabbMax.X(), aabbMin.X()),
								Math::Min(aabbMax.Y(), aabbMin.Y()),
								Math::Min(aabbMax.Z(), aabbMin.Z()));
	
	//TODO: right now the bounds are placed REALLY strangely;
	//it's like the AABB is rotated 90 degs along Z axis,
	//and this occurs regardless of the model data source.
	//This swap's needed to place the bounds properly.
	Vector3 finalBounds = finalMax - finalMin;
	F32 swap = finalBounds.X();
	finalBounds.SetX(finalBounds.Y());
	finalBounds.SetY(swap);

	//AABB's done; calculate and set center
	//And assign the actual AABB dimensions.
	//This will also set the radius.
	SetAABBBounds(finalBounds);
	SetBoundsCenter((finalMax + finalMin) / 2.0f);
}

void Model::Clear()
{
	meshes.clear();
	//reset bounds, too!
	aabbHalfBounds = Vector3::Zero;
	boundingRadius = 0.0f;
}

Mesh* Model::GetMesh(U32 index)
{
	if(index < meshes.size())
	{
		return &meshes[index];
	}
	return NULL;
}

const Mesh* Model::GetMesh(U32 index) const
{
	if(index < meshes.size())
	{
		return &meshes[index];
	}
	return NULL;
}

//returns an iterator to the start of the mesh list
Model::ConstMeshIt Model::GetMeshBegin() const
{
	return meshes.cbegin();
}

Model::ConstMeshIt Model::GetMeshEnd() const
{
	return meshes.cend();
}