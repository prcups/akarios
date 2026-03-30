#include <process.h>
#include <string.h>

extern u64 ContextReg[31];
extern u64 ContextPC;
extern u64 ContextID;

static u16 t = 0;

static u16 getId() {
    return ++t;
}

Process::Process(u8 priority, u8 nice, u64 startVAddress):Priority(priority), Nice(nice){
    Id = getId();
    space = new MemSpace(0x0, 0xFFFFFFFFFFFF);
    pc = startVAddress;
    sp = 0xFF000FFC;
    for (u8 i = 0; i < 30; ++i) {
        reg[i] = 0;
    }
    space->AddZone(new TNode<Zone>(new Zone(0xFF000000, 0xFFFFFFFF, ZoneConfig{1, 3, 0, 1, 0, 0})));
    pwd[0] = '/';
}

Process::~Process() {
    delete space;
}

void Process::Resume() {
    for (u64 i = 0; i < 30; ++i) {
        ContextReg[i] = reg[i];
    }
    ContextReg[30] = sp;
    ContextPC = pc;
    ContextID = Id;
    GetSpace()->MMUService.SetPGDL();
}

void Process::Pause() {
    for (u64 i = 0; i < 30; ++i) {
        reg[i] = ContextReg[i];
    }
    sp = ContextReg[30];
    pc = ContextPC;
}

void Process::SetArg(void* argc, u64 argv) {
    reg[2] = (u64) argc;
    reg[3] = argv;
}

void ProcessController::StopCurrentProcess() {
    if (CurrentProcess == nullptr) return;
    CurrentProcess->Pause();
    CurrentProcess->Deadline = Jiffies(prioRatios[CurrentProcess->Nice]) + Jiffies::GetJiffies();
    CurrentProcess->Next = bfsHead[CurrentProcess->Priority];
    bfsHead[CurrentProcess->Priority] = CurrentProcess;
    headBitMap |= (1 << (CurrentProcess->Priority));
    CurrentProcess = nullptr;
}

void ProcessController::HandleSchedule() {
    uPut << "\nScheduling\n";
    if (CurrentProcess == nullptr && (!(headBitMap & 0xFFu ))) return;
    StopCurrentProcess();
    if (headBitMap)
    for (int i = 7; i >= 0; --i) {
        if (headBitMap & (1 << i)) {
            Process *processToExec = bfsHead[i], *processToExecFrom = nullptr;
            for (Process *pt = bfsHead[i], *from = nullptr; pt != nullptr; from = pt, pt = pt->Next) {
                uPut << pt->Deadline << '\n';
                if (pt->Deadline < processToExec->Deadline) {
                    processToExecFrom = from;
                    processToExec = pt;
                }
            }
            CurrentProcess = processToExec;
            if (processToExecFrom != nullptr) {
                processToExecFrom->Next = CurrentProcess->Next;
            }
            else {
                bfsHead[i] = CurrentProcess->Next;
                if (bfsHead[i] == nullptr) headBitMap &= (~(1 << i));
            }
            CurrentProcess->Next = nullptr;
            CurrentProcess->Resume();
            break;
        }
    }
}

void ProcessController::InsertProcess(Process* process) {
    if (CurrentProcess == nullptr || process->Priority > CurrentProcess->Priority) {
        StopCurrentProcess();
        CurrentProcess = process;
        CurrentProcess->Resume();
        return;
    }
    process->Deadline = Jiffies(prioRatios[process->Nice]) + Jiffies::GetJiffies();
    process->Next = bfsHead[process->Priority];
    bfsHead[process->Priority] = process;
    headBitMap |= (1 << process->Priority);
}

ProcessController::ProcessController() {}
