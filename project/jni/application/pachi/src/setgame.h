#define easy 1
#define normal 2
#define hard 3

void setgame()
{
    stage=startstage;
    startstage=0;
    score=0;
    gametimer=0;

    mplayer[easy].realspeed=150;
    mplayer[normal].realspeed=150;
    mplayer[hard].realspeed=180;

    mplayer[easy].fallspeed=150;
    mplayer[normal].fallspeed=150;
    mplayer[hard].fallspeed=180;

    mplayer[easy].realmaxjump=80;
    mplayer[normal].realmaxjump=70;
    mplayer[hard].realmaxjump=70;

    mplayer[easy].tolerance=10;
    mplayer[normal].tolerance=8;
    mplayer[hard].tolerance=5;

    
    M_speed[easy]=130;
    M_speed[normal]=150;
    M_speed[hard]=200;

    for(int n=1;n<=3;n++)
    {
	mplayer[n].lives=maxlives;
	mplayer[n].frames=10;
	mplayer[n].h=P_h;
	mplayer[n].w=P_w;
    }
    
}
