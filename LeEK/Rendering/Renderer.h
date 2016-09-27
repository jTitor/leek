#pragma once
#include "GraphicsWrappers/IGraphicsWrapper.h"
#include "Math/Matrix4x4.h"
#include "DataStructures/STLContainers.h"
#include "Rendering/Camera/Camera.h"
#include "EngineLogic/SceneGraph/GroupingNode.h"
#include "Culling/Culler.h"
#include "ResourceManagement/ResourceManager.h"

namespace LeEK
{
	/**
	High level system responsible for rendering to the screen.
	*/
	class Renderer
	{
	private:
		//transform stack
		//maybe use a Transform object rather than matrix?
		Vector<Matrix4x4> worldStack;
		GfxWrapperHandle gfx;
		CameraHandle camera;
		TypedHandle<GroupingNode> sceneRoot;
		TypedHandle<Culler> culler;
		TypedHandle<ResourceManager> resMgr;
		ResGUID defaultTexGUID;
		ResPtr defTexPtr;
		Texture2D* defaultTex;

		//Stats.
		//Can probably be kept in a compiler option, or something.
		U64 numModelsDrawn;

		void notifyCullerNodeAdded(SpatialHnd node);
		void notifyCullerNodeMoved(SpatialHnd node);
		void notifyCullerNodeRemoved(SpatialHnd node);
		void notifyCullerNodeUpdated(SpatialHnd node);
	protected:
		void setTextures(const Material& mat);
		/**
		Draws the specified model to the drawing surface.
		@param model the model to be drawn. All of its geometry must have been initialized via InitGeometry().
		*/
		void drawModel(Model& model);
		void setLightUniforms(const Shader& shader);
		void setTexUniforms(const Shader& shader);
		void onDraw(Model& model, TypedHandle<Shader> shader, const Matrix4x4& worldMat);
	public:
		Renderer(	GfxWrapperHandle pGfx, CameraHandle pCam,
					TypedHandle<Culler> pCuller, TypedHandle<ResourceManager> pResMgr,
					const ResGUID& pDefTex);
		~Renderer(void);
		//void Draw(VisibleSet set)
		const GfxWrapperHandle& GetGraphicsWrapper() const;
		void SetGraphicsWrapper(GfxWrapperHandle newGfx);

		const CameraHandle& GetCamera() const;
		void SetCamera(CameraHandle newCam);

		const TypedHandle<GroupingNode>& GetSceneRoot() const;
		void SetSceneRoot(TypedHandle<GroupingNode> newRoot);

		const TypedHandle<Culler>& GetCuller() const;
		void GetCuller(TypedHandle<Culler> newCuller);

		const Matrix4x4& GetCurrWorldMatrix() const;

		U64 GetNumModelsDrawn() const;
		//void PushWorldMatrix(const Matrix4x4& mat);
		//Matrix4x4 PopWorldMatrix();

		void Init();

		//Hierarchy methods
		/**
		Attempts to insert a node into the scene graph at the given position.
		@param node the node to be inserted. May contain descendant nodes.
		@param parent the node that will serve as <b>node</b>'s parent.
		If null, the root will be used as the parent.
		*/
		void InsertNodeAt(TypedHandle<SpatialNode> node, TypedHandle<SpatialNode> parent = 0);
		/**
		Attempts to change the parent of the given node.
		@param node the node to be moved in the hierarchy.
		@param newParent the node that will be the new parent of <b>node</b>.
		If null, the root will serve as the node's new parent.
		*/
		void MoveNode(TypedHandle<SpatialNode> node, TypedHandle<SpatialNode> newParent = 0);
		/**
		Removes the given node from the scene graph. Note that the node is <i>not</i> deleted;
		the caller must delete the node.
		*/
		void RemoveNode(TypedHandle<SpatialNode> node);
		/**
		Refreshes the node's representation in the culler as needed.
		*/
		void UpdateNode(TypedHandle<SpatialNode> node);

		/**
		Draws the visible elements of the scene graph.
		*/
		void DrawScene();
	};
}
