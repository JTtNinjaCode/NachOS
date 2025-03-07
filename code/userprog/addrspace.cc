// addrspace.cc
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -n -T 0 option
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "addrspace.h"
#include "machine.h"
#include "noff.h"

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------


bool AddrSpace::usedPhyPage[NumPhysPages] = {0};

static void
SwapHeader (NoffHeader *noffH)
{
    noffH->noffMagic = WordToHost(noffH->noffMagic);
    noffH->code.size = WordToHost(noffH->code.size);
    noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
    noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
    noffH->initData.size = WordToHost(noffH->initData.size);
    noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
    noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
    noffH->uninitData.size = WordToHost(noffH->uninitData.size);
    noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
    noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Set up the translation from program memory to physical
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//----------------------------------------------------------------------


AddrSpace::AddrSpace()
{

//     pageTable = new TranslationEntry[NumPhysPages];
//     for (unsigned int i = 0; i < NumPhysPages; i++) {
// 	pageTable[i].virtualPage = i;	// for now, virt page # = phys page #
// 	pageTable[i].physicalPage = i;
// //	pageTable[i].physicalPage = 0;
// 	pageTable[i].valid = TRUE;
// //	pageTable[i].valid = FALSE;
// 	pageTable[i].use = FALSE;
// 	pageTable[i].dirty = FALSE;
// 	pageTable[i].readOnly = FALSE;
//     }

    // zero out the entire address space
//    bzero(kernel->machine->mainMemory, MemorySize);
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
   for(int i = 0; i < numPages; i++)
        AddrSpace::usedPhyPage[pageTable[i].physicalPage] = false;
   delete pageTable;
}


//----------------------------------------------------------------------
// AddrSpace::Load
// 	Load a user program into memory from a file.
//
//	Assumes that the page table has been initialized, and that
//	the object code file is in NOFF format.
//
//	"fileName" is the file containing the object code to load into memory
//----------------------------------------------------------------------

bool
AddrSpace::Load(char *fileName)
{
    // 讀取出 exe 檔案內容，並且解析資訊到 NoffHeader
    OpenFile *executable = kernel->fileSystem->Open(fileName);
    NoffHeader noffH;
    unsigned int size;

    if (executable == NULL) {
	cerr << "Unable to open file " << fileName << "\n";
	return FALSE;
    }
    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) &&
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

// how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size
			+ UserStackSize;	// we need to increase the size
						// to leave room for the stack
    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;

    numPages = divRoundUp(size,PageSize);

    pageTable = new TranslationEntry[numPages];
    // 嘗試把 VM pages 塞進去 phy pages，如果沒位置了，就先不要放到 RAM，改成放到 Disk 內
    unsigned int i = 0, j = 0;
    for(i=0, j=0; i<numPages; i++){
        pageTable[i].virtualPage = i; // pages index
        while(j < NumPhysPages && AddrSpace::usedPhyPage[j] == true) // 如果 frame 被占走了，看下一個
            j++;
        if (j == NumPhysPages) break; // 如果滿了，就跳出迴圈，處理剩下還沒分配到 frames 內的 VM pages
        AddrSpace::usedPhyPage[j] = true;
        pageTable[i].physicalPage = j;
        pageTable[i].valid = true;
        pageTable[i].use = false;
        pageTable[i].dirty = false;
        pageTable[i].readOnly = false;
        if (kernel->mem_algo_flag == MemFIFO) {
            kernel->fifo_entry.push(&pageTable[i]); // 以 FIFO 交換 page 時需要
        } else if (kernel->mem_algo_flag == MemLRU) {
            kernel->lru_entry.push_back(&pageTable[i]); // 以 LRU 交換 page 時需要
        }
    }

    // 跳出迴圈，可能是因為 Phy Frame 滿了，就跳出迴圈，處理剩下的 VM Pages
    if (j == NumPhysPages) {
        while (i < numPages) {
            pageTable[i].virtualPage = i;
            pageTable[i].physicalPage = -(kernel->fake_disk.size() + 1);
            pageTable[i].valid = false;
            pageTable[i].use = false;
            pageTable[i].dirty = false;
            pageTable[i].readOnly = false;
            kernel->pages_in_disk.push_back(&pageTable[i]);
            kernel->fake_disk.emplace_back(); // 在假硬碟內增加一個 Block
            i++;
        }
    }

    size = numPages * PageSize;

    DEBUG(dbgAddr, "Initializing address space: " << numPages << ", " << size);

    // 都先複製到 blocks 內，這個 blocks 是假的 Virtual Memory
    std::vector<Block> blocks(numPages);
    // then, copy in the code and data segments into memory
	if (noffH.code.size > 0) {
        DEBUG(dbgAddr, "Initializing code segment.");
	    DEBUG(dbgAddr, noffH.code.virtualAddr << ", " << noffH.code.size);

        char *start = (char *)&blocks.at(0) + noffH.code.virtualAddr;
        executable->ReadAt(start, noffH.code.size, noffH.code.inFileAddr);

        std::cout << "code section start:" << noffH.code.virtualAddr << std::endl;
        std::cout << "code section end  :" << noffH.code.virtualAddr + noffH.code.size << std::endl;
    }

	if (noffH.initData.size > 0) {
        DEBUG(dbgAddr, "Initializing data segment.");
	    DEBUG(dbgAddr, noffH.initData.virtualAddr << ", " << noffH.initData.size);

        char *start = (char *)&blocks.at(0) + noffH.initData.virtualAddr;
        executable->ReadAt(start, noffH.initData.size, noffH.initData.inFileAddr);

        std::cout << "init data section start:" << noffH.initData.virtualAddr << std::endl;
        std::cout << "init data section end  :" << noffH.initData.virtualAddr + noffH.initData.size << std::endl;
    }

    if (noffH.uninitData.size > 0) {
        DEBUG(dbgAddr, "Initializing bss segment.");
	    DEBUG(dbgAddr, noffH.uninitData.virtualAddr << ", " << noffH.uninitData.size);

        // bss 本身裡面就是都初始化為 0，不需要複製
        std::cout << "bss section start:" << noffH.uninitData.virtualAddr << std::endl;
        std::cout << "bss section end  :" << noffH.uninitData.virtualAddr + noffH.uninitData.size << std::endl;
    }

    for (int i = 0; i < blocks.size(); i++) {
        if (pageTable[i].physicalPage >= 0) { // 在 RAM 裡面
            int frame_index = pageTable[i].physicalPage;
            memcpy(&kernel->machine->mainMemory[frame_index * PageSize], &blocks[i], PageSize);
        } else {
            int fake_disk_index = -(pageTable[i].physicalPage + 1);
            memcpy(&kernel->fake_disk[fake_disk_index], &blocks[i], PageSize);
            kernel->stats->numDiskWrites += 1; // write into fake disk, numDiskWrites++
        }
    }

    delete executable;			// close file
    return TRUE;			// success
}

//----------------------------------------------------------------------
// AddrSpace::Execute
// 	Run a user program.  Load the executable into memory, then
//	(for now) use our own thread to run it.
//
//	"fileName" is the file containing the object code to load into memory
//----------------------------------------------------------------------

void
AddrSpace::Execute(char *fileName)
{
    if (!Load(fileName)) {
	cout << "inside !Load(FileName)" << endl;
	return;				// executable not found
    }

    //kernel->currentThread->space = this;
    this->InitRegisters();		// set the initial register values
    this->RestoreState();		// load page table register

    kernel->machine->Run();		// jump to the user progam

    ASSERTNOTREACHED();			// machine->Run never returns;
					// the address space exits
					// by doing the syscall "exit"
}


//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    Machine *machine = kernel->machine;
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG(dbgAddr, "Initializing stack pointer: " << numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, don't need to save anything!
//----------------------------------------------------------------------

void AddrSpace::SaveState()
{
        pageTable=kernel->machine->pageTable;
        numPages=kernel->machine->pageTableSize;
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState()
{
    kernel->machine->pageTable = pageTable;
    kernel->machine->pageTableSize = numPages;
}
