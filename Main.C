#include <unistd.h>
#include <string>
#include <iostream>
#include <fstream>
#include <getopt.h>
#include <iomanip>
#include "PageTable.h"
#include "byutr.h"
#include "Level.h"

using namespace std;
void print_table(LEVEL* currentLevel, unsigned int pageNum, ostream &os);
bool PageInsert(PAGETABLE *PageTable, unsigned int LogicalAddress, unsigned int Frame);
MAP * PageLookup(PAGETABLE *PageTable, unsigned int LogicalAddress);


int main(int argc, char** argv) {
	int Option;
	int NumberOfAddr = -1;
	string PageFile = " ";
	string Filename;
	bool translateReq = false;



	// Read the argument
	while ((Option = getopt(argc, argv, "n:p:t")) != -1)
	{
		switch (Option)
		{
		case 'n':
			NumberOfAddr = atoi(optarg);
			/* Number of addresses to process */
			// optarg will contain the string following -n
			// Process appropriately (e.g. convert to integer atoi(optarg))
			break;
		case 'p': /* produce map of pages */
	// optarg contains name of page file?      
			PageFile = optarg;
			break;
		case 't': /* Show address translation */
	// No argument this time, just set a flag
			translateReq = true;
			break;
		default:
			cout << "error";
		}
		// print something about the usage and die?			exit(BADFLAG); // BADFLAG is an error # defined in a header
	}
	/* first mandatory argument, optind is defined by getopt */
	string idx = argv[optind];
	FILE *traceFile;
	p2AddrTr trace_item;
	traceFile = fopen(idx.c_str(), "rb");
	if (traceFile == NULL) {
		fprintf(stderr, "cannot open %s for reading\n", idx.c_str());
		exit(1);
	}
	//Init Page Table
	optind++;    //move to neext argument
	PAGETABLE* page_table = new PAGETABLE();
	page_table->LevelCount = argc - optind;
	page_table->BitMaskAry = new unsigned int[page_table->LevelCount];
	page_table->ShiftAry = new int[page_table->LevelCount];
	page_table->EntryCount = new unsigned int[page_table->LevelCount];
	page_table->numberOfBits = new int[page_table->LevelCount];
	//RootLevel
	//Get info from argument
	int offsetBit = 32;
	for (int i = 0; i < page_table->LevelCount; i++) {

		page_table->numberOfBits[i] = atoi(argv[optind]);
		offsetBit = offsetBit - atoi(argv[optind]);
		page_table->ShiftAry[i] = offsetBit;
		page_table->EntryCount[i] = 1 << atoi(argv[optind]);
		//Init the bit mask
		unsigned int mask = 1;
		for (int j = 0; j < page_table->numberOfBits[i] - 1; j++) {
			mask = mask << 1;
			mask = mask | 1;
		}
		mask = mask << page_table->ShiftAry[i];
		page_table->BitMaskAry[i] = mask;
		optind++;
	}
	//Create Level 0 page
	page_table->RootNode->currentDepth = 0;
	page_table->RootNode->NextLevelPtr =
		new LEVEL *[page_table->EntryCount[page_table->RootNode->currentDepth]];

	//PageInsert
	unsigned int  AddrCounter = 0;
	unsigned int frame = 0;
	unsigned int ReadyForOutput = 0;
	unsigned int offsetMask = 1;
	for (int i = 0; i < offsetBit - 1; i++) {
		offsetMask = offsetMask << 1;
		offsetMask = offsetMask | 1;
	}

	while (NumberOfAddr != 0) {
		//Read the address based on -n
		if (!feof(traceFile)) {

			int bytesread = NextAddress(traceFile, &trace_item);
			if (bytesread == 0) {
				break;
			}
			if (PageInsert(page_table, trace_item.addr, frame)) {
				frame = frame + 1;
			}

			if (translateReq) {
				ReadyForOutput = (offsetMask & trace_item.addr) +
					(PageLookup(page_table, trace_item.addr)->frame << offsetBit);
				cout << setw(8) << setfill('0') << hex << trace_item.addr << " -> "
					<< setw(8) << setfill('0') << ReadyForOutput << endl;
			}
			AddrCounter++;
		}
		else {
			break;
		}
		NumberOfAddr--;
	}
	fclose(traceFile);
	if (PageFile != " ") {
		//Print out the PageTable
		ofstream os;
		os.open(PageFile);
		print_table(page_table->RootNode, 0, os);
		os.close();
	}

	//Basic summary

	cout << "Page size: " << dec << (1 << offsetBit) << endl;
	cout << "Hit: " << AddrCounter - frame << "("
		<< setiosflags(ios::fixed) << setprecision(2) << double(AddrCounter - frame) / double(AddrCounter) * 100 << "%" << ")"
		<< ",Misses: " << frame
		<< setiosflags(ios::fixed) << setprecision(2) << "(" << double(frame) / double(AddrCounter) * 100 << "%" << ")"
		"# Address " << AddrCounter << endl;

}

//Get the index 
unsigned int LogicalToPage(unsigned int LogicalAddress, unsigned int Mask, unsigned int Shift) {
	return ((LogicalAddress & Mask) >> Shift);
}


//Insert the Page and frame
bool PageInsert(PAGETABLE *PageTable, unsigned int LogicalAddress, unsigned int Frame) {

	LEVEL *temp = PageTable->RootNode;
	unsigned int index;
	if (PageLookup(PageTable, LogicalAddress) == nullptr) {
		for (int i = 0; i < PageTable->LevelCount; i++) {
			index = LogicalToPage(LogicalAddress, PageTable->BitMaskAry[i], PageTable->ShiftAry[i]);

			if (temp->NextLevelPtr == nullptr) {
				temp->NextLevelPtr = new LEVEL*[PageTable->EntryCount[i]];
				temp->NextLevelPtr[index] = new LEVEL(PageTable);
				temp->NextLevelPtr[index]->currentDepth = temp->currentDepth + 1;
			}
			else if (temp->NextLevelPtr[index] == nullptr) {
				temp->NextLevelPtr[index] = new LEVEL(PageTable);
				temp->NextLevelPtr[index]->currentDepth = temp->currentDepth + 1;

			}
			temp = temp->NextLevelPtr[index];

		}
		if (temp->MapPtr == nullptr)
		{
			temp->MapPtr = new MAP();
			temp->MapPtr->Vaild = true;
			temp->MapPtr->frame = Frame;
			return true;
		}
	}


	return false;
}

//Check if the frame exsit and get frame
MAP * PageLookup(PAGETABLE *PageTable, unsigned int LogicalAddress) {
	LEVEL *temp = PageTable->RootNode;
	unsigned int index;
	for (int i = 0; i < PageTable->LevelCount; i++) {
		index = LogicalToPage(LogicalAddress, PageTable->BitMaskAry[i], PageTable->ShiftAry[i]);

		if (temp->NextLevelPtr == nullptr) {
			return nullptr;
		}
		if (temp->NextLevelPtr[index] == nullptr) {
			return nullptr;

		}
		temp = temp->NextLevelPtr[index];

	}

	return temp->MapPtr;
}







void print_table(LEVEL* currentLevel, unsigned int pageNum, ostream &os) {
	if (currentLevel->currentDepth != currentLevel->PageTablePtr->LevelCount) {
		pageNum = pageNum << currentLevel->PageTablePtr->numberOfBits[currentLevel->currentDepth];
	}

	if (currentLevel->NextLevelPtr == nullptr) {
		if (currentLevel->MapPtr != nullptr) {
			os << hex << setw(8) << setfill('0') << 
				pageNum << " -> " << setw(8) << setfill('0') << currentLevel->MapPtr->frame << endl;
		}
	}
	else {
		for (int i = 0; i < currentLevel->PageTablePtr->EntryCount[currentLevel->currentDepth]; i++)
		{
			if (currentLevel->NextLevelPtr[i] != nullptr) {
				print_table(currentLevel->NextLevelPtr[i], pageNum + i, os);
			}
		}
	}
}