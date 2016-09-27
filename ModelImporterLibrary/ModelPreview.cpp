#include "ModelPreview.h"
#include <Platforms/IPlatform.h>
#include <Constants/Constants.h>
#include <Rendering/Texture.h>
#include "ConverterManager.h"
#include "ModelLog.h"
#include "Devices.h"

using namespace LeEK;

Previewer* Previewer::instance = NULL;
Texture2D defTex = Texture2D::BuildSolidRGBA8Tex(1,1, Colors::Gray50Pct);

bool Previewer::initShaders()
{
	Vector< ShaderFilePair > pathsColor, pathsDiffuse, pathsDiffuseTextured;
	pathsColor.push_back( ShaderFilePair(VERTEX, Path((String(Filesystem::GetProgDir()) + "Shaders/Basic/Color.vp").c_str())) );
	pathsColor.push_back( ShaderFilePair(FRAGMENT, Path((String(Filesystem::GetProgDir()) + "Shaders/Basic/Color.fp").c_str())) );

	pathsDiffuse.push_back( ShaderFilePair(VERTEX, Path((String(Filesystem::GetProgDir()) + "Shaders/Basic/Diffuse.vp").c_str())) );
	pathsDiffuse.push_back( ShaderFilePair(FRAGMENT, Path((String(Filesystem::GetProgDir()) + "Shaders/Basic/Diffuse.fp").c_str())) );

	pathsDiffuseTextured.push_back( ShaderFilePair(VERTEX, Path((String(Filesystem::GetProgDir()) + "Shaders/Basic/DiffuseTextured.vp").c_str())) );
	pathsDiffuseTextured.push_back( ShaderFilePair(FRAGMENT, Path((String(Filesystem::GetProgDir()) + "Shaders/Basic/DiffuseTextured.fp").c_str())) );

	return	gfx->MakeShader("Color", 2, pathsColor) &&
			gfx->MakeShader("Diffuse", 2, pathsDiffuse) &&
			gfx->MakeShader("DiffuseTextured", 2, pathsDiffuseTextured);
}

void Previewer::preVPortInit()
{
	ModelLog::D("ModelPreview: Initializing previewer...");
	vPortReady = false;
	//Create the platform-dependent module.
	IPlatform* plat = Devices::GetPlatform();
	if(!plat)
	{
		ModelLog::E("ModelPreview: Failed to get platform instance!");
		return;
	}
	gfx = Devices::GetGfx();
	if(!gfx)
	{
		ModelLog::E("ModelPreview: Failed to get graphics wrapper!");
		return;
	}
	ModelLog::D("ModelPreview: Previewer initialized.");
}

void Previewer::postVPortInit()
{
	//initialize fields
	//load up shaders
	ModelLog::D("ModelPreview: Connecting previewer to screen...");
	if(!initShaders())
	{
		ModelLog::E("ModelPreview: Failed to load shaders!");
		return;
	}
	lightColor = Colors::White;
	lightPos = Vector3(50, 50, 0);
	cam = new OrbitCamera();
	cam->SetAspectRatio(gfx->ScreenAspect());
	cam->SetNearDist(DEF_NEARPLANE);
	cam->SetFarDist(DEF_FARPLANE);
	cam->SetFOV(Math::PI_OVER_2);
	cam->SetRadius(7.5f);
	//initialize the texture
	if(!gfx->InitTexture(defTex, TextureMeta::MapType::DIFFUSE))
	{
		ModelLog::E("Failed to initialize default texture!");
		return;
	}
	vPortReady = true;
	//and initialize any currently loaded model
	Model& mdl = ConverterManager::GetModel(0);
	if(mdl.MeshCount() > 0)
	{
		cam->SetCenter(mdl.BoundsCenter());
		InitModel(mdl);
	}
	ModelLog::D("ModelPreview: Previewer connected.");
}

void Previewer::drawModel(const Model& modelToDraw)
{
	Texture2D& defTexRef = defTex;
	gfx->SetShader("DiffuseTextured");
	gfx->SetVec3Uniform("lightDiffuse", lightColor.GetRGB());
	gfx->SetVec3Uniform("lightPos", lightPos);
	//Texture2D& texRef = *(Texture2D*)(void*)texPtr->Buffer();
	//gfx->SetTexture(texRef, TextureMeta::DIFFUSE);
	gfx->SetIntUniform("diffTex", TextureMeta::DIFFUSE);
	for(U32 i = 0; i < modelToDraw.MeshCount(); ++i)
	{
		const Mesh& mesh = *modelToDraw.GetMesh(i);
		const Material& mat = mesh.GetMaterial();
		const Geometry& geom = mesh.GetGeometry();
		//need to setup mesh uniforms;
		//since system doesn't use materials yet,
		//that's just the texture
		//use a default material if there's no texture specified
		//ResPtr texPtr = resMgr.GetResource(mat.DiffuseTexGUID);
		const Texture2D& texRef = defTex;//*(Texture2D*)texPtr->Buffer();
		gfx->SetTexture(texRef, TextureMeta::DIFFUSE);

		gfx->Draw(geom);
	}
}

Previewer::Previewer()
{
	preVPortInit();
}

Previewer* Previewer::GetInstance()
{
	if(!instance)
	{
		instance = new Previewer();
	}
	return instance;
}

void Previewer::SetViewport(HWND vPort)
{
	vPortReady = false;
	ModelLog::D(String("ModelPreview: Setting viewport to ") + StrFromPtr(vPort));
	//shut down the old texture
	gfx->ShutdownTexture(defTex);
	//shut down the model, if possible.
	Model& mdl = ConverterManager::GetModel(0);
	if(mdl.MeshCount() > 0)
	{
		ShutdownModel(mdl);
	}
	auto plat = Devices::GetPlatform();
	if(!plat || !plat->SetViewport(gfx, vPort))
	{
		ModelLog::E("ModelPreview: Couldn't change viewport!");
		return;
	}
	postVPortInit();
	ModelLog::D("ModelPreview: Viewport set.");
}

void Previewer::InitModel(Model& mdl)
{
	ModelLog::D("ModelPreview: Loading model into renderer...");
	if(!vPortReady)
	{
		ModelLog::W("Can't initialize model, preview window isn't connected!");
		return;
	}

	for(U16 i = 0; i < mdl.MeshCount(); ++i)
	{
		gfx->InitGeometry(mdl.GetMesh(i)->GetGeometry());
	}
	ModelLog::D("ModelPreview: Model loaded into renderer.");
}

void Previewer::ShutdownModel(Model& mdl)
{
	ModelLog::D("ModelPreview: Shutting down model...");
	if(!vPortReady)
	{
		ModelLog::W("Can't shutdown model, preview window isn't connected!");
		return;
	}

	for(U16 i = 0; i < mdl.MeshCount(); ++i)
	{
		gfx->ShutdownMesh(mdl.GetMesh(i)->GetGeometry());
	}
	ModelLog::D("ModelPreview: Model shut down.");
}

void Previewer::NotifyModelChanged()
{
	ModelLog::D("ModelPreview: switching model...");
	Model& mdl = ConverterManager::GetModel(0);
	if(mdl.MeshCount() > 0)
	{
		ShutdownModel(mdl);
		InitModel(mdl);
	}
}

void Previewer::SpinCamera(float xEuler, float yEuler)
{
	cam->RotateEuler(xEuler, yEuler, 0);
}

void Previewer::ZoomCamera(float zoomDist)
{
	cam->SetRadius(cam->Radius() + zoomDist);
}

void Previewer::Draw()
{
	if(!gfx)
	{
		return;
	}
	gfx->Clear(Colors::Gray20Pct);
	//draw the model and its bounds, as needed
	//Going to need to set some shader...
	gfx->SetWorldViewProjection(Matrix4x4::Identity,
								cam->GetViewMatrix(),
								cam->GetProjMatrix());
	Model& mdl = ConverterManager::GetModel(0);
	if(mdl.MeshCount() > 0)
	{
		gfx->DebugDrawAABB(mdl.BoundsCenter(), mdl.AABBHalfBounds(), Colors::Green);
		gfx->DebugDrawSphere(mdl.BoundsCenter(), mdl.BoundingRadius(), Colors::LtGreen);
		drawModel(mdl);
	}
}