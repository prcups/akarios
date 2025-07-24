.globl ContextReg
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
.fill 30, 8, 0
SysSp:
.quad 0xA00000
.text

.macro loadContext
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
    csrrd $sp, 0x30
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
    csrrd $sp, 0x1
    andi $sp, $sp, 0x11
    bne $sp, $r0, 12
    csrrd $sp, 0x30
    b 16
    la.local $sp, SysSp
    ld.d $sp, $sp, 0
.endm

HandleMachineErrorEntry:
    storeContext
    bl handleMachineError
    loadContext
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
    ertn

StartProcess:
    li.d $t0, 0x7
    csrwr $t0, 0x1
    la.local $t0, SysSp
    st.d $sp, $t0, 0
    loadContext
    ertn
