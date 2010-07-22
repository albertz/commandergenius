/*************************************************************************** 
  alienBlaster 
  Copyright (C) 2004 
  Paul Grathwohl, Arne Hormann, Daniel Kuehn, Soenke Schwardt

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2, or (at your option)
  any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
***************************************************************************/

/**
 *  @file Options.h
 *  @brief The Options-class can manage a configuration file.
 *  @date 23.06.02
 *  The Options-class can manage a configuration file of this style:
 *  KEYWORD=value
 *  Provides easy-access functions for the keywords.
 *  $Id: options.h,v 1.2 2003/12/08 18:21:21 schwardt Exp $
 */

#ifndef __OPTIONS_H__
#define __OPTIONS_H__


#include <map>
#include <string>
#include <fstream>
#include <unistd.h>
#include <fcntl.h>
#include "asstring.h"

class Options {

  private:
  std::map< std::string, std::string > keymap;
  std::string filename;

  public:
  /// Constructor. Reads the configurationfile specified by lFilename.
  Options( const std::string lFilename );
  /// Destructor. Frees all memory allocated.
  ~Options();

  /// Returns structure used for filelocking
  //  flock* Options::fileLock(const short type);   // FIXME
  /// Returns true if the given keyword exist
  bool exist(const std::string keyword);
  /// Returns the int value of keyword.
  /// If keyword is not found returns 0.
  bool getInt( const std::string keyword, int &val );
  /// Returns the unsigned int value of keyword.
  /// If keyword is not found returns 0.
  bool getUInt( const std::string keyword, unsigned int &val );
  /// Returns a copy of the char* value of keyword.
  /// If keyword is not found returns NULL.
  bool getStr( const std::string keyword, std::string &val );
  
  /// sets the value of the given keyword to newValue
  bool setInt( const int newValue, const std::string keyword );
  /// sets the value of the given keyword to newValue
  bool setUInt( const unsigned int newValue, const std::string keyword );
  /// sets the value of the given keyword to newValue
  bool setStr( const std::string newValue, const std::string keyword );

  /// Stores the config-File to disk (filename as specified in the constructor).
  /// You should call this function after changing a value.
  int saveFile( const std::string lFilename );

  /// Prints all lines of the actual configuration to cout.
  void printall();
  
};

#endif
