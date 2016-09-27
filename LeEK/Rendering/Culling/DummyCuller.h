#pragma once
#include "Culler.h"

namespace LeEK
{
	/**
	A testing culler that does ABSOLUTELY NOTHING;
	the visible set will always be all models in the scene.
	Used for testing performance of actual cullers.
	*/
	class DummyCuller : public Culler
	{
	private:
		Vector<VisibleElement> allMdls;
	public:
		DummyCuller(void);
		~DummyCuller(void);

		void Insert(TypedHandle<SpatialNode> node, 
			TypedHandle<Shader> globalShader);
		void OnSceneNodeAdded(TypedHandle<SpatialNode> newNode);
		void OnSceneNodeUpdated(TypedHandle<SpatialNode> node);
		void OnSceneNodeMoved(TypedHandle<SpatialNode> movedNode);
		void OnSceneNodeRemoved(TypedHandle<SpatialNode> node);

		void CalcVisibleSet();
	};
}
