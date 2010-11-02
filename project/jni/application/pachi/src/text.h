void print_text(SDL_Surface **font, SDL_Surface *surface, int font_w, int font_h, int text_x, int text_y, char *str, ...)  //Rutina para imprimir texto estatico en la pantalla
{
    SDL_Rect srctxt; // la posicion donde se encuentra el caracter en el bitmap
    SDL_Rect dsttxt; // la posicion donde se imprimira el texto

    char texto [100];
    
    va_list ap;
    va_start(ap, str);
	vsprintf(texto, str, ap);
    va_end(ap);
    
    srctxt.w = font_w;
    srctxt.h = font_h;
    srctxt.y = 0;
    int linecounter = 0 ; // este contador se utiliza para saber en que linea imprimimos el texto
    int charpos = 0;
    for(int charcounter = 0; charcounter <= (strlen(texto));charcounter++)
    {
	int curchar=texto[charcounter];
	if(curchar == 94)
	{
	    linecounter++;
	    charpos = -1;
	}
	srctxt.x = (curchar >= 64 ? curchar - 62 : curchar - 32) * font_w;
	dsttxt.x = (text_x + (charpos * font_w));
	dsttxt.y = (text_y + (linecounter * font_h));
	charpos++;
	SDL_BlitSurface (font[ (curchar >= 64 ? 1 : 0) ],&srctxt,surface,&dsttxt);
    }
}

void print_text(SDL_Surface *font, SDL_Surface *surface, int font_w, int font_h, int text_x, int text_y, char *str, ...)  //Rutina para imprimir texto estatico en la pantalla
{
    SDL_Rect srctxt; // la posicion donde se encuentra el caracter en el bitmap
    SDL_Rect dsttxt; // la posicion donde se imprimira el texto

    char texto [100];
    
    va_list ap;
    va_start(ap, str);
	vsprintf(texto, str, ap);
    va_end(ap);
    
    srctxt.w = font_w;
    srctxt.h = font_h;
    srctxt.y = 0;
    int linecounter = 0 ; // este contador se utiliza para saber en que linea imprimimos el texto
    int charpos = 0;
    for(int charcounter = 0; charcounter <= (strlen(texto));charcounter++)
    {
	int curchar=texto[charcounter];
	if(curchar == 94)
	{
	    linecounter++;
	    charpos = -1;
	}
	srctxt.x = (curchar - 32) * font_w;
	dsttxt.x = (text_x + (charpos * font_w));
	dsttxt.y = (text_y + (linecounter * font_h));
	charpos++;
	SDL_BlitSurface (font,&srctxt,surface,&dsttxt);
    }
}

void load_font(SDL_Surface **font, char * file)
{
	SDL_Surface * temp = LoadT8(file, false);
	font[0] = SDL_CreateRGBSurface(SDL_HWSURFACE, temp->w/2, temp->h, screen->format->BitsPerPixel, 
									screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
	font[1] = SDL_CreateRGBSurface(SDL_HWSURFACE, temp->w/2, temp->h, screen->format->BitsPerPixel, 
									screen->format->Rmask, screen->format->Gmask, screen->format->Bmask, screen->format->Amask);
	SDL_Rect r;
	r.w = temp->w/2;
	r.h = temp->h;
	r.x = 0;
	r.y = 0;
	SDL_BlitSurface(temp, &r, font[0], NULL);
	SDL_SetColorKey(font[0], SDL_SRCCOLORKEY, SDL_MapRGB(font[0]->format,0,255,0));
	r.x = temp->w/2;
	SDL_BlitSurface(temp, &r, font[1], NULL);
	SDL_SetColorKey(font[1], SDL_SRCCOLORKEY, SDL_MapRGB(font[0]->format,0,255,0));
};

void unload_font(SDL_Surface **font)
{
	if( font[0] )
		SDL_FreeSurface(font[0]);
	font[0] = NULL;
	if( font[1] )
		SDL_FreeSurface(font[1]);
	font[1] = NULL;
}
