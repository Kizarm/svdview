/*
*/

/* Entry Point */
ENTRY(Reset_Handler)

/* Generate a link error if heap and stack don't fit into RAM */
_Min_Heap_Size  = 0; /* required amount of heap  */
_Min_Stack_Size = 0; /* required amount of stack */

/* Specify the memory areas */
MEMORY {
  FLASH (rx)      : ORIGIN = 0x08000000, LENGTH = 1024K
  RAM (xrw)       : ORIGIN = 0x20000000, LENGTH = 112K
}

/* Highest address of the user mode stack */
_estack = ORIGIN(RAM) + LENGTH(RAM);

/* Define output sections */
SECTIONS {
  /* The startup code goes first into FLASH */
  /* The program code and other data goes into FLASH */
  .text :  {
    . = ALIGN(4);
    PROVIDE_HIDDEN (_vect_tab_begin = .);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)

    *(.init)
    *(.fini)
    /* Pro použití statických konstruktorů v C++, KEEP musí být použit při gc */
    . = ALIGN(4);
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end   = .);
    
    /* Pro použití statických destruktorů v C++, KEEP musí být použit při gc */
    . = ALIGN(4);
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(.dtors))		/* for clang */
    KEEP (*(.fini_array*))	/* for gcc   */
    PROVIDE_HIDDEN (__fini_array_end   = .);
    
    . = ALIGN (128);
    *(.image*)
    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >FLASH

  /* used by the startup to initialize data */
  _sidata = .;

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data : AT ( _sidata )  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */

    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } >RAM

  /* Uninitialized data section */
  . = ALIGN(4);
  .bss : {
    /* This is used by the startup in order to initialize the .bss secion */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    __bss_end__ = _ebss;
  } >RAM

  /* User_heap_stack section, used to check that there is enough RAM left */
  ._user_heap_stack (NOLOAD) : {
    . = ALIGN(4);
    *(.heap)
    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    PROVIDE ( end = . );
    PROVIDE ( _end = . );
    . = . + _Min_Heap_Size;
    . = . + _Min_Stack_Size;
    . = ALIGN(4);
  } >RAM

  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    /*
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
    *(.debug*)
    *(.comment*)
    *(.ARM.*)
    */
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
