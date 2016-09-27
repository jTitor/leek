#pragma once
#include "DataStructures/OcTree.h"
#include "Memory/Handle.h"
#include "EngineLogic/SceneGraph/VisibleSet.h"

namespace LeEK
{
	/**
	Implementation of the OcTree
	that contains visible set elements.
	*/
	class SpatialOcTree : public OcTree<VisibleElement>
	{
	protected:
		Vector3 getValuePosition(const VisibleElement data);
		bool compareValue(const Value& val, const Node& node);
		bool shouldInsert(const Value& pData);
	public:
		SpatialOcTree(F32 pRegionSize) : OcTree<VisibleElement>(pRegionSize)
		{
		}
		~SpatialOcTree()
		{
		}
	};
}