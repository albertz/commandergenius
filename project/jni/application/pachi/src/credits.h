char creditstext[] = 
"       CONGRATULATIONS!!!          \0"
"       YOU WON THE GAME!!          \0"
"                                   \0"
"      NOW THANKS TO YOU,           \0"
" THE MISTERY IS SOLVED AND PACHI   \0"
" WILL BE ABLE TO GO BACK TO MARS,  \0"
"      PLAY martian memory          \0"
"           AND VISIT               \0"
"   dragontech.sourceforge.net      \0"
"                                   \0"
"                                   \0"
"                                   \0"
"                                   \0"
" AND NOW THE CREDITS               \0"
"                                   \0"
"  PROJECT MANAGER, PROGRAMMER:     \0"
"    santiago radeff                \0"
"                                   \0"
"  DRAWINGS, PACHI FATHER           \0"
"   nicolas radeff                  \0"
"                                   \0"
"  GRAPHICS:                        \0"
"    nicolas radeff                 \0"
"    santiago radeff                \0"
"                                   \0"
"      MUSICS COMPOSED BY           \0"
"     peter -skaven- hajba          \0"
"                                   \0"
"                                   \0"
"                                   \0"
"                                   \0"
"    THANKS TO THE PROGRAMMERS,     \0"
"  AND PLAYERS FOR THEIR SUPPORT    \0"
"                                   \0"
"                                   \0"
"                                   \0"
"                                   \0"
"                                   \0"
"                                   \0"
"                                   \0"
"                                   \0"
"                                   \0"
"                                   \0"
"                                   \0"
"      ThAnKs fOr pLaYiNg           \0"
"                                   \0";
int creditslines = 46; // esta variable determina cuantas lineas de texto hay en los creditos


int creditslinecounter = 0; // esta variable almacena que linea de texto dee imprimirse (es un contador)
int needinput = 1; // esta variable indica si debe imprimirse una nueva linea de texto en los creditos
int linegone = 0; // esta variable indica cuando una linea de texto ya hizo scroll por completo para imprimir la proxima
int wrapcounter = 0; // esta variable es un contador que una vez que termino el texto de los creditos empieza a incrementarse y cuando llega a cierto valor reinicia los creditos
char curchar;
int charcounter;
