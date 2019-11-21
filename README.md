# CFED_Plugin
This repository holds the code for the GCC plugin that can automatically add multiple CFE detection techniques which was developed during the PhD of Jens Vankeirsbilck.
This plugin is meant to be compiled with g++-7 and to be executed with arm-none-eabi-gcc-7.3 .

## Build Instructions
To build this plugin, you have to adjust the `PATH_PLUGIN_HEADERS` to point to your local path to the GCC plugin headers. These are typically located at `<pathToArmNoneEabiGcc>/lib/gcc/arm-none-eabi/<version>/plugin/include`

Once adjusted, just execute `make` to build the plugin.

## How to Use the Plugin
This section describes how to use the plugin. 

### Reserving Registers
Each supported CFE detection technique requires hardware registers to be implemented. Since the GCC plugin is a late RTL pass, these registers must be reserved while compiling the target code using the GCC option `-ffixed-r<number>` in the C and C++ flags. The table below shows which registers are needed by which technique for which supported ARM ISA.

Supported Technique | ARMv6-M | ARMv7-M 
------------------- | ------- | -------
RACFED | r7 | r11
SEDSR | r7 | r11
CFCSS | r7 & r5 | r11 & r10
RSCFC | r7 & r5 | r11 & r10
ECCA | r7 & r5 | r11 & r10
SCFC | r7 & r5 | r11 & r10
YACCA | r7 & r5 & r4 | r11 & r10 & r9
YACCA_Fast | r7 & r5 & r4 | r11 & r10 & r9
SIED | r7 & r5 & r4 | r11 & r10 & r9

Since register r7 in ARMv6-M and register r11 in ARMv7-M can be used as frame pointers, it might be necessary to add the GCC option `-fomit-frame-pointer` to the C and C++ flags of the target code.

### Second Stack
Important to know about this plugin, is that it needs a second descending stack to push and pop the used register(s) of the implemented technique. This means that the linker file must provide room for this second stack and that the startup code must initialize the stack pointer of the plugin.

The register used as stack pointer is register r6 both for ARMv6-M as ARMv7-M. This register must thus also be reserved during compilation, using `-ffixed-r6` in the C and C++ flags of the target code. 

### Eliminating jump tables
Most supported techniques cannot handle jump tables, so it is best to make sure that GCC does not generate jump tabels by using the option `-fno-jump-tables` in the C and C++ flags of the target code. 

### Adding the Plugin to the Compilation Options
1) Specifying the plugin
To specify which plugin to use, the following must be added to the C and C++ flags of the target code: `-fplugin=<fullPathToPlugin>/CFED_plugin64.so` 

2) The plugin-arguments
To know which CFE detection technique to implement and which funtion(s) of the target code to protect, several plugin-arguments must be specified. 
* `-fplugin-arg-CFED_plugin64-function=<value>`: This argument specifies which function to protect with the selected CFE detection technique. <value> can have one out of two values:
   * *<functionName>*, then only that function of the target code is protected; or
   * *<empty>*, then all functions of the target code are protected. To explicitly exclude a function from protection, use the following function attribute for that function `__attribute__((noProtection))`
* `-fplugin-arg-CFED_plugin64-techniqueType=<value>`: This argument specifies whether only the inter-block CFE detection instructions of a technique should be implemented or if both the intra-block and inter-block CFE detection instructions should be implemented. <value> can have one out of two values:
   * *SigMon*: Only the inter-block CFE detection instructions are inserted.
   * *fullCFED*: Both the inter-block and intra-block CFE detection instructions are inserted. This is only supported by RACFED, RSCFC and SIED!
* `-fplugin-arg-CFED_plugin64-techniqueSpecific=<value>`: This argument specifies which technique to implement. For values, see the first column of the table above. 
* `-fplugin-arg-CFED_plugin64-selectiveLevel=<value>`: This argument specifies whether or not the specified technique should be implemented selectively. <value> can have one out of two values:
   * *0*: The selected technique is fully implemented, meaning that comparison instructions are inserted in each basic block. This leads to a higher overhead, but a low error detection latency.
   * *1*: The selected technique is selectively implemented, meaning that comparison instructions are only inserted in exit basic blocks. This reduces the overhead, but increases the error detection latency. This is only supported by RACFED, RSCFC and SIED!
  
## References to the Supported Techniques
Technique | DOI
--------- | ---
RACFED | [10.1007/978-3-319-99130-6_15](https://doi.org/10.1007/978-3-319-99130-6_15)
SEDSR | [10.4236/jsea.2012.59078](https://doi.org/10.4236/jsea.2012.59078)
CFCSS | [10.1109/24.994926](https://doi.org/10.1109/24.994926)
RSCFC | [10.1016/j.ast.2006.06.006](https://doi.org/10.1016/j.ast.2006.06.006)
ECCA | [10.1109/71.774911](https://doi.org/10.1109/71.774911)
SCFC | [10.1109/TII.2013.2248373](https://doi.org/10.1109/TII.2013.2248373)
YACCA & YACCA_Fast | [10.1109/DFTVS.2003.1250158](https://doi.org/10.1109/DFTVS.2003.1250158)
SIED | [10.1109/DFTVS.2003.1250159](https://doi.org/10.1109/DFTVS.2003.1250159)
