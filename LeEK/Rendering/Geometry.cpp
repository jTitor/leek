#include "StdAfx.h"
#include "Geometry.h"
#include "Constants/LogTags.h"
#include <cstring>

using namespace LeEK;

Geometry::~Geometry()
{
}

void Geometry::Initialize(TypedArrayHandle<Vertex> verts, TypedArrayHandle<U32> inds, U32 vertCountParam, U32 indexCountParam, U8 uvChannelCountParam)
{
	//should really ensure both buffers exist, but whatever
	GeomBase::Initialize(verts, inds, vertCountParam, indexCountParam);

	uvChannelCount = uvChannelCountParam;
}

TypedArrayHandle<Vertex> GeomHelpers::BuildVertArray(Vector3* PosList, Vector3* NormList, Color* ColorList, Vector2* UVList, 
						  U32 numVertices)
{
	//we'll use the C library for now
	//since this is all pointless if there's no positions,
	//quit if there's no position data
	if(!PosList)
	{
		return false;
	}

	//need to do a for loop to create the data
	//since we're given separate lists for each attribute
	//will break if the model has mismatching attribute lists!

	//can return NULL if system's out of memory
	Vertex* vertices = CustomArrayNew<Vertex>(numVertices, 0, "DebugMeshInit");//new Vertex[numVertices];
	//Vertex vertData[3];
	if(!vertices)
	{
		return 0;
	}
	for(U32 i = 0; i < numVertices; i++)
	{
		vertices[i].Position = PosList[i];

		if(NormList)
		{
			vertices[i].Normal = NormList[i];
		}
		else
		{
			vertices[i].Normal = Vector3::Zero;
		}

		if(ColorList)
		{
			vertices[i].Color = Vector3(ColorList[i].R(), ColorList[i].G(), ColorList[i].B());
		}
		else
		{
			vertices[i].Color = Vector3(1.0f, 1.0f, 1.0f);
		}

		if(UVList)
		{
			vertices[i].UV = Vector3(UVList[i].X(), UVList[i].Y(), 0);
		}
		else
		{
			vertices[i].UV = Vector3::Zero;
		}
	}
	//now copy our generated vertices
	LogV("Copied vertex positions");//, LogTags::GEOM_INIT);

	return HandleMgr::RegisterPtr((void*)vertices);
}

bool GeomHelpers::BuildGeometry(Geometry& geom, Vector3* PosList, Vector3* NormList, Color* ColorList, Vector2* UVList, U32* IndexList, U32 numVertices, U32 numIndices, U8 numUVChannels)
{
	TypedArrayHandle<Vertex> vertHnd = GeomHelpers::BuildVertArray(PosList, NormList, ColorList, UVList, numVertices);
	if(!vertHnd.GetHandle())
	{
		return false;
	}
	U32* inds = CustomArrayNew<U32>(numIndices * 3, MESH_ALLOC, "MeshIndexAlloc");
	memcpy(inds, IndexList, numIndices * sizeof(U32));
	geom.Initialize(vertHnd, HandleMgr::RegisterPtr((void*)inds), numVertices, numIndices, numUVChannels);
	return true;
}
