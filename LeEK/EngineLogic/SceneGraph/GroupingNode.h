#pragma once
#include "SpatialNode.h"

namespace LeEK
{
	/**
	Denotes a node that has children.
	Note that this implies ModelNodes must be leaf nodes.
	You can access children via their index number; 
	any negative number is guaranteed to be an invalid index.
	*/
	class GroupingNode : public SpatialNode
	{
	protected:
		Vector<SpatialHnd> children;
		typedef Vector<SpatialHnd>::iterator childrenIt;
	public:

		GroupingNode(U32 reserve = 1, U32 growRate = 1);
		virtual ~GroupingNode();

		/**
		Gets the number of reserved elements in the node.
		*/
		U32 GetReserve() const;
		/**
		Gets the number of used elements (actual child nodes) in the node.
		*/
		U32 GetNumChildren() const;
		/**
		Attaches a node to this node as a child.
		Returns the child node's index in this node.
		*/
		I32 AttachChild(SpatialHnd child);
		/**
		Detaches a node that is attached to this node,
		and returns the index where the child was stored.
		If there is no child matching this pointer, returns -1.
		*/
		I32 DetachChild(SpatialHnd child);
		/**
		Detaches a child at the specified index, 
		and returns the detached node.
		If this is an invalid index, returns NULL.
		*/
		SpatialHnd DetachChildAt(I32 index);
		/**
		Directly assigns a child node to the given index.
		If this index contained a child, 
		returns the child node that was at the index.
		*/
		SpatialHnd SetChild(I32 index, SpatialHnd child);
		SpatialHnd GetChild(I32 index);
		//TODO: GetBounds()?

		//TODO
		void OnGetVisibleSet(Culler& culler, bool shouldNotCull, bool pRecalcTrans);
	};
}