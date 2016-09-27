#pragma once
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Vector2.h"
#include "Rendering/Color.h"
#include "Memory/Allocator.h"
#include "Memory/Handle.h"

namespace LeEK
{
	/**
	Vertex element. Currently, structure consists of (in order):
	Position (Vector3)
	Color (Vector3)
	Normal (Vector3)
	UV (Vector3)
	*/
	struct Vertex
	{
		//F32 x,y,z;
		//F32 r,g,b;
		Vector3 Position;
		Vector3 Color; //Vector3?
		Vector3 Normal;
		//a primary UV coord?
		Vector3 UV;
		//U32 _pad;
		//rest (mostly UV array) to be added later
		Vertex() : Position(), Color(), Normal(), UV() {}
	};

	/**
	Contains handles to geometry data buffers on system RAM (NOT the graphics memory).
	*/
	template<typename VertType, typename IndType>
	class geometryData
	{
	public:
		//vertex array
		//can load both of these either with memcpy or std::copy
		//since they're both POD
		TypedArrayHandle<VertType> VertexHandle;
		TypedArrayHandle<IndType> IndexHandle;
		U32 VertexCount;
		U32 IndexCount;
		geometryData() : VertexHandle(0), IndexHandle(0), VertexCount(0), IndexCount(0) {}//, uvChannelCount(0) {}
	};

	/**
	Provides handles to geometry data buffers created by the renderer.
	*/
	class geomInfo
	{
	public:
		U32 vertexBufferHandle;
		U32 indexBufferHandle;
		U32 vertexArrayHandle;

		geomInfo(void) : vertexBufferHandle(0), indexBufferHandle(0), vertexArrayHandle(0) {}
		//Geometry(const Geometry& other);
		~geomInfo(void) {}
	};

	/**
	Generic class for holding indexed primitives.
	*/
	template<typename VertType, typename IndType>
	class GeomBase
	{
	protected:
		geometryData<VertType, IndType> data;
		geomInfo info;

	public:
		GeomBase(void) : data(), info() {}
		//Geometry(const Geometry& other);
		~GeomBase(void) {}

		inline const TypedArrayHandle<VertType>& VertexHandle() const { return data.VertexHandle; }
		inline const TypedArrayHandle<IndType>& IndexHandle() const { return data.IndexHandle; }
		inline const VertType* Vertices() const { return VertexHandle().Ptr(); }
		inline const IndType* Indices() const { return IndexHandle().Ptr(); }
		inline U32 VertexCount() const { return data.VertexCount; }
		inline U32 IndexCount() const { return data.IndexCount; }
		inline U32 VertexSizeAsBuffer() const { return VertexCount() * sizeof(VertType); }
		inline U32 IndexSizeAsBuffer() const { return IndexCount() * sizeof(IndType); }

		#pragma region Handle Properties
		//sometimes necessary for OpenGL functions; some expect an array of buffers and will only accept a pointer
		//returns a const pointer so you can't, for instance, delete the handle
		inline const U32* const VertexBufferHandlePtr() const { return &info.vertexBufferHandle; }
		inline const U32 VertexBufferHandle() const { return info.vertexBufferHandle; }
		inline void SetVertexBufferHandle(U32 val) { info.vertexBufferHandle = val; }
		inline const U32* const IndexBufferHandlePtr() const { return &info.indexBufferHandle; }
		inline const U32 IndexBufferHandle() const { return info.indexBufferHandle; }
		inline void SetIndexBufferHandle(U32 val) { info.indexBufferHandle = val; }
		inline const U32* const VertexArrayHandlePtr() const { return &info.vertexArrayHandle; }
		inline const U32 VertexArrayHandle() const { return info.vertexArrayHandle; }
		inline void SetVertexArrayHandle(U32 val) { info.vertexArrayHandle = val; }
		#pragma endregion

		virtual void Initialize(TypedArrayHandle<VertType> verts, TypedArrayHandle<IndType> inds, U32 vertCountParam, U32 indexCountParam)
		{
			data.VertexHandle = verts;
			data.IndexHandle = inds;
			data.VertexCount = vertCountParam;
			data.IndexCount = indexCountParam;
		}
	};

	/** 
	Class for holding vertex and maybe normal data.
	Texture data (or hopefully, handles into the resource system)
	will be handled by an owning Model class.
	*/
	class Geometry : public GeomBase<Vertex, U32> //strange, but necessary to encapsulate state and remain POD
	{
	private:
		U8 uvChannelCount;
		//TODO: put bounds data here?
		//Scaling?
	public:
		Geometry(void) : uvChannelCount(0) {}
		~Geometry(void);

		/**
		Fills vertex and index buffers with data, 
		and specifies how many UV channels this piece of geometry has.
		*/
		void Initialize(TypedArrayHandle<Vertex> verts, TypedArrayHandle<U32> inds, U32 vertCountParam, U32 indexCountParam, U8 uvChannelCountParam);
		inline U8 UVChannelCount() const { return uvChannelCount; }
	};

	namespace GeomHelpers
	{
		TypedArrayHandle<Vertex> BuildVertArray(Vector3* PosList, Vector3* NormList, Color* ColorList, Vector2* UVList, 
						  U32 numVertices);
		/**
		Fills a geometry object with dynamically allocated buffers.
		Remember that it's up to you to delete these buffers!
		*/
		bool BuildGeometry(Geometry& geom, Vector3* PosList, Vector3* NormList, Color* ColorList, Vector2* UVList, U32* IndexList, U32 numVertices, U32 numIndices, U8 numUVChannels);
	}
}
