.globl ContextReg
.globl ContextID
.globl ContextPC
.globl SysSp
.globl HandleMachineErrorEntry
.globl HandleDefaultExceptionEntry
.globl HandleTLBExceptionEntry
.globl StartProcess

.extern handleMachineError
.extern handleDefaultException
.extern handleTLBException

.data
ContextReg:
.fill 32, 8, 0
ContextPC:
.quad 0
ContextID:
.quad 0
SysSp:
.quad 0
.text

.macro loadContext
    la.local $t0, ContextPC
    ld.d $t1, $t0, 0
    csrwr $t1, 0x6
    la.local $t0, ContextID
    ld.d $t1, $t0, 0
    csrwr $t1, 0x18
    la.local $t0, SysSp
    st.d $sp, $t0, 0
    la.local $sp, ContextReg
    ld.d $r1, $sp, 0
    ld.d $r2, $sp, 8
    ld.d $r4, $sp, 16
    ld.d $r5, $sp, 24
    ld.d $r6, $sp, 32
    ld.d $r7, $sp, 40
    ld.d $r8, $sp, 48
    ld.d $r9, $sp, 56
    ld.d $r10, $sp, 64
    ld.d $r11, $sp, 72
    ld.d $r12, $sp, 80
    ld.d $r13, $sp, 88
    ld.d $r14, $sp, 96
    ld.d $r15, $sp, 104
    ld.d $r16, $sp, 112
    ld.d $r17, $sp, 120
    ld.d $r18, $sp, 128
    ld.d $r19, $sp, 136
    ld.d $r20, $sp, 144
    ld.d $r21, $sp, 152
    ld.d $r22, $sp, 160
    ld.d $r23, $sp, 168
    ld.d $r24, $sp, 176
    ld.d $r25, $sp, 184
    ld.d $r26, $sp, 192
    ld.d $r27, $sp, 200
    ld.d $r28, $sp, 208
    ld.d $r29, $sp, 216
    ld.d $r30, $sp, 224
    ld.d $r31, $sp, 232
    ld.d $sp, $sp, 240
.endm

.macro storeContext
    csrwr $sp, 0x30
    la.local $sp, ContextReg
    st.d $r1, $sp, 0
    st.d $r2, $sp, 8
    st.d $r4, $sp, 16
    st.d $r5, $sp, 24
    st.d $r6, $sp, 32
    st.d $r7, $sp, 40
    st.d $r8, $sp, 48
    st.d $r9, $sp, 56
    st.d $r10, $sp, 64
    st.d $r11, $sp, 72
    st.d $r12, $sp, 80
    st.d $r13, $sp, 88
    st.d $r14, $sp, 96
    st.d $r15, $sp, 104
    st.d $r16, $sp, 112
    st.d $r17, $sp, 120
    st.d $r18, $sp, 128
    st.d $r19, $sp, 136
    st.d $r20, $sp, 144
    st.d $r21, $sp, 152
    st.d $r22, $sp, 160
    st.d $r23, $sp, 168
    st.d $r24, $sp, 176
    st.d $r25, $sp, 184
    st.d $r26, $sp, 192
    st.d $r27, $sp, 200
    st.d $r28, $sp, 208
    st.d $r29, $sp, 216
    st.d $r30, $sp, 224
    st.d $r31, $sp, 232
    csrrd $t0, 0x30
    st.d $t0, $sp, 240
    la.local $t0, SysSp
    ld.d $sp, $t0, 0
    la.local $t0, ContextPC
    csrrd $t1, 0x6
    st.d $t1, $t0, 0
.endm

HandleMachineErrorEntry:
    storeContext
    bl handleMachineError
    loadContext
    invtlb 0, $r0, $r0
    ertn

HandleTLBExceptionEntry:
    storeContext
    bl handleTLBException
    tlbfill
    loadContext
    ertn

HandleDefaultExceptionEntry:
    storeContext
    bl handleDefaultException
    loadContext
    invtlb 0, $r0, $r0
    ertn

StartProcess:
    li.d $t0, 0x7
    csrwr $t0, 0x1
    loadContext
    invtlb 0, $r0, $r0
    ertn
