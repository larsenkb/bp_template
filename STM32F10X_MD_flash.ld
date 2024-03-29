/*
Default linker script for STM32F10x_128K_20K with GNU-ld
Martin Thomas, 9/2009
A heavily modified "Default linker script for STM32F10x_512K_64K" 
Copyright RAISONANCE S.A.S. 2008 found in the STM fw-library.
*/

/* Program Entry, set to mark it as "used" and avoid gc */
ENTRY(Reset_Handler)

/* Memory Spaces Definitions */
MEMORY
{
  RAM      (RWX) : ORIGIN = 0x20000000+0, LENGTH = 20K-0
/*  EXTSRAM  (RWX) : ORIGIN = 0x68000000, LENGTH = 0 */
  FLASH    (RX)  : ORIGIN = 0x08000000+0, LENGTH = 128K-0-0
/*  EEMUL    (RWX) : ORIGIN = 0x08000000+128k-1k, LENGTH = 1k */
/*  FLASHB1  (RX)  : ORIGIN = 0x00000000, LENGTH = 0 */
/*  EXTMEMB0 (RX)  : ORIGIN = 0x00000000, LENGTH = 0 */
/*  EXTMEMB1 (RX)  : ORIGIN = 0x00000000, LENGTH = 0 */
/*  EXTMEMB2 (RX)  : ORIGIN = 0x00000000, LENGTH = 0 */
/*  EXTMEMB3 (RX)  : ORIGIN = 0x00000000, LENGTH = 0 */
}

/* higher address of the user mode stack */
_estack = ORIGIN(RAM)+LENGTH(RAM);

/* start of the two 1kByte pages used by EEprom emulation at end of flash */
_seemul = ORIGIN(EEMUL);

/* There will be a link error if there is not this amount of RAM free at the end. */
_Minimum_Stack_Size = 0x100 ;

/* Check valid alignment for VTOR */
ASSERT(ORIGIN(FLASH) == ALIGN(ORIGIN(FLASH), 0x80), "Start of memory region flash not aligned for startup vector table");

/* Sections Definitions */

SECTIONS
{

    /* the program code is stored in the .text section, which goes to Flash */
    .text :
    {
        . = ALIGN(0x80); /* PM0056, Rev.1 (4/2009), 4.3.3 */
        _isr_vectorsflash_offs = . - 0x08000000;
        KEEP(*(.isr_vectorsflash))
        . = ALIGN(4);
        *(.text)                   /* code */
        *(.text.*)                 /* remaining code (-ffunction-sections)*/
        *(.stub .gnu.linkonce.t.*)
        *(.rodata .rodata.* .gnu.linkonce.r.*) /* read-only data (constants) */
        *(.glue_7 .glue_7t)        /* redundant for thumb2 ?! */
        . = ALIGN(4);
        _etext = .;
        /* This is used by the startup in order to initialize the .data secion */
        _sidata = _etext;
    } >FLASH


    /* This is the initialized data section
    The program executes knowing that the data is in the RAM
    but the loader puts the initial values in the FLASH (inidata).
    It is one task of the startup to copy the initial values from FLASH to RAM. */
    .data  : AT ( _sidata )
    {
        . = .;
        . = ALIGN(4);
        /* This is used by the startup in order to initialize the .data section */
        _sdata = . ;

        . = ALIGN(0x80);
        _isr_vectorsram_offs = . - 0x20000000;
        KEEP(*(.isr_vectorsram))
        . = ALIGN(4);

        *(.ramfunc*)
        . = ALIGN(4);
        *(.data .data.* .gnu.linkonce.d.*)
        . = ALIGN(4);
        /* This is used by the startup in order to initialize the .data section */
        _edata = . ;
    } >RAM


    /* This is the uninitialized data section */
    .bss (NOLOAD):
    {
        . = ALIGN(4);
        /* This is used by the startup in order to initialize the .bss section */
        _sbss = .;
        *(.bss .bss.*)
        *(COMMON)
        . = ALIGN(4);
         /* This is used by the startup in order to initialize the .bss section */
         _ebss = . ;
    } >RAM

    PROVIDE ( end = _ebss );
    PROVIDE ( _end = _ebss );
    
    /* This is the user stack section 
    This is just to check that there is enough RAM left for the User mode stack
    It should generate an error if it's full.
     */
    ._usrstack (NOLOAD):
    {
        . = ALIGN(4);
        _susrstack = . ;
        . = . + _Minimum_Stack_Size ;
        . = ALIGN(4);
        _eusrstack = . ;
    } >RAM


    /* this is the FLASH Bank1 */
    /* the C or assembly source must explicitly place the code or data there
    using the "section" attribute */
/*    .b1text :
    {
        *(.b1text)
        *(.b1rodata)
        *(.b1rodata*)
    } >FLASHB1
*/    
    /* this is the EXTMEM */
    /* the C or assembly source must explicitly place the code or data there
    using the "section" attribute */
    
    /* EXTMEM Bank0 */
/*    .eb0text :
    {
        *(.eb0text)
        *(.eb0rodata)
        *(.eb0rodata*)
    } >EXTMEMB0
*/    
    /* EXTMEM Bank1 */
/*    .eb1text :
    {
        *(.eb1text) 
        *(.eb1rodata) 
        *(.eb1rodata*)
    } >EXTMEMB1
*/    
    /* EXTMEM Bank2 */
/*    .eb2text :
    {
        *(.eb2text)  
        *(.eb2rodata) 
        *(.eb2rodata*)
    } >EXTMEMB2
*/    
    /* EXTMEM Bank0 */
/*    .eb3text :
    {
        *(.eb3text) 
        *(.eb3rodata)
        *(.eb3rodata*)
    } >EXTMEMB3
*/    
    
    
    /* after that it's only debugging information. */
    
    /* remove the debugging information from the standard libraries */
/*    DISCARD :
    {
     libc.a ( * )
     libm.a ( * )
     libgcc.a ( * )
     }
*/

  /* Stabs debugging sections.  */
  .stab          0 : { *(.stab) }
  .stabstr       0 : { *(.stabstr) }
  .stab.excl     0 : { *(.stab.excl) }
  .stab.exclstr  0 : { *(.stab.exclstr) }
  .stab.index    0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
  .comment       0 : { *(.comment) }
  /* DWARF debug sections.
     Symbols in the DWARF debugging sections are relative to the beginning
     of the section so we begin them at 0.  */
  /* DWARF 1 */
  .debug          0 : { *(.debug) }
  .line           0 : { *(.line) }
  /* GNU DWARF 1 extensions */
  .debug_srcinfo  0 : { *(.debug_srcinfo) }
  .debug_sfnames  0 : { *(.debug_sfnames) }
  /* DWARF 1.1 and DWARF 2 */
  .debug_aranges  0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
  /* DWARF 2 */
  .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
  .debug_abbrev   0 : { *(.debug_abbrev) }
  .debug_line     0 : { *(.debug_line) }
  .debug_frame    0 : { *(.debug_frame) }
  .debug_str      0 : { *(.debug_str) }
  .debug_loc      0 : { *(.debug_loc) }
  .debug_macinfo  0 : { *(.debug_macinfo) }
  /* SGI/MIPS DWARF 2 extensions */
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames  0 : { *(.debug_varnames) }
  /* DWARF 3 */
  .debug_pubtypes 0 : { *(.debug_pubtypes) }
  .debug_ranges   0 : { *(.debug_ranges) }

  .ARM.attributes 0 : { KEEP (*(.ARM.attributes)) KEEP (*(.gnu.attributes)) }
  .note.gnu.arm.ident 0 : { KEEP (*(.note.gnu.arm.ident)) }
  /DISCARD/ : { *(.note.GNU-stack) *(.gnu_debuglink) }
}

/* mthomas:  from the STM example - moved here for now to keep it as reference, not 
   used in the project (useful anyway?)*/

/* default stack size(s). These are used by the startup in order to allocate stacks for 
the different modes. */
__Stack_Size = 1024 ;

PROVIDE ( _Stack_Size = __Stack_Size ) ;
__Stack_Init = _estack - __Stack_Size ;

/*"PROVIDE" allows to easily override these values from an object file or the commmand line.*/
PROVIDE ( _Stack_Init = __Stack_Init ) ;
