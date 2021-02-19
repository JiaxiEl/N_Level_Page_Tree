#pragma once
#include "PageTable.h"
class PAGETABLE;
struct MAP {
	bool Vaild = false;
	int frame;
};
class LEVEL {
public:
	LEVEL **NextLevelPtr;
	int currentDepth;
	PAGETABLE*  PageTablePtr;
	MAP *MapPtr;
	LEVEL();
	LEVEL(PAGETABLE *PageTabel);
};
