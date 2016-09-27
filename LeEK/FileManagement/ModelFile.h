#pragma once
#include "Datatypes.h"
#include "Rendering/Color.h"
#include "Math/Vector3.h"
#include "FileManagement/DataStream.h"
#include "Rendering/Model.h"

namespace LeEK
{
	const U16 MIN_MODEL_VER = 200;
	const U16 CURR_MODEL_VER = 200;

	//File structs follow.
	//Specify no packing so that this matches what's written to disk.
#pragma pack(1)
	struct ModelHeader
	{
		enum { SIGNATURE = 0x4C4B4D44 };
		U32 Signature;
		U16 Version;
		U16 NumUnits;
		U32 MeshHdrStart;
		F32 BoundsX;
		F32 BoundsY;
		F32 BoundsZ;
		F32 BoundingRadius;
	};

	struct MeshHeader
	{
		enum { SIGNATURE = 0x4C4B4D55 };
		U32 Signature;
		U32 UnitDataStart;
		U32 UnitDataSize;
		U32 VertexDataSize;
		U32 IndexDataSize;
		U32 NumVerts;
		U32 NumIndices;
		U8 NumTexChannels;
		Color DiffuseColor;
		Color SpecularColor;
		Color EmissiveColor;
		U32 DiffuseGUIDLen;
		U32 SpecularGUIDLen;
		U32 EmissiveGUIDLen;
		U32 NormalMapGUIDLen;
		U32 NextMeshOffset;
	};
#pragma pack()

	namespace ModelMgr
	{
		bool WriteModelMemory(const Model& model, char* buf, size_t bufSize);
		bool WriteModelFile(const Model& model, DataStream* file);
		//bool ReadModelFile(Model& model, DataStream* file);
		bool ReadModelMemory(Model& model, const char* dataIn, size_t inSize, char* geomDataOut, size_t outSize);
		//Finds the total size of the model if written to disk.
		size_t FindFileSize(const Model& model);
		//Finds the total size of the geometry data in the file.
		size_t FindFileGeomSize(const char* fileBuf);
	}
}