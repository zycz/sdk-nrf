/* Place your custom section at the very end of FLASH (after all other sections) */
/* Place .myflashend right before the end of FLASH */
.myflashend :
{
    KEEP(*(.myflashend))
} > FLASH
