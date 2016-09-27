#pragma once
#include "DataTypes.h"
#include "STLContainers.h"

namespace LeEK
{
	/**
	Divides a space containing volumes into traversable nodes.
	*/
	class OcTreeNodeBase
	{
	public:
		/**
		Gives a more readable way to index location.
		*/
		enum ChildLocation
		{
			UPPER_FRONT_RIGHT,
			UPPER_FRONT_LEFT,
			UPPER_BACK_LEFT,
			UPPER_BACK_RIGHT,
			LOWER_FRONT_RIGHT,
			LOWER_FRONT_LEFT,
			LOWER_BACK_LEFT,
			LOWER_BACK_RIGHT,
			LOCATION_COUNT,
			LOCATION_INVALID
		};

		//Maybe base on handles instead?
		OcTreeNodeBase* Parent;
		OcTreeNodeBase* Children[LOCATION_COUNT];
		U16 Level;
		U8 NumChildren; //The number of nodes that are actually children

		OcTreeNodeBase(OcTreeNodeBase* pParent = NULL);
		virtual ~OcTreeNodeBase(void);
		bool IsLeaf() const;
		bool IsRoot() const;
	};

	/**
	The actually useful implementation of the OcTreeNode.
	Carries some form of data.
	*/
	template<typename T> class OcTreeNode : public OcTreeNodeBase
	{
	protected:
		T data;
	public:
		OcTreeNode(OcTreeNodeBase* pParent = NULL, const T& pData = T()) : OcTreeNodeBase(pParent)
		{
			data = pData;
		}
		~OcTreeNode() {}
		const T& Data() const
		{
			return data;
		}
		T& WritableData()
		{
			return data;
		}
		void SetData(const T& pData)
		{
			data = pData;
		}
	};

	/**
	Carries a bucket that can hold mesh bounds.
	*/
	template<typename valueT> class OcTreeBucketNode : public OcTreeNode<Vector<valueT>>
	{
	public:
		static const U32 BUCKET_SIZE = 3;
		OcTreeBucketNode(OcTreeNodeBase* pParent = NULL) : OcTreeNode(pParent, Vector<valueT>())
		{
			//try to set aside some space
			data.reserve(BUCKET_SIZE);
		}
		const Vector<valueT>& Data() const
		{
			return data;
		}
		Vector<valueT>& WritableData()
		{
			return data;
		}
		/**
		Returns true if there is space for another bound to be added.
		*/
		bool HasSpace()
		{
			return data.size() < BUCKET_SIZE;
		}
		/**
		Attempts to insert data into this node.
		Returns true only when data has been successfully inserted.
		*/
		bool PushData(const valueT& pData)
		{
			if(!HasSpace())
			{
				return false;
			}
			data.push_back(pData);
		}
	};
}