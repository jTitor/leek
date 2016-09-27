#pragma once
#include "Rendering/Camera/Camera.h"
#include "EngineLogic/SceneGraph/GroupingNode.h"
#include "EngineLogic/SceneGraph/VisibleSet.h"
#include "Rendering/Shader.h"

namespace LeEK
{
	class Culler
	{
	protected:
		TypedHandle<CameraBase> camera;
		//Frustum viewFrust;
		VisibleSet visible;

	public:
		Culler(	I32 pMaxNodes = 0, I32 pGrowRate = 0, 
				TypedHandle<CameraBase> pCamera = 0);
		virtual ~Culler(void);

		/**
		Attempts to insert a node into the culler's visible set.
		*/
		virtual void Insert(TypedHandle<SpatialNode> node, 
							TypedHandle<Shader> globalShader) = 0;
		/**
		Called when a node is added to the scene graph
		the culler is operating on.
		*/
		virtual void OnSceneNodeAdded(TypedHandle<SpatialNode> newNode) = 0;
		/**
		Called when a node in the scene graph
		has changed in a way significant to the culler.
		*/
		virtual void OnSceneNodeUpdated(TypedHandle<SpatialNode> node) = 0;
		/**
		Called when a node in the scene graph that the culler is operating on
		is moved in the scene hierarchy.
		*/
		virtual void OnSceneNodeMoved(TypedHandle<SpatialNode> movedNode) = 0;
		/**
		Called when a node in the scene graph that the culler is operating on
		is removed from the scene.
		*/
		virtual void OnSceneNodeRemoved(TypedHandle<SpatialNode> node) = 0;
		//virtual void CullScene(	const CameraBase& cam, 
		//						const GroupingNode& scene) = 0;
		TypedHandle<CameraBase> GetCamera();
		void SetCamera(TypedHandle<CameraBase> val);
		VisibleSet& GetVisibleSet();
		virtual void CalcVisibleSet() = 0;
	};
}
