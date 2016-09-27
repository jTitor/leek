#pragma once
#include "OcTreeBase.h"
#include "Rendering/Bounds/Bounds.h"
#include "Rendering/Model.h"

namespace LeEK
{
	/**
	Implementation of the OcTree
	that contains bounding volumes.
	*/
	class BoundsOcTree : public OcTree<Bounds*>
	{
	protected:
		Vector3 getValuePosition(Value data);
		bool compareValue(const Value& val, const Node& node);
	public:
		BoundsOcTree(F32 pRegionSize) : OcTree<Bounds*>(pRegionSize)
		{
		}
		~BoundsOcTree()
		{
		}
	};

	class DebugBoundsOcTree : public BoundsOcTree
	{
	protected:
		Vector3 getValuePosition(Value data);
	public:
		DebugBoundsOcTree(F32 pRegionSize) : BoundsOcTree(pRegionSize)
		{
		}

		//debug functions!
		Vector3 GetNodeCenter(const Vector3& thisSectorCenter, F32 thisSectorSize, ChildLocation subSecLoc)
		{
			return getSubSectorCenter(thisSectorCenter, thisSectorSize, subSecLoc);
		}
		const OcTreeNode<Bounds*>* Root()
		{
			return root;
		}
		/**
		Returns a bounding AABB containing the given subsector.
		*/
		AABBBounds GetSubSectorAABB(OcTreeNode<Bounds*>* node, const Vector3& nodeCenter, F32 nodeSectorSize, ChildLocation subSecLoc)
		{
			return getSubSectorAABB(node, nodeCenter, nodeSectorSize, subSecLoc);
		}
	};
}