/*
Swe4r :: Swiss Ephemeris for Ruby - A C extension for the Swiss Ephemeris library (http://www.astro.com/swisseph/)
Copyright (C) 2012 Andrew Kirk (andrew.kirk@windhorsemedia.com)
Additional work (C) 2024-25 David Lowenfels (dfl@alum.mit.edu)

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
 * Close Swiss Ephemeris and release resources
 * void swe_close(void);
 */
static VALUE t_swe_close(VALUE self)
{
	swe_close();
	return Qnil;
}

/*
 * Get Swiss Ephemeris version string
 * char *swe_version(char *);
 */
static VALUE t_swe_version(VALUE self)
{
	char version[AS_MAXCH];
	swe_version(version);
	return rb_str_new_cstr(version);
}

/*
 * Get planet name by number
 * char *swe_get_planet_name(int ipl, char *spname);
 */
static VALUE t_swe_get_planet_name(VALUE self, VALUE ipl)
{
	char name[AS_MAXCH];
	swe_get_planet_name(NUM2INT(ipl), name);
	return rb_str_new_cstr(name);
}

/*
 * Get ayanamsa name by mode number
 * const char *swe_get_ayanamsa_name(int32 isidmode);
 */
static VALUE t_swe_get_ayanamsa_name(VALUE self, VALUE isidmode)
{
	const char *name = swe_get_ayanamsa_name(NUM2INT(isidmode));
	if (name == NULL)
		return Qnil;
	return rb_str_new_cstr(name);
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
 * Convert UTC to Julian day
 * Returns array of [jd_et, jd_ut] or raises error
 * int32 swe_utc_to_jd(int32 iyear, int32 imonth, int32 iday,
 *                     int32 ihour, int32 imin, double dsec,
 *                     int32 gregflag, double *dret, char *serr);
 */
static VALUE t_swe_utc_to_jd(int argc, VALUE *argv, VALUE self)
{
	if (argc < 6 || argc > 7)
		rb_raise(rb_eArgError, "wrong number of arguments (6 or 7)");

	int32 gregflag = (argc == 7) ? NUM2INT(argv[6]) : SE_GREG_CAL;
	double dret[2];
	char serr[AS_MAXCH];

	int32 result = swe_utc_to_jd(
		NUM2INT(argv[0]), NUM2INT(argv[1]), NUM2INT(argv[2]),
		NUM2INT(argv[3]), NUM2INT(argv[4]), NUM2DBL(argv[5]),
		gregflag, dret, serr);

	if (result < 0)
		rb_raise(rb_eRuntimeError, serr);

	VALUE output = rb_ary_new();
	rb_ary_push(output, rb_float_new(dret[0])); // JD ET
	rb_ary_push(output, rb_float_new(dret[1])); // JD UT
	return output;
}

/*
 * Convert Julian day UT to UTC
 * void swe_jdut1_to_utc(double tjd_ut, int32 gregflag,
 *                       int32 *iyear, int32 *imonth, int32 *iday,
 *                       int32 *ihour, int32 *imin, double *dsec);
 */
static VALUE t_swe_jdut1_to_utc(int argc, VALUE *argv, VALUE self)
{
	if (argc < 1 || argc > 2)
		rb_raise(rb_eArgError, "wrong number of arguments (1 or 2)");

	int32 gregflag = (argc == 2) ? NUM2INT(argv[1]) : SE_GREG_CAL;
	int32 year, month, day, hour, min;
	double sec;

	swe_jdut1_to_utc(NUM2DBL(argv[0]), gregflag, &year, &month, &day, &hour, &min, &sec);

	VALUE output = rb_ary_new();
	rb_ary_push(output, INT2NUM(year));
	rb_ary_push(output, INT2NUM(month));
	rb_ary_push(output, INT2NUM(day));
	rb_ary_push(output, INT2NUM(hour));
	rb_ary_push(output, INT2NUM(min));
	rb_ary_push(output, rb_float_new(sec));
	return output;
}

/*
 * Get day of week from Julian day
 * Returns 0=Monday, 1=Tuesday, ..., 6=Sunday
 * int swe_day_of_week(double jd);
 */
static VALUE t_swe_day_of_week(VALUE self, VALUE jd)
{
	return INT2NUM(swe_day_of_week(NUM2DBL(jd)));
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
 * Sidereal time with explicit obliquity and nutation
 * double swe_sidtime0(double tjd_ut, double eps, double nut);
 */
static VALUE t_swe_sidtime0(VALUE self, VALUE julian_ut, VALUE eps, VALUE nut)
{
	double sidtime = swe_sidtime0(NUM2DBL(julian_ut), NUM2DBL(eps), NUM2DBL(nut));
	return rb_float_new(sidtime);
}

/*
 * Normalize degrees to range 0-360
 * double swe_degnorm(double x);
 */
static VALUE t_swe_degnorm(VALUE self, VALUE deg)
{
	return rb_float_new(swe_degnorm(NUM2DBL(deg)));
}

/*
 * Normalize radians to range 0-2*PI
 * double swe_radnorm(double x);
 */
static VALUE t_swe_radnorm(VALUE self, VALUE rad)
{
	return rb_float_new(swe_radnorm(NUM2DBL(rad)));
}

/*
 * Split degrees into degrees, minutes, seconds
 * void swe_split_deg(double ddeg, int32 roundflag, int32 *ideg, int32 *imin, int32 *isec, double *dsecfr, int32 *isgn);
 * roundflag: SE_SPLIT_DEG_ROUND_SEC, SE_SPLIT_DEG_ROUND_MIN, SE_SPLIT_DEG_ROUND_DEG, SE_SPLIT_DEG_ZODIACAL, SE_SPLIT_DEG_KEEP_SIGN, SE_SPLIT_DEG_KEEP_DEG
 * Returns [deg, min, sec, secfr, sign]
 */
static VALUE t_swe_split_deg(VALUE self, VALUE ddeg, VALUE roundflag)
{
	int32 ideg, imin, isec, isgn;
	double dsecfr;

	swe_split_deg(NUM2DBL(ddeg), NUM2INT(roundflag), &ideg, &imin, &isec, &dsecfr, &isgn);

	VALUE output = rb_ary_new();
	rb_ary_push(output, INT2NUM(ideg));
	rb_ary_push(output, INT2NUM(imin));
	rb_ary_push(output, INT2NUM(isec));
	rb_ary_push(output, rb_float_new(dsecfr));
	rb_ary_push(output, INT2NUM(isgn));
	return output;
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

/*
 * Houses with flags (sidereal, etc.)
 * int swe_houses_ex(double tjd_ut, int32 iflag, double geolat, double geolon, int hsys, double *cusps, double *ascmc);
 */
static VALUE t_swe_houses_ex(VALUE self, VALUE julian_day, VALUE iflag, VALUE latitude, VALUE longitude, VALUE house_system)
{
	double cusps[13];
	double ascmc[10];
	char serr[AS_MAXCH];

	if (swe_houses_ex(NUM2DBL(julian_day), NUM2INT(iflag), NUM2DBL(latitude), NUM2DBL(longitude), NUM2CHR(house_system), cusps, ascmc) < 0)
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

/*
 * Houses from ARMC (sidereal time)
 * int swe_houses_armc(double armc, double geolat, double eps, int hsys, double *cusps, double *ascmc);
 */
static VALUE t_swe_houses_armc(VALUE self, VALUE armc, VALUE latitude, VALUE eps, VALUE house_system)
{
	double cusps[13];
	double ascmc[10];
	char serr[AS_MAXCH];

	if (swe_houses_armc(NUM2DBL(armc), NUM2DBL(latitude), NUM2DBL(eps), NUM2CHR(house_system), cusps, ascmc) < 0)
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

// char *swe_house_name(
// 			int hsys);          /* house method, ascii code of one of the letters PKORCAEVXHTBG */
static VALUE t_swe_house_name(VALUE self, VALUE hsys)
{
	return rb_str_new_cstr(swe_house_name(NUM2CHR(hsys)));
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

/*
 * Reverse of swe_azalt: horizontal to ecliptic/equatorial
 * void swe_azalt_rev(double tjd_ut, int32 calc_flag, double *geopos, double *xin, double *xout);
 * calc_flag: SE_HOR2ECL or SE_HOR2EQU
 * xin: [azimuth, true_altitude]
 * xout: [longitude, latitude] or [ra, dec]
 */
static VALUE t_swe_azalt_rev(VALUE self, VALUE julian_day, VALUE flag, VALUE lon, VALUE lat, VALUE height, VALUE azimuth, VALUE altitude)
{
	double geopos[3];
	geopos[0] = NUM2DBL(lon);
	geopos[1] = NUM2DBL(lat);
	geopos[2] = NUM2DBL(height);

	double xin[2];
	xin[0] = NUM2DBL(azimuth);
	xin[1] = NUM2DBL(altitude);
	double xout[2];

	swe_azalt_rev(NUM2DBL(julian_day), NUM2INT(flag), geopos, xin, xout);

	VALUE output = rb_ary_new();
	rb_ary_push(output, rb_float_new(xout[0]));
	rb_ary_push(output, rb_float_new(xout[1]));
	return output;
}

/*
 * Atmospheric refraction
 * double swe_refrac(double inalt, double atpress, double attemp, int32 calc_flag);
 * calc_flag: SE_TRUE_TO_APP (0) or SE_APP_TO_TRUE (1)
 */
static VALUE t_swe_refrac(VALUE self, VALUE inalt, VALUE atpress, VALUE attemp, VALUE calc_flag)
{
	double result = swe_refrac(NUM2DBL(inalt), NUM2DBL(atpress), NUM2DBL(attemp), NUM2INT(calc_flag));
	return rb_float_new(result);
}

/*
 * Planetary phenomena (phase, elongation, magnitude, etc.)
 * int32 swe_pheno_ut(double tjd_ut, int32 ipl, int32 iflag, double *attr, char *serr);
 * Returns array of 7 values:
 * [0] phase angle (earth-planet-sun)
 * [1] phase (illumined fraction of disc)
 * [2] elongation of planet
 * [3] apparent diameter of disc
 * [4] apparent magnitude
 * [5] horizontal parallax (Moon)
 * [6] (reserved)
 */
static VALUE t_swe_pheno_ut(VALUE self, VALUE julian_ut, VALUE ipl, VALUE iflag)
{
	double attr[20];
	char serr[AS_MAXCH];

	if (swe_pheno_ut(NUM2DBL(julian_ut), NUM2INT(ipl), NUM2INT(iflag), attr, serr) < 0)
		rb_raise(rb_eRuntimeError, serr);

	VALUE output = rb_ary_new();
	for (int i = 0; i < 7; i++)
		rb_ary_push(output, rb_float_new(attr[i]));
	return output;
}

/*
 * Equation of time
 * int32 swe_time_equ(double tjd, double *te, char *serr);
 * Returns the difference between local apparent time and local mean time in days
 */
static VALUE t_swe_time_equ(VALUE self, VALUE tjd)
{
	double te;
	char serr[AS_MAXCH];

	if (swe_time_equ(NUM2DBL(tjd), &te, serr) < 0)
		rb_raise(rb_eRuntimeError, serr);

	return rb_float_new(te);
}

/*
 * Local mean time to local apparent time
 * int32 swe_lmt_to_lat(double tjd_lmt, double geolon, double *tjd_lat, char *serr);
 */
static VALUE t_swe_lmt_to_lat(VALUE self, VALUE tjd_lmt, VALUE geolon)
{
	double tjd_lat;
	char serr[AS_MAXCH];

	if (swe_lmt_to_lat(NUM2DBL(tjd_lmt), NUM2DBL(geolon), &tjd_lat, serr) < 0)
		rb_raise(rb_eRuntimeError, serr);

	return rb_float_new(tjd_lat);
}

/*
 * Local apparent time to local mean time
 * int32 swe_lat_to_lmt(double tjd_lat, double geolon, double *tjd_lmt, char *serr);
 */
static VALUE t_swe_lat_to_lmt(VALUE self, VALUE tjd_lat, VALUE geolon)
{
	double tjd_lmt;
	char serr[AS_MAXCH];

	if (swe_lat_to_lmt(NUM2DBL(tjd_lat), NUM2DBL(geolon), &tjd_lmt, serr) < 0)
		rb_raise(rb_eRuntimeError, serr);

	return rb_float_new(tjd_lmt);
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

/*
 * Coordinate transformation with speed
 * void swe_cotrans_sp(double *xpo, double *xpn, double eps);
 * xpo: [lon, lat, dist, lon_speed, lat_speed, dist_speed]
 * xpn: output [lon, lat, dist, lon_speed, lat_speed, dist_speed]
 */
static VALUE t_swe_cotrans_sp(VALUE self, VALUE eps, VALUE lon, VALUE lat, VALUE dist, VALUE lon_speed, VALUE lat_speed, VALUE dist_speed)
{
	double xpo[6];
	xpo[0] = NUM2DBL(lon);
	xpo[1] = NUM2DBL(lat);
	xpo[2] = NUM2DBL(dist);
	xpo[3] = NUM2DBL(lon_speed);
	xpo[4] = NUM2DBL(lat_speed);
	xpo[5] = NUM2DBL(dist_speed);

	double xpn[6];

	swe_cotrans_sp(xpo, xpn, NUM2DBL(eps));

	VALUE output = rb_ary_new();
	for (int i = 0; i < 6; i++)
		rb_ary_push(output, rb_float_new(xpn[i]));
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
 * Find the crossing of the Sun over a given ecliptic position
 * double swe_solcross_ut(double x2cross, double tjd_ut, int32 iflag, char *serr);
 */
static VALUE t_swe_solcross_ut(VALUE self, VALUE x2cross, VALUE tjd_ut, VALUE iflag)
{
	char serr[AS_MAXCH];

	double retval = swe_solcross_ut(NUM2DBL(x2cross), NUM2DBL(tjd_ut), NUM2INT(iflag), serr);
	if (retval < tjd_ut)
		rb_raise(rb_eRuntimeError, serr);
	return rb_float_new(retval);
}

/*
 * Find the crossing of the Moon over a given ecliptic position
 * double swe_mooncross_ut(double x2cross, double tjd_ut, int32 iflag, char *serr);
 */
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
	rb_ary_push(output, rb_float_new(retval));
	rb_ary_push(output, rb_float_new(xlon));
	rb_ary_push(output, rb_float_new(xlat));

	return output;
}
/*
 * Find heliocentric crossing of a planet over a given ecliptic position
 * int32 swe_helio_cross_ut(int32 ipl, double x2cross, double tjd_ut, int32 iflag, int32 dir, double *jx, char *serr);
 */
static VALUE t_swe_helio_cross_ut(VALUE self, VALUE body, VALUE x2cross, VALUE tjd_ut, VALUE iflag, VALUE dir)
{
	char serr[AS_MAXCH];
	double jx;
	double retval = swe_helio_cross_ut(NUM2INT(body), NUM2DBL(x2cross), NUM2DBL(tjd_ut), NUM2INT(iflag), NUM2INT(dir), &jx, serr);
	if (retval < tjd_ut)
		rb_raise(rb_eRuntimeError, serr);
	VALUE output = rb_ary_new();
	rb_ary_push(output, rb_float_new(retval));
	rb_ary_push(output, rb_float_new(jx));

	return output;
}

/*
 * Compute planetary nodes and apsides (perihelia, aphelia)
 * Returns [ascending_node, descending_node, perihelion, aphelion] each with 6 values
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
		rb_ary_push(perihelion, rb_float_new(xperi[i]));
	rb_ary_push(output, perihelion);

	VALUE aphelion = rb_ary_new();
	for (int i = 0; i < 6; i++)
		rb_ary_push(aphelion, rb_float_new(xaphe[i]));
	rb_ary_push(output, aphelion);

	return output; // return array of arrays
}

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

/*
 * Calculate orbital elements of a planet or asteroid
 * https://www.astro.com/swisseph/swephprg.htm#_Toc112949042
 * int32 swe_get_orbital_elements(
 *   double tjd_et,      // Julian day in ET/TT
 *   int32 ipl,          // planet number
 *   int32 iflag,        // flag bits
 *   double *dret,       // return values, see below
 *   char *serr);        // error string
 *
 * Returns array with the following values:
 * - 0: semi-major axis (AU)
 * - 1: eccentricity
 * - 2: inclination (degrees)
 * - 3: longitude of ascending node (degrees)
 * - 4: argument of perihelion (degrees)
 * - 5: longitude of perihelion (degrees)
 * - 6: mean anomaly (degrees)
 * - 7: true anomaly (degrees)
 * - 8: eccentric anomaly (degrees)
 * - 9: mean longitude (degrees)
 * - 10: semi-minor axis (AU)
 * - 11: focal distance (AU)
 * - 12: perihelion distance (AU)
 * - 13: aphelion distance (AU)
 * - 14: orbital period (years)
 * - 15: mean daily motion (degrees)
 * - 16: daily motion at tjd_et (degrees)
 */
static VALUE t_swe_get_orbital_elements(VALUE self, VALUE julian_et, VALUE body, VALUE iflag)
{
	char serr[AS_MAXCH];
	double dret[17]; // Array to store the return values

	if (swe_get_orbital_elements(NUM2DBL(julian_et), NUM2INT(body), NUM2INT(iflag), dret, serr) < 0)
		rb_raise(rb_eRuntimeError, serr);

	VALUE output = rb_ary_new();
	for (int i = 0; i < 17; i++)
		rb_ary_push(output, rb_float_new(dret[i]));

	return output;
}

/*
 * Get the Delta T value (difference between ET and UT)
 * https://www.astro.com/swisseph/swephprg.htm#_Toc112949107
 * double swe_deltat(double tjd_ut);
 */
static VALUE t_swe_deltat(VALUE self, VALUE julian_ut)
{
	double delta_t = swe_deltat(NUM2DBL(julian_ut));
	return rb_float_new(delta_t);
}

/*
 * Get the Delta T value (difference between ET and UT) with explicit ephemeris
 * https://www.astro.com/swisseph/swephprg.htm#_Toc112949107
 * double swe_deltat_ex(double tjd_ut, int32 iflag, char *serr);
 */
static VALUE t_swe_deltat_ex(VALUE self, VALUE julian_ut, VALUE iflag)
{
	char serr[AS_MAXCH];
	double delta_t = swe_deltat_ex(NUM2DBL(julian_ut), NUM2INT(iflag), serr);

	// If there's an error message, return it along with the delta_t value
	if (serr[0] != '\0')
	{
		VALUE result = rb_ary_new();
		rb_ary_push(result, rb_float_new(delta_t));
		rb_ary_push(result, rb_str_new_cstr(serr));
		return result;
	}

	return rb_float_new(delta_t);
}

static VALUE t_swe_fixstar(VALUE self, VALUE star, VALUE julian_et, VALUE iflag)
{
	char serr[AS_MAXCH];
	double results[6];

	if (swe_fixstar(StringValuePtr(star), NUM2DBL(julian_et), NUM2INT(iflag), results, serr) < 0)
		rb_raise(rb_eRuntimeError, serr);

	VALUE output = rb_ary_new();
	for (int i = 0; i < 6; i++)
		rb_ary_push(output, rb_float_new(results[i]));

	return output;
}

static VALUE t_swe_fixstar_ut(VALUE self, VALUE star, VALUE julian_ut, VALUE iflag)
{
	char serr[AS_MAXCH];
	double results[6];

	if (swe_fixstar_ut(StringValuePtr(star), NUM2DBL(julian_ut), NUM2INT(iflag), results, serr) < 0)
		rb_raise(rb_eRuntimeError, serr);

	VALUE output = rb_ary_new();
	for (int i = 0; i < 6; i++)
		rb_ary_push(output, rb_float_new(results[i]));

	return output;
}

static VALUE t_swe_fixstar_mag(VALUE self, VALUE star)
{
	char serr[AS_MAXCH];
	double mag;

	if (swe_fixstar_mag(StringValuePtr(star), &mag, serr) < 0)
		rb_raise(rb_eRuntimeError, serr);

	return rb_float_new(mag);
}

static VALUE t_swe_fixstar2(VALUE self, VALUE star, VALUE julian_et, VALUE iflag)
{
	char serr[AS_MAXCH];
	double results[6];

	if (swe_fixstar2(StringValuePtr(star), NUM2DBL(julian_et), NUM2INT(iflag), results, serr) < 0)
		rb_raise(rb_eRuntimeError, serr);

	VALUE output = rb_ary_new();
	for (int i = 0; i < 6; i++)
		rb_ary_push(output, rb_float_new(results[i]));

	return output;
}

static VALUE t_swe_fixstar2_ut(VALUE self, VALUE star, VALUE julian_ut, VALUE iflag)
{
	char serr[AS_MAXCH];
	double results[6];

	if (swe_fixstar2_ut(StringValuePtr(star), NUM2DBL(julian_ut), NUM2INT(iflag), results, serr) < 0)
		rb_raise(rb_eRuntimeError, serr);

	VALUE output = rb_ary_new();
	for (int i = 0; i < 6; i++)
		rb_ary_push(output, rb_float_new(results[i]));

	return output;
}

static VALUE t_swe_fixstar2_mag(VALUE self, VALUE star)
{
	char serr[AS_MAXCH];
	double mag;

	if (swe_fixstar2_mag(StringValuePtr(star), &mag, serr) < 0)
		rb_raise(rb_eRuntimeError, serr);

	return rb_float_new(mag);
}

/*
 * Find heliacal rising/setting times
 * int32 swe_heliacal_ut(double tjdstart_ut, double *geopos, double *datm, double *dobs, char *ObjectName, int32 TypeEvent, int32 iflag, double *dret, char *serr);
 * geopos: [lon, lat, height]
 * datm: [pressure, temp, humidity, extinction_coeff] (4 doubles)
 * dobs: [age, snellen_left, snellen_right, telescope_mag, telescope_diam, binoc_factor] (6 doubles)
 * TypeEvent: SE_HELIACAL_RISING, SE_HELIACAL_SETTING, SE_EVENING_FIRST, SE_MORNING_LAST
 * Returns array of 50 values (see Swiss Ephemeris documentation)
 */
static VALUE t_swe_heliacal_ut(int argc, VALUE *argv, VALUE self)
{
	if (argc != 9)
		rb_raise(rb_eArgError, "wrong number of arguments (9 required: tjd_ut, object_name, type_event, iflag, lon, lat, height, datm_array, dobs_array)");

	char serr[AS_MAXCH];
	double geopos[3];
	geopos[0] = NUM2DBL(argv[4]); // lon
	geopos[1] = NUM2DBL(argv[5]); // lat
	geopos[2] = NUM2DBL(argv[6]); // height

	// datm: atmospheric data array (4 doubles)
	VALUE datm_arr = argv[7];
	if (TYPE(datm_arr) != T_ARRAY || RARRAY_LEN(datm_arr) < 4)
		rb_raise(rb_eArgError, "datm must be array of 4 values [pressure, temp, humidity, extinction_coeff]");
	double datm[4];
	for (int i = 0; i < 4; i++)
		datm[i] = NUM2DBL(rb_ary_entry(datm_arr, i));

	// dobs: observer data array (6 doubles)
	VALUE dobs_arr = argv[8];
	if (TYPE(dobs_arr) != T_ARRAY || RARRAY_LEN(dobs_arr) < 6)
		rb_raise(rb_eArgError, "dobs must be array of 6 values [age, snellen_left, snellen_right, telescope_mag, telescope_diam, binoc_factor]");
	double dobs[6];
	for (int i = 0; i < 6; i++)
		dobs[i] = NUM2DBL(rb_ary_entry(dobs_arr, i));

	double dret[50];
	char *object_name = StringValuePtr(argv[1]);

	if (swe_heliacal_ut(NUM2DBL(argv[0]), geopos, datm, dobs, object_name, NUM2INT(argv[2]), NUM2INT(argv[3]), dret, serr) < 0)
		rb_raise(rb_eRuntimeError, serr);

	VALUE output = rb_ary_new();
	for (int i = 0; i < 50; i++)
		rb_ary_push(output, rb_float_new(dret[i]));
	return output;
}

/*
 * Compute visibility limit magnitude
 * int32 swe_vis_limit_mag(double tjdut, double *geopos, double *datm, double *dobs, char *ObjectName, int32 helflag, double *dret, char *serr);
 * Returns array of 8 values with magnitude information
 */
static VALUE t_swe_vis_limit_mag(int argc, VALUE *argv, VALUE self)
{
	if (argc != 8)
		rb_raise(rb_eArgError, "wrong number of arguments (8 required: tjd_ut, object_name, helflag, lon, lat, height, datm_array, dobs_array)");

	char serr[AS_MAXCH];
	double geopos[3];
	geopos[0] = NUM2DBL(argv[3]); // lon
	geopos[1] = NUM2DBL(argv[4]); // lat
	geopos[2] = NUM2DBL(argv[5]); // height

	// datm: atmospheric data array (4 doubles)
	VALUE datm_arr = argv[6];
	if (TYPE(datm_arr) != T_ARRAY || RARRAY_LEN(datm_arr) < 4)
		rb_raise(rb_eArgError, "datm must be array of 4 values [pressure, temp, humidity, extinction_coeff]");
	double datm[4];
	for (int i = 0; i < 4; i++)
		datm[i] = NUM2DBL(rb_ary_entry(datm_arr, i));

	// dobs: observer data array (6 doubles)
	VALUE dobs_arr = argv[7];
	if (TYPE(dobs_arr) != T_ARRAY || RARRAY_LEN(dobs_arr) < 6)
		rb_raise(rb_eArgError, "dobs must be array of 6 values [age, snellen_left, snellen_right, telescope_mag, telescope_diam, binoc_factor]");
	double dobs[6];
	for (int i = 0; i < 6; i++)
		dobs[i] = NUM2DBL(rb_ary_entry(dobs_arr, i));

	double dret[50];
	char *object_name = StringValuePtr(argv[1]);

	if (swe_vis_limit_mag(NUM2DBL(argv[0]), geopos, datm, dobs, object_name, NUM2INT(argv[2]), dret, serr) < 0)
		rb_raise(rb_eRuntimeError, serr);

	VALUE output = rb_ary_new();
	for (int i = 0; i < 8; i++)
		rb_ary_push(output, rb_float_new(dret[i]));
	return output;
}

/*
 * Compute Gauquelin sector position
 * int32 swe_gauquelin_sector(double t_ut, int32 ipl, char *starname, int32 iflag, int32 imeth, double *geopos, double atpress, double attemp, double *dgsect, char *serr);
 * imeth: 0=sector from MC/IC, 1=sector from Asc/Desc, 2=sector from 36 houses, 3=sector from 36 houses with MC/IC, 4=sector from MC/IC/Asc/Desc
 * Returns sector position (1-36)
 */
static VALUE t_swe_gauquelin_sector(VALUE self, VALUE tjd_ut, VALUE body, VALUE iflag, VALUE imeth, VALUE lon, VALUE lat, VALUE height, VALUE atpress, VALUE attemp)
{
	char serr[AS_MAXCH];
	double geopos[3];
	geopos[0] = NUM2DBL(lon);
	geopos[1] = NUM2DBL(lat);
	geopos[2] = NUM2DBL(height);
	double dgsect;
	int ipl;
	char *starname;

	if (TYPE(body) == T_STRING) {
		starname = StringValuePtr(body);
		ipl = 0;
	} else {
		ipl = NUM2INT(body);
		starname = NULL;
	}

	if (swe_gauquelin_sector(NUM2DBL(tjd_ut), ipl, starname, NUM2INT(iflag), NUM2INT(imeth), geopos, NUM2DBL(atpress), NUM2DBL(attemp), &dgsect, serr) < 0)
		rb_raise(rb_eRuntimeError, serr);

	return rb_float_new(dgsect);
}

/*
 * Find time of next solar eclipse globally
 * int32 swe_sol_eclipse_when_glob(double tjd_start, int32 ifl, int32 ifltype, double *tret, int32 backward, char *serr);
 * ifltype: SE_ECL_TOTAL, SE_ECL_ANNULAR, SE_ECL_PARTIAL, SE_ECL_ANNULAR_TOTAL, or 0 for any
 * backward: 1 for backward search, 0 for forward
 * Returns array of 10 times:
 * [0] time of maximum eclipse
 * [1] time when eclipse takes place at local apparent noon (not implemented)
 * [2] time of eclipse begin
 * [3] time of eclipse end
 * [4] time of totality begin
 * [5] time of totality end
 * [6] time of center line begin
 * [7] time of center line end
 * [8] time when annular-total eclipse becomes total (not implemented)
 * [9] time when annular-total eclipse becomes annular again (not implemented)
 */
static VALUE t_swe_sol_eclipse_when_glob(VALUE self, VALUE tjd_start, VALUE ifl, VALUE ifltype, VALUE backward)
{
	char serr[AS_MAXCH];
	double tret[10];

	int32 result = swe_sol_eclipse_when_glob(NUM2DBL(tjd_start), NUM2INT(ifl), NUM2INT(ifltype), tret, NUM2INT(backward), serr);
	if (result < 0)
		rb_raise(rb_eRuntimeError, serr);

	VALUE output = rb_ary_new();
	rb_ary_push(output, INT2NUM(result)); // eclipse type flags
	for (int i = 0; i < 10; i++)
		rb_ary_push(output, rb_float_new(tret[i]));
	return output;
}

/*
 * Find time of next local solar eclipse
 * int32 swe_sol_eclipse_when_loc(double tjd_start, int32 ifl, double *geopos, double *tret, double *attr, int32 backward, char *serr);
 * Returns [type, tret[10], attr[20]]
 */
static VALUE t_swe_sol_eclipse_when_loc(VALUE self, VALUE tjd_start, VALUE ifl, VALUE lon, VALUE lat, VALUE height, VALUE backward)
{
	char serr[AS_MAXCH];
	double geopos[3];
	geopos[0] = NUM2DBL(lon);
	geopos[1] = NUM2DBL(lat);
	geopos[2] = NUM2DBL(height);
	double tret[10];
	double attr[20];

	int32 result = swe_sol_eclipse_when_loc(NUM2DBL(tjd_start), NUM2INT(ifl), geopos, tret, attr, NUM2INT(backward), serr);
	if (result < 0)
		rb_raise(rb_eRuntimeError, serr);

	VALUE output = rb_ary_new();
	rb_ary_push(output, INT2NUM(result)); // eclipse type flags

	VALUE _tret = rb_ary_new();
	for (int i = 0; i < 10; i++)
		rb_ary_push(_tret, rb_float_new(tret[i]));
	rb_ary_push(output, _tret);

	VALUE _attr = rb_ary_new();
	for (int i = 0; i < 20; i++)
		rb_ary_push(_attr, rb_float_new(attr[i]));
	rb_ary_push(output, _attr);

	return output;
}

/*
 * Compute attributes of solar eclipse for given tjd and location
 * int32 swe_sol_eclipse_how(double tjd, int32 ifl, double *geopos, double *attr, char *serr);
 * Returns [type, attr[20]]
 * attr[0]: fraction of solar diameter covered by moon (= magnitude for partial and annular)
 * attr[1]: ratio of lunar diameter to solar one
 * attr[2]: fraction of solar disc covered by moon (obscuration)
 * attr[3]: diameter of core shadow in km
 * attr[4]: azimuth of sun at tjd
 * attr[5]: true altitude of sun above horizon at tjd
 * attr[6]: apparent altitude of sun above horizon at tjd
 * attr[7]: elongation of moon in degrees
 * attr[8]: magnitude acc. to NASA
 * attr[9]: saros series number (if >= 0)
 * attr[10]: saros series member number (if >= 0)
 */
static VALUE t_swe_sol_eclipse_how(VALUE self, VALUE tjd, VALUE ifl, VALUE lon, VALUE lat, VALUE height)
{
	char serr[AS_MAXCH];
	double geopos[3];
	geopos[0] = NUM2DBL(lon);
	geopos[1] = NUM2DBL(lat);
	geopos[2] = NUM2DBL(height);
	double attr[20];

	int32 result = swe_sol_eclipse_how(NUM2DBL(tjd), NUM2INT(ifl), geopos, attr, serr);
	if (result < 0)
		rb_raise(rb_eRuntimeError, serr);

	VALUE output = rb_ary_new();
	rb_ary_push(output, INT2NUM(result)); // eclipse type flags
	for (int i = 0; i < 20; i++)
		rb_ary_push(output, rb_float_new(attr[i]));
	return output;
}

/*
 * Find geographic location and attributes of solar eclipse at given tjd
 * int32 swe_sol_eclipse_where(double tjd, int32 ifl, double *geopos, double *attr, char *serr);
 * Returns [type, geopos[2], attr[20]]
 */
static VALUE t_swe_sol_eclipse_where(VALUE self, VALUE tjd, VALUE ifl)
{
	char serr[AS_MAXCH];
	double geopos[10];
	double attr[20];

	int32 result = swe_sol_eclipse_where(NUM2DBL(tjd), NUM2INT(ifl), geopos, attr, serr);
	if (result < 0)
		rb_raise(rb_eRuntimeError, serr);

	VALUE output = rb_ary_new();
	rb_ary_push(output, INT2NUM(result)); // eclipse type flags

	VALUE _geopos = rb_ary_new();
	rb_ary_push(_geopos, rb_float_new(geopos[0])); // longitude
	rb_ary_push(_geopos, rb_float_new(geopos[1])); // latitude
	rb_ary_push(output, _geopos);

	VALUE _attr = rb_ary_new();
	for (int i = 0; i < 20; i++)
		rb_ary_push(_attr, rb_float_new(attr[i]));
	rb_ary_push(output, _attr);

	return output;
}

/*
 * Find time of next lunar eclipse
 * int32 swe_lun_eclipse_when(double tjd_start, int32 ifl, int32 ifltype, double *tret, int32 backward, char *serr);
 * ifltype: SE_ECL_TOTAL, SE_ECL_PENUMBRAL, SE_ECL_PARTIAL, or 0 for any
 * Returns [type, tret[10]]
 * tret[0]: time of maximum eclipse
 * tret[1]: (not used)
 * tret[2]: time of partial phase begin
 * tret[3]: time of partial phase end
 * tret[4]: time of totality begin
 * tret[5]: time of totality end
 * tret[6]: time of penumbral phase begin
 * tret[7]: time of penumbral phase end
 */
static VALUE t_swe_lun_eclipse_when(VALUE self, VALUE tjd_start, VALUE ifl, VALUE ifltype, VALUE backward)
{
	char serr[AS_MAXCH];
	double tret[10];

	int32 result = swe_lun_eclipse_when(NUM2DBL(tjd_start), NUM2INT(ifl), NUM2INT(ifltype), tret, NUM2INT(backward), serr);
	if (result < 0)
		rb_raise(rb_eRuntimeError, serr);

	VALUE output = rb_ary_new();
	rb_ary_push(output, INT2NUM(result)); // eclipse type flags
	for (int i = 0; i < 10; i++)
		rb_ary_push(output, rb_float_new(tret[i]));
	return output;
}

/*
 * Find time of next local lunar eclipse
 * int32 swe_lun_eclipse_when_loc(double tjd_start, int32 ifl, double *geopos, double *tret, double *attr, int32 backward, char *serr);
 * Returns [type, tret[10], attr[20]]
 */
static VALUE t_swe_lun_eclipse_when_loc(VALUE self, VALUE tjd_start, VALUE ifl, VALUE lon, VALUE lat, VALUE height, VALUE backward)
{
	char serr[AS_MAXCH];
	double geopos[3];
	geopos[0] = NUM2DBL(lon);
	geopos[1] = NUM2DBL(lat);
	geopos[2] = NUM2DBL(height);
	double tret[10];
	double attr[20];

	int32 result = swe_lun_eclipse_when_loc(NUM2DBL(tjd_start), NUM2INT(ifl), geopos, tret, attr, NUM2INT(backward), serr);
	if (result < 0)
		rb_raise(rb_eRuntimeError, serr);

	VALUE output = rb_ary_new();
	rb_ary_push(output, INT2NUM(result)); // eclipse type flags

	VALUE _tret = rb_ary_new();
	for (int i = 0; i < 10; i++)
		rb_ary_push(_tret, rb_float_new(tret[i]));
	rb_ary_push(output, _tret);

	VALUE _attr = rb_ary_new();
	for (int i = 0; i < 20; i++)
		rb_ary_push(_attr, rb_float_new(attr[i]));
	rb_ary_push(output, _attr);

	return output;
}

/*
 * Compute attributes of lunar eclipse for given tjd
 * int32 swe_lun_eclipse_how(double tjd_ut, int32 ifl, double *geopos, double *attr, char *serr);
 * geopos is optional for some values (can pass 0,0,0)
 * Returns [type, attr[20]]
 * attr[0]: umbral magnitude at tjd
 * attr[1]: penumbral magnitude
 * attr[2]: (not used)
 * attr[3]: (not used)
 * attr[4]: azimuth of moon at tjd
 * attr[5]: true altitude of moon above horizon at tjd
 * attr[6]: apparent altitude of moon above horizon at tjd
 * attr[7]: distance of moon from opposition in degrees
 * attr[8]: umbral magnitude at tjd (same as attr[0])
 * attr[9]: saros series number
 * attr[10]: saros series member number
 */
static VALUE t_swe_lun_eclipse_how(VALUE self, VALUE tjd, VALUE ifl, VALUE lon, VALUE lat, VALUE height)
{
	char serr[AS_MAXCH];
	double geopos[3];
	geopos[0] = NUM2DBL(lon);
	geopos[1] = NUM2DBL(lat);
	geopos[2] = NUM2DBL(height);
	double attr[20];

	int32 result = swe_lun_eclipse_how(NUM2DBL(tjd), NUM2INT(ifl), geopos, attr, serr);
	if (result < 0)
		rb_raise(rb_eRuntimeError, serr);

	VALUE output = rb_ary_new();
	rb_ary_push(output, INT2NUM(result)); // eclipse type flags
	for (int i = 0; i < 20; i++)
		rb_ary_push(output, rb_float_new(attr[i]));
	return output;
}

void Init_swe4r()
{
	// Module
	rb_mSwe4r = rb_define_module("Swe4r");

	// Module Functions
	rb_define_module_function(rb_mSwe4r, "swe_set_ephe_path", t_swe_set_ephe_path, 1);
	rb_define_module_function(rb_mSwe4r, "swe_set_jpl_file", t_swe_set_jpl_file, 1);
	rb_define_module_function(rb_mSwe4r, "swe_close", t_swe_close, 0);
	rb_define_module_function(rb_mSwe4r, "swe_version", t_swe_version, 0);
	rb_define_module_function(rb_mSwe4r, "swe_get_planet_name", t_swe_get_planet_name, 1);
	rb_define_module_function(rb_mSwe4r, "swe_get_ayanamsa_name", t_swe_get_ayanamsa_name, 1);
	rb_define_module_function(rb_mSwe4r, "swe_julday", t_swe_julday, -1);
	rb_define_module_function(rb_mSwe4r, "swe_revjul", t_swe_revjul, -1);
	rb_define_module_function(rb_mSwe4r, "swe_utc_to_jd", t_swe_utc_to_jd, -1);
	rb_define_module_function(rb_mSwe4r, "swe_jdut1_to_utc", t_swe_jdut1_to_utc, -1);
	rb_define_module_function(rb_mSwe4r, "swe_day_of_week", t_swe_day_of_week, 1);
	rb_define_module_function(rb_mSwe4r, "swe_set_topo", t_swe_set_topo, 3);
	rb_define_module_function(rb_mSwe4r, "swe_calc_ut", t_swe_calc_ut, 3);
	rb_define_module_function(rb_mSwe4r, "swe_sidtime", t_swe_sidtime, 1);
	rb_define_module_function(rb_mSwe4r, "swe_sidtime0", t_swe_sidtime0, 3);
	rb_define_module_function(rb_mSwe4r, "swe_degnorm", t_swe_degnorm, 1);
	rb_define_module_function(rb_mSwe4r, "swe_radnorm", t_swe_radnorm, 1);
	rb_define_module_function(rb_mSwe4r, "swe_split_deg", t_swe_split_deg, 2);
	rb_define_module_function(rb_mSwe4r, "swe_set_sid_mode", t_swe_set_sid_mode, 3);
	rb_define_module_function(rb_mSwe4r, "swe_get_ayanamsa_ut", t_swe_get_ayanamsa_ut, 1);
	rb_define_module_function(rb_mSwe4r, "swe_get_ayanamsa_ex_ut", t_swe_get_ayanamsa_ex_ut, 2);
	rb_define_module_function(rb_mSwe4r, "swe_houses", t_swe_houses, 4);
	rb_define_module_function(rb_mSwe4r, "swe_houses_ex", t_swe_houses_ex, 5);
	rb_define_module_function(rb_mSwe4r, "swe_houses_ex2", t_swe_houses_ex2, 5);
	rb_define_module_function(rb_mSwe4r, "swe_houses_armc", t_swe_houses_armc, 4);
	rb_define_module_function(rb_mSwe4r, "swe_house_name", t_swe_house_name, 1);
	rb_define_module_function(rb_mSwe4r, "swe_house_pos", t_swe_house_pos, 6);
	rb_define_module_function(rb_mSwe4r, "swe_rise_trans", t_swe_rise_trans, 9);
	rb_define_module_function(rb_mSwe4r, "swe_rise_trans_true_hor", t_swe_rise_trans_true_hor, 10);
	rb_define_module_function(rb_mSwe4r, "swe_azalt", t_swe_azalt, 10);
	rb_define_module_function(rb_mSwe4r, "swe_azalt_rev", t_swe_azalt_rev, 7);
	rb_define_module_function(rb_mSwe4r, "swe_refrac", t_swe_refrac, 4);
	rb_define_module_function(rb_mSwe4r, "swe_pheno_ut", t_swe_pheno_ut, 3);
	rb_define_module_function(rb_mSwe4r, "swe_time_equ", t_swe_time_equ, 1);
	rb_define_module_function(rb_mSwe4r, "swe_lmt_to_lat", t_swe_lmt_to_lat, 2);
	rb_define_module_function(rb_mSwe4r, "swe_lat_to_lmt", t_swe_lat_to_lmt, 2);
	rb_define_module_function(rb_mSwe4r, "swe_cotrans", t_swe_cotrans, -1);
	rb_define_module_function(rb_mSwe4r, "swe_cotrans_sp", t_swe_cotrans_sp, 7);
	rb_define_module_function(rb_mSwe4r, "swe_solcross_ut", t_swe_solcross_ut, 3);
	rb_define_module_function(rb_mSwe4r, "swe_mooncross_ut", t_swe_mooncross_ut, 3);
	rb_define_module_function(rb_mSwe4r, "swe_mooncross_node_ut", t_swe_mooncross_node_ut, 2);
	rb_define_module_function(rb_mSwe4r, "swe_helio_cross_ut", t_swe_helio_cross_ut, 5);
	rb_define_module_function(rb_mSwe4r, "swe_nod_aps_ut", t_swe_nod_aps_ut, 4);
	rb_define_module_function(rb_mSwe4r, "swe_calc_pctr", t_swe_calc_pctr, 4);
	rb_define_module_function(rb_mSwe4r, "swe_get_orbital_elements", t_swe_get_orbital_elements, 3);
	rb_define_module_function(rb_mSwe4r, "swe_deltat", t_swe_deltat, 1);
	rb_define_module_function(rb_mSwe4r, "swe_deltat_ex", t_swe_deltat_ex, 2);
	rb_define_module_function(rb_mSwe4r, "swe_fixstar", t_swe_fixstar, 3);
	rb_define_module_function(rb_mSwe4r, "swe_fixstar_ut", t_swe_fixstar_ut, 3);
	rb_define_module_function(rb_mSwe4r, "swe_fixstar_mag", t_swe_fixstar_mag, 1);
	rb_define_module_function(rb_mSwe4r, "swe_fixstar2", t_swe_fixstar2, 3);
	rb_define_module_function(rb_mSwe4r, "swe_fixstar2_ut", t_swe_fixstar2_ut, 3);
	rb_define_module_function(rb_mSwe4r, "swe_fixstar2_mag", t_swe_fixstar2_mag, 1);
	rb_define_module_function(rb_mSwe4r, "swe_sol_eclipse_when_glob", t_swe_sol_eclipse_when_glob, 4);
	rb_define_module_function(rb_mSwe4r, "swe_sol_eclipse_when_loc", t_swe_sol_eclipse_when_loc, 6);
	rb_define_module_function(rb_mSwe4r, "swe_sol_eclipse_how", t_swe_sol_eclipse_how, 5);
	rb_define_module_function(rb_mSwe4r, "swe_sol_eclipse_where", t_swe_sol_eclipse_where, 2);
	rb_define_module_function(rb_mSwe4r, "swe_lun_eclipse_when", t_swe_lun_eclipse_when, 4);
	rb_define_module_function(rb_mSwe4r, "swe_lun_eclipse_when_loc", t_swe_lun_eclipse_when_loc, 6);
	rb_define_module_function(rb_mSwe4r, "swe_lun_eclipse_how", t_swe_lun_eclipse_how, 5);
	rb_define_module_function(rb_mSwe4r, "swe_gauquelin_sector", t_swe_gauquelin_sector, 9);
	rb_define_module_function(rb_mSwe4r, "swe_heliacal_ut", t_swe_heliacal_ut, -1);
	rb_define_module_function(rb_mSwe4r, "swe_vis_limit_mag", t_swe_vis_limit_mag, -1);

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

	rb_define_const(rb_mSwe4r, "SE_NODBIT_MEAN", INT2FIX(SE_NODBIT_MEAN));
	rb_define_const(rb_mSwe4r, "SE_NODBIT_OSCU", INT2FIX(SE_NODBIT_OSCU));
	rb_define_const(rb_mSwe4r, "SE_NODBIT_OSCU_BAR", INT2FIX(SE_NODBIT_OSCU_BAR));
	rb_define_const(rb_mSwe4r, "SE_NODBIT_FOPOINT", INT2FIX(SE_NODBIT_FOPOINT));

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

	/* for swe_azalt_rev() */
	rb_define_const(rb_mSwe4r, "SE_HOR2ECL", INT2FIX(SE_HOR2ECL));
	rb_define_const(rb_mSwe4r, "SE_HOR2EQU", INT2FIX(SE_HOR2EQU));

	/* for swe_refrac() */
	rb_define_const(rb_mSwe4r, "SE_TRUE_TO_APP", INT2FIX(SE_TRUE_TO_APP));
	rb_define_const(rb_mSwe4r, "SE_APP_TO_TRUE", INT2FIX(SE_APP_TO_TRUE));

	/* for swe_split_deg() */
	rb_define_const(rb_mSwe4r, "SE_SPLIT_DEG_ROUND_SEC", INT2FIX(SE_SPLIT_DEG_ROUND_SEC));
	rb_define_const(rb_mSwe4r, "SE_SPLIT_DEG_ROUND_MIN", INT2FIX(SE_SPLIT_DEG_ROUND_MIN));
	rb_define_const(rb_mSwe4r, "SE_SPLIT_DEG_ROUND_DEG", INT2FIX(SE_SPLIT_DEG_ROUND_DEG));
	rb_define_const(rb_mSwe4r, "SE_SPLIT_DEG_ZODIACAL", INT2FIX(SE_SPLIT_DEG_ZODIACAL));
	rb_define_const(rb_mSwe4r, "SE_SPLIT_DEG_NAKSHATRA", INT2FIX(SE_SPLIT_DEG_NAKSHATRA));
	rb_define_const(rb_mSwe4r, "SE_SPLIT_DEG_KEEP_SIGN", INT2FIX(SE_SPLIT_DEG_KEEP_SIGN));
	rb_define_const(rb_mSwe4r, "SE_SPLIT_DEG_KEEP_DEG", INT2FIX(SE_SPLIT_DEG_KEEP_DEG));

	/* for eclipse functions */
	rb_define_const(rb_mSwe4r, "SE_ECL_CENTRAL", INT2FIX(SE_ECL_CENTRAL));
	rb_define_const(rb_mSwe4r, "SE_ECL_NONCENTRAL", INT2FIX(SE_ECL_NONCENTRAL));
	rb_define_const(rb_mSwe4r, "SE_ECL_TOTAL", INT2FIX(SE_ECL_TOTAL));
	rb_define_const(rb_mSwe4r, "SE_ECL_ANNULAR", INT2FIX(SE_ECL_ANNULAR));
	rb_define_const(rb_mSwe4r, "SE_ECL_PARTIAL", INT2FIX(SE_ECL_PARTIAL));
	rb_define_const(rb_mSwe4r, "SE_ECL_ANNULAR_TOTAL", INT2FIX(SE_ECL_ANNULAR_TOTAL));
	rb_define_const(rb_mSwe4r, "SE_ECL_HYBRID", INT2FIX(SE_ECL_HYBRID));
	rb_define_const(rb_mSwe4r, "SE_ECL_PENUMBRAL", INT2FIX(SE_ECL_PENUMBRAL));
	rb_define_const(rb_mSwe4r, "SE_ECL_ALLTYPES_SOLAR", INT2FIX(SE_ECL_ALLTYPES_SOLAR));
	rb_define_const(rb_mSwe4r, "SE_ECL_ALLTYPES_LUNAR", INT2FIX(SE_ECL_ALLTYPES_LUNAR));
	rb_define_const(rb_mSwe4r, "SE_ECL_VISIBLE", INT2FIX(SE_ECL_VISIBLE));
	rb_define_const(rb_mSwe4r, "SE_ECL_MAX_VISIBLE", INT2FIX(SE_ECL_MAX_VISIBLE));
	rb_define_const(rb_mSwe4r, "SE_ECL_1ST_VISIBLE", INT2FIX(SE_ECL_1ST_VISIBLE));
	rb_define_const(rb_mSwe4r, "SE_ECL_2ND_VISIBLE", INT2FIX(SE_ECL_2ND_VISIBLE));
	rb_define_const(rb_mSwe4r, "SE_ECL_3RD_VISIBLE", INT2FIX(SE_ECL_3RD_VISIBLE));
	rb_define_const(rb_mSwe4r, "SE_ECL_4TH_VISIBLE", INT2FIX(SE_ECL_4TH_VISIBLE));
	rb_define_const(rb_mSwe4r, "SE_ECL_ONE_TRY", INT2FIX(SE_ECL_ONE_TRY));

	/* for heliacal functions */
	rb_define_const(rb_mSwe4r, "SE_HELIACAL_RISING", INT2FIX(SE_HELIACAL_RISING));
	rb_define_const(rb_mSwe4r, "SE_HELIACAL_SETTING", INT2FIX(SE_HELIACAL_SETTING));
	rb_define_const(rb_mSwe4r, "SE_MORNING_FIRST", INT2FIX(SE_MORNING_FIRST));
	rb_define_const(rb_mSwe4r, "SE_EVENING_LAST", INT2FIX(SE_EVENING_LAST));
	rb_define_const(rb_mSwe4r, "SE_EVENING_FIRST", INT2FIX(SE_EVENING_FIRST));
	rb_define_const(rb_mSwe4r, "SE_MORNING_LAST", INT2FIX(SE_MORNING_LAST));
	rb_define_const(rb_mSwe4r, "SE_HELFLAG_LONG_SEARCH", INT2FIX(SE_HELFLAG_LONG_SEARCH));
	rb_define_const(rb_mSwe4r, "SE_HELFLAG_HIGH_PRECISION", INT2FIX(SE_HELFLAG_HIGH_PRECISION));
	rb_define_const(rb_mSwe4r, "SE_HELFLAG_OPTICAL_PARAMS", INT2FIX(SE_HELFLAG_OPTICAL_PARAMS));
	rb_define_const(rb_mSwe4r, "SE_HELFLAG_NO_DETAILS", INT2FIX(SE_HELFLAG_NO_DETAILS));
}
