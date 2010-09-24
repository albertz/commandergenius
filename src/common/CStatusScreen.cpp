/*
 * CStatusScreen.cpp
 *
 *  Created on: 15.10.2009
 *      Author: gerstrong
 *
 *  This Class is only used for the Status screen. For now it only draw it to the screen
 *  deciding between the episodes. In future we might expand its functions or add support for
 *  more games.
 */

#include "CStatusScreen.h"
#include "../graphics/CGfxEngine.h"
#include "../sdl/CVideoDriver.h"
#include "../StringUtils.h"
#include "../common/Playerdefines.h"
#include "../common/CBehaviorEngine.h"
#include "../graphics/effects/CColorMerge.h"
#include "../engine/vorticon/ai/CAnkhShield.h"

#define FADE_SPEED	10

CStatusScreen::CStatusScreen
(char episode, stInventory *p_inventory, bool *p_level_completed, int ankhtime, int baseframe) :
 mp_level_completed(p_level_completed),
 m_ankhtime(ankhtime),
 m_closing(false),
 m_closed(false),
 mp_StatusSfc(NULL)
{
	m_episode = episode;
	mp_inventory = p_inventory;
	m_baseframe = baseframe;
	if(m_baseframe<0) m_baseframe=0;

	// draw the episode-specific stuff
	if (m_episode==1)
		createInventorySfcEp1();
	else if (m_episode==2)
		createInventorySfcEp2();
	else if (m_episode==3)
		createInventorySfcEp3();

}

void CStatusScreen::draw()
{
	// Fade in and out process
	if(m_closing)
	{
		if(g_pVideoDriver->getSpecialFXConfig() && !g_pGfxEngine->runningEffect())
			m_closed = true;
		else if(!g_pVideoDriver->getSpecialFXConfig())
			m_closed = true;
	}


	if(!m_closed)
		SDL_BlitSurface(mp_StatusSfc, NULL, g_pVideoDriver->FGLayerSurface, &m_StatusRect );
}

SDL_Surface* CStatusScreen::CreateStatusSfc()
{
	SDL_Surface *p_blitSurface = g_pVideoDriver->FGLayerSurface;
	const Uint32 rmask = p_blitSurface->format->Rmask;
	const Uint32 gmask = p_blitSurface->format->Gmask;
	const Uint32 bmask = p_blitSurface->format->Bmask;
	const Uint32 amask = p_blitSurface->format->Amask;
	const Uint8 bpp = p_blitSurface->format->BitsPerPixel;

	return SDL_CreateRGBSurface(p_blitSurface->flags, m_StatusRect.w, m_StatusRect.h, bpp, rmask, gmask, bmask, amask);
}

void CStatusScreen::createInventorySfcEp1()
{
	int x,t,i,j;
	std::string tempbuf;
	int dlgW,dlgH;
	CFont &Font = g_pGfxEngine->getFont(0);

	m_StatusRect.x = 5*8;
	m_StatusRect.y = 5*8;
	dlgW = 29;
	dlgH = 15;
	m_StatusRect.w = (dlgW+1)*8;
	m_StatusRect.h = dlgH*8;
	
	SDL_Surface *p_surface = CreateStatusSfc();
	CTilemap &Tilemap = g_pGfxEngine->getTileMap(1);

	g_pGfxEngine->drawDialogBox( p_surface, 0, 0, dlgW,dlgH, Font.getBGColour(true));
	Font.drawFont( p_surface, g_pBehaviorEngine->getString("EP1_StatusBox"), 1<<3, 1<<3, true);
	
	// Now draw some white rects. Those are the holders for items, numbers, etc.
	SDL_Rect rect;
	rect.x = 8;	rect.w = 12*8; // Score
	rect.y = 2*8;	rect.h = 1*8;
	SDL_FillRect(p_surface,&rect, Font.getBGColour(false));

	rect.x = 14*8;	rect.w = 15*8; // Extra keen at
	SDL_FillRect(p_surface,&rect, Font.getBGColour(false));

	rect.x = 8;	rect.w = 14*8; // Keens
	rect.y = 4*8;	rect.h = 3*8;
	SDL_FillRect(p_surface,&rect, Font.getBGColour(false));

	rect.x = 16*8;	rect.w = 13*8; // Ship Parts
	SDL_FillRect(p_surface,&rect, Font.getBGColour(false));

	rect.x = 8;	rect.w = 8*8; // Raygun
	rect.y = 8*8;	rect.h = 3*8;
	SDL_FillRect(p_surface,&rect, Font.getBGColour(false));

	rect.x = 10*8;	rect.w = 6*8; // Pogo
	rect.h = 5*8;
	SDL_FillRect(p_surface,&rect, Font.getBGColour(false));

	rect.x = 17*8;	rect.w = 12*8; // Keycards
	SDL_FillRect(p_surface,&rect, Font.getBGColour(false));

	rect.x = 8;	rect.w = 8*8; // Charge
	rect.y = 12*8;	rect.h = 1*8;
	SDL_FillRect(p_surface,&rect, Font.getBGColour(false));

	// fill in what we have
	// 321: joystick/battery/vacuum/fuel not gotten
	// 424: yellow/red/green/blue cards
	// 448: ship parts, gotten
	CPhysicsSettings &Phy = g_pBehaviorEngine->getPhysicsSettings();
	// raygun icon
	Tilemap.drawTile(p_surface, 4<<3, (8<<3)+3, Phy.misc.raygun_tile);
	// pogo
	if (mp_inventory->HasPogo) Tilemap.drawTile(p_surface, 12<<3, (9<<3)+3, Phy.misc.pogo_tile);
	// cards
	if (mp_inventory->HasCardYellow)
	{
		Tilemap.drawTile(p_surface, 20<<3, (8<<3)+3, 424);
		if(mp_inventory->HasCardYellow > 1)
			Font.drawFont( p_surface, itoa(mp_inventory->HasCardYellow),(20)<<3,((8)<<3)+3, false);
	}
	if (mp_inventory->HasCardRed)
	{
		Tilemap.drawTile(p_surface, 24<<3, (8<<3)+3, 425);
		
		if(mp_inventory->HasCardRed > 1)
			Font.drawFont( p_surface, itoa(mp_inventory->HasCardRed),(24)<<3,((8)<<3)+3, false);
	}
	if (mp_inventory->HasCardGreen)
	{
		Tilemap.drawTile(p_surface, 20<<3, (10<<3)+4, 426);
		
		if (mp_inventory->HasCardGreen > 1)
			Font.drawFont( p_surface, itoa(mp_inventory->HasCardGreen),(20)<<3,((10)<<3)+3, false);
	}
	if (mp_inventory->HasCardBlue)
	{
		Tilemap.drawTile(p_surface, 24<<3, (10<<3)+4, 427);
		
		if(mp_inventory->HasCardBlue > 1)
			Font.drawFont( p_surface, itoa(mp_inventory->HasCardBlue),(24)<<3,((10)<<3)+3, false);
	}
	// ship parts
	if (mp_inventory->HasJoystick) t=448; else t=321;
	Tilemap.drawTile(p_surface, (17)<<3, ((0+4)<<3)+3, t);
	if (mp_inventory->HasBattery) t=449; else t=322;
	Tilemap.drawTile(p_surface, (0+20)<<3, ((0+4)<<3)+3, t);
	if (mp_inventory->HasVacuum) t=450; else t=323;
	Tilemap.drawTile(p_surface, (0+23)<<3, ((0+4)<<3)+3, t);
	if (mp_inventory->HasWiskey) t=451; else t=324;
	Tilemap.drawTile(p_surface, (0+26)<<3, ((0+4)<<3)+3, t);
	// ray gun charges
	i = mp_inventory->charges;
	if (i>999) i=999;
	tempbuf = itoa(i);
	Font.drawFont( p_surface, tempbuf, (0+4)<<3, (0+12)<<3, false);
	
	// score
	i = mp_inventory->score;
	tempbuf = itoa(i);
	Font.drawFont( p_surface, tempbuf, (0+11-tempbuf.size())<<3, (0+2)<<3, false);
	// extra life at
	i = mp_inventory->extralifeat;
	tempbuf = itoa(i);
	Font.drawFont( p_surface, tempbuf, (0+27-tempbuf.size())<<3, (0+2)<<3, false);
	// lives
	i = mp_inventory->lives;
	x = ((0+2)<<3);
	if (i>6) i=6;
	for(j=0;j<i;j++)
	{
		CSprite &Sprite = g_pGfxEngine->getSprite(m_baseframe);
		Sprite.drawSprite(  p_surface, x, (0+4)<<3);
		x += Sprite.getWidth();
	}

	mp_StatusSfc = SDL_DisplayFormat(p_surface);
	SDL_FreeSurface(p_surface);
}

void CStatusScreen::createInventorySfcEp2()
{
	int x,i,j;
	std::string tempbuf;
	int dlgW,dlgH;
	CFont &Font = g_pGfxEngine->getFont(0);

	CTilemap &Tilemap = g_pGfxEngine->getTileMap(1);

	m_StatusRect.x = 5*8;
	m_StatusRect.y = 5*8;
	dlgW = 29;
	dlgH = 14;
	m_StatusRect.w = (dlgW+1)*8;
	m_StatusRect.h = dlgH*8;

	SDL_Surface *p_surface = CreateStatusSfc();
	tempbuf = g_pBehaviorEngine->getString("EP2_StatusBox");
	g_pGfxEngine->drawDialogBox( p_surface, 0,0,dlgW,dlgH, Font.getBGColour(true));
	Font.drawFont( p_surface, tempbuf, (0+1)<<3, (0+1)<<3, true);

	// Now draw some white rects. Those are the holders for items, numbers, etc.
	SDL_Rect rect;
	rect.x = (0+1)*8;	rect.w = 12*8; // Score
	rect.y = (0+2)*8;	rect.h = 1*8;
	SDL_FillRect(p_surface,&rect, Font.getBGColour(false));

	rect.x = 0*8+14*8;	rect.w = 15*8; // Extra keen at
	SDL_FillRect(p_surface,&rect, Font.getBGColour(false));

	rect.x = (0+1)*8;	rect.w = 18*8; // Keens
	rect.y = (0+4)*8;	rect.h = 3*8;
	SDL_FillRect(p_surface,&rect, Font.getBGColour(false));

	rect.x = (0+20)*8;	rect.w = 9*8; // Pistol
	SDL_FillRect(p_surface,&rect, Font.getBGColour(false));

	rect.x = (0+1)*8;	rect.w = 18*8; // Targets Saved
	rect.y = (0+8)*8;	rect.h = 4*8;
	SDL_FillRect(p_surface,&rect, Font.getBGColour(false));

	rect.x = (0+20)*8;	rect.w = 9*8; // Keys
	SDL_FillRect(p_surface,&rect, Font.getBGColour(false));

	// raygun icon
	Tilemap.drawTile(p_surface, (0+21)<<3, ((0+5)<<3), 414);

	// cards
	if (mp_inventory->HasCardYellow)
	{
		Tilemap.drawTile(p_surface, (0+21)<<3, ((0+8)<<3), 424);

		if(mp_inventory->HasCardYellow > 1)
			  Font.drawFont( p_surface, itoa(mp_inventory->HasCardYellow),(0+21)<<3,(0+8)<<3);
	}
	if (mp_inventory->HasCardRed)
	{
		Tilemap.drawTile(p_surface, (0+25)<<3, ((0+8)<<3), 425);

		if(mp_inventory->HasCardRed > 1)
			  Font.drawFont( p_surface, itoa(mp_inventory->HasCardRed),(0+25)<<3,(0+8)<<3);

	}
	if (mp_inventory->HasCardGreen)
	{
		Tilemap.drawTile(p_surface, (0+21)<<3, (0+10)<<3, 426);

		if(mp_inventory->HasCardGreen > 1)
			  Font.drawFont( p_surface, itoa(mp_inventory->HasCardGreen),(0+21)<<3,(0+10)<<3);
	}
	if (mp_inventory->HasCardBlue)
	{
		Tilemap.drawTile(p_surface, (0+25)<<3, (0+10)<<3, 427);

		if(mp_inventory->HasCardBlue > 1)
			  Font.drawFont( p_surface, itoa(mp_inventory->HasCardBlue),(0+25)<<3,(0+10)<<3);
	}

	// pistol charges
	i = mp_inventory->charges;
	if (i>999) i=999;
	tempbuf = itoa(i);
	Font.drawFont( p_surface, tempbuf, (0+27-tempbuf.size())<<3, ((0+5)<<3)-1);

	// score
	i = mp_inventory->score;
	tempbuf = itoa(i);
	Font.drawFont( p_surface, tempbuf, (0+11-tempbuf.size())<<3, (0+2)<<3);
	// extra life at
	i = mp_inventory->extralifeat;
	tempbuf = itoa(i);
	Font.drawFont( p_surface, tempbuf, (0+27-tempbuf.size())<<3, (0+2)<<3);
	// lives
	i = mp_inventory->lives;
	x = ((0 + 1)<<3)+4;
	if (i>7) i=7;
	for(j=0;j<i;j++)
	{
		CSprite &Sprite = g_pGfxEngine->getSprite(m_baseframe);
		Sprite.drawSprite( p_surface, x, (0+4)<<3 );
		x += Sprite.getWidth();
	}

	// cities saved
	if (mp_level_completed[4]) Font.drawFont( p_surface, g_pBehaviorEngine->getString("EP2_LVL4_TargetName"), (0+1)<<3, (0+8)<<3);
	if (mp_level_completed[6]) Font.drawFont( p_surface, g_pBehaviorEngine->getString("EP2_LVL6_TargetName"), (0+8)<<3, (0+8)<<3);
	if (mp_level_completed[7]) Font.drawFont( p_surface, g_pBehaviorEngine->getString("EP2_LVL7_TargetName"), (0+1)<<3, (0+9)<<3);
	if (mp_level_completed[13]) Font.drawFont( p_surface, g_pBehaviorEngine->getString("EP2_LVL13_TargetName"), (0+8)<<3, (0+9)<<3);
	if (mp_level_completed[11]) Font.drawFont( p_surface, g_pBehaviorEngine->getString("EP2_LVL11_TargetName"), (0+1)<<3, (0+10)<<3);
	if (mp_level_completed[9]) Font.drawFont( p_surface, g_pBehaviorEngine->getString("EP2_LVL9_TargetName"), (0+8)<<3, (0+10)<<3);
	if (mp_level_completed[15]) Font.drawFont( p_surface, g_pBehaviorEngine->getString("EP2_LVL15_TargetName"), (0+1)<<3, (0+11)<<3);
	if (mp_level_completed[16]) Font.drawFont( p_surface, g_pBehaviorEngine->getString("EP2_LVL16_TargetName"), (0+8)<<3, (0+11)<<3);

	mp_StatusSfc = SDL_DisplayFormat(p_surface);
	SDL_FreeSurface(p_surface);
}

void CStatusScreen::createInventorySfcEp3()
{
	int x,i,j;
	std::string tempbuf;
	int dlgW,dlgH;
	CFont &Font = g_pGfxEngine->getFont(0);

	CTilemap &Tilemap = g_pGfxEngine->getTileMap(1);

	m_StatusRect.x = 5*8;
	m_StatusRect.y = 5*8;
	dlgW = 29;
	dlgH = 13;
	m_StatusRect.w = (dlgW+1)*8;
	m_StatusRect.h = dlgH*8;

	SDL_Surface *p_surface = CreateStatusSfc();
	tempbuf = g_pBehaviorEngine->getString("EP3_StatusBox");
	g_pGfxEngine->drawDialogBox( p_surface, 0,0,dlgW,dlgH, Font.getBGColour(true));
	Font.drawFont( p_surface, tempbuf, (0+1)<<3, (0+1)<<3, true);

	// Now draw some white rects. Those are the holders for items, numbers, etc.
	SDL_Rect rect;
	rect.x = (0+1)*8;	rect.w = 12*8; // Score
	rect.y = (0+2)*8;	rect.h = 1*8;
	SDL_FillRect(p_surface, &rect, Font.getBGColour(false));

	rect.x = 0*8+14*8;	rect.w = 15*8; // Extra keen at
	SDL_FillRect(p_surface,&rect, Font.getBGColour(false));

	rect.x = (0+1)*8;	rect.w = 18*8; // Keens
	rect.y = (0+4)*8;	rect.h = 3*8;
	SDL_FillRect(p_surface,&rect, Font.getBGColour(false));

	rect.x = (0+20)*8;	rect.w = 9*8; // Pistol
	SDL_FillRect(p_surface,&rect, Font.getBGColour(false));

	rect.x = (0+1)*8;	rect.w = 10*8; // Ankh
	rect.y = (0+8)*8;	rect.h = 3*8;
	SDL_FillRect(p_surface,&rect, Font.getBGColour(false));

	rect.x = (0+12)*8;	rect.w = 17*8; // Keys
	SDL_FillRect(p_surface,&rect, Font.getBGColour(false));

	// raygun icon
	Tilemap.drawTile(p_surface, (0+21)<<3, ((0+5)<<3)-5, 216);

	// calculate % ankh time left
	int ankhtimepercent = (int)((float)m_ankhtime / (PLAY_ANKH_TIME/100));
	// ankh time
	Tilemap.drawTile(p_surface, (0+3)<<3, ((0+8)<<3)+3, 214);
	 
	Font.drawFont( p_surface, itoa(ankhtimepercent), (0+7)<<3, ((0+8)<<3)+7);
	

	// cards
	if (mp_inventory->HasCardYellow)
	{
		Tilemap.drawTile(p_surface, ((0+13)<<3)+4, ((0+8)<<3)+4, 217);

		if(mp_inventory->HasCardYellow > 1)
			Font.drawFont( p_surface, itoa(mp_inventory->HasCardYellow),(0+12)<<3,((0+8)<<3)+3);
	}
	if (mp_inventory->HasCardRed)
	{
		Tilemap.drawTile(p_surface, ((0+17)<<3)+4, ((0+8)<<3)+4, 218);

		if(mp_inventory->HasCardRed > 1)
			Font.drawFont(p_surface, itoa(mp_inventory->HasCardRed),(0+16)<<3,((0+8)<<3)+3);
	}
	if (mp_inventory->HasCardGreen)
	{
		Tilemap.drawTile(p_surface, ((0+21)<<3)+4, ((0+8)<<3)+4, 219);

		if(mp_inventory->HasCardGreen > 1)
			Font.drawFont(p_surface, itoa(mp_inventory->HasCardGreen),(0+20)<<3,((0+8)<<3)+3);
	}
	if (mp_inventory->HasCardBlue)
	{
		Tilemap.drawTile(p_surface, ((0+25)<<3)+4, ((0+8)<<3)+4, 220);

		if(mp_inventory->HasCardBlue > 1)
			Font.drawFont(p_surface, itoa(mp_inventory->HasCardBlue),(0+24)<<3,((0+8)<<3)+3);
	}

	// pistol charges
	i = mp_inventory->charges;
	if (i>999) i=999;
	tempbuf = itoa(i);
	Font.drawFont( p_surface, tempbuf, (0+27-tempbuf.size())<<3, ((0+5)<<3)-1);

	// score
	i = mp_inventory->score;
	tempbuf = itoa(i);
	Font.drawFont( p_surface, tempbuf, (0+11-tempbuf.size())<<3, (0+2)<<3);
	// extra life at
	i = mp_inventory->extralifeat;
	tempbuf = itoa(i);
	Font.drawFont( p_surface, tempbuf, (0+27-tempbuf.size())<<3, (0+2)<<3);
	// lives
	i = mp_inventory->lives;
	x = ((0 + 1)<<3)+4;
	if (i>7) i=7;
	for(j=0;j<i;j++)
	{
		CSprite &Sprite = g_pGfxEngine->getSprite(m_baseframe);
		Sprite.drawSprite(  p_surface, x, (0+4)<<3);
		x += Sprite.getWidth();
	}

	mp_StatusSfc = SDL_DisplayFormat(p_surface);
	SDL_FreeSurface(p_surface);
}

CStatusScreen::~CStatusScreen()
{
	if(mp_StatusSfc)
		SDL_FreeSurface(mp_StatusSfc);
}