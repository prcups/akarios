#include <process.h>

ELFProgram::ELFProgram(const char* filename) {
    file *f = new file;
    open(filename, f);
    char *p = new char [f->size];
    read(f, (u8 *) p ,f->size);

    header = (ELFHeader*) p;
    if (header->MagicNum == 0x464C457F) {
        phtb = (PHTB*) ((u64) p + header->PHTBAddr);
    }
    delete f;
}

void ELFProgram::ShowInfo() {
    if (header->MagicNum != 0x464C457F) {
        uPut << "Wrong ELF File\n";
        return;
    }
    uPut << (header->BitSize == 2 ? 64 : 32) << " Bit, ";
    uPut << (header->Endian == 1 ? "Little" : "Big") << " Endian, ";
    switch (header->Type) {
        case 1:
            uPut << "Relocatable";
            break;
        case 2:
            uPut << "Executable";
            break;
        case 3:
            uPut << "Shared";
            break;
        case 4:
            uPut << "Core";
    }
    uPut << " ELF File\n";

    if (header->Type == 2) {
        for (u8 i = 0; i < header->NumofEntriesInPHTB; ++i) {
            uPut << (void *)(phtb + i)->DataAddr << " " << (void*) (phtb + i)->MemAddr << " " << (void*) (phtb + i)->SizeOfSegmentInMem << '\n';
        }
    }
}

void ELFProgram::CreateProcess() {
    if (header->Type != 2) return;
    Process *p = new Process(7, 0, (void*) header->ProgramEntryAddr);
    for (u8 i = 0; i < header->NumofEntriesInPHTB; ++i) {
        if ((phtb + i)->Type == 1) {
            p->GetSpace()->AddZone(new TNode<Zone>(new DirectZone((phtb + i)->MemAddr, (phtb + i)->MemAddr + (phtb + i)->SizeOfSegmentInMem - 1, (u64)((u8*) header + (phtb + i)->DataAddr) - (phtb + i)->MemAddr, ZoneConfig{1, 3, 0, 0, 0, 0})));
        }
    }
    processController.InsertProcess(p);
}
