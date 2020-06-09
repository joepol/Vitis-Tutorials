



# [Putty] Connect to remote dev machine 
`vncserver :3 -geometry 1920x1080`
## Machine GUI Terminal
`source /opt/Xilinx/Vitis/2019.2/settings64.sh`

`vitis`


# [Vitis] Create an application project
#sysroot 
-Path where the toolchain is installed

- Build `Hardware`
- copy all contents from `PROJECT_NAME/Hardware/sd_card` to SD card.
- Connect to board [Putty]
- `/mnt` run `.exe` file with `.xclbin` file as parameter. 

# [Vitis] Build host and FPGA binary
add host cpp and H files, Kernel files. 
in the project `.prj` settings add `Hardware Functions`. 
press on the project settings file. build `Emulation-HW`, then `build Hardware`

# [Vitis] Compiler and Linker flags
xrt project properties > C/C++ Build > Settings > V++ compiler/linker > Miscellaneous

# [Vitis-Vivado] Hw layout 
Xilinx > Vivado integration > open vivado project

# [Vivado] Block Design
IP INTEGRATOR >  Open Block Desgin

