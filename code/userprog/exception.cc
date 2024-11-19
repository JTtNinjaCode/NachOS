// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1996 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "main.h"
#include "syscall.h"
#include "translate.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions
//	are in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{
	int	type = kernel->machine->ReadRegister(2);
	int	val;

	int temp;
	int page_index;
	int fake_disk_index;
	TranslationEntry *new_page_entry = nullptr; // this page in Disk
	TranslationEntry *replaced_page_entry = nullptr; // this page in RAM
	TranslationEntry *temp_entry = nullptr;
	Block temp_block;

    switch (which) {
	case SyscallException:
	    switch(type) {
		case SC_Halt:
		    DEBUG(dbgAddr, "Shutdown, initiated by user program.\n");
   		    kernel->interrupt->Halt();
		    break;
		case SC_PrintInt:
			val=kernel->machine->ReadRegister(4);
			cout << "Print integer:" <<val << endl;
			return;
/*		case SC_Exec:
			DEBUG(dbgAddr, "Exec\n");
			val = kernel->machine->ReadRegister(4);
			kernel->StringCopy(tmpStr, retVal, 1024);
			cout << "Exec: " << val << endl;
			val = kernel->Exec(val);
			kernel->machine->WriteRegister(2, val);
			return;
*/		case SC_Exit:
			DEBUG(dbgAddr, "Program exit\n");
			val=kernel->machine->ReadRegister(4);
			cout << "return value:" << val << endl;
			kernel->currentThread->Finish();
			break;
		default:
		    cerr << "Unexpected system call " << type << "\n";
 		    break;
	    }
	    break;
	case PageFaultException:
		kernel->stats->numPageFaults += 1;

		std::cout << "badVAddr" << kernel->machine->ReadRegister(BadVAddrReg) << std::endl;
		std::cout << "page fault" << std::endl;

		// 透過想要存取的 Memory 地址，得到它在哪個 page table、它目前被存在硬碟的哪裡
		page_index = kernel->machine->ReadRegister(BadVAddrReg) / PageSize;
		fake_disk_index = -(kernel->currentThread->space->pageTable[page_index].physicalPage + 1);
		new_page_entry = &kernel->currentThread->space->pageTable[page_index];

		// std::cout << "want to put page " << page_index << "to ram, it is current in disk " << fake_disk_index << std::endl;

		// 找到哪個 page 要被換掉，得到它的 entry
		if (kernel->mem_algo_flag == MemLRU) {
		}
		else {
			// FIFO 會把先放入 RAM 的 entry 儲存到 stack，然後 pop
			replaced_page_entry = kernel->fifo_entry.front();
			kernel->fifo_entry.pop();
		}

		// std::cout << "page " << replaced_page_entry->virtualPage << "want to be replaced, it is currently in frame" << replaced_page_entry->physicalPage << std::endl;

		// 把記憶體內的 page 寫回 disk 內，然後把要被換上的新 page 內容寫到對應的 Frame
		memcpy(&temp_block, &(kernel->machine->mainMemory[replaced_page_entry->physicalPage * PageSize]), PageSize);
		memcpy(&(kernel->machine->mainMemory[replaced_page_entry->physicalPage * PageSize]), &(kernel->fake_disk[-(new_page_entry->physicalPage + 1)]), PageSize);
		memcpy(&(kernel->fake_disk[-(new_page_entry->physicalPage + 1)]), &temp_block, PageSize);
		kernel->stats->numDiskReads += 1;
		kernel->stats->numDiskWrites += 1;

		// 修改換上來的的 TranslationEntry 的狀態
		new_page_entry->physicalPage = replaced_page_entry->physicalPage;
		new_page_entry->valid = true;
		new_page_entry->dirty = false;
		new_page_entry->readOnly = false;
		new_page_entry->use = false;
		// std::cout << "page" << new_page_entry->virtualPage << "has been put into frame" << new_page_entry->physicalPage << std::endl;

		// 修改被換掉的 TranslationEntry 的狀態
		replaced_page_entry->physicalPage = -(fake_disk_index + 1);
		replaced_page_entry->dirty = false;
		replaced_page_entry->readOnly = false;
		replaced_page_entry->use = false;
		replaced_page_entry->valid = false;
		// std::cout << "page" << replaced_page_entry->virtualPage << "has been put into disk" << -(replaced_page_entry->physicalPage + 1) << std::endl;
		std::cout << "frame " << new_page_entry->physicalPage << "swapped" << std::endl;

		if (kernel->mem_algo_flag == MemLRU) {
		}
		else { // FIFO，把新放入的 page 放到 queue
			kernel->fifo_entry.push(new_page_entry);
		}
		return;
	default:
	    cerr << "Unexpected user mode exception" << which << "\n";
	    break;
    }
    ASSERTNOTREACHED();
}
