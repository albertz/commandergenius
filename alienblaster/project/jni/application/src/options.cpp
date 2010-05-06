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
using namespace std;

#include "options.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <sstream>

Options::Options(const string lFilename) {
  if (lFilename == "") { 
    cout << "Options: no filename given!" << endl;
    filename = string("config");
  } else {
    // save the filename for later use
    filename = lFilename;
  }
  
  // open the file for reading
  ifstream inputFile (filename.c_str(), ios::in);

  //  TODO FIXME ordentliches Filelocking machen
  //  fcntl(inputFile.rdbuf()->fd(), F_SETLKW, fileLock(F_RDLCK));

  if (inputFile.good()) {
    // while still data there
    while (!inputFile.eof()) {
      string newLine;
      getline( inputFile, newLine );
      // search delimiter
      unsigned int pos = newLine.find("=");
      // comment line or no delimiter found
      if (( newLine[0] != '#' ) && ( pos != string::npos )) {
	string key = newLine.substr(0,pos);
	string val = newLine.substr(pos+1);
	keymap[ key ] = val;
      }
    }
  }
  else {
    cout << "Options: Error while opening " << filename << endl;
  }

  //  TODO FIXME ordentliches Filelocking machen
  //  fcntl(inputFile.rdbuf()->fd(), F_SETLKW, fileLock(F_UNLCK));

  inputFile.close();
//   if (inputFile.fail()) {
//     cout << "Options: Error while closing " << filename << endl;
//   }
}


Options::~Options() {
  ;
}


// flock* Options::fileLock(const short type) {
//   static flock ret ;
//   ret.l_type = type ;
//   ret.l_start = 0 ;
//   ret.l_whence = SEEK_SET ;
//   ret.l_len = 0 ;
//   ret.l_pid = getpid() ;
//   return &ret ;
// }


int Options::saveFile( const string lFilename ) {
  // alternative filename given ?
  if (lFilename != "") {
    filename = lFilename;
  }

  // delete original options-file
  remove(filename.c_str());

  ofstream outputFile( filename.c_str(), ios::out );

  //  TODO FIXME ordentliches Filelocking machen
  //  fcntl(outputFile.rdbuf()->fd(), F_SETLKW, fileLock(F_WRLCK));

  if (outputFile.good()) {
    map< string, string >::const_iterator iter;
    for(iter = keymap.begin(); iter != keymap.end(); iter++){
      outputFile << iter->first << "=" << iter->second << endl;
    }
  }
  else {
    cout << "Options::saveFile(): error while opening file " << filename << endl;
    return -1;
  }

  //  TODO FIXME ordentliches Filelocking machen
  //  fcntl(outputFile.rdbuf()->fd(), F_SETLKW, fileLock(F_UNLCK));

  outputFile.close();
  if (!outputFile.good()) {
    cout << "Options::saveFile(): error while closing file " << filename << endl;
    return -2;
  }
  
  return 0;  // everything's fine
}
  

bool Options::exist(const string keyword) {
  return (keymap.find(keyword) != keymap.end());
}


bool Options::getStr(const string keyword, string &val) {
  if (exist(keyword)) {
    val = keymap[keyword];
    return true;
  }
  return false;
}


bool Options::getInt(const string keyword, int &val) {
  if (exist(keyword)) {
    val = atoi( keymap[keyword].c_str() );
    return true;
  }
  return false;
}


bool Options::getUInt(const string keyword, unsigned int &val) {
  if (exist(keyword)) {
    val = atoll( keymap[keyword].c_str() );
    return true;
  }
  return false;
}

	    
bool Options::setStr(const string newValue, const string keyword) {
  keymap[keyword] = newValue;
  return true;
}


bool Options::setInt(const int newValue, const string keyword) {
  keymap[keyword] = asString<int>( newValue );
  return true;
}


bool Options::setUInt(const unsigned int newValue, const string keyword) {
  keymap[keyword] = asString<unsigned int>( newValue );
  return true;
}


void Options::printall() {
  map< string, string >::const_iterator iter;
  for(iter = keymap.begin(); iter != keymap.end(); iter++){
    cout << iter->first << "=" << iter->second << endl;
  }
}
