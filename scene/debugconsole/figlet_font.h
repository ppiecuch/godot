/*************************************************************************/
/*  figlet_font.h                                                        */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                      https://godotengine.org                          */
/*************************************************************************/
/* Copyright (c) 2007-2021 Juan Linietsky, Ariel Manzur.                 */
/* Copyright (c) 2014-2021 Godot Engine contributors (cf. AUTHORS.md).   */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/

/*
Copyright (c) 2014, Enrico Bertolazzi (email: enrico.bertolazzi@unitn.it)
All rights reserved.
*/

// check for bugged compiler
#ifdef _MSC_VER
#define EMBED_FIGLET_USE_VISUAL_STUDIO
#endif

// http://ruletheweb.co.uk/figlet/

/*!
  \mainpage  Embeddable Figlet Library
  \author    Enrico Bertolazzi (enrico.bertolazzi@unitn.it), homepage: http://www.ing.unitn.it/~bertolaz
  \version   1.0.2
  \date      2013

  \details

  This library available at

  - https://github.com/ebertolazzi/embedFiglet
  - https://bitbucket.org/ebertolazzi/embedfiglet

  implement a subset of Figlet capability, i.e.
  print large letters out of ordinary text

  ~~~~~~~~~~~~~
   _ _ _          _   _     _
  | (_) | _____  | |_| |__ (_)___
  | | | |/ / _ \ | __| '_ \| / __|
  | | |   <  __/ | |_| | | | \__ \_
  |_|_|_|\_\___|  \__|_| |_|_|___(_)

  ~~~~~~~~~~~~~

  Differently of standard FIGlet command (http://www.figlet.org/)
  this print can be done inside a user program using run time
  generated string as follows

  ~~~~~~~~~~~~~{.c++}
  #include "Figlet.hh"
  #include <sstream>
  using namespace std;

  int
  main() {
	Figlet::standard.print("Fractions");
	for ( int i = 2; i <= 4; ++i ) {
	  ostringstream ss;
	  ss << "5/" << i << " = " << 5.0/i;
	  Figlet::small.print(ss.str().c_str());
	}
	cout << "ALL DONE!\n";
	return 0;
  }
  ~~~~~~~~~~~~~

  which output is the following

  ~~~~~~~~~~~~~
   _____               _   _
  |  ___| __ __ _  ___| |_(_) ___  _ __  ___
  | |_ | '__/ _` |/ __| __| |/ _ \| '_ \/ __|
  |  _|| | | (_| | (__| |_| | (_) | | | \__ \
  |_|  |_|  \__,_|\___|\__|_|\___/|_| |_|___/

   ___   _____         ___   ___
  | __| / /_  )  ___  |_  ) | __|
  |__ \/ / / /  |___|  / / _|__ \
  |___/_/ /___| |___| /___(_)___/

   ___   ______        _    __  __  __  ______
  | __| / /__ /  ___  / |  / / / / / / / /__  |
  |__ \/ / |_ \ |___| | |_/ _ Y _ Y _ Y _ \/ /
  |___/_/ |___/ |___| |_(_)___|___|___|___/_/

   ___   ___ _          _   ___ ___
  | __| / / | |   ___  / | |_  ) __|
  |__ \/ /|_  _| |___| | |_ / /|__ \
  |___/_/   |_|  |___| |_(_)___|___/
  ~~~~~~~~~~~~~

  No initialization files are necessary.
  The fonts are hardware embedded in the library.
  The available fonts (modified by P.P.) are:

  - Future
  - Calvin S
  - ANSI Regular
  - DOS Rebel
  - maxiwi
  - maxii

  A simple ruby script permits to convert figlet .flf
  (and some .tlf - P.P.) files to structures which can
  be embedded in the library.

  - \ref printmode
  - \ref framemode

 */

/*!
  \page printmode Available printing mode

  Only four mode of print the same string:

	- FIGLET_SMUSHED     standard way of figlet print
	- FIGLET_PACKED      letters are moved left but no overlapping are permitted
	- FIGLET_FULLWIDTH   letters are printed at width defined in the font
	- FIGLET_MONOSPACED  letters are printed with equal width

  The effect are the following
  ~~~~~~~~~~~~~~~~~~
   ____                      _              _
  / ___| _ __ ___  _   _ ___| |__   ___  __| |
  \___ \| '_ ` _ \| | | / __| '_ \ / _ \/ _` |
   ___) | | | | | | |_| \__ \ | | |  __/ (_| |
  |____/|_| |_| |_|\__,_|___/_| |_|\___|\__,_|
   ____               _              _
  |  _ \  __ _   ___ | | __ ___   __| |
  | |_) |/ _` | / __|| |/ // _ \ / _` |
  |  __/| (_| || (__ |   <|  __/| (_| |
  |_|    \__,_| \___||_|\_\\___| \__,_|
   _____           _   _  __        __  _       _   _     _
  |  ___|  _   _  | | | | \ \      / / (_)   __| | | |_  | |__
  | |_    | | | | | | | |  \ \ /\ / /  | |  / _` | | __| | '_ \
  |  _|   | |_| | | | | |   \ V  V /   | | | (_| | | |_  | | | |
  |_|      \__,_| |_| |_|    \_/\_/    |_|  \__,_|  \__| |_| |_|
   __  __                                                                       _
  |  \/  |   ___    _ __     ___     ___    _ __     __ _     ___     ___    __| |
  | |\/| |  / _ \  | '_ \   / _ \   / __|  | '_ \   / _` |   / __|   / _ \  / _` |
  | |  | | | (_) | | | | | | (_) |  \__ \  | |_) | | (_| |  | (__   |  __/ | (_| |
  |_|  |_|  \___/  |_| |_|  \___/   |___/  | .__/   \__,_|   \___|   \___|  \__,_|
										   |_|
  ~~~~~~~~~~~~~~~~~~
 */

/*!
  \page framemode Available framing mode

  Only two framing are available:

	- FIGLET_SINGLE  single frame around a string
	- FIGLET_DOUBLE  double frame around a string

  The effect are the following

  ~~~~~~~~~~~~~~~~~~
  Figlet::small.printFramed("SINGLE",cout,Figlet::FIGLET_SINGLE);
  +------------------------------+
  |  ___ ___ _  _  ___ _    ___  |
  | / __|_ _| \| |/ __| |  | __| |
  | \__ \| || .` | (_ | |__| _|  |
  | |___/___|_|\_|\___|____|___| |
  |                              |
  +------------------------------+
  ~~~~~~~~~~~~~~~~~~
  and
  ~~~~~~~~~~~~~~~~~~
  Figlet::small.printFramed("DOUBLE",cout,Figlet::FIGLET_DOUBLE);
  @=================================@
  #  ___   ___  _   _ ___ _    ___  #
  # |   \ / _ \| | | | _ ) |  | __| #
  # | |) | (_) | |_| | _ \ |__| _|  #
  # |___/ \___/ \___/|___/____|___| #
  #                                 #
  @=================================@
  ~~~~~~~~~~~~~~~~~~
 */

#ifndef FIGLET_FONT_H
#define FIGLET_FONT_H

#include <stdint.h>
#include <iostream>

/*! \brief
 * Collects structures and classes for banner generation
 */
namespace Figlet {

typedef std::basic_ostream<char> ostream_type;

static unsigned const maxHeight = 11; //!< maximum allowed (lines) height of the font
static unsigned const maxLenght = 256; //!< maximum number of characters x line of the banner
static unsigned const maxTableSize = 256; //!< maximum number of allowed character x font

//! Structure used to store a charater of the font
typedef struct {
	unsigned short nchar; //!< character (ascii) number
	uint8_t lspaces[maxHeight]; //!< number of spaces on the left side x line
	uint8_t rspaces[maxHeight]; //!< number of spaces on the right side x line
	char const *rows[maxHeight]; //!< charater definition
} FontFiglet;

//! Available way to print a string, see \ref printmode
typedef enum {
	FIGLET_SMUSHED = 0,
	FIGLET_PACKED,
	FIGLET_FULLWIDTH,
	FIGLET_MONOSPACED,
} PrintMode;

//! Available way to print a frames string, see \ref framemode
typedef enum {
	FIGLET_SINGLE = 0,
	FIGLET_DOUBLE,
} FrameMode;

//! Class implementing the "figlet" algorithm
class Banner {
	FontFiglet const *characters; //!< pointer to the font structures
	char const Hardblank; //!< character used for the "hardblank" in the font definition
	unsigned const Height; //!< vertical dimension (lines) of the font
	unsigned Width; //!< width of the charater M used in Monospaced print
	unsigned const FontMaxLen; //!< maximum width of the letters of the font
	unsigned const FontSize; //!< total number of characters in the font
	uint8_t rspaces[maxHeight]; //!< extra right spaces availables after the last insertion
	char lines[maxHeight][maxLenght]; //!< lines buffer
	char smush[maxHeight]; //!< charater used in the "smushing" algorithm
	unsigned short charToTable[maxTableSize]; //!< map ascii character to font structure
	unsigned short charWidth[maxTableSize]; //!< size width of each charater of the font
	unsigned charPosition; //!< position of last inserted character

	const char *mappingFrom; //!< map from set of these characters
	const char *mappingTo; //!< to these characters

	PrintMode printMode; //!< the type of printing mode used

	Banner const &operator=(Banner const &);
	Banner(Banner const &);

	//! evaluate smushing rules for 2 characters, return '\0' if no rules apply
	char smushingRules(char left, char right) const;

	bool pushMonospaced(unsigned c);
	bool pushFullWidth(unsigned c);
	bool pushPacked(unsigned c);
	bool pushSmushed(unsigned c);
	void fillForPrint(char const message[]);

public:
	//! Constructor of `Banner` class
	/*!
	:|: \param characters none
	:|: \param Hardblank  none
	:|: \param Height     none
	:|: \param FontMaxLen none
	:|: \param FontSize   none
	\*/
	explicit Banner(FontFiglet const *characters,
			char Hardblank,
			unsigned Height,
			unsigned FontMaxLen,
			unsigned FontSize,
			PrintMode PrintMode = FIGLET_PACKED,
			const char *MappingFrom = 0,
			const char *MappingTo = 0);

	//! initialize Banner class
	void init();

	//! Set print mode to `monospaced`, see \ref printmode
	void setMonospaced() { printMode = FIGLET_MONOSPACED; }
	Banner &monospaced() {
		setMonospaced();
		return *this;
	}

	//! Set print mode to `full width`, see \ref printmode
	void setFullWidth() { printMode = FIGLET_FULLWIDTH; }
	Banner &fullWidth() {
		setFullWidth();
		return *this;
	}

	//! Set print mode to `packed`, see \ref printmode
	void setPacked() { printMode = FIGLET_PACKED; }
	Banner &packed() {
		setPacked();
		return *this;
	}

	//! Set print mode to `smushed` (figlet default), see \ref printmode
	void setSmushed() { printMode = FIGLET_SMUSHED; }
	Banner &smushed() {
		setSmushed();
		return *this;
	}

	//! Print large letters of string `message` on stream `s`, see \ref printmode
	unsigned
	print(
			char const message[],
			ostream_type &s = std::cout,
			char const top[] = "",
			char const bottom[] = "");

	//! \ref framemode
	void
	printFramed(
			char const message[],
			ostream_type &s = std::cout,
			FrameMode fm = FIGLET_SINGLE);
};

extern Banner future; //!< instance `Banner` class using figlet font `future`
extern Banner calvins; //!< instance `Banner` class using figlet font `calvins`
extern Banner maxii; //!< instance `Banner` class using figlet font `maxii`
extern Banner maxiwi; //!< instance `Banner` class using figlet font `maxiwi`
extern Banner dosrebel; //!< instance `Banner` class using figlet font `dosrebel`
extern Banner ansiregular; //!< instance `Banner` class using figlet font `ansiregular`
} // namespace Figlet

#endif // FIGLET_FONT_H
