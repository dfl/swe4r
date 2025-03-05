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
- Update Ruby version to 2.6
- Workflow improvements
- Remove Gemfile
- Update gemspec

## [0.0.8] - 2024-12-10
- Bugfix and test for swe_nod_aps_ut
- Revert "Exclude C files that have main method; multiple main methods causing compile issues"

## [0.0.7] - 2024-11-19
- Add basic CI + minor upgrades to support install/test
- Test in CI against ruby 2.0 - 3.3 on ubuntu and mac
- Fail fast on any failing step in `rake install`

## [0.0.6] - 2024-11-10
- Update Swiss Ephemeris to latest - 2.10.3a (Sep 20 2023)
- Update gemspec
- Fix compilation errors
- Improve tests

## [0.0.5] - 2024-01-15
- Add new functions (WIP untested)

## [0.0.4] - 2023-12-28
- Add solcross and mooncross methods
- Bump version

## [0.0.3] - 2023-03-28
- Add swe_house_pos()
- Fix house pos argument length

## [0.0.2] - 2023-02-20
- Add additional ayanamsa constants
- Fix warnings

## [0.0.1] - 2023-02-03
- Add swe_azalt and swe_cotrans
- Add swe_revjul() and variable arguments
- Fix lat/lon reversal
- Add sunrise/sunset calculations
- Fix ayanamsa_ex_ut
- Fix bindings and tests
- Update to latest Swiss Ephemeris

## [0.0.0] - 2012-05-14
- Initial commit
- Add constants for body numbers, iflags, and sidereal modes
- Documentation for gem version 0.0.1
