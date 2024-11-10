/*
Swe4r :: Swiss Ephemeris for Ruby - A C extension for the Swiss Ephemeris library (http://www.astro.com/swisseph/)
Copyright (C) 2012 Andrew Kirk (andrew.kirk@windhorsemedia.com)
Additional work (C) 2024 David Lowenfels (dfl@alum.mit.edu)

This file is part of Swe4r.

Swe4r is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Swe4r is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Swe4r.  If not, see <http://www.gnu.org/licenses/>.
*/

// https://docs.ruby-lang.org/en/3.0/extension_rdoc.html
#include <ruby.h>
#include "swephexp.h"

// Module Name
VALUE rb_mSwe4r = Qnil;

/*
 * Set directory path of ephemeris files
 * http://www.astro.com/swisseph/swephprg.htm#_Toc283735481
 * int swe_set_ephe_path(char *path);
 */
static VALUE t_swe_set_ephe_path(VALUE self, VALUE path)
{
	swe_set_ephe_path(StringValuePtr(path));
	return Qnil;
}

/*
 * Set directory path of ephemeris files to work with jpl
 * http://www.astro.com/swisseph/swephprg.htm#_Toc283735481
 * int swe_set_jpl_file(char *fname);
 */
static VALUE t_swe_set_jpl_file(VALUE self, VALUE path)
{
	swe_set_jpl_file(StringValuePtr(path));
	return Qnil;
}

/*
 * Get the Julian day number from year, month, day, hour
 * http://www.astro.com/swisseph/swephprg.htm#_Toc283735468
	double swe_julday(
		int year,
		int month,
		int day,
		double hour,
		int gregflag	// Gregorian calendar: 1, Julian calendar: 0
	);
 */
static VALUE t_swe_julday(int argc, VALUE *argv, VALUE self)
{
	VALUE greg_flag;

	if (argc > 5 || argc < 4)
	{ // there should only be 4 or 5 arguments
		rb_raise(rb_eArgError, "wrong number of arguments");
	}
	greg_flag = (argc == 5) ? NUM2INT(argv[4]) : SE_GREG_CAL;

	double julday = swe_julday(NUM2INT(argv[0]), NUM2INT(argv[1]), NUM2INT(argv[2]), NUM2DBL(argv[3]), greg_flag);
	return rb_float_new(julday);
}
/*
 * Get the year, month, day, hour from Julian day
 * http://www.astro.com/swisseph/swephprg.htm#_Toc283735468
	void swe_revjul(
		double tjd,         	 // Julian day number
		int gregflag,            // Gregorian calendar: 1, Julian calendar: 0
		int *year,               // target addresses for year, etc.
		int *month,
		int *day,
		double *hour
	);
*/
static VALUE t_swe_revjul(int argc, VALUE *argv, VALUE self)
{
	if (argc > 2 || argc < 1)
	{ // there should only be 2 or 3 arguments
		rb_raise(rb_eArgError, "wrong number of arguments");
	}
	VALUE greg_flag = (argc == 2) ? NUM2INT(argv[1]) : SE_GREG_CAL;

	int year, month, day;
	double hour;

	swe_revjul(NUM2DBL(argv[0]), greg_flag, &year, &month, &day, &hour);

	VALUE output = rb_ary_new();
	rb_ary_push(output, rb_uint_new(year));
	rb_ary_push(output, rb_uint_new(month));
	rb_ary_push(output, rb_uint_new(day));
	rb_ary_push(output, rb_float_new(hour));
	return output;
}

/*
 * Set the geographic location for topocentric planet computation
 * The longitude and latitude must be in degrees, the altitude in meters.
 * http://www.astro.com/swisseph/swephprg.htm#_Toc283735476
	void swe_set_topo (
		double geolon,      // geographic longitude: eastern longitude is positive, western longitude is negative
		double geolat,      // geographic latitude: northern latitude is positive, southern latitude is negative
		double altitude		// altitude above sea
	);
*/
static VALUE t_swe_set_topo(VALUE self, VALUE lon, VALUE lat, VALUE alt)
{
	swe_set_topo(NUM2DBL(lon), NUM2DBL(lat), NUM2DBL(alt));
	return Qnil;
}

/*
 * Calculation of planets, moon, asteroids, lunar nodes, apogees, fictitious bodies
 * http://www.astro.com/swisseph/swephprg.htm#_Toc283735419
	long swe_calc_ut(
		double tjd_ut,	// Julian day number, Universal Time
		int ipl,		// planet number
		long iflag,		// flag bits
		double *xx,  	// target address for 6 position values: longitude, latitude, distance, long.speed, lat.speed, dist.speed
		char *serr		// 256 bytes for error string
	);
 */
static VALUE t_swe_calc_ut(VALUE self, VALUE julian_ut, VALUE body, VALUE iflag)
{
	double results[6];
	char serr[AS_MAXCH];

	if (swe_calc_ut(NUM2DBL(julian_ut), NUM2INT(body), NUM2LONG(iflag), results, serr) < 0)
		rb_raise(rb_eRuntimeError, serr);

	VALUE output = rb_ary_new();
	for (int i = 0; i < 6; i++)
		rb_ary_push(output, rb_float_new(results[i]));

	return output;
}

static VALUE t_swe_sidtime(VALUE self, VALUE julian_ut)
{
	double sidtime = swe_sidtime(NUM2DBL(julian_ut));
	return rb_float_new(sidtime);
}

/*
 * This function can be used to specify the mode for sidereal computations
 * http://www.astro.com/swisseph/swephprg.htm#_Toc283735478
	void swe_set_sid_mode (
		int32 sid_mode, 	// Mode
		double t0, 		// Reference date
		double ayan_t0	// Initial value of the ayanamsha
	);
 */
static VALUE t_swe_set_sid_mode(VALUE self, VALUE mode, VALUE t0, VALUE ayan_t0)
{
	swe_set_sid_mode(NUM2INT(mode), NUM2DBL(t0), NUM2DBL(ayan_t0));
	return Qnil;
}

/*
 * This function computes the ayanamsha, the distance of the tropical vernal point from the sidereal zero point of the zodiac.
 * The ayanamsha is used to compute sidereal planetary positions from tropical ones:
 * pos_sid = pos_trop – ayanamsha
 * Before calling swe_get_ayanamsha(), you have to set the sidereal mode with swe_set_sid_mode, unless you want the default sidereal mode, which is the Fagan/Bradley ayanamsha.
 * http://www.astro.com/swisseph/swephprg.htm#_Toc283735479
 * double swe_get_ayanamsa_ut(double tjd_ut);
 */
static VALUE t_swe_get_ayanamsa_ut(VALUE self, VALUE julian_ut)
{
	double ayanamsa = swe_get_ayanamsa_ut(NUM2DBL(julian_ut));
	return rb_float_new(ayanamsa);
}

// * This function computes the ayanamsha using a Delta T consistent with the ephe_flag specified.
// * https://www.astro.com/swisseph/swephprg.htm#_Toc112949018
// * input variables:
// * tjd_ut = Julian day number in UT
// * (tjd_et = Julian day number in ET/TT)
// * iflag = ephemeris flag (one of SEFLG_SWIEPH, SEFLG_JPLEPH, SEFLG_MOSEPH)
// * plus some other optional SEFLG_...
// * output values
// * daya = ayanamsha value (pointer to double)
// * serr = error message or warning (pointer to string)
// * The function returns either the ephemeris flag used or ERR (-1)

static VALUE t_swe_get_ayanamsa_ex_ut(VALUE self, VALUE julian_ut, VALUE flag)
{
	double ayanamsha;
	char serr[AS_MAXCH];

	// if(TYPE(flag) == T_NIL) { // default to Moshier Ephemeris
	// 	flag = SEFLG_MOSEPH;
	// }

	if (swe_get_ayanamsa_ex_ut(NUM2DBL(julian_ut), NUM2INT(flag), &ayanamsha, serr) < 0)
		rb_raise(rb_eRuntimeError, serr);

	return rb_float_new(ayanamsha);
}

/*
 * This function computes house cusps, ascendant, midheaven, etc
 * http://www.astro.com/swisseph/swephprg.htm#_Toc283735486
	int swe_houses(
		double tjd_ut,      // Julian day number, UT
		double geolat,      // geographic latitude, in degrees
		double geolon,      // geographic longitude, in degrees (eastern longitude is positive, western longitude is negative, northern latitude is positive, southern latitude is negative
		int hsys,           // house method, ascii code of one of the letters PKORCAEVXHTBG
		double *cusps,      // array for 13 doubles
		double *ascmc	    // array for 10 doubles
	);
 * House method codes...
 * ‘P’ 			= Placidus
 * ‘K’     		= Koch
 * ‘O’     		= Porphyrius
 * ‘R’     		= Regiomontanus
 * ‘C’     		= Campanus
 * ‘A’ or ‘E’  	= Equal (cusp 1 is Ascendant)
 * ‘V’     		= Vehlow equal (Asc. in middle of house 1)
 * ‘W’     		= Whole sign
 * ‘X’     		= axial rotation system
 * ‘H’     		= azimuthal or horizontal system
 * ‘T’     		= Polich/Page (“topocentric” system)
 * ‘B’     		= Alcabitus
 * ‘M’     		= Morinus
 * ‘U’     		= Krusinski-Pisa
 * ‘G’     		= Gauquelin sectors
 */

static VALUE t_swe_houses(VALUE self, VALUE julian_day, VALUE latitude, VALUE longitude, VALUE house_system)
{
	double cusps[13];
	double ascmc[10];
	char serr[AS_MAXCH];

	if (swe_houses(NUM2DBL(julian_day), NUM2DBL(latitude), NUM2DBL(longitude), NUM2CHR(house_system), cusps, ascmc) < 0)
		rb_raise(rb_eRuntimeError, serr);

	VALUE _cusps = rb_ary_new();
	for (int i = 0; i < 13; i++)
		rb_ary_push(_cusps, rb_float_new(cusps[i]));

	VALUE _ascmc = rb_ary_new();
	for (int i = 0; i < 10; i++)
		rb_ary_push(_ascmc, rb_float_new(ascmc[i]));

	VALUE output = rb_ary_new();
	rb_ary_push(output, _cusps);
	rb_ary_push(output, _ascmc);
	return output;
}

// This function is better than swe_houses and returns speeds as well
// https://www.astro.com/swisseph/swephprg.htm#_Toc112949026
// int swe_houses_ex2(
// double tjd_ut,      /* Julian day number, UT */
// int32 iflag,        /* 0 or SEFLG_SIDEREAL or SEFLG_RADIANS or SEFLG_NONUT */
// double geolat,      /* geographic latitude, in degrees */
// double geolon,      /* geographic longitude, in degrees
//                     * eastern longitude is positive,
//                     * western longitude is negative,
//                     * northern latitude is positive,
//                     * southern latitude is negative */
// int hsys,           /* house method, one-letter case sensitive code (list, see further below) */
// double *cusps,      /* array for 13 (or 37 for hsys G) doubles, explained further below */
// double *ascmc,      /* array for 10 doubles, explained further below */
// double *cusp_speed,  /* like cusps */
// double *ascmc_speed, /* like ascmc */
// char *serr);

static VALUE t_swe_houses_ex2(VALUE self, VALUE julian_day, VALUE flag, VALUE latitude, VALUE longitude, VALUE house_system)
{
	double cusps[13];
	double ascmc[10];
	double cusps_speed[13];
	double ascmc_speed[10];
	char serr[AS_MAXCH];

	if (swe_houses_ex2(NUM2DBL(julian_day), NUM2INT(flag), NUM2DBL(latitude), NUM2DBL(longitude), NUM2CHR(house_system), cusps, ascmc, cusps_speed, ascmc_speed, serr) < 0)
		rb_raise(rb_eRuntimeError, serr);

	VALUE _cusps = rb_ary_new();
	for (int i = 0; i < 13; i++)
		rb_ary_push(_cusps, rb_float_new(cusps[i]));

	VALUE _ascmc = rb_ary_new();
	for (int i = 0; i < 10; i++)
		rb_ary_push(_ascmc, rb_float_new(ascmc[i]));

	VALUE _cusps_speed = rb_ary_new();
	for (int i = 0; i < 13; i++)
		rb_ary_push(_cusps_speed, rb_float_new(cusps_speed[i]));

	VALUE _ascmc_speed = rb_ary_new();
	for (int i = 0; i < 10; i++)
		rb_ary_push(_ascmc_speed, rb_float_new(ascmc_speed[i]));

	VALUE output = rb_ary_new();
	rb_ary_push(output, _cusps);
	rb_ary_push(output, _ascmc);
	rb_ary_push(output, _cusps_speed);
	rb_ary_push(output, _ascmc_speed);
	return output;
}

// int32 swe_rise_trans(
// double tjd_ut,      /* search after this time (UT) */
// int32 ipl,               /* planet number, if planet or moon */
// char *starname,     /* star name, if star; must be NULL or empty, if ipl is used */
// int32 epheflag,     /* ephemeris flag */
// int32 rsmi,              /* integer specifying that rise, set, or one of the two meridian transits is wanted. see definition below */
// double *geopos,     /* array of three doubles containing
//                         * geograph. long., lat., height of observer */
// double atpress      /* atmospheric pressure in mbar/hPa */
// double attemp,      /* atmospheric temperature in deg. C */
// double *tret,            /* return address (double) for rise time etc. */
// char *serr);             /* return address for error message */

static VALUE t_swe_rise_trans(VALUE self, VALUE julian_day, VALUE body, VALUE flag, VALUE rmsi, VALUE lon, VALUE lat, VALUE height, VALUE pressure, VALUE temp)
{
	double geopos[3];
	geopos[0] = NUM2DBL(lon);
	geopos[1] = NUM2DBL(lat);
	geopos[2] = NUM2DBL(height);
	int ipl;
	char *starname;
	if (TYPE(body) == T_STRING)
	{
		starname = StringValuePtr(body);
		ipl = 0;
	}
	else
	{
		ipl = NUM2INT(body);
		starname = NULL;
	}
	char serr[AS_MAXCH];
	double retval;

	if (swe_rise_trans(NUM2DBL(julian_day), ipl, starname, NUM2INT(flag), NUM2INT(rmsi), geopos, NUM2DBL(pressure), NUM2DBL(temp), &retval, serr) < 0)
		rb_raise(rb_eRuntimeError, serr);
	return rb_float_new(retval);
}

// int32 swe_rise_trans_true_hor(
// double tjd_ut,      /* search after this time (UT) */
// int32 ipl,               /* planet number, if planet or moon */
// char *starname,     /* star name, if star; must be NULL or empty, if ipl is used */
// int32 epheflag,     /* ephemeris flag */
// int32 rsmi,              /* integer specifying that rise, set, or one of the two meridian transits is wanted. see definition below */
// double *geopos,     /* array of three doubles containing
//                         * geograph. long., lat., height of observer */
// double atpress,     /* atmospheric pressure in mbar/hPa */
// double attemp,      /* atmospheric temperature in deg. C */
// double horhgt,      /* height of local horizon in deg at the point where the body rises or sets */
// double *tret,       /* return address (double) for rise time etc. */
// char *serr);        /* return address for error message */

static VALUE t_swe_rise_trans_true_hor(VALUE self, VALUE julian_day, VALUE body, VALUE flag, VALUE rmsi, VALUE lon, VALUE lat, VALUE height, VALUE pressure, VALUE temp, VALUE hor_height)
{
	double geopos[3];
	geopos[0] = NUM2DBL(lon);
	geopos[1] = NUM2DBL(lat);
	geopos[2] = NUM2DBL(height);
	int ipl;
	char *starname;
	if (TYPE(body) == T_STRING)
	{
		starname = StringValuePtr(body);
		ipl = 0;
	}
	else
	{
		ipl = NUM2INT(body);
		starname = NULL;
	}
	char serr[AS_MAXCH];
	double retval;

	if (swe_rise_trans_true_hor(NUM2DBL(julian_day), ipl, starname, NUM2INT(flag), NUM2INT(rmsi), geopos, NUM2DBL(pressure), NUM2DBL(temp), NUM2DBL(hor_height), &retval, serr) < 0)
		rb_raise(rb_eRuntimeError, serr);
	return rb_float_new(retval);
}

// https://www.astro.com/swisseph/swephprg.htm#_Toc112948998
// swe_azalt() computes the horizontal coordinates (azimuth and altitude) of a planet or a star from either ecliptical or equatorial coordinates.
// void swe_azalt(
// double tjd_ut,      // UT
// int32 calc_flag,    // SE_ECL2HOR or SE_EQU2HOR
// double *geopos,     // array of 3 doubles: geograph. long., lat., height
// double atpress,     // atmospheric pressure in mbar (hPa)
// double attemp,      // atmospheric temperature in degrees Celsius
// double *xin,        // array of 3 doubles: position of body in either ecliptical or equatorial coordinates, depending on calc_flag
// double *xaz);       // return array of 3 doubles, containing azimuth, true altitude, apparent altitude
// If calc_flag = SE_ECL2HOR, set xin[0] = ecl. long., xin[1] = ecl. lat., (xin[2] = distance (not required));
// else
// if calc_flag = SE_EQU2HOR, set xin[0] = right ascension, xin[1] = declination, (xin[2] = distance (not required));
// #define SE_ECL2HOR  0
// #define SE_EQU2HOR  1
// The return values are:
// ·     xaz[0] = azimuth, i.e. position degree, measured from the south point to west;
// ·     xaz[1] = true altitude above horizon in degrees;
// ·     xaz[2] = apparent (refracted) altitude above horizon in degrees.
// The apparent altitude of a body depends on the atmospheric pressure and temperature. If only the true altitude is required, these parameters can be neglected.
// If atpress is given the value 0, the function estimates the pressure from the geographical altitude given in geopos[2] and attemp. If geopos[2] is 0, atpress will be estimated for sea level.
static VALUE t_swe_azalt(VALUE self, VALUE julian_day, VALUE flag, VALUE lon, VALUE lat, VALUE height, VALUE pressure, VALUE temp, VALUE in0, VALUE in1, VALUE in2)
{
	double geopos[3];
	geopos[0] = NUM2DBL(lon);
	geopos[1] = NUM2DBL(lat);
	geopos[2] = NUM2DBL(height);

	double xin[3];
	xin[0] = NUM2DBL(in0);
	xin[1] = NUM2DBL(in1);
	xin[2] = NUM2DBL(in2);
	double xaz[3];

	swe_azalt(NUM2DBL(julian_day), NUM2INT(flag), geopos, NUM2DBL(pressure), NUM2DBL(temp), xin, xaz);

	VALUE output = rb_ary_new();
	rb_ary_push(output, rb_float_new(xaz[0]));
	rb_ary_push(output, rb_float_new(xaz[1]));
	rb_ary_push(output, rb_float_new(xaz[2]));
	return output;
}
// https://www.astro.com/swisseph/swephprg.htm#_Toc112949076
/* equator -> ecliptic    : eps must be positive
 * ecliptic -> equator    : eps must be negative
 * eps, longitude and latitude are in positive degrees! */
// void swe_cotrans(
// double *xpo,        /* 3 doubles: long., lat., dist. to be converted; distance remains unchanged, can be set to 1.00 */
// double *xpn,        /* 3 doubles: long., lat., dist. Result of the conversion */
// double eps);        /* obliquity of ecliptic, in degrees. */
static VALUE t_swe_cotrans(int argc, VALUE *argv, VALUE self)
{
	// VALUE self, VALUE VALUE lon, VALUE lat, VALUE distance
	if (argc < 3 || argc > 4)
	{ // there should only be 3 or 4 arguments
		rb_raise(rb_eArgError, "wrong number of arguments");
	}
	double eps = NUM2DBL(argv[0]);
	double xpo[3];
	xpo[0] = NUM2DBL(argv[1]); // NUM2DBL(lon);
	xpo[1] = NUM2DBL(argv[2]); // NUM2DBL(lat);
	xpo[2] = NUM2DBL((argc == 4) ? argv[3] : 1.0);

	double xpn[3];

	swe_cotrans(xpo, xpn, eps);

	VALUE output = rb_ary_new();
	rb_ary_push(output, rb_float_new(xpn[0]));
	rb_ary_push(output, rb_float_new(xpn[1]));
	if (argc == 4)
		rb_ary_push(output, rb_float_new(xpn[2]));
	return output;
}

static VALUE t_swe_house_pos(VALUE self, VALUE armc, VALUE geolat, VALUE eps, VALUE hsys, VALUE eclon, VALUE eclat)
{
	// double armc,        /* ARMC */
	// double geolat,      /* geographic latitude, in degrees */
	// double eps,              /* ecliptic obliquity, in degrees */
	// int hsys,                /* house method, one of the letters PKRCAV */
	// double *xpin,       /* array of 2 doubles: ecl. longitude and latitude of the planet */
	// char *serr);             /* return area for error or warning message */

	double eclpos[2];
	eclpos[0] = NUM2DBL(eclon);
	eclpos[1] = NUM2DBL(eclat);
	char serr[AS_MAXCH];

	double retval = swe_house_pos(NUM2DBL(armc), NUM2DBL(geolat), NUM2DBL(eps), NUM2INT(hsys), eclpos, serr);
	if (retval < 0)
		rb_raise(rb_eRuntimeError, serr);

	VALUE output = rb_float_new(retval);
	return output;
}

/*
 find the crossing of the Sun over a given ecliptic position:

double swe_solcross(
	double x2cross,
	double tjd_tt,
	int32 iflag,
	char *serr);

double swe_solcross_ut(
	double x2cross,
	double tjd_ut,
	int32 iflag,
	char *serr);
*/
// static VALUE t_swe_solcross( VALUE self, VALUE x2cross, VALUE tjd_tt, VALUE iflag)
// {
// 	char serr[AS_MAXCH];

// 	double retval = swe_solcross(NUM2DBL(x2cross), NUM2DBL(tjd_tt), NUM2INT(iflag), serr);
// 	if (retval < tjd_tt)
// 		rb_raise(rb_eRuntimeError, serr);
// 	return rb_float_new(retval);
// }

static VALUE t_swe_solcross_ut(VALUE self, VALUE x2cross, VALUE tjd_ut, VALUE iflag)
{
	char serr[AS_MAXCH];

	double retval = swe_solcross_ut(NUM2DBL(x2cross), NUM2DBL(tjd_ut), NUM2INT(iflag), serr);
	if (retval < tjd_ut)
		rb_raise(rb_eRuntimeError, serr);
	return rb_float_new(retval);
}

/*
find the crossing of the Moon over a given ecliptic position:
double swe_mooncross(
	double x2cross,
	double tjd_et,
	int32 iflag,
	char *serr);

double swe_mooncross_ut(
	double x2cross,
	double tjd_ut,
	int32 iflag,
	char *serr);
*/
// static VALUE t_swe_mooncross( VALUE self, VALUE x2cross, VALUE tjd_tt, VALUE iflag)
// {
// 	char serr[AS_MAXCH];

// 	double retval = swe_mooncross(NUM2DBL(x2cross), NUM2DBL(tjd_tt), NUM2INT(iflag), serr);
// 	if (retval < tjd_tt)
// 		rb_raise(rb_eRuntimeError, serr);
// 	return rb_float_new(retval);
// }

static VALUE t_swe_mooncross_ut(VALUE self, VALUE x2cross, VALUE tjd_ut, VALUE iflag)
{
	char serr[AS_MAXCH];

	double retval = swe_mooncross_ut(NUM2DBL(x2cross), NUM2DBL(tjd_ut), NUM2INT(iflag), serr);
	if (retval < tjd_ut)
		rb_raise(rb_eRuntimeError, serr);
	return rb_float_new(retval);
}

/*
double swe_mooncross_node_ut(
	double tjd_ut,
	int32 iflag,
	double *xlon,
	double *xlat,
	char *serr);
*/
static VALUE t_swe_mooncross_node_ut(VALUE self, VALUE tjd_ut, VALUE iflag)
{
	char serr[AS_MAXCH];
	double xlon, xlat;
	double retval = swe_mooncross_node_ut(NUM2DBL(tjd_ut), NUM2INT(iflag), &xlon, &xlat, serr);
	if (retval < tjd_ut)
		rb_raise(rb_eRuntimeError, serr);
	VALUE output = rb_ary_new();
	rb_ary_push(output, retval);
	rb_ary_push(output, xlon);
	rb_ary_push(output, xlat);

	return output;
}
/*
int32 swe_helio_cross(
	int32 ipl,
	double x2cross,
	double tjd_ut,
	int32 iflag,
	int32 dir,
	double *jx,
	char *serr);

int32 swe_helio_cross_ut(
	int32 ipl,
	double x2cross,
	double tjd_ut,
	int32 iflag,
	int32 dir,
	double *jx,
	char *serr);
*/
// static VALUE t_swe_helio_cross( VALUE self, VALUE body, VALUE x2cross, VALUE tjd_tt, VALUE iflag, VALUE dir)
// {
// 	char serr[AS_MAXCH];
// 	double jx;
// 	double retval = swe_helio_cross_ut(NUM2INT(ipl), NUM2DBL(x2cross), NUM2DBL(tjd_tt), NUM2INT(iflag), NUM2INT(dir), &jx, serr);
// 	if (retval < tjd_tt)
// 		rb_raise(rb_eRuntimeError, serr);
// 	VALUE output = rb_ary_new();
// 	rb_ary_push(output, retval);
// 	rb_ary_push(output, jx);

// 	return output;
// }
static VALUE t_swe_helio_cross_ut(VALUE self, VALUE body, VALUE x2cross, VALUE tjd_ut, VALUE iflag, VALUE dir)
{
	char serr[AS_MAXCH];
	double jx;
	double retval = swe_helio_cross_ut(NUM2INT(body), NUM2DBL(x2cross), NUM2DBL(tjd_ut), NUM2INT(iflag), NUM2INT(dir), &jx, serr);
	if (retval < tjd_ut)
		rb_raise(rb_eRuntimeError, serr);
	VALUE output = rb_ary_new();
	rb_ary_push(output, retval);
	rb_ary_push(output, jx);

	return output;
}

/*
int32 swe_nod_aps_ut(
	double tjd_ut, // Julian day number in UT
	int32 ipl,     // planet number
	int32 iflag,   // flag bits
	int32 method,  // method, see explanations below
	double *xnasc, // array of 6 double for ascending node
	double *xndsc, // array of 6 double for descending node
	double *xperi, // array of 6 double for perihelion
	double *xaphe, // array of 6 double for aphelion
	char *serr);   // character string to contain error messages, 256 chars

int32 swe_nod_aps(
	double tjd_et, // Julian day number in TT
	int32 ipl,
	int32 iflag,
	int32 method,
	double *xnasc,
	double *xndsc,
	double *xperi,
	double *xaphe,
	char *serr);
*/
static VALUE t_swe_nod_aps_ut(VALUE self, VALUE julian_ut, VALUE body, VALUE iflag, VALUE method)
{
	char serr[AS_MAXCH];
	double xnasc[6];
	double xndsc[6];
	double xperi[6];
	double xaphe[6];

	if (swe_nod_aps(NUM2DBL(julian_ut), NUM2INT(body), NUM2INT(iflag), NUM2INT(method), xnasc, xndsc, xperi, xaphe, serr) < 0)
		rb_raise(rb_eRuntimeError, serr);

	VALUE output = rb_ary_new();

	VALUE ascending = rb_ary_new();
	for (int i = 0; i < 6; i++)
		rb_ary_push(ascending, rb_float_new(xnasc[i]));
	rb_ary_push(output, ascending);

	VALUE descending = rb_ary_new();
	for (int i = 0; i < 6; i++)
		rb_ary_push(descending, rb_float_new(xndsc[i]));
	rb_ary_push(output, descending);

	VALUE perihelion = rb_ary_new();
	for (int i = 0; i < 6; i++)
		rb_ary_push(output, rb_float_new(xperi[i]));
	rb_ary_push(output, perihelion);

	VALUE aphelion = rb_ary_new();
	for (int i = 0; i < 6; i++)
		rb_ary_push(output, rb_float_new(xaphe[i]));
	rb_ary_push(output, aphelion);

	return output; // return array of arrays
}

// static VALUE t_swe_nod_aps( VALUE self, VALUE julian_tt, VALUE body, VALUE iflag, VALUE method)
// {
// 	char serr[AS_MAXCH];
// 	double xnasc[6];
// 	double xndsc[6];
// 	double xperi[6];
// 	double xaphe[6];

// 	if (swe_nod_aps(NUM2DBL(julian_tt), NUM2INT(body), NUM2INT(iflag), xnasc, xndsc, xperi, xaphe, serr) < 0)
// 		rb_raise(rb_eRuntimeError, serr);

// 	VALUE output = rb_ary_new();

// 	VALUE ascending = rb_ary_new();
// 	for (int i = 0; i < 6; i++)
// 		rb_ary_push(ascending, rb_float_new(xnasc[i]));
// 	rb_ary_push(output, ascending);

// 	VALUE descending = rb_ary_new();
// 	for (int i = 0; i < 6; i++)
// 		rb_ary_push(descending, rb_float_new(xndsc[i]));
// 	rb_ary_push(output, descending);

// 	VALUE perihelion = rb_ary_new();
// 	for (int i = 0; i < 6; i++)
// 		rb_ary_push(output, rb_float_new(xperi[i]));
// 	rb_ary_push(output, perihelion);

// 	VALUE aphelion = rb_ary_new();
// 	for (int i = 0; i < 6; i++)
// 		rb_ary_push(output, rb_float_new(xaphe[i]));
// 	rb_ary_push(output, aphelion);

// 	return output; // return array of arrays
// }

/* swe_calc_pctr() calculates planetocentric positions of planets, i. e. positions as observed from some different planet, e.g. Jupiter-centric ephemerides. The function can actually calculate any object as observed from any other object, e.g. also the position of some asteroid as observed from another asteroid or from a planetary moon. The function declaration is as follows:
int32 swe_calc_pctr(
	double tjd,    // input time in TT
	int32 ipl,     // target object
	int32 iplctr,  // center object
	int32 iflag,
	double *xxret,
	char *serr);
*/
// tjd_et   = Julian day, Ephemeris time, where tjd_et = tjd_ut + swe_deltat(tjd_ut)
static VALUE t_swe_calc_pctr(VALUE self, VALUE julian_et, VALUE body, VALUE center, VALUE iflag)
{
	char serr[AS_MAXCH];
	double xxret[6];

	if (swe_calc_pctr(NUM2DBL(julian_et), NUM2INT(body), NUM2INT(center), NUM2INT(iflag), xxret, serr) < 0)
		rb_raise(rb_eRuntimeError, serr);

	VALUE output = rb_ary_new();
	for (int i = 0; i < 6; i++)
		rb_ary_push(output, rb_float_new(xxret[i]));

	return output;
}

void Init_swe4r()
{
	// Module
	rb_mSwe4r = rb_define_module("Swe4r");

	// Module Functions
	rb_define_module_function(rb_mSwe4r, "swe_set_ephe_path", t_swe_set_ephe_path, 1);
	rb_define_module_function(rb_mSwe4r, "swe_set_jpl_file", t_swe_set_jpl_file, 1);
	rb_define_module_function(rb_mSwe4r, "swe_julday", t_swe_julday, -1);
	rb_define_module_function(rb_mSwe4r, "swe_revjul", t_swe_revjul, -1);
	rb_define_module_function(rb_mSwe4r, "swe_set_topo", t_swe_set_topo, 3);
	rb_define_module_function(rb_mSwe4r, "swe_calc_ut", t_swe_calc_ut, 3);
	rb_define_module_function(rb_mSwe4r, "swe_sidtime", t_swe_sidtime, 1);
	rb_define_module_function(rb_mSwe4r, "swe_set_sid_mode", t_swe_set_sid_mode, 3);
	rb_define_module_function(rb_mSwe4r, "swe_get_ayanamsa_ut", t_swe_get_ayanamsa_ut, 1);
	rb_define_module_function(rb_mSwe4r, "swe_houses", t_swe_houses, 4);
	rb_define_module_function(rb_mSwe4r, "swe_houses_ex2", t_swe_houses_ex2, 5);
	rb_define_module_function(rb_mSwe4r, "swe_get_ayanamsa_ex_ut", t_swe_get_ayanamsa_ex_ut, 2);
	rb_define_module_function(rb_mSwe4r, "swe_rise_trans", t_swe_rise_trans, 9);
	rb_define_module_function(rb_mSwe4r, "swe_rise_trans_true_hor", t_swe_rise_trans_true_hor, 10);
	rb_define_module_function(rb_mSwe4r, "swe_azalt", t_swe_azalt, 10);
	rb_define_module_function(rb_mSwe4r, "swe_cotrans", t_swe_cotrans, -1);
	rb_define_module_function(rb_mSwe4r, "swe_house_pos", t_swe_house_pos, 6);
	// rb_define_module_function(rb_mSwe4r, "swe_solcross", t_swe_solcross, 3);
	rb_define_module_function(rb_mSwe4r, "swe_solcross_ut", t_swe_solcross_ut, 3);
	// rb_define_module_function(rb_mSwe4r, "swe_mooncross", t_swe_mooncross, 3);
	rb_define_module_function(rb_mSwe4r, "swe_mooncross_ut", t_swe_mooncross_ut, 3);
	rb_define_module_function(rb_mSwe4r, "swe_mooncross_node_ut", t_swe_mooncross_node_ut, 2);
	// rb_define_module_function(rb_mSwe4r, "swe_helio_cross", t_swe_helio_cross, 4);
	rb_define_module_function(rb_mSwe4r, "swe_helio_cross_ut", t_swe_helio_cross_ut, 5);
	rb_define_module_function(rb_mSwe4r, "swe_nod_aps_ut", t_swe_nod_aps_ut, 4);
	// rb_define_module_function(rb_mSwe4r, "swe_nod_aps", t_swe_nod_aps, 4);
	rb_define_module_function(rb_mSwe4r, "swe_calc_pctr", t_swe_calc_pctr, 4);

	// Constants

	rb_define_const(rb_mSwe4r, "SE_SUN", INT2FIX(SE_SUN));
	rb_define_const(rb_mSwe4r, "SE_MOON", INT2FIX(SE_MOON));
	rb_define_const(rb_mSwe4r, "SE_MERCURY", INT2FIX(SE_MERCURY));
	rb_define_const(rb_mSwe4r, "SE_VENUS", INT2FIX(SE_VENUS));
	rb_define_const(rb_mSwe4r, "SE_MARS", INT2FIX(SE_MARS));
	rb_define_const(rb_mSwe4r, "SE_JUPITER", INT2FIX(SE_JUPITER));
	rb_define_const(rb_mSwe4r, "SE_SATURN", INT2FIX(SE_SATURN));
	rb_define_const(rb_mSwe4r, "SE_URANUS", INT2FIX(SE_URANUS));
	rb_define_const(rb_mSwe4r, "SE_NEPTUNE", INT2FIX(SE_NEPTUNE));
	rb_define_const(rb_mSwe4r, "SE_PLUTO", INT2FIX(SE_PLUTO));
	rb_define_const(rb_mSwe4r, "SE_MEAN_NODE", INT2FIX(SE_MEAN_NODE));
	rb_define_const(rb_mSwe4r, "SE_TRUE_NODE", INT2FIX(SE_TRUE_NODE));
	rb_define_const(rb_mSwe4r, "SE_MEAN_APOG", INT2FIX(SE_MEAN_APOG));
	rb_define_const(rb_mSwe4r, "SE_OSCU_APOG", INT2FIX(SE_OSCU_APOG));
	rb_define_const(rb_mSwe4r, "SE_EARTH", INT2FIX(SE_EARTH));
	rb_define_const(rb_mSwe4r, "SE_CHIRON", INT2FIX(SE_CHIRON));
	rb_define_const(rb_mSwe4r, "SE_PHOLUS", INT2FIX(SE_PHOLUS));
	rb_define_const(rb_mSwe4r, "SE_CERES", INT2FIX(SE_CERES));
	rb_define_const(rb_mSwe4r, "SE_PALLAS", INT2FIX(SE_PALLAS));
	rb_define_const(rb_mSwe4r, "SE_JUNO", INT2FIX(SE_JUNO));
	rb_define_const(rb_mSwe4r, "SE_VESTA", INT2FIX(SE_VESTA));
	rb_define_const(rb_mSwe4r, "SE_CUPIDO", INT2FIX(SE_CUPIDO));
	rb_define_const(rb_mSwe4r, "SE_HADES", INT2FIX(SE_HADES));
	rb_define_const(rb_mSwe4r, "SE_ZEUS", INT2FIX(SE_ZEUS));
	rb_define_const(rb_mSwe4r, "SE_KRONOS", INT2FIX(SE_KRONOS));
	rb_define_const(rb_mSwe4r, "SE_APOLLON", INT2FIX(SE_APOLLON));
	rb_define_const(rb_mSwe4r, "SE_ADMETOS", INT2FIX(SE_ADMETOS));
	rb_define_const(rb_mSwe4r, "SE_VULKANUS", INT2FIX(SE_VULKANUS));
	rb_define_const(rb_mSwe4r, "SE_POSEIDON", INT2FIX(SE_POSEIDON));

	rb_define_const(rb_mSwe4r, "SE_INTP_APOG", INT2FIX(SE_INTP_APOG));
	rb_define_const(rb_mSwe4r, "SE_INTP_PERG", INT2FIX(SE_INTP_PERG));

	rb_define_const(rb_mSwe4r, "SEFLG_JPLEPH", INT2FIX(SEFLG_JPLEPH));
	rb_define_const(rb_mSwe4r, "SEFLG_SWIEPH", INT2FIX(SEFLG_SWIEPH));
	rb_define_const(rb_mSwe4r, "SEFLG_MOSEPH", INT2FIX(SEFLG_MOSEPH));
	rb_define_const(rb_mSwe4r, "SEFLG_HELCTR", INT2FIX(SEFLG_HELCTR));
	rb_define_const(rb_mSwe4r, "SEFLG_TRUEPOS", INT2FIX(SEFLG_TRUEPOS));
	rb_define_const(rb_mSwe4r, "SEFLG_J2000", INT2FIX(SEFLG_J2000));
	rb_define_const(rb_mSwe4r, "SEFLG_NONUT", INT2FIX(SEFLG_NONUT));
	rb_define_const(rb_mSwe4r, "SEFLG_SPEED3", INT2FIX(SEFLG_SPEED3));
	rb_define_const(rb_mSwe4r, "SEFLG_SPEED", INT2FIX(SEFLG_SPEED));
	rb_define_const(rb_mSwe4r, "SEFLG_NOGDEFL", INT2FIX(SEFLG_NOGDEFL));
	rb_define_const(rb_mSwe4r, "SEFLG_NOABERR", INT2FIX(SEFLG_NOABERR));
	rb_define_const(rb_mSwe4r, "SEFLG_EQUATORIAL", INT2FIX(SEFLG_EQUATORIAL));
	rb_define_const(rb_mSwe4r, "SEFLG_XYZ", INT2FIX(SEFLG_XYZ));
	rb_define_const(rb_mSwe4r, "SEFLG_RADIANS", INT2FIX(SEFLG_RADIANS));
	rb_define_const(rb_mSwe4r, "SEFLG_BARYCTR", INT2FIX(SEFLG_BARYCTR));
	rb_define_const(rb_mSwe4r, "SEFLG_TOPOCTR", INT2FIX(SEFLG_TOPOCTR));
	rb_define_const(rb_mSwe4r, "SEFLG_SIDEREAL", INT2FIX(SEFLG_SIDEREAL));
	rb_define_const(rb_mSwe4r, "SEFLG_ICRS", INT2FIX(SEFLG_ICRS));

	/* sidereal modes (ayanamsas) */
	rb_define_const(rb_mSwe4r, "SE_SIDM_FAGAN_BRADLEY", INT2FIX(SE_SIDM_FAGAN_BRADLEY)); // 0
	rb_define_const(rb_mSwe4r, "SE_SIDM_LAHIRI", INT2FIX(SE_SIDM_LAHIRI));
	rb_define_const(rb_mSwe4r, "SE_SIDM_DELUCE", INT2FIX(SE_SIDM_DELUCE));
	rb_define_const(rb_mSwe4r, "SE_SIDM_RAMAN", INT2FIX(SE_SIDM_RAMAN));
	rb_define_const(rb_mSwe4r, "SE_SIDM_USHASHASHI", INT2FIX(SE_SIDM_USHASHASHI));
	rb_define_const(rb_mSwe4r, "SE_SIDM_KRISHNAMURTI", INT2FIX(SE_SIDM_KRISHNAMURTI));
	rb_define_const(rb_mSwe4r, "SE_SIDM_DJWHAL_KHUL", INT2FIX(SE_SIDM_DJWHAL_KHUL));
	rb_define_const(rb_mSwe4r, "SE_SIDM_YUKTESHWAR", INT2FIX(SE_SIDM_YUKTESHWAR));
	rb_define_const(rb_mSwe4r, "SE_SIDM_JN_BHASIN", INT2FIX(SE_SIDM_JN_BHASIN));
	rb_define_const(rb_mSwe4r, "SE_SIDM_BABYL_KUGLER1", INT2FIX(SE_SIDM_BABYL_KUGLER1));
	rb_define_const(rb_mSwe4r, "SE_SIDM_BABYL_KUGLER2", INT2FIX(SE_SIDM_BABYL_KUGLER2));
	rb_define_const(rb_mSwe4r, "SE_SIDM_BABYL_KUGLER3", INT2FIX(SE_SIDM_BABYL_KUGLER3));
	rb_define_const(rb_mSwe4r, "SE_SIDM_BABYL_HUBER", INT2FIX(SE_SIDM_BABYL_HUBER));
	rb_define_const(rb_mSwe4r, "SE_SIDM_BABYL_ETPSC", INT2FIX(SE_SIDM_BABYL_ETPSC));
	rb_define_const(rb_mSwe4r, "SE_SIDM_ALDEBARAN_15TAU", INT2FIX(SE_SIDM_ALDEBARAN_15TAU));
	rb_define_const(rb_mSwe4r, "SE_SIDM_HIPPARCHOS", INT2FIX(SE_SIDM_HIPPARCHOS));
	rb_define_const(rb_mSwe4r, "SE_SIDM_SASSANIAN", INT2FIX(SE_SIDM_SASSANIAN));
	rb_define_const(rb_mSwe4r, "SE_SIDM_GALCENT_0SAG", INT2FIX(SE_SIDM_GALCENT_0SAG));
	rb_define_const(rb_mSwe4r, "SE_SIDM_J2000", INT2FIX(SE_SIDM_J2000));
	rb_define_const(rb_mSwe4r, "SE_SIDM_J1900", INT2FIX(SE_SIDM_J1900));
	rb_define_const(rb_mSwe4r, "SE_SIDM_B1950", INT2FIX(SE_SIDM_B1950));
	rb_define_const(rb_mSwe4r, "SE_SIDM_SURYASIDDHANTA", INT2FIX(SE_SIDM_SURYASIDDHANTA));
	rb_define_const(rb_mSwe4r, "SE_SIDM_SURYASIDDHANTA_MSUN", INT2FIX(SE_SIDM_SURYASIDDHANTA_MSUN));
	rb_define_const(rb_mSwe4r, "SE_SIDM_ARYABHATA", INT2FIX(SE_SIDM_ARYABHATA));
	rb_define_const(rb_mSwe4r, "SE_SIDM_ARYABHATA_MSUN", INT2FIX(SE_SIDM_ARYABHATA_MSUN));
	rb_define_const(rb_mSwe4r, "SE_SIDM_SS_REVATI", INT2FIX(SE_SIDM_SS_REVATI));
	rb_define_const(rb_mSwe4r, "SE_SIDM_SS_CITRA", INT2FIX(SE_SIDM_SS_CITRA));
	rb_define_const(rb_mSwe4r, "SE_SIDM_TRUE_CITRA", INT2FIX(SE_SIDM_TRUE_CITRA));
	rb_define_const(rb_mSwe4r, "SE_SIDM_TRUE_REVATI", INT2FIX(SE_SIDM_TRUE_REVATI));
	rb_define_const(rb_mSwe4r, "SE_SIDM_TRUE_PUSHYA", INT2FIX(SE_SIDM_TRUE_PUSHYA));
	rb_define_const(rb_mSwe4r, "SE_SIDM_GALCENT_RGILBRAND", INT2FIX(SE_SIDM_GALCENT_RGILBRAND));
	rb_define_const(rb_mSwe4r, "SE_SIDM_GALEQU_IAU1958", INT2FIX(SE_SIDM_GALEQU_IAU1958));
	rb_define_const(rb_mSwe4r, "SE_SIDM_GALEQU_TRUE", INT2FIX(SE_SIDM_GALEQU_TRUE));
	rb_define_const(rb_mSwe4r, "SE_SIDM_GALEQU_MULA", INT2FIX(SE_SIDM_GALEQU_MULA));
	rb_define_const(rb_mSwe4r, "SE_SIDM_GALALIGN_MARDYKS", INT2FIX(SE_SIDM_GALALIGN_MARDYKS));
	rb_define_const(rb_mSwe4r, "SE_SIDM_TRUE_MULA", INT2FIX(SE_SIDM_TRUE_MULA));
	rb_define_const(rb_mSwe4r, "SE_SIDM_GALCENT_MULA_WILHELM", INT2FIX(SE_SIDM_GALCENT_MULA_WILHELM));
	rb_define_const(rb_mSwe4r, "SE_SIDM_ARYABHATA_522", INT2FIX(SE_SIDM_ARYABHATA_522));
	rb_define_const(rb_mSwe4r, "SE_SIDM_BABYL_BRITTON", INT2FIX(SE_SIDM_BABYL_BRITTON));
	rb_define_const(rb_mSwe4r, "SE_SIDM_TRUE_SHEORAN", INT2FIX(SE_SIDM_TRUE_SHEORAN));
	rb_define_const(rb_mSwe4r, "SE_SIDM_GALCENT_COCHRANE", INT2FIX(SE_SIDM_GALCENT_COCHRANE));
	rb_define_const(rb_mSwe4r, "SE_SIDM_GALEQU_FIORENZA", INT2FIX(SE_SIDM_GALEQU_FIORENZA));
	rb_define_const(rb_mSwe4r, "SE_SIDM_VALENS_MOON", INT2FIX(SE_SIDM_VALENS_MOON));
	rb_define_const(rb_mSwe4r, "SE_SIDM_LAHIRI_1940", INT2FIX(SE_SIDM_LAHIRI_1940));
	rb_define_const(rb_mSwe4r, "SE_SIDM_LAHIRI_VP285", INT2FIX(SE_SIDM_LAHIRI_VP285));
	rb_define_const(rb_mSwe4r, "SE_SIDM_KRISHNAMURTI_VP291", INT2FIX(SE_SIDM_KRISHNAMURTI_VP291));
	rb_define_const(rb_mSwe4r, "SE_SIDM_LAHIRI_ICRC", INT2FIX(SE_SIDM_LAHIRI_ICRC)); // 46
	rb_define_const(rb_mSwe4r, "SE_SIDM_USER", INT2FIX(SE_SIDM_USER));							 // 255

	rb_define_const(rb_mSwe4r, "SE_GREG_CAL", INT2FIX(SE_GREG_CAL));
	rb_define_const(rb_mSwe4r, "SE_JUL_CAL", INT2FIX(SE_JUL_CAL));

	rb_define_const(rb_mSwe4r, "SE_ECL2HOR", INT2FIX(SE_ECL2HOR));
	rb_define_const(rb_mSwe4r, "SE_EQU2HOR", INT2FIX(SE_EQU2HOR));

	rb_define_const(rb_mSwe4r, "SE_CALC_RISE", INT2FIX(SE_CALC_RISE));
	rb_define_const(rb_mSwe4r, "SE_CALC_SET", INT2FIX(SE_CALC_SET));
	rb_define_const(rb_mSwe4r, "SE_CALC_MTRANSIT", INT2FIX(SE_CALC_MTRANSIT));
	rb_define_const(rb_mSwe4r, "SE_CALC_ITRANSIT", INT2FIX(SE_CALC_ITRANSIT));
	rb_define_const(rb_mSwe4r, "SE_BIT_DISC_CENTER", INT2FIX(SE_BIT_DISC_CENTER));
	rb_define_const(rb_mSwe4r, "SE_BIT_DISC_BOTTOM", INT2FIX(SE_BIT_DISC_BOTTOM));
	rb_define_const(rb_mSwe4r, "SE_BIT_GEOCTR_NO_ECL_LAT", INT2FIX(SE_BIT_GEOCTR_NO_ECL_LAT));
	rb_define_const(rb_mSwe4r, "SE_BIT_NO_REFRACTION", INT2FIX(SE_BIT_NO_REFRACTION));
	rb_define_const(rb_mSwe4r, "SE_BIT_CIVIL_TWILIGHT", INT2FIX(SE_BIT_CIVIL_TWILIGHT));
	rb_define_const(rb_mSwe4r, "SE_BIT_NAUTIC_TWILIGHT", INT2FIX(SE_BIT_NAUTIC_TWILIGHT));
	rb_define_const(rb_mSwe4r, "SE_BIT_ASTRO_TWILIGHT", INT2FIX(SE_BIT_ASTRO_TWILIGHT));
	rb_define_const(rb_mSwe4r, "SE_BIT_FIXED_DISC_SIZE", INT2FIX(SE_BIT_FIXED_DISC_SIZE));
	rb_define_const(rb_mSwe4r, "SE_BIT_HINDU_RISING", INT2FIX(SE_BIT_HINDU_RISING));
}
