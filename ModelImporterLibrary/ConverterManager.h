#pragma once
#include "../LeEK/Datatypes.h"
#include "../LeEK/Rendering/Model.h"

namespace LeEK
{
	class ConverterManager
	{
	public:
		ConverterManager(void);
		~ConverterManager(void);

		static Model& GetModel(int modelIdx);
		static int GetNumModels();
		static int GetModelConvertedSize(int modelIdx);
		/**
		Converts a buffer of data into a model.
		If successful, returns the model's index.
		If conversion failed, returns -1.
		*/
		static int ImportModel(char* data, size_t dataSize);
		/**
		Builds a model from a .lmdl file 
		stored in a memory buffer.
		If successful, returns the model's index.
		If load +failed, returns -1.
		*/
		static int LoadModel(char* data, size_t dataSize);
	};
}