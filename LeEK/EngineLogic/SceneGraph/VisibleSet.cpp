#include "VisibleSet.h"

using namespace LeEK;

VisibleSet::VisibleSet(U32 elemReserve, U32 lightReserve)
{
	Elements = Vector<VisibleElement>();
	LightNodes = Vector<LightNode>();
	Elements.reserve(elemReserve);
}

VisibleSet::~VisibleSet()
{
	//TODO
}

void VisibleSet::Clear()
{
	Elements.clear();
	LightNodes.clear();
}

void VisibleSet::CopyElements(Vector<VisibleElement> newElements)
{
	//check that this doesn't result in corrupt data
	Elements = newElements;
}