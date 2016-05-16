SECTIONS
{
	. = 0x0C000000;
	Image_RO_Base = .;
	.text : { *(.text) }
	Image_RO_Limit = .;
	Image_RW_Base = .;
	.data : { *(.data) }
	.rodata : { *(.rodata) }
	Image_RW_Limit = .;
	Image_ZI_Base = .;
	.bss : { *(.bss) }
	Image_ZI_Limit = .;
	__bss_start__ = .;
	__bss_end__ = .;
}