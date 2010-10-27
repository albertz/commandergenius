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
