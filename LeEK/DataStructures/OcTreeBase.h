#pragma once
#include "OcTreeNode.h"
#include "Math/Vector3.h"
#include "Rendering/Bounds/Bounds.h"
#include "Rendering/Bounds/AABBBounds.h"

namespace LeEK
{
	/**
	Template deducers to ensure you only use
	OcTreeNodeBase types with this class.
	*/
	template<class T> class NodeTraits
	{
	public:
		const static bool isNodeType = false;
		const static bool isBucketed = false;
	};

	template<typename T> class NodeTraits<OcTreeNode<T>>
	{
	public:
		const static bool isNodeType = true;
		const static bool isBucketed = false;
	};

	template<typename T> class NodeTraits<OcTreeBucketNode<T>>
	{
	public:
		const static bool isNodeType = true;
		const static bool isBucketed = true;
	};

	/**
	Template for octree implementations.
	*/
	template<class nodeT, class valueT> class OcTreeBase
	{
		/*
		Optimization Remarks:
			* Vector3s are often passed by reference;
			as most platforms are 64-bit now anyway, it might be better to focus on
			locality and pass by value.
		*/

		static_assert(	NodeTraits<nodeT>::isNodeType,
						"Cannot instantiate an OcTree with non-OcTreeNodeBase nodes!");
	public:
		typedef nodeT Node;
		typedef valueT Value;
	protected:
		typedef OcTreeNodeBase::ChildLocation ChildLocation;

		Node* root;
		const F32 INIT_REGION_SIZE;
		F32 regionSize;

		/**
		Calculates where the given point is, relative to the center of a sector.
		*/
		ChildLocation getRelLocation(const Vector3& center, const Vector3& point)
		{
			Vector3 disp = point - center;
			//simple enough checks - remember that forward is -1, however!
			//in case of edge cases, always bias for positive value.

			//first - is this above or below center?
			if(disp.Y() >= 0)
			{
				//backwards or forward? (remember that < 0 = forward of center)
				if(disp.Z() < 0)
				{
					//left or right?
					if(disp.X() >= 0)
					{
						return ChildLocation::UPPER_FRONT_RIGHT;
					}
					else
					{
						return ChildLocation::UPPER_FRONT_LEFT;
					}
				}
				else
				{
					if(disp.X() >= 0)
					{
						return ChildLocation::UPPER_BACK_RIGHT;
					}
					else
					{
						return ChildLocation::UPPER_BACK_LEFT;
					}
				}
			}
			//repeat format for this branch.
			else
			{
				//backwards or forward? (remember that < 0 = forward of center)
				if(disp.Z() < 0)
				{
					//left or right?
					if(disp.X() >= 0)
					{
						return ChildLocation::LOWER_FRONT_RIGHT;
					}
					else
					{
						return ChildLocation::LOWER_FRONT_LEFT;
					}
				}
				else
				{
					if(disp.X() >= 0)
					{
						return ChildLocation::LOWER_BACK_RIGHT;
					}
					else
					{
						return ChildLocation::LOWER_BACK_LEFT;
					}
				}
			}
			//should never be able to get here
			L_ASSERT(false && "getRelPosition could not resolve position!");
		}
		/**
		Determines where the child node currently is relative to its parent.
		Returns LOCATION_INVALID if the node has no parent,
		or if the node is somehow not in the parent.
		*/
		ChildLocation getNodeRelLocation(const Node* node)
		{
			auto parent = node->Parent;
			if(!parent)
			{
				return ChildLocation::LOCATION_INVALID;
			}
			//find this node in the parent's children; it better be there
			for(int i = 0; i < ChildLocation::LOCATION_COUNT; ++i)
			{
				if(parent->Children[i] == node)
				{
					return (ChildLocation)i;
				}
			}
			L_ASSERT(false && "Node not found below its parent!");
			return ChildLocation::LOCATION_INVALID;
		}
		Vector3 getSubSectorCenter(const Vector3& thisSectorCenter, F32 thisSectorSize, ChildLocation subSecLoc)
		{
			Vector3 disp = Vector3::Zero;
			switch(subSecLoc)
			{
				case ChildLocation::UPPER_FRONT_RIGHT:
					disp = Vector3::Up + Vector3::Forward + Vector3::Right;
					break;

				case ChildLocation::UPPER_FRONT_LEFT:
					disp = Vector3::Up + Vector3::Forward - Vector3::Right;
					break;

				case ChildLocation::UPPER_BACK_LEFT:
					disp = Vector3::Up - Vector3::Forward - Vector3::Right;
					break;

				case ChildLocation::UPPER_BACK_RIGHT:
					disp = Vector3::Up - Vector3::Forward + Vector3::Right;
					break;

				case ChildLocation::LOWER_FRONT_RIGHT:
					disp = -Vector3::Up + Vector3::Forward + Vector3::Right;
					break;

				case ChildLocation::LOWER_FRONT_LEFT:
					disp = -Vector3::Up + Vector3::Forward - Vector3::Right;
					break;

				case ChildLocation::LOWER_BACK_LEFT:
					disp = -Vector3::Up - Vector3::Forward - Vector3::Right;
					break;

				case ChildLocation::LOWER_BACK_RIGHT:
					disp = -Vector3::Up - Vector3::Forward + Vector3::Right;
					break;

				default:
					//can't really return this center - default to the upper front right
					disp = Vector3::Up + Vector3::Forward + Vector3::Right;
					break;
			}
			//now scale the displacement by this subsector's size
			//remember that this point's the center of the subsector -
			//it must be displaced by 1/2 the subsector's size,
			//and the subsector's edge length is already 1/2 the sector's size
			disp *= thisSectorSize / 4;
			return thisSectorCenter + disp;
		}
		/**
		Calculates the center of the given node.
		*/
		Vector3 getNodeCenter(const Node* node)
		{
			//If this is the root, center's 0
			if(node->IsRoot())
			{
				return Vector3::Zero;
			}
			F32 parentSectorSize = regionSize / (1 << node->Parent->Level);
			//otherwise, figure out where we are relative to the parent
			//and displace by that
			return getSubSectorCenter(getNodeCenter((const Node*)node->Parent), parentSectorSize, getNodeRelLocation(node));
		}
		bool isInNodeSpace(const Node* node, const Vector3& pt)
		{
			F32 sectorSize = regionSize / (1 << node->Level);
			Vector3 sectorCenter = getNodeCenter(node);
			F32 x = pt.X();
			F32 y = pt.Y();
			F32 z = pt.Z();
			F32 xMin, xMax, yMin, yMax, zMin, zMax;
			xMin = sectorCenter.X() - sectorSize;
			xMax = sectorCenter.X() + sectorSize;
			yMin = sectorCenter.Y() - sectorSize;
			yMax = sectorCenter.Y() + sectorSize;
			zMin = sectorCenter.Z() - sectorSize;
			zMax = sectorCenter.Z() + sectorSize;
			return	(x >= xMin && x <= xMax) &&
					(y >= yMin && y <= yMax) &&
					(z >= zMin && z <= zMax);
		}

		/**
		Generates an AABB bounding the given subsector of the node.
		@param node the node containing the desired subsector.
		@param nodeCenter the center of the node.
		@param nodeSectorSize the size of the node;
		specifically the length of any edge on the node's sector.
		@param ChildLocation the location of the child node to generate
		the subsector bounds from, relative to the node.
		*/
		AABBBounds getSubSectorAABB(Node* node, const Vector3& nodeCenter, F32 nodeSectorSize, ChildLocation subSecLoc)
		{
			//we know the edge length, at least
			F32 subSecSize = nodeSectorSize/2;
			Vector3 subSecCenter = getSubSectorCenter(nodeCenter, nodeSectorSize, subSecLoc);
			return AABBBounds(subSecCenter, subSecSize);
		}
		virtual Vector3 getValuePosition(Value data) = 0;

		/**
		*/
		Node* insertAtPosition(Node* node, ChildLocation childLoc, const Value& pData)
		{
			if(node->Children[childLoc] != NULL)
			{
				L_ASSERT(false && "Attempted insertAtPosition(), but the node's given subsector already exists!");
				return NULL;
			}

			Node* child = LNew(Node, AllocType::DATASTRUCT_ALLOC, "DataStructAlloc")
						(node);
			child->SetData(pData);
			onInsert(child, pData);
			node->Children[childLoc] = child;
			node->NumChildren++;
			return child;
		}

		/**
		Removes the child at the given location, but does not delete the child node.
		If this leaves the node with no children, recurses with DELETION on the node itself.
		If this leaves the node with one child, promotes that child
		to the node's position.
		*/
		Node* popAtPosition(Node* node, ChildLocation childLoc)
		{
			if(!node || childLoc == ChildLocation::LOCATION_INVALID)
			{
				return NULL;
			}
			auto child = node->Children[childLoc];
			//quit if the child doesn't actually exist
			if(!child)
			{
				return NULL;
			}
			node->Children[childLoc] = NULL;
			node->NumChildren--;

			//if this node's root, quit right here
			if(node->IsRoot())
			{
				return (Node*)child;
			}

			//if this leaves the node with no children,
			//remove the node from its parent
			if(node->NumChildren == 0)
			{
				ChildLocation thisLoc = getNodeRelLocation(node);
				removeAtPosition((Node*)node->Parent, thisLoc);
			}
			//if it leaves the node with one child, promote that child
			if(node->NumChildren == 1)
			{
				promoteNodeChild(node);
			}

			//now return the popped child
			return (Node*)child;
		}

		/**
		Removes the child at the given location.
		If this leaves the parent node with no children, recurses on the node itself.
		If this leaves the parent node with one child, promotes that child
		to the node's position.
		*/
		void removeAtPosition(Node* node, ChildLocation childLoc)
		{
			if(!node || childLoc == ChildLocation::LOCATION_INVALID)
			{
				return;
			}

			auto child = node->Children[childLoc];
			//quit if the child doesn't actually exist
			if(!child)
			{
				return;
			}
			node->Children[childLoc] = NULL;
			node->NumChildren--;
			CustomDelete(child);

			//if this node's root, quit right here
			if(node->IsRoot())
			{
				return;
			}

			//if this leaves the node with no children,
			//remove the node from its parent
			if(node->NumChildren == 0)
			{
				ChildLocation thisLoc = getNodeRelLocation(node);
				removeAtPosition((Node*)node->Parent, thisLoc);
			}
			//if it leaves the node with one child, promote that child
			if(node->NumChildren == 1)
			{
				promoteNodeChild(node);
			}
		}

		void updateChildrenLevel(Node* node)
		{
			if(node->IsLeaf())
			{
				return;
			}

			for(int i = 0; i < ChildLocation::LOCATION_COUNT; ++i)
			{
				Node* child = (Node*)node->Children[i];
				if(child != NULL)
				{
					if(child->Level - node->Level > 1)
					{
						child->Level = node->Level + 1;
						//recurse on children
						updateChildrenLevel(child);
					}
				}
			}
		}

		/**
		If the node has only one child, moves that child
		to the node's position in the tree.
		Recurses on the node's parent if the parent
		also has only one child.
		*/
		void promoteNodeChild(Node* node)
		{
			if(node->NumChildren != 1 || !node->Parent)
			{
				return;
			}
			//find that last child
			Node* lastChild = NULL;
			int lastChildIdx = -1;
			for(int i = 0; i < ChildLocation::LOCATION_COUNT; ++i)
			{
				if(node->Children[i] != NULL)
				{
					lastChild = (Node*)node->Children[i];
					lastChildIdx = i;
					break;
				}
			}
			//also find our position in the parent
			ChildLocation nodeLoc = getNodeRelLocation(node);
			Node* parent = (Node*)node->Parent;
			//simple to swap out - make that child's parent pointer point to *our*
			//parent
			lastChild->Parent = parent;
			//update the child's level
			lastChild->Level = node->Level;
			//recurse on children if necessary
			updateChildrenLevel(lastChild);
			//make our parent's subsector point to the child
			parent->Children[nodeLoc] = lastChild;
			//then delete the operative node
			L_ASSERT(lastChildIdx >= 0 && "Octree node is supposed to have a child to promote!");
			node->Children[lastChildIdx] = NULL;
			node->Parent = NULL;
			LDelete(node);

			//NOTE: could repeat this on parent if necessary?
			if(parent->NumChildren == 1)
			{
				promoteNodeChild(parent);
			}
		}

		//Virtual operations called during tree queries and modifications.
		//For a non-bucketed tree, these generally don't need to be overloaded.
		//They have to be overloaded for a bucketed tree, however.
		/**
		Determines if the given value should be inserted into the tree.
		*/
		virtual bool shouldInsert(const Value& pData)
		{
			return true;
		}
		/**
		Called on each node when doInsert is called,
		allowing implementation-specific work to be done.
		Returns true if this function finished inserting the data passed to doInsert()
		and nothing more needs to be done.
		*/
		virtual bool onInsert(Node* node, const Value& pData)
		{
			return false;
		}
		/**
		Called during doInsert, when the node must reinsert a child node
		after having created a container node for the child and the node to be inserted.
		This can invalidate the child pointer, as when a bucketed node is reinserted.
		*/
		virtual void onReinsertChild(Node* container, const Vector3& containerCenter, F32 containerSize, Node* child)
		{
			L_ASSERT(container->IsLeaf() && "Attempted onReinsertChild() on an internal node that already has children!");
			ChildLocation childLoc = getRelLocation(containerCenter, getValuePosition(child->Data()));
			container->Children[childLoc] = child;
			container->NumChildren++;
			child->Parent = container;
			child->Level = container->Level + 1;
		}
		/**
		Called during doRemove(), when the given node is confirmed
		to have the necessary data.
		Returns true if the passed node can now be removed from
		the tree.
		*/
		virtual bool onRemoveData(Node* node, const Value& toRemove)
		{
			return true;
		}
		/**
		Determines if a given node has the requested data.
		*/
		virtual bool nodeHasData(Node* node, const Value& toRemove)
		{
			//First check that the node exists;
			//then try to check reference equality.
			//If that fails, check value equality.
			return (node != NULL && (&node->Data() == &toRemove || compareValue(toRemove, *node)));
		}
		/**
		Called during doFindAllInBounds(), when the given node
		is confirmed to be within the bounds.
		The node is known to be a leaf node.
		*/
		virtual void addLeafToResults(Node* node, Vector<Value>* resList)
		{
			//just add the leaf's data
			resList->push_back(node->Data());
		}
		/**
		Called when an internal node is traversed in
		doFindAllInBounds(), before the node's children are traversed.
		*/
		virtual void onContainerPreInsert(Node* node, Vector<Value>* resList) {}
		/**
		Called when an internal node is traversed in
		doFindAllInBounds(), after the node's children are traversed.
		*/
		virtual void onContainerPostInsert(Node* node, Vector<Value>* resList) {}
		/**
		Called when a value must be compared against a node.
		Should return true if val equals node's value, and should return false
		otherwise.
		*/
		virtual bool compareValue(const Value& val, const Node& node) = 0;
		Node* doInsert(Node* node, const Vector3& nodeCenter, F32 nodeSectorSize, const Value& pData)
		{
			//do implementation-specific work
			if(!shouldInsert(pData))
			{
				return NULL;
			}

			if(onInsert(node, pData))
			{
				node->NumChildren++;
				return node;
			}

			//figure out what sector this new node's in
			ChildLocation childLoc = getRelLocation(nodeCenter, getValuePosition(pData));
			U32 level = node->Level;
			//is that sector empty?
			Node* child = (Node*)node->Children[childLoc];
			if(child == NULL)
			{
				//good, make a node for the data and insert
				return insertAtPosition(node, childLoc, pData);
			}
			//We need to subdivide the sector this node
			//will reside in.
			F32 subSecSize = nodeSectorSize / 2;
			Vector3 subSecCenter = getSubSectorCenter(nodeCenter, nodeSectorSize, childLoc);
			//If the child's a leaf, it's content;
			//we now need to put it and the new data in a container
			if(child->IsLeaf())
			{
				//make the current node a container node
				Node* container = LNew(Node, AllocType::DATASTRUCT_ALLOC, "DataStructAlloc")
												(node);
				//Node* swapChild = child;
				//put the container where the child used to be
				node->Children[childLoc] = container;
				//node->NumChildren++;
				child->Parent = NULL;
				//insert the new node
				//and reinsert the child's *data* and the new node
				onReinsertChild(container, subSecCenter, subSecSize, child);
				return doInsert(container, subSecCenter, subSecSize, pData);
			}
			//otherwise the child's an organizing node;
			//recurse on it
			return doInsert(child, subSecCenter, subSecSize, pData);
		}
		void doRemove(Node* node, const Vector3& nodeCenter, F32 nodeSectorSize, const Value& toRemove)
		{
			//Checking against children - quit if we don't have any.
			if(!node || node->IsLeaf())
			{
				return;
			}
			//otherwise, try to get the child node
			ChildLocation childLoc = getRelLocation(nodeCenter, getValuePosition(toRemove));

			Node* child = (Node*)node->Children[childLoc];
			//This is pointless if the child doesn't exist.
			if(child == NULL)
			{
				return;
			}
			//If the child's a leaf and content matches, we can remove.
			if(child->IsLeaf() && nodeHasData(child, toRemove))
			{
				//Certain implementations (mostly bucketed trees)
				//may perform some task that makes it unnecessary
				//to actually move nodes; only remove the node
				//if the implementation says to.
				if(onRemoveData(child, toRemove))
				{
					removeAtPosition(node, childLoc);
				}

				//We're done!
				return;
			}
			//Otherwise, we need to recurse on the child.
			Vector3 childCenter = getSubSectorCenter(	nodeCenter, 
														nodeSectorSize,
														childLoc);
			doRemove(child, childCenter, nodeSectorSize/2, toRemove);
		}
		/**
		Fills a given list with all values within the given bounds.
		*/
		void doFindAllInBounds(Node* node, const Vector3& nodeCenter, F32 nodeSectorSize, Vector<Value>* resList, const Bounds& bnd)
		{
			//check on each subsector -
			//do the bounds collide with the subsector?
			for(int i = 0; i < ChildLocation::LOCATION_COUNT; ++i)
			{
				Node* child = (Node*)node->Children[i];
				if(child != NULL)
				{
					AABBBounds subSecBnds = getSubSectorAABB(node, nodeCenter, nodeSectorSize, (ChildLocation)i);
					if(bnd.Test(subSecBnds))
					{	
						//if this is a leaf, add data
						if(child->IsLeaf())
						{
							addLeafToResults(child, resList);
						}
						//this node is just a container; check children
						else
						{
							onContainerPreInsert(child, resList);
							doFindAllInBounds(child, subSecBnds.Center(), subSecBnds.GetWidth(), resList, bnd);
							onContainerPostInsert(child, resList);
						}
					}
				}
			}
		}

		/**
		Recursive call for Find().
		*/
		Node* doFind(Value& val, Node* currRoot, const Vector3& valPos)
		{
			if(currRoot == NULL)
			{
				return NULL;
			}
			//First, figure out what direction this value would be in.
			ChildLocation childLoc = getRelLocation(getNodeCenter(currRoot), valPos);
			//This node, if it's not empty, is either the matching node
			//or can contain the matching node.
			Node* child = currRoot->Children[childLoc];
			if(child == NULL)
			{
				return NULL;
			}
			//Do the values match?
			if(compareValue(val, *child))
			{
				return child;
			}
			//If not, recurse on child.
			return doFind(val, child, valPos);
		}
	public:
		OcTreeBase(F32 pRegionSize = 100.0f) : INIT_REGION_SIZE(pRegionSize)
		{
			regionSize = INIT_REGION_SIZE;
			root = LNew(Node, AllocType::DATASTRUCT_ALLOC, "DataStructAlloc")();
		}
		F32 RegionSize() const { return regionSize; }
		Node* Insert(const Value& pData)
		{
			return doInsert(root, Vector3::Zero, regionSize, pData);
		}
		/**
		Removes the given value from the octree.
		*/
		virtual void Remove(const Value& valToRemove)
		{
			return doRemove(root, Vector3::Zero, regionSize, valToRemove);
		}
		/**
		Removes the given value from the given node in the octree,
		if the node exists.
		*/
		void RemoveAt(const Value& valToRemove, Node* node)
		{
			//Ensure the node exists and contains the value.
			if(!node || !nodeHasData(node, valToRemove))
			{
				return;
			}

			//Check that the implementation will allow a removal;
			//if so, get our relative location to the parent and remove.
			if(onRemoveData(node, valToRemove))
			{
				ChildLocation childLoc = getNodeRelLocation(node);
				if(childLoc == ChildLocation::LOCATION_INVALID)
				{
					return;
				}
				removeAtPosition(node, childLoc);
			}
		}
		/**
		Returns a list of values within the given bounds.
		*/
		Vector<Value> FindAllInBounds(const Bounds& bnd)
		{
			Vector<Value> results = Vector<Value>();
			doFindAllInBounds(root, Vector3::Zero, regionSize, &results, bnd);
			return results;
		}
		/**
		If necessary, moves the given node to its proper location in the tree.
		*/
		Node* UpdateNode(Node* node)
		{
			//should ONLY operate on content nodes.
			if(!node->IsLeaf() || !node->Parent)
			{
				return node;
			}
			bool shouldMove = !isInNodeSpace(node, getValuePosition(node->Data()));
			ChildLocation oldLoc = getNodeRelLocation(node);
			if(!shouldMove)
			{
				//recheck where this node is relative to the parent
				Vector3 parentCenter = getNodeCenter((const Node*)node->Parent);
				//recalculate relative position
				ChildLocation newLoc = getRelLocation(parentCenter, getValuePosition(node->Data()));
				if(	oldLoc == ChildLocation::LOCATION_INVALID ||
					newLoc == ChildLocation::LOCATION_INVALID)
				{
					return node;
				}
				shouldMove |= newLoc != oldLoc;
			}
			//if this and current location differ, remove and reinsert
			if(shouldMove)
			{
				popAtPosition((Node*)node->Parent, oldLoc);
				Node* res = doInsert(root, Vector3::Zero, regionSize, node->Data());
				LDelete(node);
				return res;
			}
			return node;
		}
		/**
		Attempts to find a node with the given value.
		*/
		Node* Find(const Value& val)
		{
			//Convert the value to a position...
			Vector3 valPos = getValuePosition(val);
			//Now fire off a recursive call.
			return doFind(val, root, valPos);
		}
	};

	template<class Value> class OcTree : public OcTreeBase<OcTreeNode<Value>, Value>
	{
	public:
		OcTree(F32 pRegionSize = 100.0f) : OcTreeBase(pRegionSize)
		{
		}
	};

	/**
	Template for octree implementations using a bucketed node type.
	*/
	template<class Value> class BucketedOcTreeBase : public OcTreeBase<OcTreeBucketNode<Value>, Value>
	{
		typedef BucketedOcTreeBase<Value> Node;
		static_assert(	NodeTraits<Node>::isBucketed,
						"Cannot instantiate an BucketedOcTree with non-OcTreeBucketNode nodes!");
	protected:
		bool onInsert(Node* node, ChildLocation childLoc, const Value& pData)
		{
			//check if this node has space
			if(!node->IsRoot() && node->HasSpace())
			{
				//if it does, just insert and quit
				node->PushData(pData);
				return true;
			}
			return false;
		}
		void onReinsertChild(Node* container, const Vector3& containerCenter, F32 containerSize, Node* child)
		{
			//Need to iterate over EACH element in the node.
			for(int i = 0; i < child->Data().size(); ++i)
			{
				//Reinsert each element.
				doInsert(container, containerCenter, containerSize, child->Data()[i]);
			}
			
			//Afterwards, delete the child.
			LDelete(child);
		}
		bool onRemoveData(Node* node, const Value& toRemove)
		{
			//try and find the element
			for(int i = 0; i < node->Data().size(); ++i)
			{
				if(node->Data()[i] == toRemove)
				{
					node->Data().erase(node->Data().begin() + i);
					break;
				}
			}
			//only mark for deletion if there's now no data in the bucket
			if(node->Data().size() == 0)
			{
				return true;
			}
			return false;
		}
		bool nodeHasData(Node* node, const Value& toRemove)
		{
			//do any of the elements match?
			for(int i = 0; i < node->Data().size(); ++i)
			{
				if(node->Data()[i] == toRemove)
				{
					return true;
				}
			}
			return false;
		}
		void addLeafToResults(Node* node, Vector<Value>* resList)
		{
			//insert any data in this bucket
			for(int i = 0; i < node->Data().size(); ++i)
			{
				resList.push_back(child->Data()[i]);
			}
		}
	};
}