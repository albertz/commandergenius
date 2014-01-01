#ifndef __ANDROID_DEBUG_H__
#define __ANDROID_DEBUG_H__

#include <stdio.h>
#include <android/log.h>

#ifdef __cplusplus
// Include everything beforehand, so we wont' get compiler eerors because of our #define
#include <string>
#include <ios>
#include <streambuf>
#include <sstream>
#include <fstream>
#include <iostream>

namespace std
{
	class android_cout: public ostringstream
	{
		public:
		android_cout() {}
		template <class T>
		android_cout &operator<<(const T &v)
		{
			*((ostringstream*)this) << v;
			if( this->str().find('\n') != ::std::string::npos )
			{
				__android_log_print(ANDROID_LOG_INFO, "SDL-app", "%s", this->str().c_str());
				this->str().clear();
			}
			return *this;
		}
		~android_cout()
		{
			__android_log_print(ANDROID_LOG_INFO, "SDL-app", "%s", this->str().c_str());
			this->str().clear();
		}
	};
	static const char * android_endl = "\n";
}
#define cout android_cout()
#define cerr android_cout()
#define endl android_endl

#endif

#define printf(...) __android_log_print(ANDROID_LOG_INFO, "SDL-app", __VA_ARGS__)

#endif
