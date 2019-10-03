# Therminator: Thermal Simulator for Portable Devices

## CONTENTS
1. License
2. Change log
3. Introduction
4. Requirements
5. Installation
6. Usage
7. Contact us
8. Reference

## License
Please refer to the LICENSE file.

## Change Log
```
    Date   | Version |    Details
---------------------------------------------------------------------------------
 9/24/2014 |  v1.00  | Initial release.
---------------------------------------------------------------------------------
11/13/2014 |  v1.01  | - Adding CUDA library path to Makefile for GPU processing
           |         | - Adding an assertion to check the components listed in
           |         |    the power trace file against the design file
```

## Introduction
### Therminator
Therminator is a compact thermal modeling-based component-level thermal 
simulator that targets small form-factor mobile devices (such as 
smartphones). It produces temperature maps for all components, including 
the application processor, battery, display, and other key device components, 
as well as the skin of the device itself, with high accuracy and short runtime.

Therminator takes two input files as explained next. The specs.xml file 
describes the smartphone design, including components of interest and 
their geometric dimensions (length, width, and thickness) and relative 
positions. Therminator has a built-in library storing properties (i.e., 
thermal conductivity, density, and specific heat) of common materials that 
are used to manufacture smartphones. In addition, users can override
these properties or specify new materials through the specs.xml file. The
power.trace file provides the usage information (power consumption) of 
those components that consume power and generate heat, e.g., ICs, battery,
and display. The power.trace can be obtained through real measurements 
or other power estimation tools/methods. power.trace is a separate file 
so that one can easily interface a performance-power simulator with Therminator. 
Section (6) of this README file explains the syntax of input files.

More details about Therminator can be found in [1].

### Developers
[Mohammad Javad Dousti](<dousti@usc.edu>)
[Qing Xie](<xqing@usc.edu>)
[Massoud Pedram](<pedram@usc.edu>)



## Requirements
- GNU build utilities (GCC 4+ and Make). 
  You are recommended to use Linux. In case you want to use Windows, make 
  sure to install MinGW or Cygwin.

- Install CULA Dense library along with an NVIDIA GPU with CUDA support (optional).
  Therminator allows a parallel computing feature that uses NVIDIA GPUs 
  to perform matrix computations. It is highly recommended to enable this 
  feature as it can significantly reduce the runtime. To utilize this feature, 
  you need to download the [CULA Dense library] and install it on your machine. Next,
	open the Makefile in the Therminator package and change lines 7 and 8, so that
	`INCLUDE` and `LIBRARIES` variables point to the directory where CULA is installed.
  
**Note**: Therminator can be run on Linux, Mac OS X and Windows given the above requirements 
      are properly installed. To install GCC on Windows, it is recommended to use MinGW
      which can be obtained from [here](http://www.mingw.org.)
 
## Installation
1. Extract the downloaded terminator package.
2. Build Therminator with either of the following options:
   * If you have CULA Dense library, you may enable the parallel computing feature by using the following command:

         make SOLVER=GPU
   * If you don't have CULA Dense library installed, use this command:

         make
3. To remove all of the built files, type:
    
       make clean


## Usage
### Directories
```
Therminator
|--src
    |-- header
        `-- *.h -> header files
    |-- libs
        `-- pugixml -> Light-weight, simple and fast XML parser for C++
    `-- *.cpp -> Therminator source code
|-- examples
    |-- package_*.xml -> design specification files
    |-- power_*.trace -> power trace files
    `-- Temperature_Results.txt -> Sample output file for the given GS4 and its power trace
|-- LICENSE -> license file
`-- README -> this file
```

### Input files syntax
1. Design specification file (`specs.xml`)

    a) Two example files (package_GS4.xml and package_MDP.xml) have been provided
    along with the package of Therminator. You can basically follow them to specify
    a design specification.

    b) The outer hierarchy is the device level description, where you can specify
    the overall dimensions of the device and the ambient temperature. Three sub-
    hierarchies are material, floorplan, and component.

    c) The material hierarchy specifies the material properties that are used
    to build the device. The current version of Therminator is concerned
    with steady-state temperature maps, and thereby, only the thermal conductivity
    information matters. The feature that captures transient temperatures will be
    developed later.

    d) The floorplan hierarchy is an optional input that specifies the
    floorplan of some selected components. For example, in package_GS4.xml
    file, a floor plan is specified for the Snapdragon_600 processor. The
    purpose of this is to provide higher accuracy of temperature maps, given
    more detailed information about processors, or other IC components.
    Note that the coordinates in the floorplan hierarchy is re-normalized such
    that the left-bottom corner of the component is set to be (0,0).

    e) The component hierarchy specifies the components of interest. The properties
    of each component include:
     * `coordinates` - numerical values. The coordinates (x, y, and z) are relative
        to the left-bottom corner of the device.

     * `dimensions` -  numerical values. Length, width, and height of the component.

     * `materials` - The type of material shall be specified in the material
        hierarchy.

     * `power gen` (`yes`/`no`) - This property indicates whether this component
        consumes power and generates heat. If it is specified as `yes`, the power
        consumption shall be provided in the power trace file.

     * `fill` - some IC chips have a thermal pad pasted on it. If the value is set
        as `yes`, Therminator will automatically calculates the free space above this
        component, and fills the free space with material specified in filling_material.

     * `lateral_connectivity` - Many materials are orthotropic (i.e., having
        different thermal conductivity along x, y, and z directions.) This tells
        Therminator whether this component is orthotropic.

     * `resolution` - How many sub-components will Therminator divide this
        component to. You can specify resolution in x, y, and z directions. An
        automated approach for determining the resolution will be developed later.

    Note that, currently, users need to identify major free space manually
        and specify them as air blocks (ab). This will prevent singularity issue
        when reversing the conduction matrix. Automatic gap-fill will be
        developed in next release.

2. Power trace file
    This file specifies the power consumption of each component that is declared
    as `yes` in its power gen property in the design specification file.

	a) If you did not enable the floorplan feature, then the number power
    consumptions you provide here is the same as those components that consume
    power in design specification file.

    b) If you enable the floorplan feature and the component consumes power,
    you need to provide the power consumption for each component in the
    floorplan. You can add column with name of `floorplanname-componentname`.

### Running Therminator
```
usage: therminator -d <file> -p <file> -o <file>
Therminator produces accurate temperature maps given the power consumption
and physical characteristics.
 -h            Shows this help menu
 -d <file>    Input design specification file name
 -p <file>    Input power trace file
 -o <file>    Output file
```

### Example Outputs
Therminator outputs the temperature (in Celsius) of each sub-component in the 
output results file. In case of multiple layers specified in the z-direction, 
results on the top have the highest z value.

For example, the provided design for Snapdragon 600 in `package_GS4.xml `
has the resolution of 6x6x3 (a total of 108 subcomponents). Running 
```
$ ./therminator -d examples/package_GS4.xml -p examples/power_GS4.trace \
    -o Temperature_Results.txt
```
gives this output:
```
y
^                    z = 3 (top layer)
|60.376     60.7172    61.0108    60.9561    60.8312   60.0348    
|60.9646    61.3455    61.6756    61.6117    61.4722   60.5633    
|61.3078    61.7083    62.0559    61.987     61.8393   60.8779    
|61.3934    61.7916    62.1365    62.0672    61.9197   60.9622    
|61.2314    61.6056    61.9277    61.8628    61.7237   60.8269    
|60.8264    61.1592    61.4399    61.3836    61.2596   60.4854    
|----------------------------------------------------> x
|                           z = 2
|62.08      62.3741    62.6072    62.5708    62.4319   61.7898    
|62.5968    62.921     63.1801    63.1376    62.9835   62.2609    
|62.9024    63.2422    63.5144    63.4684    63.3058   62.5431    
|62.9796    63.3177    63.5877    63.5411    63.3786   62.6184    
|62.8445    63.164     63.4166    63.3724    63.2184   62.5039    
|62.5136    62.8031    63.0259    62.9867    62.8478   62.2204    
|----------------------------------------------------> x
|                    z = 1  (bottom layer)
|61.9131    62.1597    62.3395    62.3151   62.1786    61.6693    
|62.3804    62.6498    62.8478    62.819    62.6683    62.101    
|62.6489    62.9304    63.1378    63.1064   62.9479    62.3507    
|62.7171    62.9973    63.203     63.1709   63.0122    62.4168    
|62.609     62.8749    63.0677    63.0367   62.8854    62.3235    
|62.3428    62.5869    62.7584    62.73     62.5923    62.0925    
-----------------------------------------------------> x
```

## Contact us
If you have any question, find any bug, or encounter any problem when using
Therminator, please contact [Mohammad Javad Dousti](<dousti@usc.edu>) or [Qing Xie](<xqing@usc.edu>).


## Reference
[1] Qing Xie, Mohammad Javad Dousti, and Massoud Pedram, "Therminator: A
Thermal Simulator for Smartphones Producing Accurate Chip and Skin Temperature
Maps", in *Proc. of the International Symposium on Low Power Electronics and 
Design* (ISLPED), pp. 117-122, Aug, 2014.
