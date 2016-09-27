#pragma once
#include "DataStructures/STLContainers.h"
#include "EngineLogic/Transform.h"
#include "Time/GameTime.h"
#include "Rendering/Shader.h"
#include "Memory/Handle.h"

namespace LeEK
{
	class Culler;
	class LightNode;

	/**
	Denotes an element in the scene graph hierarchy.
	The element might not be visible (like a trigger) or have position
	(like a directional light); this just indicates that there is a thing
	in the scene.
	*/
	class SpatialNode
	{
	public:
		typedef TypedHandle<Shader> ShaderHnd;
		typedef TypedHandle<LightNode> LightHnd;
		/**
		Specifies how this node should be culled.
		CULL_ALWAYS indicates that this node should always be culled,
		CULL_NEVER that the node should never be culled,
		and CULL_DYNAMIC that the node *can* be culled.
		*/
		enum CullMode { CULL_ALWAYS, CULL_NEVER, CULL_DYNAMIC };
		/**
		Specifies if this node can contain other nodes.
		NODE_CONTAINER indicates the node can contain nodes,
		NODE_LEAF indicates that the node cannot contain nodes.
		*/
		enum NodeContainerMode { NODE_CONTAINER, NODE_LEAF };
	protected:
		SpatialNode* parent;
		CullMode cullMode;
		NodeContainerMode containerMode;

		//Rendering fields.
		Vector<ShaderHnd> localShaders;
		Vector<ShaderHnd> globalShaders;

		Transform localTrans;
		Transform worldTrans;
		bool shouldRecalcTrans;

		Vector<LightHnd> localLights;
		Vector<LightHnd> globalLights;
		//bool shouldRecalcLights;

		TypedHandle<SpatialNode> thisHnd;
		SpatialNode()
		{
			containerMode = NODE_LEAF;
			localTrans = Transform();
			worldTrans = Transform();
			localShaders = Vector<ShaderHnd>();
			globalShaders = Vector<ShaderHnd>();
			localLights = Vector<LightHnd>();
			globalLights = Vector<LightHnd>();
			parent = NULL;
			cullMode = CULL_DYNAMIC;
			shouldRecalcTrans = true;
			//shouldRecalcLights = true;
			thisHnd = HandleMgr::RegisterPtr(this);
		}
		//virtual void UpdateWorldBound() = 0;
		void recalcWorldTrans();
		//void PropagateBoundChange();
	public:
		virtual ~SpatialNode()
		{
			HandleMgr::DeleteHandle(thisHnd);
		}

		//Accessors
		SpatialNode* GetWritableParent();
		const SpatialNode* GetParent() const;
		const CullMode& GetCullMode() const;
		const NodeContainerMode& GetContainMode() const;
		void SetCullMode(const CullMode& newMode);
		/**
		Gets the node's world transformation.
		Use this for READING.
		*/
		const Transform& GetWorldTransform() const;
		/**
		Gets the node's editable world transformation.
		Use this for EDITING; this marks the node for internal processing.
		*/
		Transform& WorldTransform();
		/**
		Gets the node's local transformation.
		Use this for READING.
		*/
		const Transform& GetLocalTransform() const;
		/**
		Gets the node's editable local transformation.
		Use this for EDITING; this marks the node for internal processing.
		*/
		Transform& LocalTransform();

		/**
		Updates node data based on the parent's information.
		*/
		virtual void UpdateGraphInfo();

		/**
		Updates any changes to the bounds of this node.
		(Since I'm not sure we'll be using something beyond an octree, this might not be implemented.)
		*/
		//void UpdateBounds();
		/**
		Updates any changes to the node's geometric data.
		Since the engine's not designed to handle animations,
		this probably won't implemented!
		*/
		//void UpdateModel();

		/**
		Callback for visible scene generation.
		@param culler
			The culling object that will generate the set to render.
		@param shouldNotCull
			If true, specifies that this nodes
			will never be culled from the scene.
			Note that this does not prevent 
			this node's children from being culled!
		@param pRecalcTrans
			If true, specifies that this node needs its world transform recalculated.
		*/
		void GetVisibleSet(Culler& culler, bool shouldNotCull, bool pRecalcTrans);
		/**
		Actual function that generates the visible set for this node.
		@param culler
			The culling object that will generate the set to render.
		@param shouldNotCull
			If true, specifies that this nodes
			will never be culled from the scene.
			Note that this does not prevent 
			this node's children from being culled!
		@param pRecalcTrans
			If true, specifies that this node needs its world transform recalculated.
		*/
		virtual void OnGetVisibleSet(Culler& culler, bool shouldNotCull, bool pRecalcTrans) = 0;
		void OnInsert();

		U32 GetNumLocalShaders() const;
		ShaderHnd GetLocalShader(U32 idx);
		U32 AttachLocalShader(ShaderHnd shader);
		void DetachLocalShader(U32 idx);
		void DetachAllLocalShaders();

		/**
		Generates a list of all shaders on parents that affect this node.
		*/
		Vector<ShaderHnd> FindGlobalShaders();

		U32 GetNumLocalLights() const;
		LightHnd GetLocalLight(U32 idx);
		U32 AttachLocalLight(LightHnd light);
		void DetachLocalLight(U32 idx);
		void DetachAllLocalLights();

		/**
		Generates a list of all lights on parents that affect this node.
		*/
		Vector<LightHnd> FindGlobalLights();
	//Considering making this internal use only.
	public:
		void SetParent(SpatialNode* newParent);
	};

	typedef SpatialNode* SpatialPtr;
	typedef TypedHandle<SpatialNode> SpatialHnd;
}