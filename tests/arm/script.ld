/* Entry Point - musí být definován     */
ENTRY(test)

/* Specifikace paměťových prostorů čipu */
MEMORY {
  RAM (xrw)       : ORIGIN = 0x10000000, LENGTH = 4K
  ROM (xr)        : ORIGIN = 0x00000000, LENGTH = 32K
}
/* Kam umístíme stack - navrch RAM */
_estack = ORIGIN(RAM) + LENGTH(RAM);
/* Definice výstupních sekcí       */
SECTIONS {
  .fixed : {
    /* Takto se vkládají symboly, které se pak mohou použít jako konstanty v programu.
    Tečka představuje aktuální hodnotu PC (jako v asm), tedy adresu symbolu. */
    _test = .;
    /* Obvykle se tato sekce nějak pojmenuje, zde víme, že gcc to pojmenuje
       právě takto, není pak třeba používat __attribute__((section("nazev"))).
       Linker by to mohl uklidit (gc-sections), ale ENTRY (Vectors) to zachová
       takže není potřeba používat KEEP. Tohle jsou ale už dost divné finty */
    *(.rodata.test)
    *(SORT_BY_NAME(.text*)) /* .text*   sections (code)                     */
    *(.rodata*)             /* .rodata* sections (constants, strings, etc.) */
    *(.eh_frame)
    . = ALIGN(4);
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(.ctors))		/* for clang (old version) */
    KEEP (*(.init_array*))	/* for gcc   */
    PROVIDE_HIDDEN (__init_array_end   = .);
  } >ROM
  .reloc : {
    *(.data*)            /* .data* sections */
    *(.bss*)             /* .bss*  sections */
    *(COMMON)
  } >RAM

  .ARM.attributes 0 : { *(.ARM.attributes) }
}
