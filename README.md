# Therminator 2: A fast thermal simulator for portable devices

## Description
Therminator 2 is a compact thermal modeling-based component-level thermal simulator that targets small form-factor mobile devices (such as smartphones). It produces temperature maps for all components, including the application processor, battery, display, and other key device components, as well as the skin of the device itself, with high accuracy and short runtime.

Therminator 2 takes two input files as explained next. The `specs.xml` file describes the smartphone design, including components of interest and their geometric dimensions (length, width, and thickness) and relative positions. Therminator 2 has a built-in library storing properties (i.e., thermal conductivity, density, and specific heat) of common materials that are used to manufacture smartphones. In addition, users can override these properties or specify new materials through the `specs.xml` file. The `power.trace` file provides the usage information (power consumption) of those components that consume power and generate heat, e.g., ICs, battery, and display. The power.trace can be obtained through real measurements or other power estimation tools/methods. `power.trace` is a separate file so that one can easily interface a performance-power simulator with Therminator 2. Section (6) of this `README.md` file explains the syntax of input files.

More details about Therminator 2 can be found in [1].

## Changelog
See [changelog](CHANGELOG.md) file for details.

## Requirements
* Operating system: You are recommended to use Linux. In case you want to use Windows, we stronge encourage you to use [Windows Subsystem for Linux (WSL)](https://docs.microsoft.com/en-us/windows/wsl/about). We are going to update the project to support MacOS in near future.

* GNU build utilities (G++ 5+ and Make).

* Intel MKL

* Eigen 3

* Boost C++ Libraries

**NOTE:** Commands listed below are tested in *Ubuntu 20.10*, but should work on other modern Linux distributions.

## Installation
1. Clone the project using the following commnad.

		git clone https://github.com/mjdousti/Therminator && cd Therminator
2. Install the requirements using this command:

        sudo apt install build-essential libmkl-dev libboost1.71-dev libeigen3-dev
2. Build Therminator 2 with the following option:

        make -j
    * We are going to add options to build Therminator 2 for NVIDIA GPUs but as indicated in [1], the result will be inferior compared to the default CPU mode.
3. To remove all of the built files, type:

       make clean


## Usage
### Directories
```
Therminator
├── examples
│   ├── package_*.xml -> design specification files
│   ├── power_*.trace -> power trace files 
│   ├── temperature_results.txt* -> sample output files for the given GS4 and its power trace
├── experiments
│   ├── package_N5_*.xml -> design specification files used in [1]
├── src
│   ├── headers
│   │   ├── *.h -> header files
│   │   └── cuda_helper -> NVIDIA CUDA helper library
│   ├── libs
│   │   └── pugixml -> pugixml source code
│   └── *.cpp -> Therminator 2 source code
├── therminator1_src.tar.gz -> Therminator v1 source code
├── CHANGELOG.md -> list of changes
├── LICENSE -> license file
├── Makefile -> Make file
└── README.md -> this file
```

### Input files syntax
1. Design specification file (`specs.xml`)

    a) Two example files (`package_GS4.xml` and `package_MDP.xml`) have been provided along with the package of Therminator 2. You can basically follow them to specify a design specification.

    b) The outer hierarchy is the device level description, where you can specify the overall dimensions of the device and the ambient temperature. Three sub-hierarchies are material, floorplan, and component.

    c) The material hierarchy specifies the material properties that are used to build the device. The current version of Therminator 2 is concerned with steady-state temperature maps, and thereby, only the thermal conductivity information matters. The feature that captures transient temperatures will be developed later.

    d) The floorplan hierarchy is an optional input that specifies the floorplan of some selected components. For example, in `package_GS4.xml` file, a floor plan is specified for the Snapdragon_600 processor. The purpose of this is to provide higher accuracy of temperature maps, given more detailed information about processors, or other IC components. Note that the coordinates in the floorplan hierarchy is re-normalized such that the left-bottom corner of the component is set to be (0,0).

    e) The component hierarchy specifies the components of interest. The properties of each component include:
     * `coordinates`: numerical values. The coordinates (x, y, and z) are relative to the left-bottom corner of the device.

     * `dimensions`: numerical values. Length, width, and height of the component.

     * `materials`: The type of material shall be specified in the material hierarchy.

     * `power gen` (`yes`/`no`): This property indicates whether this component consumes power and generates heat. If it is specified as `yes`, the power consumption shall be provided in the power trace file.

     * `fill`: some IC chips have a thermal pad pasted on it. If the value is set as `yes`, Therminator 2 will automatically calculates the free space above this component, and fills the free space with material specified in filling_material.

     * `lateral_connectivity`: Many materials are orthotropic (i.e., having different thermal conductivity along x, y, and z directions.) This tells Therminator 2 whether this component is orthotropic.

     * `resolution`: How many sub-components will Therminator 2 divide this component to. You can specify resolution in x, y, and z directions. An automated approach for determining the resolution will be developed later.

    Note that, currently, users need to identify major free space manually and specify them as air blocks (ab). This will prevent singularity issue when reversing the conduction matrix. Automatic gap-fill will be developed in next release.

2. Power trace file
    This file specifies the power consumption of each component that is declared as `yes` in its power gen property in the design specification file. Depending on the use of floorplan feature, you can use either of the following options:

	a) If you did not enable the floorplan feature, then the number of power consumption values you provide here is the same as those components that consume power in design specification file.

    b) If you enable the floorplan feature and the component consumes power, you need to provide the power consumption for each component in the floorplan. You can add a column with name of `floorplanname-componentname`.

    A line starting with a `#` character will be treated as a comment and ignored. During the steady-state simulation, only the first line is read and used; however, during the transient-state simulation, each line of the file represent the power trace that has been available for each second. The time duration is hard-coded in `model.cpp` file and will soon can be modified through an input argument.

### Running Therminator 2
```
usage: therminator -d <file> -p <file> -o <file> [-t]
Therminator v2: A fast thermal simulator for portable devices
 -d <file>      Input design specification file
 -p <file>      Input power trace file
 -o <file>      Output file
 -t             Transient analysis
 -e             Export internal matrices and vectors
 -h             Shows this help menu
```

### Example Outputs
Therminator 2 outputs the temperature (in Celsius) of each sub-component in the output results file. In case of multiple layers specified in the z-direction, results on the top have the highest z value.

For example, the provided design for Snapdragon 600 in `package_GS4.xml`
has the resolution of 6x6x3 (a total of 108 subcomponents). Running 
```
$ ./therminator2 -d examples/package_GS4.xml -p examples/power_GS4.trace -o temperature_results.txt
```
gives this output for Snapdragon 600:
```
y
^                   z = 3 (top layer)
|60.4    60.7    61.0    60.9    60.8    60.0
|60.9    61.3    61.6    61.6    61.4    60.5
|61.3    61.7    62.0    62.0    61.8    60.8
|61.4    61.8    62.1    62.0    61.9    60.9
|61.2    61.6    61.9    61.8    61.7    60.8
|60.8    61.1    61.4    61.4    61.2    60.5  
|----------------------------------------------------> x
|                   z = 2
|62.1    62.4    62.6    62.6    62.4    61.8
|62.6    62.9    63.2    63.1    63.0    62.2
|62.9    63.2    63.5    63.4    63.3    62.5
|63.0    63.3    63.6    63.5    63.3    62.6
|62.8    63.1    63.4    63.3    63.2    62.5
|62.5    62.8    63.0    63.0    62.8    62.2 
|----------------------------------------------------> x
|                   z = 1  (bottom layer)
|61.9    62.2    62.3    62.3    62.2    61.7
|62.4    62.6    62.8    62.8    62.6    62.1
|62.6    62.9    63.1    63.1    62.9    62.3
|62.7    63.0    63.2    63.1    63.0    62.4
|62.6    62.8    63.0    63.0    62.9    62.3
|62.4    62.6    62.7    62.7    62.6    62.1  
-----------------------------------------------------> x
```

In order to use transient-state simulation capability, `-t` argument should be added to the above command as follows:
```
$ ./therminator2 -d examples/package_GS4.xml -p examples/power_GS4.trace -o temperature_results.txt -t
```
This results in generations of two files:
* `temperature_results.txt_0`: This file is identical to the steady-state simulation result obtained above and demonstrates the initial condition of the device.
* `temperature_results.txt_1`: This file represents the transient-state thermal status of the device after 1 sec and due to the second power trace given in the file.

## Developers
* Mohammad Javad Dousti
* Qing Xie
* Mahdi Nazemi
* Massoud Pedram

## Contact us
If you have any question, find any bug, or encounter any problem when using Therminator 2, feel free to contact Mohammad Javad Dousti (`me [at] mjdousti [dot] com`).

## References
If you use Therminator, please ensure to cite the following papers:

[1] Mohammad Javad Dousti, Qing Xie, Mahdi Nazemi, and Massoud Pedram, "Therminator 2: A Fast Thermal Simulator for Portable Devices," *IEEE Transactions on Computer-Aided Design of Integrated Circuits and Systems* (TCAD), Jan. 2021.

[2] Qing Xie, Mohammad Javad Dousti, and Massoud Pedram, "Therminator 2: A Thermal Simulator for Smartphones Producing Accurate Chip and Skin Temperature Maps", in *Proc. of the International Symposium on Low Power Electronics and Design* (ISLPED), pp. 117-122, Aug. 2014.

## License
Please refer to the [LICENSE](LICENSE) file.
