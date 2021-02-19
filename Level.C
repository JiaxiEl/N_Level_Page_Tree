#include "Level.h"

LEVEL::LEVEL()
{
	NextLevelPtr = nullptr;
	MapPtr = nullptr;
	currentDepth = 0;
}

LEVEL::LEVEL(PAGETABLE *PageTabel)
{
	PageTablePtr = PageTabel;
	NextLevelPtr = nullptr;
	MapPtr = nullptr;
	currentDepth = 0;
}
