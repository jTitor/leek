#include "Actor.h"
#include "DataStructures/STLContainers.h"
#include "Constants/AllocTypes.h"
#include "EngineLogic/IDManager.h"

using namespace LeEK;

Actor nullActor;
//make this an actor handle
typedef Map<idT, Handle> ActorMap;
ActorMap ActorIDMap = ActorMap();
IDManager actorIDMan = IDManager();

Actor::Actor(void)
{
}

//TODO: add a constructor for const Transform&?

TypedHandle<Actor> Actor::Init()
{
	//trans = CustomNew<Transform>(TRANSF_ALLOC, "TransfAlloc");
	id = actorIDMan.GetNextID();
	//don't forget to add the actor to the global list!
	TypedHandle<Actor> res = HandleMgr::RegisterPtr(this);
	ActorIDMap[id] = res.GetHandle();
	return res;
}

Actor::~Actor(void)
{
}

TypedHandle<Actor> Actor::FindActor(idT id)
{
	ActorMap::const_iterator actorIt = ActorIDMap.find(id);
	if(actorIt != ActorIDMap.end())
	{
		return TypedHandle<Actor>(actorIt->second);
	}
	else
	{
		return TypedHandle<Actor>(0);
	}
}
