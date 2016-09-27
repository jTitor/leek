#include "Culler.h"

using namespace LeEK;

Culler::Culler(I32 pMaxNodes, I32 pGrowRate, 
				TypedHandle<CameraBase> pCamera)
{
	camera = pCamera;
	visible = VisibleSet();
}

Culler::~Culler(void)
{
}

TypedHandle<CameraBase> Culler::GetCamera()
{
	return camera;
}

void Culler::SetCamera(TypedHandle<CameraBase> val)
{
	camera = val;
}

VisibleSet& Culler::GetVisibleSet()
{
	return visible;
}