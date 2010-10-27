/***************************************************************************
*                           Gfx Functions                                  *
*                           -------------                                  *
*                                                                          *
*                          T-1000@Bigfoot.com                              *
****************************************************************************
    *******************************************************************
    *                                                                 *
    *   This program is free software; you can redistribute it and/or *
    *   modify it under the terms of the GNU General Public License   *
    *   as published by the Free Software Foundation; either version  *
    *   2 of the License, or (at your option) any later version.      *
    *                                                                 *
    *******************************************************************/

/*
    LoadT carga un grafico en formato .T de 24 bits en una superficie SDL
*/
void LoadT(SDL_Surface **Tsurface, char *str)
{
    FILE *Tsrc;

    int T_lwidth, T_hwidth, T_lheight, T_hheight; // bytes alto y bajo de anchura y altura del grafico
    long T_width, T_height;
    int R, G, B;
    Uint32 pixel;
    
    Tsrc=fopen(str, "rb");
    if ( Tsrc == NULL)
    {
        fprintf(stderr, "can't read from file \n");
        return;
    }

    // leer tamaño del bmp
    T_lwidth = getc(Tsrc);
    T_hwidth = getc(Tsrc);
    T_lheight = getc(Tsrc);
    T_hheight = getc(Tsrc);
    T_width  = T_hwidth*256+T_lwidth;
    T_height = T_hheight*256+T_lheight;

    SDL_Surface *temp;
    temp = SDL_CreateRGBSurface(SDL_SWSURFACE,T_width,T_height,24,0,0,0,0);
    
    if(SDL_MUSTLOCK(temp))
	SDL_LockSurface(temp);

    for(int n=1;n<=T_height;n++) //el tamaño del grafico es de ancho * alto * 3 (24 bits)
    {
	for(int m=1;m<=T_width;m++)
	{
	    R=getc(Tsrc);
    	    G=getc(Tsrc);
    	    B=getc(Tsrc);
	    pixel=SDL_MapRGB(temp->format,R,G,B);

	    Uint8 *p = (Uint8 *)temp->pixels + (n-1) * temp->pitch + (m-1) * 3;
	    p[0] = pixel & 0xff;
	    p[1] = (pixel >> 8) & 0xff;
	    p[2] = (pixel >> 16) & 0xff;
    	}
    }

    if(SDL_MUSTLOCK(temp))
	SDL_UnlockSurface(temp);
    *Tsurface = SDL_DisplayFormat(temp);
    SDL_FreeSurface(temp);
    fclose(Tsrc);
}

SDL_Surface * LoadT8(char *str)
{
    SDL_Surface *Tsurface;
    FILE *Tsrc;
    Tsrc=fopen(str, "rb");
    if ( Tsrc == NULL)
    {
        fprintf(stderr, "can't read from file \n");
        SDL_Quit();
	exit(0);
    }
    // leer tamaño del bmp
    fseek(Tsrc,6,SEEK_SET); // Saltamos la cabecera "T-1000"
    // leemos tamaño del bitmap
    int wh=getc(Tsrc);    int wl=getc(Tsrc);    int hh=getc(Tsrc);    int hl=getc(Tsrc);
    int width=wh*256+wl;    int height=hh*256+hl;
    // leemos la paleta
    int paleta[768];
    for(int pal=0;pal<768;pal++)
        paleta[pal]=getc(Tsrc);
    // leemos el bitmap en una superficie de 24 bits
    SDL_Surface *temp;
    temp = SDL_CreateRGBSurface(SDL_SWSURFACE,width,height,24,0,0,0,0);
    if(SDL_MUSTLOCK(temp))
	SDL_LockSurface(temp);
    Uint32 pixel;
    int x,y,color;
    for(y=0;y<height;y++) //el tamaño del grafico es de ancho * alto * 3 (24 bits)
    {
	for(x=0;x<width;x++)
	{
	    color=getc(Tsrc);
            pixel = SDL_MapRGB(temp->format,paleta[color*3],paleta[(color*3)+1],paleta[(color*3)+2]);
	    Uint8 *p = (Uint8 *)temp->pixels + (y) * temp->pitch + (x) * 3;
	    p[0] = pixel & 0xff;
	    p[1] = (pixel >> 8) & 0xff;
	    p[2] = (pixel >> 16) & 0xff;
    	}
    }
    if(SDL_MUSTLOCK(temp))
	SDL_UnlockSurface(temp);
    Tsurface = SDL_DisplayFormat(temp);
    SDL_FreeSurface(temp);
    fclose(Tsrc);
    return(Tsurface);
}
