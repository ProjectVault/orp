/* TrODO: Needs to be 64k for release but -O0 binaries won't fit */
ROM_START = 0x00100000;
ROM_SIZE = 64k;

RAM_START = 0x00200000;
RAM_SIZE = 128k;

/* Reserve static blocks of memory for tasks */
NUM_TASKS       = 4; /* +1 for main task */
TASK_SIZE       = 16384;
TASK_STACK_SIZE = 2*1024;
TASK_HEAP_SIZE  = TASK_SIZE - TASK_STACK_SIZE;

MEMORY
{
	rom (rx) : ORIGIN = 0x00100000, LENGTH = 64k 
	ram (rw) : ORIGIN = 0x00200000, LENGTH = 128k
}

/* ENTRY(_reset_vector) */

SECTIONS
{
	. = ROM_START;

	/* Vectors start at 0 (TODO: these will be remapped to remove need for addressable memory at the null address) */
	.vectors ALIGN(4) :
	{
		_vec_start = .;
		*(.vectors)
		_vec_end = .;
	} > rom

	/* The first page of addr space is reserved by or1k */
	. = ROM_START + 0x00002000;

	.text ALIGN(4) :
	{
		*(.text)
		*(.rodata)
		*(.rodata.*)
	} > rom

	/* Jump to RAM address now */
	. = RAM_START;


	/* Task mem get linked in *before* .data and .bss so that
	those addresses underflow into invalid address space */

	PROVIDE( tasks_start      = . );
	PROVIDE( tasks_size       = TASK_SIZE );
	PROVIDE( tasks_stack_size = TASK_STACK_SIZE );
 	PROVIDE( tasks_heap_size  = TASK_HEAP_SIZE );

	.tasks ALIGN(4) :
	{
		PROVIDE( stack_bottom = .); /* sbrk will hate this, shouldn't be using that anyway... */
		/* Alloc for main task */
		. += TASK_STACK_SIZE;
		PROVIDE( stack_top = .);
		. += TASK_HEAP_SIZE;

		/* Alloc space for use tasks */
		. += TASK_SIZE * NUM_TASKS;
	} >ram


	.data ALIGN(4) : 
	{
		_data_loadaddr = LOADADDR (.data);
		_data_beg = .;
		*(.data)
		*(.data.rel)
        *(.data.rel.local)
		_data_end = .;
	} > ram AT>rom

	.bss ALIGN(4) : 
	{
		__bss_start = .;
		_bss_beg = .;
		*(.bss)
        . = ALIGN(4);
		_bss_end = .;
	} >ram AT>rom

	/* This will be the initial stack pointer after reset */
	PROVIDE( end = ALIGN(RAM_SIZE) );
}
