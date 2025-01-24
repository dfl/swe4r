require 'swe4r'
require 'ostruct'
require 'victor'

class SwissEphemeris
  BODIES = {
    SO: { id: Swe4r::SE_SUN, name: 'sun', symbol: '☉' },
    MO: { id: Swe4r::SE_MOON, name: 'moon', symbol: '☽' },
    ME: { id: Swe4r::SE_MERCURY, name: 'mercury', symbol: '☿' },
    VE: { id: Swe4r::SE_VENUS, name: 'venus', symbol: '♀' },
    MA: { id: Swe4r::SE_MARS, name: 'mars', symbol: '♂' },
    JU: { id: Swe4r::SE_JUPITER, name: 'jupiter', symbol: '♃' },
    SA: { id: Swe4r::SE_SATURN, name: 'saturn', symbol: '♄' },
    UR: { id: Swe4r::SE_URANUS, name: 'uranus', symbol: '♅' },
    NE: { id: Swe4r::SE_NEPTUNE, name: 'neptune', symbol: '♆' },
    PL: { id: Swe4r::SE_PLUTO, name: 'pluto', symbol: '♇' }, # '⛢'
    MN: { id: Swe4r::SE_MEAN_NODE, name: 'n. node', symbol: '☊' },
    TN: Swe4r::SE_TRUE_NODE,
    LI: Swe4r::SE_MEAN_APOG,
    TL: Swe4r::SE_OSCU_APOG,
    EA: Swe4r::SE_EARTH,
    CH: { id: Swe4r::SE_CHIRON, name: 'chiron', symbol: '⚷' },
    CE: Swe4r::SE_CERES,
    PA: Swe4r::SE_PALLAS,
    PH: Swe4r::SE_PHOLUS,
    JN: Swe4r::SE_JUNO,
    VS: Swe4r::SE_VESTA,
    CU: Swe4r::SE_CUPIDO,
    HA: Swe4r::SE_HADES,
    ZE: Swe4r::SE_ZEUS,
    KR: Swe4r::SE_KRONOS,
    AP: Swe4r::SE_APOLLON,
    AD: Swe4r::SE_ADMETOS,
    VU: Swe4r::SE_VULKANUS,
    PO: Swe4r::SE_POSEIDON
  }.freeze

  SIGNS = %w[Ari Tau Gem Can Leo Vir Lib Sco Sag Cap Aqu Pis].freeze

  def path=(path)
    Swe4r.swe_set_ephe_path(path.to_s)
  end

  def jpl_file=(path)
    swe_set_jpl_file(path.to_s)
  end

  class Calculator
    attr_accessor :latitude, :longitude, :altitude, :date
    attr_reader :flags, :jd, :cusps, :asc, :mc, :armc, :vertex

    def initialize(params = {})
      # https://www.astro.com/swisseph/swephprg.htm
      params[:use_moshier_ephemeris] = true unless params[:moshier_ephemeris] == false
      @flags = Swe4r::SEFLG_SPEED
      # no light time correction - return true positions, not apparent
      @flags |= Swe4r::SEFLG_TRUEPOS if params[:trues]
      @flags |= Swe4r::SEFLG_SIDEREAL if params[:sidereal]
      @flags |= Swe4r::SEFLG_TOPOCTR if params[:topocentric]
      @flags |= Swe4r::SEFLG_HELCTR if params[:heliocentric]
      @flags |= Swe4r::SEFLG_MOSEPH if params[:use_moshier_ephemeris]
      @flags |= Swe4r::SEFLG_JPLEPH if params[:use_jpl_ephemeris]
      @flags |= Swe4r::SEFLG_SWIEPH if params[:use_swiss_ephemeris]
      # right ascension and declination instead of lat/lon
      @flags |= Swe4r::SEFLG_EQUATORIAL if params[:equatorial]
      self
    end

    def date=(time)
      @jd = Swe4r.swe_julday(time.year, time.month, time.day, time.hour + time.min / 60.0)
    end

    def set_topo(latitude, longitude, altitude = 0)
      @latitude = latitude
      @longitude = longitude
      Swe4r.swe_set_topo(longitude, latitude, altitude)
    end

    def position(object)
      obj = SwissEphemeris::BODIES[object]
      values = Swe4r.swe_calc_ut(jd, obj[:id], flags)
      # [longitude, latitude, distance, speed in long., speed in lat., and speed in dist.]
      Body.new(values[0], values[1], values[2], values[3], obj[:name], obj[:symbol])
    end

    def houses(calc_method = 'K')
      output = Swe4r.swe_houses(jd, latitude, longitude, calc_method)
      @cusps = output[1..12]
      ascmc = output[13..-1]
      @asc, @mc, @armc, @vertex = @ascm[0..3]
    end
  end

  class Body
    attr_reader :degree, :lat, :lon, :distance, :velocity, :name, :symbol

    def initialize(lon, lat, distance, velocity, name, symbol)
      @lat = lat
      @lon = lon
      @distance = distance
      @velocity = velocity
      @name = name
      @symbol = symbol
      @sign, @degree = lon.divmod(30)
      degree = @degree.floor
      @minute = (@degree - degree) * 60.0
      @minute = @minute.round
      @degree = degree
    end

    def sign
      SIGNS[@sign]
    end

    def to_s(kind = :full)
      pos = "#{@degree}º #{sign} #{@minute}"
      case kind
      when :short
        pos
      when :full
        "#{name.capitalize} #{pos}"
      end
    end
  end
end

calculator = SwissEphemeris::Calculator.new(light_correction: false, topocentric: true)

calculator.date = Time.new(1979, 5, 8, 10, 57, 0, '-09:00')

require 'geocoder'
results = Geocoder.search('Anchorage, AK')
calculator.set_topo(results.first.longitude, results.first.latitude)

# calculator.houses

sun = calculator.position(:SO)
moon = calculator.position(:MO)
mercury = calculator.position(:ME)
venus = calculator.position(:VE)
mars = calculator.position(:MA)
jupiter = calculator.position(:JU)
saturn = calculator.position(:SA)
uranus = calculator.position(:UR)
neptune = calculator.position(:NE)
pluto = calculator.position(:PL)

# chiron = calculator.position(:CH)
# node = calculator.position(:NN)

# pallas = calculator.position(:pallas)
# ceres = calculator.position(:ceres)
# vesta = calculator.position(:vesta)
# juno = calculator.position(:juno)

def calculate_aspects(degree1, degree2, orb = 12)
  # Create an empty array to store the harmonics
  harmonics = {}

  # Iterate over the range of harmonics
  (1..31).each do |harmonic|
    # Calculate the difference based on the harmonic and default orb value
    difference = (360.0 / harmonic - (degree1 - degree2).abs).abs

    # Check if the difference is within the orb range
    if difference <= orb / harmonic
      # If the difference is within the orb range, add the harmonic to the array
      harmonics[harmonic] = difference
    end
  end

  # Return the array of harmonics
  harmonics
end

def calc_all_aspects(planets, orb = 12)
  return if planets.size == 0

  p1 = planets.pop
  planets.each do |p2|
    puts "aspects between #{p1.name} and #{p2.name}: #{calculate_aspects(p1.lon, p2.lon, orb)}"
  end
  calc_all_aspects(planets)
end

chart = [sun, moon, mercury, venus, mars, jupiter, saturn, uranus, neptune, pluto].reverse
calc_all_aspects(chart, 24)

def draw_chart(planets, rotate = 90)
  svg = Victor::SVG.new
  dim = 800.0
  svg.setup width: dim, height: dim

  svg.build do
    # Set the background color to white:
    rect x: 0, y: 0, width: dim, height: dim, fill: 'white'

    # Add the positions of the planets and asteroids to the chart:

    # Set the radius of the chart in pixels:
    radius = 300
    circle cx: dim / 2, cy: dim / 2, r: radius + 10, fill: 'white', style: { stroke: 'black', stroke_width: 1 }

    # Calculate the x and y coordinates for each planet and asteroid based on its position and the radius of the chart:
    planets.each do |planet|
      deg = planet.lon + rotate
      y = radius * Math.cos(deg * Math::PI / 180) + dim / 2
      x = radius * Math.sin(deg * Math::PI / 180) + dim / 2
      text planet.symbol, x: x, y: y, fill: 'black'
      text planet.name.capitalize, x: x + 10, y: y + 10, fill: 'gray`'
    end
  end

  # Save the SVG to a file:
  svg.save('astro-chart.svg')

  # png_data = svg.to_png_data
  # File.write("astro-chart.png", png_data)
end

draw_chart(chart)
