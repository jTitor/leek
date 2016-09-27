#pragma once
#include <string>

#include "../LeEK/Rendering/Model.h"
#include "../LeEK/Rendering/Geometry.h"

#include "../LeEK/Libraries/Assimp/Importer.hpp"
#include "../LeEK/Libraries/Assimp/scene.h"
#include "../LeEK/Libraries/Assimp/postprocess.h"

using namespace std;

namespace LeEK
{
	namespace ModelConversion
	{
		/**
		Logs details about the imported data.
		*/
		void listMeshDetails(const aiScene* scene);
		void listMaterialDetails(const aiScene* scene);

		/**
		Converts the imported data into a list of Geometry elements.
		*/
		void convertMeshData(const aiScene* scene, Geometry* geomList);
		/**
		Compiles the given Geometry elements into a Model.
		@param model the model to be generated.
		@param geomList the input geometry.
		@param numMeshes the number of meshes in the model.
		*/
		bool buildModel(Model& model, Geometry* geomList, int numMeshes);
		//bool exportModel(Model& model, string outPath);

		/**
		Converts a model file into a Model.
		@param data the model file as a memory buffer.
		@param dataSize the size of the model file.
		@param geomList a Geometry array to be constructed. If this is NOT null, it will be deleted!
		@param outputModel the model to be generated.
		*/
		bool ImportModel(char* data, int dataSize, Geometry* geomList, Model& outputModel);
	}

	inline string operator+(string lhs, int rhs)
	{
		return lhs + std::to_string(rhs);
	}
	inline string operator+(int rhs, string lhs)
	{
		return std::to_string(rhs) + lhs;
	}
	inline string operator+(string lhs, unsigned int rhs)
	{
		return lhs + std::to_string(rhs);
	}
	inline string operator+(unsigned int rhs, string lhs)
	{
		return std::to_string(rhs) + lhs;
	}
	inline string operator+(string lhs, bool rhs)
	{
		return lhs + std::to_string(rhs);
	}
	inline string operator+(bool rhs, string lhs)
	{
		return std::to_string(rhs) + lhs;
	}
	inline string operator+(string lhs, float rhs)
	{
		return lhs + std::to_string(rhs);
	}
	inline string operator+(float rhs, string lhs)
	{
		return std::to_string(rhs) + lhs;
	}
}