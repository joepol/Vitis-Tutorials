



# Connect to remote dev machine [Putty]
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

# Compiler and Linker flags
xrt project properties > C/C++ Build > Settings > V++ compiler/linker > Miscellaneous

# Open Vivado project 
Xilinx > Vivado integration > open vivado project
