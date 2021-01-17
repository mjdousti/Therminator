# Changelog
All notable changes to Therminator will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and Therminator adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [2.0] - 2021-01-16
### Added
- Support for transient-state simulation.
- Support for fast simulation for CPUs. The new version reduces the runtime by 33x on average for steady-state thermal simulations compared to the earlier version running on a modern NVIDIA GPU. See reference [1] for more details.

### Changes
- Modernized C++ code and styled it based on [Clang format](https://clang.llvm.org/docs/ClangFormat.html).


### Fixed
- Modified matrix processor and significantly sped up the preprocessing step.


## [1.0] - 2015-02-15
### Added
- Initial release of Therminator.
- Supports steady-state thermal simulations.
- Fast simulation is only feasible on NVIDIA GPUs with a proprietry GPU library. It can be obtained from [here](therminator1_src.tar.gz). See reference [1] for more details.