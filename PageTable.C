#include "PageTable.h"

PAGETABLE::PAGETABLE()
{
	RootNode = new LEVEL(this);
	BitMaskAry = nullptr;
	EntryCount = nullptr;
	LevelCount = 0;
	numberOfBits = nullptr;
	ShiftAry = nullptr;

}
