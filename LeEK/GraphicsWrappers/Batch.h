#pragma once
#include "Datatypes.h"
#include "Rendering/Shader.h"
#include "DataStructures/STLContainers.h"
#include "Math/Matrix4x4.h"
#include "Math/Vector3.h"
#include "Memory/Handle.h"
#include "Rendering/Geometry.h"
#include "Rendering/Material.h"
#include "Rendering/Texture.h"

namespace LeEK
{
	class IUniformBase
	{
	public:
		enum Type { TYPE_NONE, TYPE_INT, TYPE_FLOAT };
		virtual U32 GetNumElems() = 0;
		virtual Type GetType() = 0;
		virtual const void* GetValue() = 0;
	};

	template<typename T> class IUniform : public IUniformBase
	{
	private:
		T value;
	public:
		IUniform(T newVal) : value(newVal) {}
		U32 GetNumElems() { return 1; }
		const void* GetValue() { return &value; }
	};

	template<typename T> class IArrayUniform : public IUniformBase
	{
	private:
		TypedArrayHandle<T> vals;
		U32 numElems;
		void init(TypedArrayHandle<T> valsParam, U32 numElemsParam)
		{
			vals = valsParam;
			numElems = numElemsParam;
		}
	public:
		IArrayUniform(TypedArrayHandle<T> valArray, U32 numElems)
		{
			init(valArray, numElems);
		}
		IArrayUniform(T* valArray, U32 numElems)
		{
			init(HandleMgr::RegisterPtr(valArray), numElems);
		}
		U32 GetNumElems() { return numElems; }
		const void* GetValue() { return vals.Ptr(); }
	};

	class IntUniform : public IUniform<U32>
	{
	public:
		Type GetType() { return TYPE_INT; }
	};

	class IntArrayUniform : public IArrayUniform<U32>
	{
	public:
		Type GetType() { return TYPE_INT; }
	};

	class FloatUniform : public IUniform<F32>
	{
	public:
		Type GetType() { return TYPE_FLOAT; }
	};

	class FloatArrayUniform : public IArrayUniform<F32>
	{
	public:
		Type GetType() { return TYPE_FLOAT; }
	};

	class BatchCall
	{
	private:
		HashMap<IUniformBase*> uniforms;
		const Geometry* geom;
	public:
		BatchCall()
		{
		}
		~BatchCall() {}
	};

	class Batch
	{
	private:
		Map<Shader*, Deque<BatchCall>> batches;
	public:
		Batch(void);
		~Batch(void);
	};
}
