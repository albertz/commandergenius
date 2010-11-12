/* -*- C++ -*- */
#ifndef _AARG_H_
#define _AARG_H_

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <map>
#include <string>
#include <sstream>

class Aargh
{
public:
  Aargh ()
  {
  }

// questo costruttore serve per avere i parametri caricati
// prima dell'avvio di main, cosi' da poter chiamare i
// costruttori degli oggetti globali che dipendono da
// dei parametri
  Aargh (char *filename)
  {
    loadConf (filename);
  }

// questi altri due costruttori sono meno necessari ma
// sono comunque utili
  Aargh (int argc, char **argv)
  {
    parseArgs (argc, argv);
  }

// vedi sopra
  Aargh (int argc, char **argv, char *filename)
  {
    parseArgs (argc, argv);
    loadConf (filename);
  }

// parametri accettati
//  -qualcosa
//  -qualcosa valore
  bool parseArgs (int argc, char **argv)
  {
    bool allright = true;
    std::string base = "";
    for (int i = 1; i < argc; ++i)
      {
	if (argv[i][0] == '-')
	  {
	    base = std::string (argv[i]);
	    argmap[base] = "_Aargh";
	  }
	else
	  {
	    if (base != "")
	      {
		argmap[base] = std::string (argv[i]);
		base = "";
	      }
	    else
	      allright = false;
	  }
      }
    return allright;
  }

  bool loadConf (const char *filename)
  {
    bool allright = true;
    FILE *fp;
    char ln[128];

    if ((fp = fopen (filename, "r")) == NULL)
      return (false);

    while (fgets (ln, 128, fp))
      if (*ln != '#' && *ln != '\n')
	{
	  char *key = ln;
	  char *value = ln;
	  while ((*value != ' ') && (*value != '\t') && (*value != '\n'))
	    value++;		// finds the first space or tab
	  if (*value == '\n')
	    {
	      *value = '\0';
	      argmap[key] = "_Aargh";
	      continue;
	    }
	  // removes spaces and tabs
	  while ((*value == ' ') || (*value == '\t'))
	    {
	      *value = '\0';
	      value++;
	    }
	  char *end = value;
	  while ((*end != '\n') && (*end))
	    end++;
	  *end = '\0';		// null terminates value (fgets puts a '\n' at the end)
	  // now, key is a null terminated string holding the key, and value is everything which
	  // is found after the first series of spaces or tabs.
	  if (strcmp (key, "") && strcmp (value, ""))
	    argmap[key] = value;
	}

    return allright;
  }

  bool getArg (std::string name)
  {
    return (argmap.find (name) != argmap.end ());
  }

  bool getArg (std::string name, std::string & value)
  {
    if (argmap.find (name) != argmap.end ())
      {
	value = argmap[name];
	return true;
      }
    else
      return false;
  }

  bool getArg (std::string name, std::string & value, const char* def)
  {
    if ( getArg(name, value) )
      return true;
    else
      value = std::string(def);
    
    return false;
  }

  bool setArg (std::string name, std::string value)
  {
    bool ret = (argmap.find (name) != argmap.end ());
    // std::cerr << "Setting " << name << " to: " << value << std::endl;
    argmap[name] = value;
    return(ret);
  }

  bool setArg (std::string name, int intvalue)
  {
    std::ostringstream value;

    value << intvalue;

    bool ret = (argmap.find (value.str()) != argmap.end ());
    // std::cerr << "Setting " << name << " to: " << value.str() << std::endl;
    argmap[name] = value.str();
    return(ret);
  }

  void dump (std::string & out)
  {
    std::map < std::string, std::string >::const_iterator b = argmap.begin ();
    std::map < std::string, std::string >::const_iterator e = argmap.end ();
    out = "";
    while (b != e)
      {
	out += b->first + ' ' + b->second + '\n';
	++b;
      }
  }

  void reset() {
    argmap.clear();
  }

private:
  std::map < std::string, std::string > argmap;
};

extern Aargh aargh;

#endif // _AARG_H_
