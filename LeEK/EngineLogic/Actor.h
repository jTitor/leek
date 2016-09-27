#pragma once
#include "Datatypes.h"
#include "DataStructures/STLContainers.h"
#include "EngineLogic/Transform.h"
#include "EngineLogic/IDManager.h"
#include "Memory/Handle.h"
#include "FileManagement/Filesystem.h"

namespace LeEK
{
	class Actor
	{
	private:
		//Transform* trans;
		//List<ComponentHnd> components;
		idT id; //this was the important part, fool :(
	public:
		Actor(void);
		~Actor(void);
		TypedHandle<Actor> Init();
		inline idT ID() { return id; }
		//make this return an actor handle
		static TypedHandle<Actor> FindActor(idT id);
	};

	//the actual system you spawn actors from.
	class ActorManager
	{
	public:
		TypedHandle<Actor> ActorFromPrefab(Path prefabPath);
		//Handle ActorFromPrefab(ResHnd prefabHnd);?
	};
}