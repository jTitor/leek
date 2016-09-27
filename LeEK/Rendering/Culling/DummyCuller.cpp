#include "DummyCuller.h"

using namespace LeEK;

DummyCuller::DummyCuller(void)
{
	allMdls = Vector<VisibleElement>();
}

DummyCuller::~DummyCuller(void)
{
}

void DummyCuller::Insert(TypedHandle<SpatialNode> node, 
	TypedHandle<Shader> globalShader)
{
}

void DummyCuller::OnSceneNodeAdded(TypedHandle<SpatialNode> newNode)
{
	newNode->OnInsert();
	allMdls.push_back(VisibleElement(newNode, newNode->FindGlobalShaders()));
}

void DummyCuller::OnSceneNodeUpdated(TypedHandle<SpatialNode> node)
{
}

void DummyCuller::OnSceneNodeMoved(TypedHandle<SpatialNode> movedNode)
{
}

void DummyCuller::OnSceneNodeRemoved(TypedHandle<SpatialNode> node)
{
	//TODO
}

void DummyCuller::CalcVisibleSet()
{
	visible.Elements = allMdls;
}