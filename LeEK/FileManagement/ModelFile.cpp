#include "ModelFile.h"
#include "Memory/Allocator.h"
#include "Logging/Log.h"

using namespace LeEK;

void logModelHeaderInfo(const ModelHeader& hdr)
{
	LogV(String("\tNum. Meshes:\t") + hdr.NumUnits);
	LogV(String("\tData Start Offset:\t") + HexStrFromVal(hdr.MeshHdrStart));
}

void logMeshHeaderInfo(const MeshHeader& hdr, const Material* mat)
{
	LogV(String("\tNum. Verts:\t") + hdr.NumVerts);
	LogV(String("\tNum. Indices:\t") + hdr.NumIndices);
	LogV(String("\tTex. Channels:\t") + hdr.NumTexChannels);
	LogV(String("\tMesh Start Offset:\t") + HexStrFromVal(hdr.UnitDataStart));
	LogV(String("\tMesh Data Size:\t") + hdr.UnitDataSize);
	U32 stringSize = hdr.UnitDataSize - (hdr.VertexDataSize + hdr.IndexDataSize);
	LogV(String("\t\tString Data Size:\t") + stringSize);
	LogV(String("\t\tGeometry Data Size:\t") + (hdr.UnitDataSize - stringSize));
	LogV(String("\t\t\tVertex Data Size:\t") + hdr.VertexDataSize);
	LogV(String("\t\t\tIndex Data Size:\t") + hdr.IndexDataSize);
	LogV("\tMaterial Data: ");
	LogV("\t\tDiffuse:");
	LogV(String("\t\t\tColor:\t") + hdr.DiffuseColor.ToString());
	if(mat)
	{
		LogV(String("\t\t\tMap GUID:\t") + mat->DiffuseTexGUID.Name);
	}
	LogV(String("\t\t\tGUID Len.:\t") + hdr.DiffuseGUIDLen);
	LogV("\t\tSpecular:");
	LogV(String("\t\t\tColor:\t") + hdr.SpecularColor.ToString());
	if(mat)
	{
		LogV(String("\t\t\tMap GUID:\t") + mat->SpecularTexGUID.Name);
	}
	LogV(String("\t\t\tGUID Len.:\t") + hdr.SpecularGUIDLen);
	LogV("\t\tEmissive:");
	LogV(String("\t\t\tColor:\t") + hdr.EmissiveColor.ToString());
	if(mat)
	{
		LogV(String("\t\t\tMap GUID:\t") + mat->EmissiveTexGUID.Name);
	}
	LogV(String("\t\t\tGUID Len.:\t") + hdr.EmissiveGUIDLen);
	if(mat)
	{
		LogV(String("\t\tNormal Map GUID:\t") + mat->NormalMapGUID.Name);
	}
	LogV(String("\t\tGUID Len.:\t") + hdr.NormalMapGUIDLen);
}

size_t getWrittenLength(const ResGUID& guid)
{
	size_t len = guid.Length();
	//if the GUID isn't empty, you need to write the extra null
	len = len == 0 ? 0 : len+1;
	return len;
}

struct CombinedHeaders
{
public:
	ModelHeader MainHeader;
	MeshHeader* MeshHeaders;
	U32 NumMeshHeaders;

	CombinedHeaders()
	{
		MainHeader = ModelHeader();
		MeshHeaders = NULL;
		NumMeshHeaders = 0;
	}
};

bool buildHeaders(const Model& model, CombinedHeaders* outHdrs)
{
	//CombinedHeaders outHdrs = CombinedHeaders();
	ModelHeader& mainHeader = outHdrs->MainHeader;
	memset(&mainHeader, 0, sizeof(ModelHeader));
	mainHeader.Signature = ModelHeader::SIGNATURE;
	mainHeader.Version = CURR_MODEL_VER;
	mainHeader.NumUnits = model.MeshCount();
	LogV(String("Header.NumUnits = ") + mainHeader.NumUnits);
	if(mainHeader.NumUnits < 1)
	{
		//there was nothing to write; not exactly an error
		LogW("No mesh data to build headers from!");
		return false;
	}
	size_t meshHdrBufSize = mainHeader.NumUnits * sizeof(MeshHeader);
	mainHeader.MeshHdrStart = sizeof(ModelHeader);
	const Vector3& bounds = model.AABBBounds();
	mainHeader.BoundsX = bounds.X();
	mainHeader.BoundsY = bounds.Y();
	mainHeader.BoundsZ = bounds.Z();
	mainHeader.BoundingRadius = model.BoundingRadius();

	outHdrs->MeshHeaders = CustomArrayNew<MeshHeader>(mainHeader.NumUnits, FILEWRITE_ALLOC, "FileWriteAlloc");
	if(!outHdrs->MeshHeaders)
	{
		//out of memory!
		//return an empty header
		return false;//CombinedHeaders();
	}
	memset(outHdrs->MeshHeaders, 0, meshHdrBufSize);
	outHdrs->NumMeshHeaders = mainHeader.NumUnits;

	//now we can get a pointer to the mesh data
	MeshHeader* meshHeaders = outHdrs->MeshHeaders;

	LogV("Building headers: ");
	//logModelHeaderInfo(mainHeader);

	//iterate through the model
	//remember that the first mesh's data starts at the end of the header data!
	U32 currDataOffset = mainHeader.MeshHdrStart + mainHeader.NumUnits * sizeof(MeshHeader);
	U32 nextMeshHeader = mainHeader.MeshHdrStart + sizeof(MeshHeader);
	for(U32 i = 0; i < mainHeader.NumUnits; ++i)
	{
		LogV(String("Header.NumUnits = ") + mainHeader.NumUnits);
		const Mesh* mesh = model.GetMesh(i);
		const Geometry& geom = mesh->GetGeometry();
		//setup the header
		MeshHeader& meshHdr = meshHeaders[i];
		meshHdr.Signature = MeshHeader::SIGNATURE;
		meshHdr.UnitDataStart = currDataOffset;
		meshHdr.NumVerts = geom.VertexCount();
		meshHdr.NumIndices = geom.IndexCount();
		meshHdr.NumTexChannels = geom.UVChannelCount();
		meshHdr.VertexDataSize = meshHdr.NumVerts * sizeof(Vertex);
		meshHdr.IndexDataSize = meshHdr.NumIndices * sizeof(U32);
		//setup material information
		const Material& mat = mesh->GetMaterial();
		meshHdr.DiffuseColor = mat.DiffuseColor;
		meshHdr.SpecularColor = mat.SpecularColor;
		//meshHdr.AmbientColor = mat.AmbientColor;
		meshHdr.EmissiveColor = mat.EmissiveColor;
		//Next, the GUID strings.
		//Remember that these lengths should include the terminating null!
		meshHdr.DiffuseGUIDLen = getWrittenLength(mat.DiffuseTexGUID);
		meshHdr.SpecularGUIDLen = getWrittenLength(mat.SpecularTexGUID);
		meshHdr.EmissiveGUIDLen = getWrittenLength(mat.EmissiveTexGUID);
		meshHdr.NormalMapGUIDLen = getWrittenLength(mat.NormalMapGUID);
		meshHdr.UnitDataSize =	meshHdr.VertexDataSize + //sizeof(VertexHeader) + 
								meshHdr.IndexDataSize + //sizeof(IndexHeader) + 
								meshHdr.DiffuseGUIDLen + meshHdr.SpecularGUIDLen + meshHdr.EmissiveGUIDLen + 
								meshHdr.NormalMapGUIDLen;
		meshHdr.NextMeshOffset = nextMeshHeader;
		//recalc the data offsets
		currDataOffset += meshHdr.UnitDataSize;
		nextMeshHeader += sizeof(MeshHeader);
		LogV(String("\tMesh ") + i + ":");
		logMeshHeaderInfo(meshHdr, &mat);

		//++headerNum;
		L_ASSERT(i < mainHeader.NumUnits);
	}
	//null out the last header's offset to indicate the end of the header list
	meshHeaders[mainHeader.NumUnits - 1].NextMeshOffset = 0;
	LogV(String("Header.NumUnits = ") + mainHeader.NumUnits);
	//return outHdrs;
	return true;
}

/**
Writes data to a buffer, and returns a pointer past the written data.
*/
void writeToBuffer(char* buf, const char* bufEnd, const char* data, size_t dataSize)
{
	memcpy_s(buf, bufEnd-buf, data, dataSize);
}

bool ModelMgr::WriteModelMemory(const Model& model, char* buf, size_t bufSize)
{
	char* bufHead = buf;
	char* bufEnd = buf + bufSize;
	//build the start header
	CombinedHeaders headers = CombinedHeaders();
	if(!buildHeaders(model, &headers))
	{
		LogE("Failed to build model headers!");
		return false;
	}
	LogD(String("Found ") + headers.NumMeshHeaders + " headers.");
	if(headers.NumMeshHeaders < 1)
	{
		LogW("Nothing to write, exiting");
		return true;
	}

	writeToBuffer(bufHead, bufEnd, (char*)&headers.MainHeader, sizeof(ModelHeader));
	bufHead += sizeof(ModelHeader);
	writeToBuffer(bufHead, bufEnd, (char*)headers.MeshHeaders, sizeof(MeshHeader)*headers.NumMeshHeaders);
	bufHead += sizeof(MeshHeader)*headers.NumMeshHeaders;
	//now write the data!
	//headerNum = 0;
	for(U32 i = 0; i < model.MeshCount(); ++i)//(Model::ConstMeshIt cit = model.GetMeshBegin(); cit != model.GetMeshEnd(); ++cit)
	{
		const MeshHeader& meshHdr = headers.MeshHeaders[i];

		const Mesh* mesh = model.GetMesh(i);
		//write the GUIDs first
		const Material& mat = mesh->GetMaterial();
		if(meshHdr.DiffuseGUIDLen > 0)
		{
			writeToBuffer(bufHead, bufEnd, mat.DiffuseTexGUID.Name, meshHdr.DiffuseGUIDLen);
			bufHead += meshHdr.DiffuseGUIDLen;
		}
		if(meshHdr.SpecularGUIDLen > 0)
		{
			writeToBuffer(bufHead, bufEnd, mat.SpecularTexGUID.Name, meshHdr.SpecularGUIDLen);
			bufHead += meshHdr.SpecularGUIDLen;
		}
		if(meshHdr.EmissiveGUIDLen > 0)
		{
			writeToBuffer(bufHead, bufEnd, mat.EmissiveTexGUID.Name, meshHdr.EmissiveGUIDLen);
			bufHead += meshHdr.EmissiveGUIDLen;
		}
		if(meshHdr.NormalMapGUIDLen > 0)
		{
			writeToBuffer(bufHead, bufEnd, mat.NormalMapGUID.Name, meshHdr.NormalMapGUIDLen);
			bufHead += meshHdr.NormalMapGUIDLen;
		}

		//and write the geometry buffers
		const Geometry& geom = mesh->GetGeometry();
		if(!geom.Vertices() || !geom.Indices())
		{
			LogE(String("Mesh ") + i + " is missing geometry or index data, halting conversion!");
			return false;
		}
		/*VertexHeader vHdr;
		memset(&vHdr, 0, sizeof(VertexHeader));
		vHdr.Signature = VertexHeader::SIGNATURE;
		vHdr.DataSize = meshHdr.VertexDataSize;
		bufHead = writeToBuffer(bufHead, bufEnd, (char*)&vHdr, sizeof(VertexHeader));*/
		writeToBuffer(bufHead, bufEnd, (char*)geom.Vertices(), meshHdr.VertexDataSize);
		bufHead += meshHdr.VertexDataSize;
		/*IndexHeader iHdr;
		memset(&iHdr, 0, sizeof(IndexHeader));
		iHdr.Signature = IndexHeader::SIGNATURE;
		iHdr.DataSize = meshHdr.IndexDataSize;
		bufHead = writeToBuffer(bufHead, bufEnd, (char*)&iHdr, sizeof(IndexHeader));*/
		writeToBuffer(bufHead, bufEnd, (char*)geom.Indices(), meshHdr.IndexDataSize);
		bufHead += meshHdr.IndexDataSize;
		//++headerNum;
	}

	//we're done!
	return true;
}

bool ModelMgr::WriteModelFile(const Model& model, DataStream* file)
{
	//build the start header
	CombinedHeaders headers = CombinedHeaders();
	if(!buildHeaders(model, &headers))
	{
		LogE("Failed to build model headers!");
		return false;
	}

	//write the headers.
	file->Write((char*)&headers.MainHeader, sizeof(ModelHeader));
	file->Write((char*)headers.MeshHeaders, sizeof(MeshHeader)*headers.NumMeshHeaders);

	//now write the data!
	//headerNum = 0;
	for(U32 i = 0; i < model.MeshCount(); ++i)//(Model::ConstMeshIt cit = model.GetMeshBegin(); cit != model.GetMeshEnd(); ++cit)
	{
		const MeshHeader& meshHdr = headers.MeshHeaders[i];

		const Mesh* mesh = model.GetMesh(i);
		//write the GUIDs first
		const Material& mat = mesh->GetMaterial();
		if(meshHdr.DiffuseGUIDLen > 0)
		{
			file->Write(mat.DiffuseTexGUID.Name, meshHdr.DiffuseGUIDLen);
		}
		if(meshHdr.SpecularGUIDLen > 0)
		{
			file->Write(mat.SpecularTexGUID.Name, meshHdr.SpecularGUIDLen);
		}
		if(meshHdr.EmissiveGUIDLen > 0)
		{
			file->Write(mat.EmissiveTexGUID.Name, meshHdr.EmissiveGUIDLen);
		}
		if(meshHdr.NormalMapGUIDLen > 0)
		{
			file->Write(mat.NormalMapGUID.Name, meshHdr.NormalMapGUIDLen);
		}

		//and write the geometry buffers
		const Geometry& geom = mesh->GetGeometry();
		if(!geom.Vertices() || !geom.Indices())
		{
			LogE(String("Mesh ") + i + " is missing geometry or index data, halting conversion!");
			return false;
		}
		/*VertexHeader vHdr;
		memset(&vHdr, 0, sizeof(VertexHeader));
		vHdr.Signature = VertexHeader::SIGNATURE;
		vHdr.DataSize = meshHdr.VertexDataSize;
		file->Write((char*)&vHdr, sizeof(VertexHeader));*/
		file->Write((char*)geom.Vertices(), meshHdr.VertexDataSize);
		/*IndexHeader iHdr;
		memset(&iHdr, 0, sizeof(IndexHeader));
		iHdr.Signature = IndexHeader::SIGNATURE;
		iHdr.DataSize = meshHdr.IndexDataSize;
		file->Write((char*)&iHdr, sizeof(IndexHeader));*/
		file->Write((char*)geom.Indices(), meshHdr.IndexDataSize);

		//++headerNum;
	}

	//we're done!
	return true;
}

/*
bool ModelMgr::ReadModelFile(Model& model, DataStream* file)
{
	//Read the whole file into a temporary buffer,
	//and then do the standard read from memory.
	//No support for streaming at the moment.
	char* data = file->ReadAll();
	bool result = ModelMgr::ReadModelMemory(model, data);
	//now we can get rid of the temporary buffer.

	return result;
}
*/

bool ModelMgr::ReadModelMemory(Model& model, const char* data, size_t inSize, char* geomDataOut, size_t outSize)
{
	//Our assumption is that the WHOLE file is in a buffer.
	//We need an output buffer to store the geometry in;
	//quit if we weren't given one.
	if(!data || !geomDataOut)
	{
		return false;
	}

	char* dataPtr = (char*)data;
	//The start should be a main header.
	ModelHeader* mainHdr = (ModelHeader*)dataPtr;
	//verify file integrity
	if(mainHdr->Signature != ModelHeader::SIGNATURE)
	{
		LogE("Couldn't verify model header!");
		return false;
	}
	if(mainHdr->Version < MIN_MODEL_VER)
	{
		LogE("Model is too old!");
		return false;
	}
	logModelHeaderInfo(*mainHdr);

	//setup model properties
	model.SetAABBBounds(Vector3(mainHdr->BoundsX, mainHdr->BoundsY, mainHdr->BoundsZ));

	U16 numMeshes = mainHdr->NumUnits;
	//now move up to and iterate through the mesh headers.
	dataPtr = (char*)(data + mainHdr->MeshHdrStart);
	char* geomOutPtr = geomDataOut;
	size_t geomDestOffset = 0;
	//keep track of total vertices
	U32 totVert = 0;
	//not as useful as it was before, but prevents an infinite loop
	for(U32 i = 0; i < numMeshes; ++i)
	{
		MeshHeader* meshHdr = (MeshHeader*)dataPtr;
		//verify header integrity
		if(meshHdr->Signature == MeshHeader::SIGNATURE)
		{
			//mesh header contains part of the data for a material
			Material mat = Material();
			mat.DiffuseColor = meshHdr->DiffuseColor;
			mat.SpecularColor = meshHdr->SpecularColor;
			mat.EmissiveColor = meshHdr->EmissiveColor;

			//now go to the data for this mesh
			char* meshData = (char*)data + meshHdr->UnitDataStart;
			//read in the GUIDs. They will be in the order
			//	1. Diffuse map
			//	2. Specular map
			//	3. Glow map
			//	4. Normal map
			//and are tightly packed, so this order does matter.
			mat.DiffuseTexGUID = meshHdr->DiffuseGUIDLen > 0 ? ResGUID(meshData) : ResGUID();
			//go to the spec GUID
			meshData += meshHdr->DiffuseGUIDLen;
			mat.SpecularTexGUID = meshHdr->SpecularGUIDLen > 0 ? ResGUID(meshData) : ResGUID();
			//now to the glow map
			meshData += meshHdr->SpecularGUIDLen;
			mat.EmissiveTexGUID = meshHdr->EmissiveGUIDLen > 0 ? ResGUID(meshData) : ResGUID();
			//and finally to the normal map
			meshData += meshHdr->EmissiveGUIDLen;
			mat.NormalMapGUID = meshHdr->NormalMapGUIDLen > 0 ? ResGUID(meshData) : ResGUID();
			LogV(String("Mesh ") + i + ":");
			logMeshHeaderInfo(*meshHdr, &mat);
			totVert += meshHdr->NumVerts;
			//now move past that, to the geometry data.
			meshData += meshHdr->NormalMapGUIDLen;
			//we're going to copy the geometry block to the output buffer.
			//check that the data's not pointing us out of the buffer!
			//VertexHeader* vHdr = (VertexHeader*)meshData;
			size_t bytesRead = (size_t)(meshData - data);
			size_t geomDataSize = meshHdr->VertexDataSize + meshHdr->IndexDataSize;//meshHdr->VertexDataSize + meshHdr->IndexDataSize;
			/*if(vHdr->SIGNATURE != VertexHeader::SIGNATURE)
			{
				LogW(String("Mesh ") + i + " has a malformed vertex header, skipping");
				continue;
			}*/
			//check that the data's not pointing us out of the buffer!
			if(geomDataSize > inSize - bytesRead)
			{
				LogW(String("Mesh ") + i + " has a malformed mesh header, skipping");
				//LogW(String("Mesh ") + i + " has malformed vertex size data " + vHdr->DataSize + ", skipping");
				continue;
			}
			//also forbid writing past the end of the output buffer
			if(geomDestOffset >= outSize)
			{
				LogE("ReadModelMemory: Destination buffer is too small!");
				return false;
			}
			//load and register the actual vertex and index data
			memcpy(geomOutPtr, meshData, geomDataSize);
			TypedArrayHandle<Vertex> verts = HandleMgr::RegisterPtr(geomOutPtr).GetHandle();
			TypedArrayHandle<U32> inds = HandleMgr::RegisterPtr(geomOutPtr + meshHdr->VertexDataSize).GetHandle();
			
			Geometry geom = Geometry();
			geom.Initialize(verts, inds, meshHdr->NumVerts, meshHdr->NumIndices, meshHdr->NumTexChannels);

			//both parts of the mesh should be ready now; attach it to the model
			Mesh mesh = Mesh(mat, geom);

			model.AddMesh(mesh);

			//and advance the output head
			geomOutPtr += geomDataSize;
			//also advance the mesh read head
			dataPtr = (char*)(data + meshHdr->NextMeshOffset);
			if(meshHdr->NextMeshOffset == 0)
			{
				break;
			}
		}
		else
		{
			LogE(String("Mesh header ") + i + "is malformed!");
			return false;
		}
	}

	//log totals
	LogV(String("\tTotal Vertices:\t") + totVert);
	return true;
}

size_t ModelMgr::FindFileSize(const Model& model)
{
	size_t result = 0;
	//files always contain a header
	result += sizeof(ModelHeader);
	//and (numMeshes) mesh headers
	result += model.MeshCount() * sizeof(MeshHeader);
	for(int i = 0; i < model.MeshCount(); ++i)
	{
		const Mesh* mesh = model.GetMesh(i);
		const Geometry& geom = mesh->GetGeometry();
		const Material& mat = mesh->GetMaterial();

		//now there is a varying amount of data; geometry is constant
		//but GUIDs are not
		result += getWrittenLength(mat.DiffuseTexGUID);
		result += getWrittenLength(mat.EmissiveTexGUID);
		result += getWrittenLength(mat.NormalMapGUID);
		result += getWrittenLength(mat.SpecularTexGUID);

		result += geom.VertexSizeAsBuffer();
		result += geom.IndexSizeAsBuffer();
	}
	return result;
}

size_t ModelMgr::FindFileGeomSize(const char* fileBuf)
{
	if(!fileBuf)
	{
		return 0;
	}

	//The start should be a main header.
	const ModelHeader* mainHdr = (ModelHeader*)fileBuf;
	//verify file integrity
	if(mainHdr->Signature != ModelHeader::SIGNATURE)
	{
		LogE("Couldn't verify model header!");
		return 0;
	}

	U16 numMeshes = mainHdr->NumUnits;
	//now move up to and iterate through the mesh headers.
	MeshHeader* meshHdrStart = (MeshHeader*)((char*)fileBuf + sizeof(ModelHeader));
	//char* geomOutPtr = geomDataOut;
	size_t geomSize = 0;
	for(U32 i = 0; i < numMeshes; ++i)
	{
		MeshHeader& meshHdr = meshHdrStart[i];
		//verify header integrity
		if(meshHdr.Signature == MeshHeader::SIGNATURE)
		{
			geomSize += meshHdr.VertexDataSize + meshHdr.IndexDataSize;
		}
	}

	return geomSize;
}