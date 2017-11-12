/*
 * FONAPlatform.h -- platform definitions includes.
 *
 * This is part of the library for the Adafruit FONA Cellular Module
 *
 * Designed specifically to work with the Adafruit FONA
 * ----> https://www.adafruit.com/products/1946
 * ----> https://www.adafruit.com/products/1963
 * ----> http://www.adafruit.com/products/2468
 * ----> http://www.adafruit.com/products/2542
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * Written by Pat Deegan, http://flyingcarsandstuff.com, for inclusion in
 * the Adafruit_FONA_Library and released under the
 * BSD license, all text above must be included in any redistribution.
 *
 *  Created on: Jan 16, 2016
 *      Author: Pat Deegan
 */


#ifndef ADAFRUIT_FONA_LIBRARY_SRC_INCLUDES_PLATFORM_FONAPLATFORM_H_
#define ADAFRUIT_FONA_LIBRARY_SRC_INCLUDES_PLATFORM_FONAPLATFORM_H_

#include "../FONAConfig.h"

// only "standard" config supported in this release -- namely AVR-based arduino type affairs
#include "FONAPlatStd.h"



#ifndef DEBUG_PRINT
// debug is disabled

#define DEBUG_PRINT(...)
#define DEBUG_PRINTLN(...)

#endif


#ifndef prog_char_strcmp
#define prog_char_strcmp(a, b)					strcmp((a), (b))
#endif

#ifndef prog_char_strstr
#define prog_char_strstr(a, b)					strstr((a), (b))
#endif

#ifndef prog_char_strlen
#define prog_char_strlen(a)						strlen((a))
#endif


#ifndef prog_char_strcpy
#define prog_char_strcpy(to, fromprogmem)		strcpy((to), (fromprogmem))
#endif


#endif /* ADAFRUIT_FONA_LIBRARY_SRC_INCLUDES_PLATFORM_FONAPLATFORM_H_ */
