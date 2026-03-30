.globl _start

#include <csr.h>

_start:
	li.d	$t0, 0x8
	csrwr	$t0, CSR_CRMD

	li.d	$t0, 0x8000000000000001
	csrwr	$t0, CSR_DMW1
	li.d	$t0, 0x1
	csrwr	$t0, CSR_DMW0
	csrwr	$zero, CSR_DMW2
	csrwr	$zero, CSR_DMW3

	li.d	$t0, 0x10
	csrwr	$t0, CSR_CRMD

	la.local	$t1, KernelMain
	jr $t1
