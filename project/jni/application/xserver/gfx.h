#ifndef _XSDL_GFX_H_
#define _XSDL_GFX_H_

enum { VID_X = 480, VID_Y = 320 };

void XSDL_initSDL();
void XSDL_deinitSDL();
void XSDL_unpackFiles();
void XSDL_showConfigMenu(int * resolutionW, int * displayW, int * resolutionH, int * displayH);

#endif
