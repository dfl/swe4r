# Swe4r

**Swiss Ephemeris for Ruby**

[![Gem Version](https://badge.fury.io/rb/swe4r.svg)](https://badge.fury.io/rb/swe4r)

Native Ruby bindings for the [Swiss Ephemeris](http://www.astro.com/swisseph/) library, providing high-precision astronomical calculations for astrological applications.

## Installation

```bash
gem install swe4r
```

Or add to your Gemfile:

```ruby
gem 'swe4r'
```

## Features

- High-precision planetary positions (Sun, Moon, planets, asteroids)
- House cusp calculations (Placidus, Koch, Equal, and many more)
- Fixed star positions and magnitudes
- Solar and lunar eclipse calculations
- Heliacal rising/setting times
- Sidereal and tropical zodiac support
- Topocentric positions
- Both Universal Time (UT) and Ephemeris Time (ET) variants

## Supported Functions

### Planetary Calculations

| Function | Description |
|----------|-------------|
| `swe_calc_ut` | Calculate planetary positions (Universal Time) |
| `swe_calc` | Calculate planetary positions (Ephemeris Time) |
| `swe_calc_pctr` | Planet-centric calculations |
| `swe_get_orbital_elements` | Get orbital elements for a body |
| `swe_nod_aps_ut` / `swe_nod_aps` | Planetary nodes and apsides |
| `swe_pheno_ut` | Planetary phenomena (phase, elongation, etc.) |

### Fixed Stars

| Function | Description |
|----------|-------------|
| `swe_fixstar_ut` / `swe_fixstar` | Fixed star positions |
| `swe_fixstar2_ut` / `swe_fixstar2` | Fixed star positions (faster) |
| `swe_fixstar_mag` / `swe_fixstar2_mag` | Fixed star magnitudes |

### House Systems

| Function | Description |
|----------|-------------|
| `swe_houses` | Calculate house cusps and angles |
| `swe_houses_ex` | Extended house calculation |
| `swe_houses_ex2` | Extended house calculation with speeds |
| `swe_houses_armc` | Houses from ARMC |
| `swe_house_pos` | House position of a body |
| `swe_house_name` | Get house system name |

### Eclipses

| Function | Description |
|----------|-------------|
| `swe_sol_eclipse_when_glob` | Find next solar eclipse globally |
| `swe_sol_eclipse_when_loc` | Find next solar eclipse at location |
| `swe_sol_eclipse_how` | Solar eclipse attributes |
| `swe_sol_eclipse_where` | Geographic location of eclipse |
| `swe_lun_eclipse_when` | Find next lunar eclipse |
| `swe_lun_eclipse_when_loc` | Find next lunar eclipse at location |
| `swe_lun_eclipse_how` | Lunar eclipse attributes |

### Crossings and Transits

| Function | Description |
|----------|-------------|
| `swe_solcross_ut` / `swe_solcross` | Sun crossing a longitude |
| `swe_mooncross_ut` / `swe_mooncross` | Moon crossing a longitude |
| `swe_mooncross_node_ut` / `swe_mooncross_node` | Moon crossing its node |
| `swe_helio_cross_ut` / `swe_helio_cross` | Heliocentric crossing |
| `swe_rise_trans` | Rising, setting, and transits |
| `swe_rise_trans_true_hor` | Rise/set with true horizon |

### Coordinate Systems

| Function | Description |
|----------|-------------|
| `swe_azalt` | Convert to horizontal coordinates |
| `swe_azalt_rev` | Convert from horizontal coordinates |
| `swe_cotrans` | Coordinate transformation |
| `swe_cotrans_sp` | Coordinate transformation with speeds |
| `swe_refrac` | Atmospheric refraction |

### Sidereal & Ayanamsa

| Function | Description |
|----------|-------------|
| `swe_set_sid_mode` | Set sidereal mode |
| `swe_get_ayanamsa_ut` / `swe_get_ayanamsa` | Get ayanamsa value |
| `swe_get_ayanamsa_ex_ut` / `swe_get_ayanamsa_ex` | Extended ayanamsa |
| `swe_get_ayanamsa_name` | Get ayanamsa name |

### Time Functions

| Function | Description |
|----------|-------------|
| `swe_julday` | Convert date to Julian day |
| `swe_revjul` | Convert Julian day to date |
| `swe_utc_to_jd` | UTC to Julian day |
| `swe_jdut1_to_utc` | Julian day to UTC |
| `swe_deltat` / `swe_deltat_ex` | Delta T (ET - UT) |
| `swe_sidtime` / `swe_sidtime0` | Sidereal time |
| `swe_time_equ` | Equation of time |
| `swe_day_of_week` | Day of week |
| `swe_lmt_to_lat` / `swe_lat_to_lmt` | Local mean time conversion |

### Special Functions

| Function | Description |
|----------|-------------|
| `swe_gauquelin_sector` | Gauquelin sector position |
| `swe_heliacal_ut` | Heliacal rising/setting |
| `swe_vis_limit_mag` | Visibility limit magnitude |

### Utility Functions

| Function | Description |
|----------|-------------|
| `swe_set_ephe_path` | Set ephemeris file path |
| `swe_set_jpl_file` | Set JPL ephemeris file |
| `swe_set_topo` | Set topocentric location |
| `swe_close` | Close Swiss Ephemeris |
| `swe_version` | Get library version |
| `swe_get_planet_name` | Get planet name |
| `swe_degnorm` / `swe_radnorm` | Normalize degrees/radians |
| `swe_split_deg` | Split degrees to components |

## Examples

### Calculate Planetary Position

```ruby
require 'swe4r'

# Get Julian day for May 14, 2012 at 10:15
jd = Swe4r.swe_julday(2012, 5, 14, 10.25)

# Calculate Sun position using Moshier ephemeris
sun = Swe4r.swe_calc_ut(jd, Swe4r::SE_SUN, Swe4r::SEFLG_MOSEPH | Swe4r::SEFLG_SPEED)

puts "Sun longitude: #{sun[0]}°"
puts "Sun latitude: #{sun[1]}°"
puts "Distance (AU): #{sun[2]}"
puts "Speed (deg/day): #{sun[3]}"
```

### Calculate House Cusps

```ruby
require 'swe4r'

jd = Swe4r.swe_julday(2012, 5, 14, 10.25)
lat = 45.45
lon = -112.183333

# Calculate Placidus houses
cusps, angles = Swe4r.swe_houses(jd, lat, lon, 'P')

puts "Ascendant: #{angles[0]}°"
puts "MC: #{angles[1]}°"
(1..12).each { |i| puts "House #{i}: #{cusps[i]}°" }
```

### Sidereal Calculations

```ruby
require 'swe4r'

jd = Swe4r.swe_julday(2012, 5, 14, 10.25)

# Set Lahiri ayanamsa
Swe4r.swe_set_sid_mode(Swe4r::SE_SIDM_LAHIRI, 0, 0)

# Get ayanamsa
ayanamsa = Swe4r.swe_get_ayanamsa_ut(jd)
puts "Ayanamsa: #{ayanamsa}°"

# Calculate sidereal position
sun = Swe4r.swe_calc_ut(jd, Swe4r::SE_SUN, Swe4r::SEFLG_MOSEPH | Swe4r::SEFLG_SIDEREAL)
puts "Sun (sidereal): #{sun[0]}°"
```

### Find Next Solar Eclipse

```ruby
require 'swe4r'

jd = Swe4r.swe_julday(2024, 1, 1, 0.0)

# Find next solar eclipse globally
result = Swe4r.swe_sol_eclipse_when_glob(jd, Swe4r::SEFLG_MOSEPH, 0, 0)

eclipse_type = result[0]
eclipse_time = result[1]

date = Swe4r.swe_revjul(eclipse_time)
puts "Next eclipse: #{date[0]}/#{date[1]}/#{date[2]}"
```

## Constants

The gem provides constants for planets, flags, house systems, and sidereal modes:

```ruby
# Planets
Swe4r::SE_SUN, SE_MOON, SE_MERCURY, SE_VENUS, SE_MARS
Swe4r::SE_JUPITER, SE_SATURN, SE_URANUS, SE_NEPTUNE, SE_PLUTO
Swe4r::SE_MEAN_NODE, SE_TRUE_NODE, SE_CHIRON, etc.

# Calculation flags
Swe4r::SEFLG_MOSEPH    # Moshier ephemeris (no files needed)
Swe4r::SEFLG_SWIEPH    # Swiss Ephemeris files
Swe4r::SEFLG_SPEED     # Include speed in results
Swe4r::SEFLG_TOPOCTR   # Topocentric positions
Swe4r::SEFLG_SIDEREAL  # Sidereal zodiac
Swe4r::SEFLG_EQUATORIAL # Equatorial coordinates

# Sidereal modes
Swe4r::SE_SIDM_LAHIRI, SE_SIDM_FAGAN_BRADLEY, SE_SIDM_RAMAN, etc.
```

## Ephemeris Files

The Moshier ephemeris (`SEFLG_MOSEPH`) is built-in and requires no additional files. For higher precision, download Swiss Ephemeris files from [astro.com](https://www.astro.com/ftp/swisseph/ephe/) and set the path:

```ruby
Swe4r.swe_set_ephe_path('/path/to/ephemeris/files')
```

## Documentation

- [Swiss Ephemeris Programmer's Documentation](https://www.astro.com/swisseph/swephprg.htm)
- [Swiss Ephemeris Homepage](http://www.astro.com/swisseph/)

## License

GPL-2.0-or-later

Swe4r is free software distributed under the GNU General Public License. See [LICENSE](LICENSE) for details.

The Swiss Ephemeris library is also distributed under GPL by Astrodienst AG.
