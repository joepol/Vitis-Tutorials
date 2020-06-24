## [Set clock frequency](https://www.xilinx.com/html_docs/xilinx2019_2/vitis_doc/Chunk1193338764.html?hl=compiler#qcm1528577331870__section_frk_xtr_t3b)
 [default frequency for kernels](https://www.xilinx.com/html_docs/xilinx2019_2/vitis_doc/Chunk1193338764.html#ans1568640653312__section_nfk_vf4_bjb)
 
You can define a clock frequency by ID, and frequencies for kernels - must verify the clock's connected to the kernel are as expected (use the Vivado block design.)
### Compiler & linker flag

`--kernel_frequency 300 --clock.defaultFreqHz 300000000`

## [multiple-ddr-banks](https://github.com/joepol/Vitis-Tutorials/blob/master/docs/mult-ddr-banks/README.md)

`--sp krnl_vadd_1.in1_re:HP0 --sp krnl_vadd_1.in1_im:HP1 --sp krnl_vadd_1.in2_re:HP2 --sp krnl_vadd_1.in2_im:HP3 --sp krnl_vadd_1.out:HP0`

`--sp KERNEL_NAME.INPUT_VECTOR1:HP0 --sp KERNEL_NAME.INPUT_VECTOR2:HP1`
