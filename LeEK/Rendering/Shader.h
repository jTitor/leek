#pragma once
#include "Datatypes.h"
#include "Strings/String.h"
#include "Hashing/HashMap.h"

namespace LeEK
{
	/**
	Allows access to handles for a compiled shader and its uniforms.
	Note that this does nothing on its own - 
	it should be obtained from an IGraphicsWrapper.
	*/
	class Shader
	{
	private:
		//parameter list - name and handle
		//HashMap<U32> paramToLocationMap;
		//uniform list
		HashMap<U32> uniformToHandleMap;
		String programName;
		//compiled program handle
		U32 programHandle;
		//does not handle data - you GET a shader
		//from a IGraphicsWrapper.MakeShader function
		void init(String progName, U32 progHandle, HashMap<U32> uniformList)
		{
			programName = progName;
			programHandle = progHandle;
			uniformToHandleMap = uniformList;
		}
	public:
		//Uniforms are passed with the uniform name as key,
		//and the uniform handle as value.
		Shader(String progName, U32 progHandle, HashMap<U32> uniformList)
		{
			init(progName, progHandle, uniformList);
		}
		Shader(void)
		{
			init("", 0, HashMap<U32>());
		}
		~Shader(void) {}
		//properties
		inline U32 ProgramHandle() { return programHandle; }
		inline U32 GetUniformHandle(const String& name) { return uniformToHandleMap[name]; }
		inline U32 GetUniformHandle(const HashedString& name) { return uniformToHandleMap[name]; }
	};
}
