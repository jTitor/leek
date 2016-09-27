#pragma once
#include "Culler.h"
#include "Rendering/Bounds/AABBBounds.h"
#include "SpatialOcTree.h"
#include "Hashing/HashTable.h"

namespace LeEK
{
	/**
	Culler that uses an octree to spatially sort geometry in the scene.
	*/
	class OcTreeCuller : public Culler
	{
	private:
		SpatialOcTree ocTree;
		//TODO: This would be better implemented as a hash table.
		//Update HashMap to hash arbitrary elements.
		//(Refactor it to HashTable, too; HashMap is a huge misnomer...)
		typedef HashTable<TypedHandle<SpatialNode>, SpatialOcTree::Node*> NodeToElemMap;
		NodeToElemMap nodeToElem;
		SpatialOcTree::Node* findVisElem(TypedHandle<SpatialNode> node);
	public:
		OcTreeCuller(void);
		~OcTreeCuller(void);

		void Insert(TypedHandle<SpatialNode> node, 
					TypedHandle<Shader> globalShader);
		void OnSceneNodeAdded(TypedHandle<SpatialNode> newNode);
		void OnSceneNodeUpdated(TypedHandle<SpatialNode> node);
		void OnSceneNodeMoved(TypedHandle<SpatialNode> movedNode);
		void OnSceneNodeRemoved(TypedHandle<SpatialNode> node);

		void CalcVisibleSet();
	};
}
