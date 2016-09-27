#pragma once
#include <Datatypes.h>
#include <GraphicsWrappers/IGraphicsWrapper.h>
#include <Rendering/Camera/Camera.h>
#include <Windows.h>

namespace LeEK
{
	//singleton class that displays a preview of the model.
	class Previewer
	{
	private:
		static Previewer* instance;
		
		GfxWrapperHandle gfx;
		OrbitCamera* cam;
		Color lightColor;
		Vector3 lightPos;
		bool vPortReady;

		bool initShaders();
		void preVPortInit();
		void postVPortInit();
		void drawModel(const Model& modelToDraw);
	public:
		static Previewer* GetInstance();
		Previewer();
		void SetViewport(HWND vPort);
		void InitModel(Model& mdl);
		void ShutdownModel(Model& mdl);
		void NotifyModelChanged();
		void SpinCamera(float xEuler, float yEuler);
		void ZoomCamera(float zoomDist);
		void Draw();
	};
}