#pragma once
#include "Datatypes.h"
#include "DataStructures/STLContainers.h"
#include "Rendering/Mesh.h"
#include "Math/Vector3.h"

namespace LeEK
{
	/**
	Holds a collection of meshes,
	as well as a bounding box and sphere.
	*/
	class Model
	{
	private:
		Vector<Mesh> meshes;
		Vector3 boundsCenter;
		//used to build AABBs.
		Vector3 aabbHalfBounds;
		//used to build bounding spheres.
		F32 boundingRadius;
	public:
		typedef Vector<Mesh>::const_iterator ConstMeshIt;

		/**
		Returns true if and only if this model has valid bounding data.
		*/
		inline bool HasBounds() const 
		{ 
			return aabbHalfBounds != Vector3::Zero && boundingRadius > 0.0f;
		}
		inline const Vector3& BoundsCenter() const { return boundsCenter; }
		inline Vector3 AABBBounds() const 
		{ 
			return AABBHalfBounds() * 2.0f;
		}
		inline const Vector3& AABBHalfBounds() const 
		{ 
			return aabbHalfBounds;
		}

		inline F32 BoundingRadius() const { return boundingRadius; }
		/**
		Sets the bounding AABB's dimensions to the given value;
		also implicitly sets the bounding radius to conform to the new
		dimensions.
		*/
		void SetAABBBounds(const Vector3& val);
		/**
		Sets the bounding AABB's dimensions to the given half values;
		also implicitly sets the bounding radius to conform to the new
		dimensions.
		*/
		void SetAABBHalfBounds(const Vector3& halfVal);
		void SetBoundsCenter(const Vector3& val);
		//void SetBoundingRadius(F32 val);

		/**
		Gets the number of meshes attached to this model.
		*/
		inline U16 MeshCount() const { return meshes.size(); }
		//TODO: be strict and use handles?
		Mesh* GetMesh(U32 index);
		const Mesh* GetMesh(U32 index) const;
		//returns an iterator to the start of the mesh list
		ConstMeshIt GetMeshBegin() const;
		ConstMeshIt GetMeshEnd() const;

		Model(U32 meshCount = 0);
		Model(const Model& other);
		~Model(void);

		/**
		Fairly self explanatory - adds a mesh to the model.
		Note that this does NOT recalculate mesh bounds.
		*/
		void AddMesh(const Mesh& mesh);
		/**
		Removes all meshes from a model,
		and resets all bounds to zero.
		*/
		void Clear();
		/**
		Recalculates the bounds enclosing all vertices in this mesh.
		If possible, do this offline, 
		as model files store basic bounds data.
		*/
		void RecalcBounds();
		/**
		Reserves the given number of Mesh objects in the model.
		Might be useful if dynamically generating meshes?
		*/
		void Reserve(U16 meshCount) { meshes.reserve(meshCount); }
	};
}
