# frozen_string_literal: true

gem 'minitest'
# require 'minitest'
require 'minitest/autorun'
# require 'minitest/reporters'
# require 'minitest/rg'

$LOAD_PATH.unshift File.expand_path('../lib', __dir__)
require 'swe4r'

class Swe4rTest < Minitest::Test
  DELTA = 1e-6 # Tolerance for floating point comparisons

  def setup
    # Common setup that can be reused across tests
    @test_date_jd = 2_444_838.972916667 # Aug 22 1981, 11:35
    @test_lat = 45.45
    @test_lon = -112.183333
    @test_altitude = 1524
  end

  def assert_float_equal(expected, actual, message = nil)
    # Helper method to compare floats with rounding
    assert_in_delta(expected.round(6), actual.round(6), DELTA, message)
  end

  def assert_float_array_equal(expected, actual, message = nil)
    # Helper method to compare arrays of floats
    assert_equal(expected.length, actual.length, "#{message}: Arrays have different lengths")
    expected.zip(actual).each_with_index do |(exp, act), idx|
      assert_float_equal(exp, act, "#{message}: Mismatch at index #{idx}")
    end
  end

  def test_swe_set_ephe_path
    assert_nil Swe4r.swe_set_ephe_path('path')
  end

  def test_swe_julday
    assert_float_equal(2_444_838.972917, Swe4r.swe_julday(1981, 8, 22, 11.35))
  end

  def test_swe_revjul
    expected = [1981, 8, 22, 11.350000]
    result = Swe4r.swe_revjul(@test_date_jd)
    # Only round the time component since others are integers
    result[3] = result[3].round(6)
    assert_equal(expected, result)
  end

  def test_swe_set_topo
    assert_nil Swe4r.swe_set_topo(@test_lon, @test_lat, @test_altitude)
  end

  def test_swe_set_sid_mode
    assert_nil Swe4r.swe_set_sid_mode(Swe4r::SE_SIDM_LAHIRI, 0, 0)
    assert_nil Swe4r.swe_set_sid_mode(Swe4r::SE_SIDM_USER, 2_415_020.5, 22.460489112721632)
  end

  def test_swe_get_ayanamsa_ut
    # Test cases with different sidereal modes
    test_cases = [
      {
        mode: Swe4r::SE_SIDM_FAGAN_BRADLEY,
        expected: 24.483840
      },
      {
        mode: Swe4r::SE_SIDM_LAHIRI,
        expected: 23.600633
      },
      {
        mode: Swe4r::SE_SIDM_USER,
        t0: 2_415_020.5,
        ayan_t0: 22.460489112721632,
        expected: 23.600591
      }
    ]

    test_cases.each do |tc|
      if tc[:mode] == Swe4r::SE_SIDM_USER
        Swe4r.swe_set_sid_mode(tc[:mode], tc[:t0], tc[:ayan_t0])
      else
        Swe4r.swe_set_sid_mode(tc[:mode], 0, 0)
      end

      result = Swe4r.swe_get_ayanamsa_ut(@test_date_jd)
      assert_float_equal(tc[:expected], result,
                         "Ayanamsa calculation failed for mode #{tc[:mode]}")
    end
  end

  def test_swe_get_ayanamsa_ex_ut
    Swe4r.swe_set_sid_mode(Swe4r::SE_SIDM_LAHIRI, 0, 0)
    assert_float_equal(23.596675,
                       Swe4r.swe_get_ayanamsa_ex_ut(@test_date_jd, Swe4r::SEFLG_MOSEPH))
  end

  def test_swe_calc_ut
    # Test #1: Basic solar position
    body = Swe4r.swe_calc_ut(@test_date_jd, Swe4r::SE_SUN, Swe4r::SEFLG_MOSEPH)
    expected = [149.265662, -0.000121, 1.011294, 0.0, 0.0, 0.0]
    assert_float_array_equal(expected, body, 'Basic solar position failed')

    # Test #2: With high precision speed
    body = Swe4r.swe_calc_ut(@test_date_jd, Swe4r::SE_SUN,
                             Swe4r::SEFLG_MOSEPH | Swe4r::SEFLG_SPEED)
    expected = [149.265662, -0.000121, 1.011294, 0.963605, 0.000014, -0.000203]
    assert_float_array_equal(expected, body, 'Solar position with speed failed')

    # Test #3: With true positions
    body = Swe4r.swe_calc_ut(@test_date_jd, Swe4r::SE_SUN,
                             Swe4r::SEFLG_MOSEPH | Swe4r::SEFLG_TRUEPOS | Swe4r::SEFLG_SPEED)
    expected = [149.271289, -0.000121, 1.011294, 0.963606, 0.000014, -0.000203]
    assert_float_array_equal(expected, body, 'Solar position with true position failed')

    # Test #4: With topocentric
    Swe4r.swe_set_topo(@test_lon, @test_lat, @test_altitude)
    body = Swe4r.swe_calc_ut(@test_date_jd, Swe4r::SE_SUN,
                             Swe4r::SEFLG_MOSEPH | Swe4r::SEFLG_TRUEPOS | Swe4r::SEFLG_SPEED | Swe4r::SEFLG_TOPOCTR)
    expected = [149.273280, -0.001368, 1.011304, 0.968366, 0.003744, -0.000358]
    assert_float_array_equal(expected, body, 'Solar position with topocentric failed')

    # Test #5: With sidereal
    Swe4r.swe_set_topo(@test_lon, @test_lat, @test_altitude)
    Swe4r.swe_set_sid_mode(1, 0, 0)
    body = Swe4r.swe_calc_ut(@test_date_jd, Swe4r::SE_SUN,
                             Swe4r::SEFLG_MOSEPH | Swe4r::SEFLG_TRUEPOS | Swe4r::SEFLG_SPEED | Swe4r::SEFLG_TOPOCTR | Swe4r::SEFLG_SIDEREAL)
    expected = [125.676605, -0.001368, 1.011304, 0.968325, 0.003747, -0.000358]
    assert_float_array_equal(expected, body, 'Solar position with sidereal failed')
  end

  def test_swe_houses
    # Test each house system
    systems = %w[P K O R C A E V X H T B]
    systems.each do |s|
      result = Swe4r.swe_houses(@test_date_jd, @test_lat, @test_lon, s)
      assert result, "House system #{s} calculation failed"
    end

    # Test Placidus system specifically
    expected = [
      133.954300, 30.873580, 28.745753, 273.240415, 116.714053,
      92.425679, 133.527995, 272.425679, 0.0, 0.0
    ]
    result = Swe4r.swe_houses(@test_date_jd, @test_lat, @test_lon, 'P')
    assert_float_array_equal(expected, result[1], 'Placidus house calculation failed')
  end

  def test_swe_rise_trans
    expected = 2_444_839.210049
    result = Swe4r.swe_rise_trans(
      @test_date_jd,
      Swe4r::SE_SUN,
      Swe4r::SEFLG_MOSEPH,
      Swe4r::SE_CALC_RISE | Swe4r::SE_BIT_HINDU_RISING,
      @test_lat,
      @test_lon,
      0, 0, 0
    )
    assert_float_equal(expected, result)
  end

  def test_swe_rise_trans_true_hor
    expected = 2_444_839.218877
    result = Swe4r.swe_rise_trans_true_hor(
      @test_date_jd,
      Swe4r::SE_SUN,
      Swe4r::SEFLG_MOSEPH,
      Swe4r::SE_CALC_RISE | Swe4r::SE_BIT_HINDU_RISING,
      @test_lat,
      @test_lon,
      0, 0, 0, 1
    )
    assert_float_equal(expected, result)
  end

  def test_swe_azalt
    lat = 61.2163129
    lon = -149.894852
    longitude = 149.271
    latitude = -0.00012
    distance = 1.0113

    azimuth, altitude, app_altitude = Swe4r.swe_azalt(
      @test_date_jd,
      Swe4r::SE_ECL2HOR,
      lon,
      lat,
      0, 0, 0,
      longitude,
      latitude,
      distance
    )

    assert_float_equal(199.962604, azimuth)
    assert_float_equal(-15.418742, altitude)
    assert_float_equal(-15.418742, app_altitude)
  end

  def test_swe_cotrans
    expected = [221.936547, -77.980346, 1.0]
    result = Swe4r.swe_cotrans(90, 99, -8, 1)
    assert_float_array_equal(expected, result)
  end

  def test_swe_sidtime
    assert_float_equal(9.395272, Swe4r.swe_sidtime(@test_date_jd))
  end

  def test_swe_nod_aps_ut
    nasc, ndsc, xperi, xaphe = Swe4r.swe_nod_aps_ut(@test_date_jd, Swe4r::SE_MERCURY, Swe4r::SEFLG_HELCTR,
                                                    Swe4r::SE_NODBIT_MEAN)
    assert_equal nasc.first.round(4), 48.1092
    assert_equal ndsc.first.round(4), 228.1092
    assert_equal xperi.first.round(4), 76.9845
    assert_equal xaphe.first.round(4), 256.9845
  end

  def test_swe_house_name
    assert_equal 'Placidus', Swe4r.swe_house_name('P')
    assert_equal 'Sunshine', Swe4r.swe_house_name('I')
    assert_equal 'Koch', Swe4r.swe_house_name('K')
  end
end
