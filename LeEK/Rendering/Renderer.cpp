#include "Renderer.h"

using namespace LeEK;

Renderer::Renderer(GfxWrapperHandle pGfx, CameraHandle pCam, TypedHandle<Culler> pCuller, TypedHandle<ResourceManager> pResMgr, const ResGUID& pDefTex)
{
	worldStack = Vector<Matrix4x4>();
	gfx = pGfx;
	camera = pCam;
	sceneRoot = HandleMgr::RegisterPtr(LNew(GroupingNode, AllocType::RENDERER_ALLOC, "RendererAlloc")());
	culler = pCuller;
	resMgr = pResMgr;

	defaultTexGUID = pDefTex;
	defaultTex = NULL;
	defTexPtr = NULL;
	
	numModelsDrawn = 0;

	//Culler really should be using the renderer's camera.
	if(camera)
	{
		culler->SetCamera(camera);
	}
}

Renderer::~Renderer(void)
{
}

void Renderer::notifyCullerNodeAdded(SpatialHnd node)
{
	//The culler will only ever care about geometry;
	//only try inserting leaf nodes.
	if(node->GetContainMode() == SpatialNode::NODE_LEAF)
	{
		culler->OnSceneNodeAdded(node);
		return;
	}
	//Otherwise, recurse on children.
	auto asGroupingNode = (TypedHandle<GroupingNode>)node;
	for(U32 i = 0; i < asGroupingNode->GetNumChildren(); ++i)
	{
		notifyCullerNodeAdded(asGroupingNode->GetChild(i));
	}
}

void Renderer::notifyCullerNodeMoved(SpatialHnd node)
{
	//The culler will only ever care about geometry;
	//only notify on leaf nodes.
	if(node->GetContainMode() == SpatialNode::NODE_LEAF)
	{
		culler->OnSceneNodeMoved(node);
		return;
	}
	//Otherwise, recurse on children.
	auto asGroupingNode = (TypedHandle<GroupingNode>)node;
	for(U32 i = 0; i < asGroupingNode->GetNumChildren(); ++i)
	{
		notifyCullerNodeMoved(asGroupingNode->GetChild(i));
	}
}


void Renderer::notifyCullerNodeRemoved(SpatialHnd node)
{
	//The culler will only ever care about geometry;
	//only try removing leaf nodes.
	if(node->GetContainMode() == SpatialNode::NODE_LEAF)
	{
		culler->OnSceneNodeRemoved(node);
		return;
	}
	//Otherwise, recurse on children.
	auto asGroupingNode = (TypedHandle<GroupingNode>)node;
	for(U32 i = 0; i < asGroupingNode->GetNumChildren(); ++i)
	{
		notifyCullerNodeRemoved(asGroupingNode->GetChild(i));
	}
}

void Renderer::notifyCullerNodeUpdated(SpatialHnd node)
{
	//The culler will only ever care about geometry;
	//only try updating leaf nodes.
	if(node->GetContainMode() == SpatialNode::NODE_LEAF)
	{
		culler->OnSceneNodeUpdated(node);
		return;
	}
	//Otherwise, recurse on children.
	auto asGroupingNode = (TypedHandle<GroupingNode>)node;
	for(U32 i = 0; i < asGroupingNode->GetNumChildren(); ++i)
	{
		notifyCullerNodeUpdated(asGroupingNode->GetChild(i));
	}
}

void Renderer::setTextures(const Material& mat)
{
	//need to setup mesh uniforms;
	//since system doesn't use materials yet,
	//that's just the texture.
	//use a default material if there's no texture specified
	ResPtr texPtr = resMgr->GetResource(mat.DiffuseTexGUID);
	const Texture2D& texRef = texPtr != NULL ? *(Texture2D*)texPtr->Buffer() : *defaultTex;
	//should really poll for if a shader uses teexture samplers.
	gfx->SetTexture(texRef, TextureMeta::DIFFUSE);
	texPtr = resMgr->GetResource(mat.SpecularTexGUID);
}

void Renderer::drawModel(Model& model)
{
	Texture2D& defTexRef = *(Texture2D*)defTexPtr->Buffer();

	for(U32 i = 0; i < model.MeshCount(); ++i)
	{
		//Get all the needed data.
		const Mesh& mesh = *model.GetMesh(i);
		const Material& mat = mesh.GetMaterial();
		const Geometry& geom = mesh.GetGeometry();
		setTextures(mat);
		gfx->Draw(geom);
	}
}

void Renderer::setLightUniforms(const Shader& shader)
{
	//Of course this'll be fixed with light nodes.
	gfx->SetVec3Uniform("lightDiffuse", Vector3::One);
	gfx->SetVec3Uniform("lightPos", Vector3::Zero);
}

void Renderer::setTexUniforms(const Shader& shader)
{
	//This needs to be decided via the loaded shader element.
	gfx->SetIntUniform("diffTex", TextureMeta::DIFFUSE);
}

void Renderer::onDraw(Model& model, TypedHandle<Shader> shader, const Matrix4x4& worldMat)
{
	gfx->SetShader(shader);
	setLightUniforms(*shader);
	setTexUniforms(*shader);
	//Each shader program might not get
	//access to the WVP matrix if we don't specify in the loop.
	gfx->SetWorldViewProjection(worldMat, camera->GetViewMatrix(), camera->GetProjMatrix());
	drawModel(model);
}

const GfxWrapperHandle& Renderer::GetGraphicsWrapper() const { return gfx; }
void Renderer::SetGraphicsWrapper(GfxWrapperHandle newGfx) { gfx = newGfx; }

const CameraHandle& Renderer::GetCamera() const { return camera; }
void Renderer::SetCamera(CameraHandle newCam)	{ camera = newCam; }

const TypedHandle<GroupingNode>& Renderer::GetSceneRoot() const { return sceneRoot; }
void Renderer::SetSceneRoot(TypedHandle<GroupingNode> newRoot) { sceneRoot = newRoot; }

const TypedHandle<Culler>& Renderer::GetCuller() const { return culler; }
void Renderer::GetCuller(TypedHandle<Culler> newCuller) { culler = newCuller; }

const Matrix4x4& Renderer::GetCurrWorldMatrix() const { return worldStack.back(); }

U64 Renderer::GetNumModelsDrawn() const { return numModelsDrawn; }

void Renderer::Init()
{
	//load up the default texture buffer if possible.
	defTexPtr = resMgr->GetResource(defaultTexGUID);
	if(!defTexPtr)
	{
		Log::E(String("Couldn't load default texture: ") + defaultTexGUID.Name + "!");
		L_ASSERT(false && "Couldn't load default texture!");
	}
	else
	{
		defaultTex = (Texture2D*)defTexPtr->Buffer();
		gfx->InitTexture(*defaultTex, TextureMeta::MapType::DIFFUSE);
	}
}

void Renderer::InsertNodeAt(TypedHandle<SpatialNode> node, TypedHandle<SpatialNode> parent)
{
	if(!node)
	{
		return;
	}

	//If the parent's null, use the root as parent.
	TypedHandle<SpatialNode> fixedParent = parent != 0 ? parent.GetHandle() : sceneRoot.GetHandle();
	
	//Need to be sure that the parent can actually contain other nodes - quit if this isn't true.
	if(fixedParent->GetContainMode() != SpatialNode::NODE_CONTAINER)
	{
		return;
	}

	//Otherwise, cast to GroupingNode and add the child.
	((TypedHandle<GroupingNode>)fixedParent)->AttachChild(node);
	//Notify the culler that a node's been inserted.
	//This node may be a container - we must recurse on its children if it has any.
	notifyCullerNodeAdded(node);
}

void Renderer::MoveNode(TypedHandle<SpatialNode> node, TypedHandle<SpatialNode> newParent)
{
	if(!node)
	{
		return;
	}

	TypedHandle<SpatialNode> fixedParent = newParent != 0 ? newParent.GetHandle() : sceneRoot.GetHandle();
	//First, figure out what the node's new local transform must be.
	//parentTransform * newTransform = worldTransform.
	//
	//Scale is simple enough - worldScale / parentScale.
	//
	//Translation is difficult, however - you can find the relative offset from the parent,
	//but the translation coordinates for a transform are relative to the parent's orientation as well.
	//It might be possible to apply the parent's inverse world transform to the vector?
	//
	//Finally, there's orientation. The new quaternion must be one such that
	//newParent.quat * newQuat == worldQuat;
	//This can then solve to
	//(newParent.quat)^-1 * newParent.quat * newQuat = newQuat = (newParent.quat)^-1 * worldQuat.

	//Fix the parent, and then notify the culler.
	//Nothing moved, but global shaders have almost certainly changed.
	notifyCullerNodeMoved(node);
}

void Renderer::RemoveNode(TypedHandle<SpatialNode> node)
{
	if(!node)
	{
		return;
	}

	//Break links between parent and node.
	if(node->GetParent() != NULL)
	{
		auto parent = node->GetWritableParent();
		L_ASSERT(	parent->GetContainMode() == SpatialNode::NODE_CONTAINER &&
					"Parent is not a container node!");
		((GroupingNode*)parent)->DetachChild(node);
		node->SetParent(NULL);
	}

	//Now notify the culler that the node's been removed.
	//Like InsertNodeAt(), the node might be a container,
	//so this must be a recursive call.
	notifyCullerNodeRemoved(node);
}

void Renderer::UpdateNode(TypedHandle<SpatialNode> node)
{
	if(!node)
	{
		return;
	}

	//Notify the culler that the node has changed in some way.
	//Like InsertNodeAt(), the node might be a container,
	//so this must be a recursive call.
	notifyCullerNodeUpdated(node);
}

void Renderer::DrawScene()
{
	//Get the culled geometry...
	culler->CalcVisibleSet();
	auto visScene = culler->GetVisibleSet();
	Frustum& camFrust = camera->GetWorldFrustum();
	//Reset any stat counters.
	numModelsDrawn = 0;
	//Now render those elements.
	for(auto elem = visScene.Elements.begin(); elem != visScene.Elements.end(); ++elem)
	{
		L_ASSERT(	elem->Spatial &&
					"Trying to render a null node!");
		L_ASSERT(	elem->Spatial->GetContainMode() == SpatialNode::NODE_LEAF &&
					"Trying to render a non-leaf node!");
		auto modelHnd = ((TypedHandle<ModelNode>)elem->Spatial)->GetModel();
		if(!modelHnd)
		{
			continue;
		}
		Model& model = *modelHnd;
		//do a sphere test on the object so we don't have to render extra stuff.
		SphereBounds objBounds = SphereBounds(	model.BoundsCenter() + elem->Spatial->GetWorldTransform().Position(),
												model.BoundingRadius());
		//gfx->DebugDrawSphere(objBounds, Colors::LtGreen);
		if(!camFrust.Test(objBounds))
		{
			continue;
		}
		//We're in the camera's frustum, mark this as being drawn.
		++numModelsDrawn;

		Matrix4x4& elemWorld = elem->Spatial->GetWorldTransform().ToMatrix();

		//First render the global shaders...
		for(auto gShader = elem->GlobalShaders.begin(); gShader != elem->GlobalShaders.end(); ++gShader)
		{
			//Each shader program might not get
			//access to the WVP matrix if we don't specify in the loop.
			onDraw(model, *gShader, elemWorld);
		}
		
		//Then the local shaders.
		for(U32 i = 0; i < elem->Spatial->GetNumLocalShaders(); ++i)
		{
			auto lShader = elem->Spatial->GetLocalShader(i);
			onDraw(model, lShader, elemWorld);
		}
	}
}