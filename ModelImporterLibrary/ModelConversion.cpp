#include "../LeEK/Datatypes.h"
//#include "../LeEK/Constants/AllocTypes.h"
//#include "../LeEK/Memory/Allocator.h"

#include "../LeEK/Math/Vector3.h"
#include "../LeEK/Math/Vector2.h"
#include "../LeEK/Math/Matrix4x4.h"
//#include "../LeEK/Logging/Log.h"

#include "../LeEK/Rendering/Material.h"

#include "../LeEK/Rendering/Model.h"
#include "../LeEK/Rendering/Mesh.h"
#include "../LeEK/Rendering/Color.h"

#include "../LeEK/Time/GameTime.h"
#include "../LeEK/ResourceManagement/ResourceManager.h"
#include "../LeEK/Memory/Handle.h"

#include "ModelLog.h"
#include "ModelConversion.h"

using namespace LeEK;
using namespace ModelConversion;

GameTime gameTime = GameTime();

void ModelConversion::listMeshDetails(const aiScene* scene)
{
	ModelLog::D("Meshes:");
	for(U32 i = 0; i < scene->mNumMeshes; ++i)
	{
		gameTime.Tick();
		ModelLog::V(string("\tMesh ") + i);
		aiMesh* mesh = scene->mMeshes[i];
		ModelLog::V(string("\tName: ") + mesh->mName.C_Str());
		ModelLog::V(string("\tVertices: ") + mesh->mNumVertices);
		ModelLog::V(string("\tFaces: ") + mesh->mNumFaces);
		//and list the indices.
		//we triangulate on import, so we can assume there's
		//3*(num faces) indices.
		ModelLog::V(string("\tIndices: ") + mesh->mNumFaces*3);
		ModelLog::V(string("\tBones: ") + mesh->mNumBones);
		ModelLog::V(string("\tUV Channels: ") + mesh->GetNumUVChannels());
		ModelLog::V(string("\tColor Channels: ") + mesh->GetNumColorChannels());
		ModelLog::V(string("\tHas Normals: ") + mesh->HasNormals());
		ModelLog::V(string("\tHas Tangent Data: ") + mesh->HasTangentsAndBitangents());
		gameTime.Tick();
		F32 readTimeSecs = gameTime.ElapsedGameTime().ToSeconds();
		ModelLog::V(string("Mesh LISTED in ") + readTimeSecs + "s");
	}
}
void ModelConversion::listMaterialDetails(const aiScene* scene)
{
	aiString name;
	aiColor3D color(0,0,0);
	U32 intVal = 0;
	F32 floatVal = 0;
	ModelLog::V("Materials:");
	for(U32 i = 0; i < scene->mNumMaterials; ++i)
	{
		gameTime.Tick();
		ModelLog::V(string("\tMaterial ") + i);
		aiMaterial* mat = scene->mMaterials[i];
		mat->Get(AI_MATKEY_NAME, name);
		ModelLog::V(string("\tName: ") + name.C_Str());
		mat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
		ModelLog::V(string("\tDiffuse: ") + "<" +  color.r + ", " + color.g + ", " + color.b + ">");
		//clear the color
		color = aiColor3D(0,0,0);
		mat->Get(AI_MATKEY_COLOR_SPECULAR, color);
		ModelLog::V(string("\tSpecular: ") + "<" +  color.r + ", " + color.g + ", " + color.b + ">");
		color = aiColor3D(0,0,0);
		mat->Get(AI_MATKEY_COLOR_AMBIENT, color);
		ModelLog::V(string("\tAmbient: ") + "<" +  color.r + ", " + color.g + ", " + color.b + ">");
		mat->Get(AI_MATKEY_ENABLE_WIREFRAME, intVal);
		ModelLog::V(string("\tWireframe: ") + ((bool)intVal));
		intVal = 0;
		mat->Get(AI_MATKEY_TWOSIDED, intVal);
		ModelLog::V(string("\tTwo Sided: ") + ((bool)intVal));
		mat->Get(AI_MATKEY_OPACITY, floatVal);
		ModelLog::V(string("\tOpacity: ") + floatVal);
		floatVal = 0;
		mat->Get(AI_MATKEY_SHININESS, floatVal);
		ModelLog::V(string("\tShininess: ") + floatVal);
		gameTime.Tick();
		F32 readTimeSecs = gameTime.ElapsedGameTime().ToSeconds();
		ModelLog::V(string("Material LISTED in ") + readTimeSecs + "s");
	}
}

void ModelConversion::convertMeshData(const aiScene* scene, Geometry* geomList)
{
	F32 totalLoadTimeSecs = 0;
	//super fun - we need to convert the mesh's vector data to our vector version,
	//then pass that converted data to the geometry.
	//since we're here, might as well dynamically allocate the temporary buffers, too.
	const U32 MAX_ELEMENTS = 1 << 19;
	Color* convertedColor = new Color[MAX_ELEMENTS];
	Vector3* convertedPos = new Vector3[MAX_ELEMENTS];
	Vector3* convertedNorms = new Vector3[MAX_ELEMENTS];
	Vector2* convertedUVs = new Vector2[MAX_ELEMENTS];
	U32* indices = new U32[MAX_ELEMENTS];
	for(U32 i = 0; i < scene->mNumMeshes; ++i)
	{
		gameTime.Tick();
		aiMesh* mesh = scene->mMeshes[i];
		U32 numVerts = mesh->mNumVertices;
		U32 numFaces = mesh->mNumFaces;
		//we can assume this, 
		//since the importer triangulates everything
		U32 numIndices = numFaces*3;
		if(	numVerts >= MAX_ELEMENTS || 
			numFaces >= MAX_ELEMENTS)
		{
			ModelLog::E("Mesh is too big!");
			return;
		}

		//need to rotate the vertices, too.
		//Blender uses +Z as up, +Y forward;
		//we use +Y up, -Z forward
		Matrix4x4 convertUpAxis = Matrix4x4::FromEulerAngles(
								-Math::PI_OVER_2, 0, Math::PI);
		#pragma region Temp Buffer Load
		const bool hasColor = mesh->GetNumColorChannels() > 0;
		const bool hasNormals = mesh->HasNormals();
		const bool hasUVs = mesh->HasTextureCoords(0);
		for(U32 j = 0; j < numVerts; ++j)
		{
			aiVector3D& pos = mesh->mVertices[j];
			Vector3 newPos = Vector3(pos.x, pos.y, pos.z);
			convertedPos[j] = convertUpAxis.MultiplyPoint(newPos);
			if(hasColor)
			{
				aiColor4D& color = mesh->mColors[0][j];
				convertedColor[j] = Color(color.r, color.g, color.b, color.a);
			}
			else
			{
				convertedColor[j] = Colors::White;
			}
			if(hasNormals)
			{
				aiVector3D& norm = mesh->mNormals[j];
				convertedNorms[j] = convertUpAxis.MultiplyVector(Vector3(norm.x, norm.y, norm.z));
			}
			else
			{
				//can't just say it has NO normals...
				convertedNorms[j] = Vector3::Up;
			}
			if(hasUVs)
			{
				aiVector3D& uv = mesh->mTextureCoords[0][j];
				Vector3 rotated = Vector3(uv.x, uv.y, uv.z);
				//rotated = convertUpAxis.MultiplyVector(rotated);
				convertedUVs[j] = Vector2(rotated.X(), rotated.Y());
				//ModelLog::V(string("UV ") + j + ":\t" + convertedUVs[j].ToString());
			}
			else
			{
				convertedUVs[j] = Vector2::Zero;
			}
		}
		//index is a little easier to iterate over.
		for(U32 j = 0; j < numFaces; ++j)
		{
			aiFace& face = mesh->mFaces[j];
			for(U32 k = 0; k < 3; ++k)
			{
				indices[(j*3) + k] = face.mIndices[k];
			}
		}
		#pragma endregion

		//now FINALLY init the geometry.
		ModelLog::D(string("Loading mesh ") + i +
				". Vertices: " + numVerts + 
				", Indices: " + numIndices);
		TypedArrayHandle<Vertex> vertHnd = GeomHelpers::BuildVertArray(convertedPos, 
			convertedNorms, 
			convertedColor, 
			convertedUVs, 
			numVerts);
		if(!vertHnd.GetHandle())
		{
			ModelLog::W(string("Failed to load mesh ") + i + "!");
		}
		U32* inds = new U32[numIndices * 3];
		memcpy(inds, indices, numIndices * sizeof(U32));
		geomList[i].Initialize(vertHnd,
							HandleMgr::RegisterPtr((void*)inds),
							numVerts, 
							numIndices, 
							1);
					
		gameTime.Tick();
		F32 loadTimeSecs = gameTime.ElapsedGameTime().ToSeconds();
		ModelLog::D(string("Mesh loaded in ") + loadTimeSecs + "s");
		totalLoadTimeSecs += loadTimeSecs;
	}
	//now get rid of the buffers!
	delete[] convertedColor;
	delete[] convertedPos;
	delete[] convertedNorms;
	delete[] indices;
	ModelLog::D(string("All meshes loaded in ") + totalLoadTimeSecs + "s");
}

bool ModelConversion::ImportModel(char* data, int dataSize, Geometry* geomList, Model& outputModel)
{
	Assimp::Importer importer;
	importer.SetPropertyInteger(AI_CONFIG_PP_FD_REMOVE, aiPrimitiveType_POINT | aiPrimitiveType_LINE);

	U32 flags = aiProcess_Triangulate |		
				aiProcess_CalcTangentSpace |
				aiProcess_GenUVCoords |
				aiProcess_GenNormals |
				aiProcess_SortByPType |
				aiProcess_OptimizeMeshes |
				aiProcess_ImproveCacheLocality |
				aiProcess_TransformUVCoords |
				aiProcess_FindDegenerates |
				aiProcess_JoinIdenticalVertices |
				aiProcess_RemoveRedundantMaterials |
				aiProcess_LimitBoneWeights |
				aiProcess_SplitLargeMeshes |
				aiProcess_FindInvalidData |
				0;

	//for now, we just want to try navigating the mesh.
	gameTime.Tick();
	const aiScene* scene = importer.ReadFileFromMemory(data, dataSize, flags);
	gameTime.Tick();
	F32 loadTimeSecs = gameTime.ElapsedGameTime().ToSeconds();
	if(!scene)
	{
		//load failed
		ModelLog::E("Load failed! Error:");
		ModelLog::E(importer.GetErrorString());
		return false;
	}
	ModelLog::D(string("Imported model in ") + loadTimeSecs + "s!");
	//begin noting information
	int numMeshes = scene->mNumMeshes;
	ModelLog::D(string("Number of meshes: ") + numMeshes);
	listMeshDetails(scene);
	ModelLog::D(string("Number of materials: ") + scene->mNumMaterials);
	listMaterialDetails(scene);

	//now load meshes!
	//delete the old data if necessary
	if(geomList)
	{
		delete[] geomList;
	}
	geomList = new Geometry[scene->mNumMeshes];
	memset(geomList, 0, scene->mNumMeshes*sizeof(Geometry));

	//convert the Assimp data into a mesh
	convertMeshData(scene, geomList);

	//now compile the model
	if(!buildModel(outputModel, geomList, numMeshes))
	{
		ModelLog::E("Model compilation failed!");
		return false;
	}

	return true;
}

bool ModelConversion::buildModel(Model& model, Geometry* geomList, int numMeshes)
{
	//there must be a geometry list, or this is pointless
	if(geomList == NULL)
	{
		return false;
	}

	//we don't import material data from assimp yet,
	//use a default
	Material defMat;
	defMat.DiffuseColor = Colors::White;
	defMat.SpecularColor = Colors::Black;
	defMat.EmissiveColor = Colors::Black;
	defMat.DiffuseTexGUID = ResGUID();

	//iterate through geometry
	for(U32 i = 0; i < numMeshes; ++i)
	{
		//we don't import material data from assimp yet,
		//use a default
		Mesh mesh(defMat, geomList[i]);
		//and insert the new mesh
		model.AddMesh(mesh);
	}
	//bounds are definitely invalid, recalc
	model.RecalcBounds();
	ModelLog::D("Built model from imported data!");

	return true;
}