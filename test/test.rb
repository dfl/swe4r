require 'test/unit'
require 'swe4r'

class Swe4rTest < Test::Unit::TestCase

  def test_swe_set_ephe_path
    assert_equal(nil, Swe4r::swe_set_ephe_path('path'))
  end

  def test_swe_julday
    assert_equal(2444838.972916667, Swe4r::swe_julday(1981, 8, 22, 11.35))
  end

  def test_swe_revjul
    assert_equal( [1981, 8, 22, 11.350000005215406], Swe4r::swe_revjul(2444838.972916667) )
  end

  def test_swe_set_topo
    assert_equal(nil, Swe4r::swe_set_topo(-112.183333, 45.45, 1524))
  end

  def test_swe_set_sid_mode
    assert_equal(nil, Swe4r::swe_set_sid_mode(Swe4r::SE_SIDM_LAHIRI, 0, 0)) # Use Lahiri mode
    assert_equal(nil, Swe4r::swe_set_sid_mode(Swe4r::SE_SIDM_USER, 2415020.5, 22.460489112721632)) # Use user defined mode
  end

  def test_swe_get_ayanamsa_ut

    # Test using default sidereal mode
    Swe4r::swe_set_sid_mode(Swe4r::SE_SIDM_FAGAN_BRADLEY, 0, 0)
    assert_equal(24.483840294903757, Swe4r::swe_get_ayanamsa_ut(2444838.972916667))

    # Test using Lahari sidereal mode
    Swe4r::swe_set_sid_mode(Swe4r::SE_SIDM_LAHIRI, 0, 0)
    assert_equal(23.600632656944242, Swe4r::swe_get_ayanamsa_ut(2444838.972916667))

    # Test using user defined sidereal mode
    Swe4r::swe_set_sid_mode(Swe4r::SE_SIDM_USER, 2415020.5, 22.460489112721632)
    assert_equal(23.600591306635067, Swe4r::swe_get_ayanamsa_ut(2444838.972916667))

  end

  def test_swe_get_ayanamsa_ex_ut
    Swe4r::swe_set_sid_mode(Swe4r::SE_SIDM_LAHIRI, 0, 0)
    assert_equal(23.596675071493447, Swe4r::swe_get_ayanamsa_ex_ut(2444838.972916667, Swe4r::SEFLG_MOSEPH))
  end

  def test_swe_calc_ut

    # The Moshier Ephemeris will be used for all tests since it does not require ephemeris files

    # Test #1...
    # Body: Sun
    # Flags: Moshier Ephemeris
    body = Swe4r::swe_calc_ut(2444838.972916667, Swe4r::SE_SUN, Swe4r::SEFLG_MOSEPH)
    assert_equal(149.26566155068903, body[0])
    assert_equal(-0.00012095608857072326, body[1])
    assert_equal(1.0112944920684557, body[2])
    assert_equal(0.0, body[3])
    assert_equal(0.0, body[4])
    assert_equal(0.0, body[5])

    # Test #2...
    # Body: Sun
    # Flags: Moshier Ephemeris, High Precision Speed
    body = Swe4r::swe_calc_ut(2444838.972916667, Swe4r::SE_SUN, Swe4r::SEFLG_MOSEPH|Swe4r::SEFLG_SPEED)
    assert_equal(149.26566155068903, body[0])
    assert_equal(-0.00012095608857040173, body[1])
    assert_equal(1.0112944920684557, body[2])
    assert_equal(0.9636052198327643, body[3])
    assert_equal(1.3573116429482971e-05, body[4])
    assert_equal(-0.0002028500180286159, body[5])

     # Test #3...
     # Body: Sun
     # Flags: Moshier Ephemeris, High Precision Speed, True Positions
     body = Swe4r::swe_calc_ut(2444838.972916667, Swe4r::SE_SUN, Swe4r::SEFLG_MOSEPH|Swe4r::SEFLG_TRUEPOS|Swe4r::SEFLG_SPEED)
     assert_equal(149.27128949344433, body[0])
     assert_equal(-0.000120860302637052, body[1])
     assert_equal(1.0112944920684555, body[2])
     assert_equal(0.9636063535153125, body[3])
     assert_equal(1.357381533531308e-05, body[4])
     assert_equal(-0.00020285001841389172, body[5])

     # Test #4...
     # Body: Sun
     # Flags: Moshier Ephemeris, High Precision Speed, True Positions, Topocentric
     Swe4r::swe_set_topo(-112.183333, 45.45, 1524)
     body = Swe4r::swe_calc_ut(2444838.972916667, Swe4r::SE_SUN, Swe4r::SEFLG_MOSEPH|Swe4r::SEFLG_TRUEPOS|Swe4r::SEFLG_SPEED|Swe4r::SEFLG_TOPOCTR)
     assert_equal(149.2732799495714, body[0])
     assert_equal(-0.001367782322315945, body[1])
     assert_equal(1.0113041763512773, body[2])
     assert_equal(0.9683657901648964, body[3])
     assert_equal(0.003744025117672333, body[4])
     assert_equal(-0.0003579187547015105, body[5])

     # Test #5...
     # Body: Sun
     # Flags: Moshier Ephemeris, High Precision Speed, True Positions, Topocentric, Sidereal (Lahiri Mode)
     Swe4r::swe_set_topo(-112.183333, 45.45, 1524)
     Swe4r::swe_set_sid_mode(1, 0, 0)
     body = Swe4r::swe_calc_ut(2444838.972916667, Swe4r::SE_SUN, Swe4r::SEFLG_MOSEPH|Swe4r::SEFLG_TRUEPOS|Swe4r::SEFLG_SPEED|Swe4r::SEFLG_TOPOCTR|Swe4r::SEFLG_SIDEREAL)
     assert_equal(125.67660487807792, body[0])
     assert_equal(-0.0013677823223154056, body[1])
     assert_equal(1.0113041763512773, body[2])
     assert_equal(0.9683251020932834, body[3])
     assert_equal(0.0037468508348008473, body[4])
     assert_equal(-0.0003579187537684393, body[5])

  end

  def test_swe_houses

    # Test each house system
    systems = ['P','K','O','R','C','A','E','V','X','H','T','B'] # 'G'
    systems.each do |s|
      Swe4r::swe_houses(2444838.972916667, 45.45, -112.183333, s)
    end

    # Test using Placidus house system
    assert_equal(
      [133.95429950260865,
        30.87358004482253,
        28.74575330914001,
        273.24041521079045,
        116.7140528342306,
        92.42567887272338,
        133.52799512898898,
        272.4256788727234,
        0.0,
        0.0],
     [133.95429950260865,
      30.87358004482253,
      28.74575330914001,
      273.24041521079045,
      116.7140528342306,
      92.42567887272338,
      133.52799512898898,
      272.4256788727234,
      0.0,
      0.0],
    Swe4r::swe_houses(2444838.972916667, 45.45, -112.183333, 'P'))
  end

  def test_swe_rise_trans
    sunrise = Swe4r::swe_rise_trans(2444838.972916667, Swe4r::SE_SUN, Swe4r::SEFLG_MOSEPH, Swe4r::SE_CALC_RISE | Swe4r::SE_BIT_HINDU_RISING, 45.45, -112.183333, 0, 0, 0)
    assert_equal 2444839.210048978, sunrise
  end

  def test_swe_rise_trans_true_hor
    sunrise = Swe4r::swe_rise_trans_true_hor(2444838.972916667, Swe4r::SE_SUN, Swe4r::SEFLG_MOSEPH, Swe4r::SE_CALC_RISE | Swe4r::SE_BIT_HINDU_RISING, 45.45, -112.183333, 0, 0, 0, 1)
    assert_equal 2444839.2188771414, sunrise
  end

  def test_swe_azalt
    lat =   61.2163129
    lon = -149.894852
    longitude, latitude, distance = 149.271, -0.00012, 1.0113

    # longitude, latitude, distance
    azimuth, altitude, app_altitude = Swe4r::swe_azalt(2444838.972916667, Swe4r::SE_ECL2HOR, lon, lat, 0,0,0, longitude, latitude, distance)
    assert_equal 199.96260368933793, azimuth
    assert_equal( -15.418741801321753, altitude )
    assert_equal( -15.418741801321753, app_altitude )
  end

  def test_swe_cotrans
    a,b,c = Swe4r::swe_cotrans( 90, 99, -8, 1)
    assert_equal 221.9365465392914, a
    assert_equal( -77.98034646731611, b )
    assert_equal 1.0, c
  end

  def test_swe_sidtime
    s = Swe4r::swe_sidtime(2444838.972916667)
    assert_equal 9.395272420609334, s
  end

end
