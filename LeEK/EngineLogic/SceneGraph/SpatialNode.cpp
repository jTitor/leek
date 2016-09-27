#include "SpatialNode.h"

using namespace LeEK;

void SpatialNode::recalcWorldTrans()
{
	if(shouldRecalcTrans)
	{
		if(!parent)
		{
			worldTrans = localTrans;
		}
		else
		{
			worldTrans = localTrans * parent->GetWorldTransform();
		}
		shouldRecalcTrans = false;
	}
}

const SpatialNode* SpatialNode::GetParent() const
{
	return parent;
}

SpatialNode* SpatialNode::GetWritableParent()
{
	return parent;
}

void SpatialNode::SetParent(SpatialNode* newParent)
{
	parent = newParent;
}

const SpatialNode::NodeContainerMode& SpatialNode::GetContainMode() const
{
	return containerMode;
}

const SpatialNode::CullMode& SpatialNode::GetCullMode() const
{
	return cullMode;
}

const Transform& SpatialNode::GetWorldTransform() const
{
	return worldTrans;
}

const Transform& SpatialNode::GetLocalTransform() const
{
	return localTrans;
}

Transform& SpatialNode::WorldTransform()
{
	//mark node for world transform recalculation?
	shouldRecalcTrans = true;
	return worldTrans;
}

Transform& SpatialNode::LocalTransform()
{
	//mark node for world transform recalculation
	shouldRecalcTrans = true;
	return localTrans;
}

void SpatialNode::UpdateGraphInfo()
{
	//TODO
}

void SpatialNode::GetVisibleSet(Culler& culler, bool shouldNotCull, bool pRecalcTrans)
{
	//All node types will enter through this, 
	//so recursive calls will also enter this BEFORE
	//entering their respective node's OnGetVisibleSet.

	//First thing to do is update the world transform if necessary.
	//If this node needs to be recalculated, 
	//ALL children below it must be recalculated; we check for this first.
	pRecalcTrans |= shouldRecalcTrans;
	if(pRecalcTrans)
	{
		//To avoid having another branch here
		//or an extra parameter in GetVisibleSet,
		//we assume this node has a parent.
		//For this to work, the root also needs a parent;
		//this can be a dummy node with an identity transform.
		//Then you just update starting at the scene's root node.
		L_ASSERT(parent && "Attempted to call GetVisibleSet on a parentless node!\nIf you called this on your scene's root node, make sure it's parented to a dummy node!");
		recalcWorldTrans();
	}

	//Now do node-specific work. This can be a recursive call.
	OnGetVisibleSet(culler, shouldNotCull, shouldRecalcTrans);
}

void SpatialNode::OnInsert()
{
	recalcWorldTrans();
}

U32 SpatialNode::GetNumLocalShaders() const
{
	return localShaders.size();
}

SpatialNode::ShaderHnd SpatialNode::GetLocalShader(U32 idx)
{
	if(idx >= 0 && idx < GetNumLocalShaders())
	{
		return localShaders.at(idx);
	}
	return 0;
}

U32 SpatialNode::AttachLocalShader(ShaderHnd shader)
{
	localShaders.push_back(shader);
	return GetNumLocalShaders() - 1;
}

void SpatialNode::DetachLocalShader(U32 idx)
{
	if(idx >= 0 && idx < GetNumLocalShaders())
	{
		localShaders.erase(localShaders.begin() + idx);
	}
}

void SpatialNode::DetachAllLocalShaders()
{
	localShaders.clear();
}

Vector<SpatialNode::ShaderHnd> SpatialNode::FindGlobalShaders()
{
	//Because SpatialNodes themselves can't have children,
	//we know that any any shaders on parents should be considered global shaders
	//on this node.
	auto result = Vector<ShaderHnd>();
	//Traverse each node that has this node as a descendant.
	for(SpatialNode* parent = this->parent; parent != NULL; parent = parent->parent)
	{
		//Insert any parent shaders.
		if(parent->GetNumLocalShaders() > 0)
		{
			for(U32 i = 0; i < parent->GetNumLocalShaders(); ++i)
			{
				result.push_back(parent->GetLocalShader(i));
			}
		}
	}
	return result;
}

U32 SpatialNode::GetNumLocalLights() const
{
	return localLights.size();
}

SpatialNode::LightHnd SpatialNode::GetLocalLight(U32 idx)
{
	if(idx >= 0 && idx < GetNumLocalLights())
	{
		return localLights.at(idx);
	}
	return 0;
}

U32 SpatialNode::AttachLocalLight(LightHnd light)
{
	localLights.push_back(light);
	return GetNumLocalLights() - 1;
}

void SpatialNode::DetachLocalLight(U32 idx)
{
	if(idx >= 0 && idx < GetNumLocalLights())
	{
		localLights.erase(localLights.begin() + idx);
	}
}

void SpatialNode::DetachAllLocalLights()
{
	localLights.clear();
}

Vector<SpatialNode::LightHnd> SpatialNode::FindGlobalLights()
{
	//Because SpatialNodes themselves can't have children,
	//we know that any any lights on parents should be considered global lights
	//on this node.
	auto result = Vector<LightHnd>();
	//Traverse each node that has this node as a descendant.
	for(SpatialNode* parent = this->parent; parent != NULL; parent = parent->parent)
	{
		//Insert any parent lights.
		if(parent->GetNumLocalLights() > 0)
		{
			for(U32 i = 0; i < parent->GetNumLocalLights(); ++i)
			{
				result.push_back(parent->GetLocalLight(i));
			}
		}
	}
	return result;
}