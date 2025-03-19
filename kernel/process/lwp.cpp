#include <process.h>
#include <string.h>
extern u64 ContextReg[30];

static u16 t = 0;

static u16 getId() {
    return t++;
}

Process::Process(u8 priority, u8 nice, void* startAddress):Priority(priority), Nice(nice){
    Id = getId();
    space = new MemSpace(0x0, 0xFFFFFFFFFFFF);
    pc = (u64) startAddress;
    sp = 0xFF000FFC;
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
    __csrwr_d(sp, 0x30);
    __csrwr_d(pc, 0x6);
    __csrwr_d(Id, 0x18);
    GetSpace()->MMUService.SetPGDL();
}

void Process::Pause() {
    for (u64 i = 0; i < 30; ++i) {
        reg[i] = ContextReg[i];
    }
    sp = __csrrd_d(0x30);
    pc = __csrrd_d(0x6);
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
    if (CurrentProcess != nullptr && (!(headBitMap & (0x11111111u << (CurrentProcess->Priority))))) return;
    StopCurrentProcess();
    if (headBitMap)
    for (int i = 7; i >= 0; --i) {
        if (headBitMap & (1 << i)) {
            Process *processToExec = bfsHead[i], *processToExecFrom = nullptr;
            for (Process *pt = bfsHead[i], *from = nullptr; pt != nullptr; from = pt, pt = pt->Next) {
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
    if (CurrentProcess == nullptr) {
        uPut << "No Available Process\n";
        while (1);
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
