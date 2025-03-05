# Changelog

All notable changes to this project will be documented in this file.

## [Unreleased]

## [0.1.0] - 2025-03-05

### Added
- Initial release with Swiss Ephemeris Ruby bindings
- Support for planetary positions calculation
- Support for house systems
- Support for ayanamsa calculations
- Basic astronomical calculations
- Examples demonstrating usage

## [0.0.9] - 2025-01-24

### Changed
- Update Ruby version to 2.6
- Update gemspec with improved metadata
- Improve workflow configuration

### Removed
- Remove Gemfile and Gemfile.lock

## [0.0.8] - 2024-12-10

### Fixed
- Bugfix and test for swe_nod_aps_ut

### Changed
- Revert "Exclude C files that have main method; multiple main methods causing compile issues"

## [0.0.7] - 2024-11-19

### Added
- Add basic CI with GitHub Actions
- Test in CI against Ruby 2.0 - 3.3 on Ubuntu and macOS

### Changed
- Fail fast on any failing step in `rake install`
- Minor upgrades to support installation and testing

## [0.0.6] - 2024-11-10

### Updated
- Update Swiss Ephemeris to latest version - 2.10.3a (Sep 20 2023)
- Update gemspec with current information

### Fixed
- Fix compilation errors
- Improve tests with better organization

## [0.0.5] - 2024-01-15

### Added
- Add new functions (WIP untested)

## [0.0.4] - 2023-12-28

### Added
- Add solcross and mooncross methods

### Changed
- Bump version in gemspec and documentation

## [0.0.3] - 2023-03-28

### Added
- Add swe_house_pos() function

### Fixed
- Fix house pos argument length

## [0.0.2] - 2023-02-20

### Added
- Add additional ayanamsa constants

### Fixed
- Fix warnings in code

## [0.0.1] - 2023-02-03

### Added
- Add swe_azalt and swe_cotrans functions
- Add swe_revjul() and variable arguments
- Add sunrise/sunset calculations

### Fixed
- Fix lat/lon reversal
- Fix ayanamsa_ex_ut
- Fix bindings and tests

### Updated
- Update to latest Swiss Ephemeris

## [0.0.0] - 2012-05-14

### Added
- Initial commit
- Add constants for body numbers, iflags, and sidereal modes
- Documentation for gem version 0.0.1
