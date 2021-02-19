#pragma once
#include "Level.h"

class LEVEL;
struct MAP;
class PAGETABLE {
public:
	int LevelCount;
	unsigned int *BitMaskAry;
	int *ShiftAry;
	unsigned int *EntryCount;
	LEVEL *RootNode;
	int *numberOfBits;
	PAGETABLE();

};



