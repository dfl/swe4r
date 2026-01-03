# Changelog

All notable changes to this project will be documented in this file.

## [1.3.0] - 2026-01-03

### Fixed
- Fixed buffer overflow crash (SIGABRT) when using Gauquelin sectors house system ('G')
- Increased cusps buffer from 13 to 37 to accommodate Gauquelin's 36 sectors

## [1.2.0] - 2025-12-24

### Added

**Eclipse Functions:**
- `swe_sol_eclipse_when_glob` - Find next solar eclipse globally
- `swe_sol_eclipse_when_loc` - Find next solar eclipse at a location
- `swe_sol_eclipse_how` - Calculate solar eclipse attributes
- `swe_sol_eclipse_where` - Find geographic location of solar eclipse
- `swe_lun_eclipse_when` - Find next lunar eclipse
- `swe_lun_eclipse_when_loc` - Find next lunar eclipse at a location
- `swe_lun_eclipse_how` - Calculate lunar eclipse attributes

**Special Functions:**
- `swe_gauquelin_sector` - Calculate Gauquelin sector position
- `swe_heliacal_ut` - Calculate heliacal rising/setting times
- `swe_vis_limit_mag` - Calculate visibility limit magnitude

**Ephemeris Time (ET) Versions:**
- `swe_calc` - Planetary calculation (ET)
- `swe_get_ayanamsa` - Ayanamsa (ET)
- `swe_get_ayanamsa_ex` - Extended ayanamsa (ET)
- `swe_solcross` - Sun crossing longitude (ET)
- `swe_mooncross` - Moon crossing longitude (ET)
- `swe_mooncross_node` - Moon crossing node (ET)
- `swe_helio_cross` - Heliocentric crossing (ET)
- `swe_nod_aps` - Planetary nodes and apsides (ET)

**Constants:**
- Eclipse type constants (`SE_ECL_*`)
- Heliacal event constants (`SE_HELIACAL_*`, `SE_HELFLAG_*`)

### Fixed
- Fixed crossing functions (`swe_solcross_ut`, `swe_mooncross_ut`, etc.) that were incorrectly comparing Ruby VALUE with C double
- Fixed `swe_helio_cross_ut` and `swe_helio_cross` to correctly return Julian day from output parameter

### Changed
- Extracted `test_helper.rb` for shared test setup
- Improved test coverage (46 tests, 185 assertions)

## [1.1.7] - 2025-07-06

### Added
- Additional Swiss Ephemeris functions
- `swe_houses_ex2` - Extended house calculation with speeds
- `swe_rise_trans` - Rising, setting, transit times
- `swe_rise_trans_true_hor` - Rise/set with true horizon
- `swe_azalt_rev` - Convert from horizontal coordinates
- `swe_refrac` - Atmospheric refraction
- `swe_pheno_ut` - Planetary phenomena
- `swe_time_equ` - Equation of time
- `swe_lmt_to_lat` / `swe_lat_to_lmt` - Local mean time conversion
- `swe_cotrans_sp` - Coordinate transformation with speeds
- `swe_sidtime0` - Sidereal time with parameters

## [1.1.6] - 2025-07-06

### Added
- `swe_fixstar_ut` - Fixed star positions (UT)
- `swe_fixstar` - Fixed star positions (ET)
- `swe_fixstar_mag` - Fixed star magnitude (legacy)
- `swe_fixstar2_ut` - Fixed star positions, faster (UT)
- `swe_fixstar2` - Fixed star positions, faster (ET)
- `swe_fixstar2_mag` - Fixed star magnitude

## [1.1.5] - 2025-03-05

### Added
- `swe_get_orbital_elements` - Orbital elements for planets/asteroids
- `swe_deltat` - Delta T (ET - UT)
- `swe_deltat_ex` - Extended delta T with flags

## [1.1.0] - Previous

### Added
- Core Swiss Ephemeris functions
- `swe_calc_ut` - Planetary positions
- `swe_houses` / `swe_houses_ex` - House cusps
- `swe_julday` / `swe_revjul` - Julian day conversion
- `swe_set_sid_mode` / `swe_get_ayanamsa_ut` - Sidereal mode
- `swe_azalt` / `swe_cotrans` - Coordinate transformations
- `swe_solcross_ut` / `swe_mooncross_ut` / `swe_helio_cross_ut` - Crossings
- `swe_nod_aps_ut` - Nodes and apsides
- Planet, flag, and sidereal mode constants
