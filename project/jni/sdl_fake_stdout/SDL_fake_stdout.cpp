#include <stdio.h>
#include <android/log.h>
#include <iostream>

/* Outputting anything to cout/cerr WILL CRASH YOUR PROGRAM on specific devices -
   x5a/x6d Android 2.1 tablet, and some other tablets,
   however the same code runs on my HTC Evo without problem.
   So I've just disabled cin/cout/cerr altogether.
*/

namespace std {

class _android_debugbuf: public std::streambuf
{
 public:
 _android_debugbuf()
 {
    pos = 0;
    buf[0] = 0;
 }

 protected:


virtual int overflow(int c = EOF)
{
	if (EOF == c)
	{
		return '\0';  // returning EOF indicates an error
	}
	else
	{
		outputchar(c);
		return c;
	}
};


// we don’t do input so always return EOF
virtual int uflow() {return EOF;}

// we don’t do input so always return 0 chars read
virtual int xsgetn(char *, int) {return 0;}

// Calls outputchar() for each character.
virtual int xsputn(const char *s, int n)
{
	for (int i = 0; i < n; ++i)
	{
		outputchar(s[i]);
	}
	return n;// we always process all of the chars
};

private:

// the buffer
char buf[256];
int pos;

void outputchar(char c)
{
	// TODO: mutex
	if( pos >= sizeof(buf)-1 || c == '\n' || c == '\r' || c == 0 )
	{
		buf[pos] = 0;
		__android_log_print(ANDROID_LOG_INFO, "libSDL", "%s", buf);
		pos = 0;
		buf[pos] = 0;
		return;
	};
	buf[pos] = c;
	pos++;
};

};

ostream __SDL_fake_cout(new _android_debugbuf());
ostream __SDL_fake_cerr(new _android_debugbuf());
ostream __SDL_fake_clog(new _android_debugbuf());
}

