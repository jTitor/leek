#pragma once
#include "Memory/Handle.h"
#include "GroupingNode.h"

namespace LeEK
{
	/**
	Contains all elements of a game world.
	*/
	class Scene
	{
	private:
		TypedHandle<GroupingNode> sceneRoot;
	public:
		Scene(void);
		~Scene(void);
	};
}