#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <SDL/SDL.h>
#include <android/log.h>

extern int android_main( int argc, char *argv[], char *envp[] );
static int unpack_files();

int main( int argc, char* argv[] )
{
	char screenres[128] = "640x480x24";
	char clientcmd[PATH_MAX*2] = "xhost +";
	char * cmd = "";
	char* args[] = {
		"XSDL",
		":1111",
		"-nolock",
		"-noreset",
		"-screen",
		screenres,
		"-exec",
		clientcmd
	};
	char * envp[] = { NULL };
	
	sprintf( screenres, "%s/%sx%s/%sx%d",
		getenv("DISPLAY_RESOLUTION_WIDTH"),
		getenv("DISPLAY_WIDTH_MM"),
		getenv("DISPLAY_RESOLUTION_HEIGHT"),
		getenv("DISPLAY_HEIGHT_MM"),
		24 );

	if( argc >= 2 )
		cmd = argv[2];
	sprintf( clientcmd, "%s/usr/bin/xhost + ; %s",
		getenv("SECURE_STORAGE_DIR"), cmd );

	if( !unpack_files() )
	{
		__android_log_print(ANDROID_LOG_INFO, "XSDL", "Error while unpacking files, try to reinstall the app");
		return 1;
	}
	
	return android_main( 8, args, envp );
}

int unpack_files()
{
	char fname[PATH_MAX*2];
	strcpy( fname, getenv("SECURE_STORAGE_DIR") );
	strcat( fname, "/usr/bin/xkbcomp" );
	struct stat st;
	if( stat( fname, &st ) == 0 )
		return 1;

	strcpy( fname, getenv("SECURE_STORAGE_DIR") );
	strcat( fname, "/busybox" );
	FILE * ff = fopen("busybox", "rb");
	FILE * fo = fopen(fname, "wb");
	if( !ff || !fo )
		return 0;

	for(;;)
	{
		char buf[2048];
		int cnt = fread( buf, 1, sizeof(buf), ff );
		if( cnt < 0 )
			return 1;
		fwrite( buf, 1, cnt, fo );
		if( cnt < sizeof(buf) )
			break;
	}

	fclose(ff);
	fclose(fo);

	if( chmod(fname, 0755) != 0 )
		return 0;

	ff = fopen("data.tar.gz", "rb");
	strcat(fname, " tar xz -C ");
	strcat(fname, getenv("SECURE_STORAGE_DIR"));
	fo = popen(fname, "w");
	if( !ff || !fo )
		return 0;

	for(;;)
	{
		char buf[2048];
		int cnt = fread( buf, 1, sizeof(buf), ff );
		if( cnt < 0 )
			return 1;
		fwrite( buf, 1, cnt, fo );
		if( cnt < sizeof(buf) )
			break;
	}

	fclose(ff);
	if( pclose(fo) != 0 )
		return 0;

	return 1;
}
