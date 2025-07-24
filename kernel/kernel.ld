OUTPUT_ARCH( "loongarch" )
ENTRY( _start )

SECTIONS
{
  . = 0xA00000;

  .text : {
    *(.text)
  }

  .data : {
    *(.data)
  }

  .bss : {
    *(.bss)
  }

  .rodata : {
    *(.rodata)
  }

  .stack : {
    *(.stack)
  }

  __init_array_start = .;
  .init_array : {
    *(.init_array)
  }
  __init_array_end = .;

  /DISCARD/ :
  {
    *(.comment)
  }
  KernelEnd = .;
}
