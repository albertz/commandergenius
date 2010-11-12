/***************************************************************************
 *   Copyright (C) 2009 by Andrey Afletdinov <fheroes2@gmail.com>          *
 *                                                                         *
 *   Part of the Free Heroes2 Engine:                                      *
 *   http://sourceforge.net/projects/fheroes2                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifdef WITH_EDITOR

#include <bitset>

#include "gamedefs.h"
#include "agg.h"
#include "settings.h"
#include "button.h"
#include "dialog.h"
#include "world.h"
#include "interface_gamearea.h"
#include "cursor.h"
#include "splitter.h"
#include "sizecursor.h"
#include "direction.h"
#include "maps_tiles.h"
#include "ground.h"
#include "editor_interface.h"
#include "game.h"

struct GroundIndexAndRotate : std::pair<u16, u8>
{
    GroundIndexAndRotate() {};
};

namespace Game
{
    namespace Editor
    {
	void ModifySingleTile(Maps::Tiles & center);
	void ModifyTileAbroad(Maps::Tiles & center);
	void SetGroundToTile(Maps::Tiles & tile, const Maps::Ground::ground_t ground);
	GroundIndexAndRotate GetTileWithCorner(u16 around, const Maps::Ground::ground_t ground);
    }
}

Game::menu_t Game::Editor::StartGame()
{
    Game::SetFixVideoMode();

    Display & display = Display::Get();
    Cursor & cursor = Cursor::Get();
    EditorInterface & I = EditorInterface::Get();

    Interface::GameArea & areaMaps = Interface::GameArea::Get();
    Interface::Radar & radar = Interface::Radar::Get();

    Settings::Get().SetScrollSpeed(SCROLL_FAST2);
    areaMaps.Build();

    radar.SetPos(display.w() - BORDERWIDTH - RADARWIDTH, BORDERWIDTH);
    radar.Build();

    const Rect & areaPos = areaMaps.GetArea();

    // cursor
    cursor.Hide();
    cursor.SetThemes(cursor.POINTER);
    I.Draw();

    const Rect area_pos(BORDERWIDTH, BORDERWIDTH, areaPos.w, areaPos.h);
    const Rect areaScrollLeft(0, BORDERWIDTH, BORDERWIDTH / 2, display.h() - 2 * BORDERWIDTH);
    const Rect areaScrollRight(display.w() - BORDERWIDTH / 2, BORDERWIDTH, BORDERWIDTH / 2, display.h() - 2 * BORDERWIDTH);
    const Rect areaScrollTop(BORDERWIDTH, 0, areaPos.w, BORDERWIDTH / 2);
    const Rect areaScrollBottom(2 * BORDERWIDTH, display.h() - BORDERWIDTH / 2, (areaMaps.GetRectMaps().w - 1) * TILEWIDTH, BORDERWIDTH / 2);
    const Rect areaLeftPanel(display.w() - 2 * BORDERWIDTH - RADARWIDTH, 0, BORDERWIDTH + RADARWIDTH, display.h());

    const Sprite & spritePanelGround = AGG::GetICN(ICN::EDITPANL, 0);
    const Sprite & spritePanelObject = AGG::GetICN(ICN::EDITPANL, 1);
    const Sprite & spritePanelInfo = AGG::GetICN(ICN::EDITPANL, 2);
    const Sprite & spritePanelRiver = AGG::GetICN(ICN::EDITPANL, 3);
    const Sprite & spritePanelRoad = AGG::GetICN(ICN::EDITPANL, 4);
    const Sprite & spritePanelClear = AGG::GetICN(ICN::EDITPANL, 5);


    LocalEvent & le = LocalEvent::Get();

    Button &btnLeftTopScroll = I.btnLeftTopScroll;
    Button &btnRightTopScroll = I.btnRightTopScroll;
    Button &btnTopScroll = I.btnTopScroll;
    Button &btnLeftBottomScroll = I.btnLeftBottomScroll;
    Button &btnLeftScroll = I.btnLeftScroll;
    Button &btnRightScroll = I.btnRightScroll;
    Button &btnRightBottomScroll = I.btnRightBottomScroll;
    Button &btnBottomScroll = I.btnBottomScroll;
    Button &btnSelectGround = I.btnSelectGround;
    Button &btnSelectObject = I.btnSelectObject;
    Button &btnSelectInfo = I.btnSelectInfo;
    Button &btnSelectRiver = I.btnSelectRiver;
    Button &btnSelectRoad = I.btnSelectRoad;
    Button &btnSelectClear = I.btnSelectClear;
    Button &btnSizeSmall = I.btnSizeSmall;
    Button &btnSizeMedium = I.btnSizeMedium;
    Button &btnSizeLarge = I.btnSizeLarge;
    Button &btnSizeManual = I.btnSizeManual;
    Button &btnZoom = I.btnZoom;
    Button &btnUndo = I.btnUndo;
    Button &btnNew = I.btnNew;
    Button &btnSpec = I.btnSpec;
    Button &btnFile = I.btnFile;
    Button &btnSystem = I.btnSystem;

    btnSelectGround.Press();
    btnSizeMedium.Press();

    btnSelectGround.Draw();
    btnSizeMedium.Draw();

    const Point dstPanel(btnSelectRiver.x, btnSelectRiver.y + btnSelectRiver.h);
    
    SizeCursor sizeCursor;
    
    sizeCursor.ModifySize(2, 2);
    sizeCursor.Hide();

    const Rect rectTerrainWater(dstPanel.x + 29, dstPanel.y + 10, 28, 28);
    const Rect rectTerrainGrass(dstPanel.x + 58, dstPanel.y + 10, 28, 28);
    const Rect rectTerrainSnow(dstPanel.x + 87, dstPanel.y + 10, 28, 28);
    const Rect rectTerrainSwamp(dstPanel.x + 29, dstPanel.y + 39, 28, 28);
    const Rect rectTerrainLava(dstPanel.x + 58, dstPanel.y + 39, 28, 28);
    const Rect rectTerrainBeach(dstPanel.x + 87, dstPanel.y + 39, 28, 28);
    const Rect rectTerrainDirt(dstPanel.x + 29, dstPanel.y + 68, 28, 28);
    const Rect rectTerrainWasteland(dstPanel.x + 58, dstPanel.y + 68, 28, 28);
    const Rect rectTerrainDesert(dstPanel.x + 87, dstPanel.y + 68, 28, 28);

    const Rect rectObjectWater(dstPanel.x + 14, dstPanel.y + 11, 28, 28);
    const Rect rectObjectGrass(dstPanel.x + 43, dstPanel.y + 11, 28, 28);
    const Rect rectObjectSnow(dstPanel.x + 72, dstPanel.y + 11, 28, 28);
    const Rect rectObjectSwamp(dstPanel.x + 101, dstPanel.y + 11, 28, 28);
    const Rect rectObjectLava(dstPanel.x + 14, dstPanel.y + 39, 28, 28);
    const Rect rectObjectDesert(dstPanel.x + 43, dstPanel.y + 39, 28, 28);
    const Rect rectObjectDirt(dstPanel.x + 72, dstPanel.y + 39, 28, 28);
    const Rect rectObjectWasteland(dstPanel.x + 101, dstPanel.y + 39, 28, 28);
    const Rect rectObjectBeach(dstPanel.x + 14, dstPanel.y + 67, 28, 28);
    const Rect rectObjectTown(dstPanel.x + 43, dstPanel.y + 67, 28, 28);
    const Rect rectObjectMonster(dstPanel.x + 72, dstPanel.y + 67, 28, 28);
    const Rect rectObjectHero(dstPanel.x + 101, dstPanel.y + 67, 28, 28);
    const Rect rectObjectArtifact(dstPanel.x + 14, dstPanel.y + 95, 28, 28);
    const Rect rectObjectResource(dstPanel.x + 101, dstPanel.y + 95, 28, 28);

    SpriteCursor selectTerrainCursor(AGG::GetICN(ICN::TERRAINS, 9), rectTerrainWater.x - 1, rectTerrainWater.y - 1);
    selectTerrainCursor.Hide();

    SpriteCursor selectObjectCursor(AGG::GetICN(ICN::TERRAINS, 9), rectObjectWater.x - 1, rectObjectWater.y - 1);
    selectTerrainCursor.Hide();

    u8 selectTerrain = 0;
    u8 selectObject = 0;

    // redraw
    areaMaps.Redraw(display, LEVEL_ALL);
    radar.RedrawArea();
    radar.RedrawCursor();
    display.Blit(spritePanelGround, dstPanel);
    selectTerrainCursor.Show();

    cursor.Show();
    display.Flip();

    //u32 ticket = 0;
    // startgame loop
    while(le.HandleEvents())
    {
	// ESC
	if(HotKeyPress(EVENT_DEFAULT_EXIT) && (Dialog::YES & Dialog::Message("", _("Are you sure you want to quit?"), Font::BIG, Dialog::YES|Dialog::NO))) return QUITGAME;

	// scroll area maps left
	if(le.MouseCursor(areaScrollLeft))	areaMaps.SetScroll(SCROLL_LEFT);
	else
	// scroll area maps right
	if(le.MouseCursor(areaScrollRight))	areaMaps.SetScroll(SCROLL_RIGHT);

	// scroll area maps top
	if(le.MouseCursor(areaScrollTop))	areaMaps.SetScroll(SCROLL_TOP);
	else
	// scroll area maps bottom
	if(le.MouseCursor(areaScrollBottom))	areaMaps.SetScroll(SCROLL_BOTTOM);

	// point radar
	if(le.MouseCursor(radar.GetArea()) &&
	    (le.MouseClickLeft(radar.GetArea()) ||
		le.MousePressLeft(radar.GetArea())))
	{
	    const Point prev(areaMaps.GetRectMaps());
            const Point & pt = le.GetMouseCursor();
            areaMaps.Center((pt.x - radar.GetArea().x) * world.w() / RADARWIDTH, (pt.y - radar.GetArea().y) * world.h() / RADARWIDTH);
	    if(prev != areaMaps.GetRectMaps())
	    {
		cursor.Hide();
		cursor.SetThemes(cursor.POINTER);
		sizeCursor.Hide();
		I.split_h.Move(areaMaps.GetRectMaps().x);
		I.split_v.Move(areaMaps.GetRectMaps().y);
		radar.RedrawCursor();
		EditorInterface::DrawTopNumberCell();
		EditorInterface::DrawLeftNumberCell();
                areaMaps.Redraw(display, LEVEL_ALL);
		cursor.Show();
		display.Flip();
	    }
	}
	else
	// pointer cursor on left panel
	if(le.MouseCursor(areaLeftPanel))
	{
	    cursor.Hide();
	    cursor.SetThemes(cursor.POINTER);
	    sizeCursor.Hide();
	    cursor.Show();
	    display.Flip();
	}
	else
	// cursor over game area
	if(le.MouseCursor(area_pos) &&
	    Maps::isValidAbsIndex(areaMaps.GetIndexFromMousePoint(le.GetMouseCursor())))
	{
            const Point & mouse_coord = le.GetMouseCursor();
            const s32 index_maps = areaMaps.GetIndexFromMousePoint(mouse_coord);
            Maps::Tiles & tile = world.GetTiles(index_maps);
            const Rect tile_pos(BORDERWIDTH + ((u16) (mouse_coord.x - BORDERWIDTH) / TILEWIDTH) * TILEWIDTH, BORDERWIDTH + ((u16) (mouse_coord.y - BORDERWIDTH) / TILEWIDTH) * TILEWIDTH, TILEWIDTH, TILEWIDTH);
            //u8 object = tile.GetObject();

    	    cursor.SetThemes(cursor.POINTER);

	    const u16 div_x = mouse_coord.x < BORDERWIDTH + TILEWIDTH * (areaMaps.GetRectMaps().w - sizeCursor.w()) ?
			    (u16) ((mouse_coord.x - BORDERWIDTH) / TILEWIDTH) * TILEWIDTH + BORDERWIDTH :
			    BORDERWIDTH + (areaMaps.GetRectMaps().w - sizeCursor.w()) * TILEWIDTH;
	    const u16 div_y = mouse_coord.y < BORDERWIDTH + TILEWIDTH * (areaMaps.GetRectMaps().h - sizeCursor.h()) ?
			    (u16) ((mouse_coord.y - BORDERWIDTH) / TILEWIDTH) * TILEWIDTH + BORDERWIDTH :
			    BORDERWIDTH + (areaMaps.GetRectMaps().h - sizeCursor.h()) * TILEWIDTH;

	    if(! sizeCursor.isVisible() || sizeCursor.GetPos().x != div_x || sizeCursor.GetPos().y != div_y)
	    {
		cursor.Hide();
		sizeCursor.Hide();
		sizeCursor.Show(div_x, div_y);
		cursor.Show();
		display.Flip();
	    }

	    if(le.MousePressLeft())
	    {
		cursor.Hide();
		sizeCursor.Hide();


		const Point topleft(areaMaps.GetRectMaps().x + (div_x - BORDERWIDTH) / 32,
				    areaMaps.GetRectMaps().y + (div_y - BORDERWIDTH) / 32);

		
		for(u8 iy = 0; iy < sizeCursor.h(); ++iy)
		{
		    for(u8 ix = 0; ix < sizeCursor.w(); ++ix)
		    {
                	Maps::Tiles & newtile = world.GetTiles(topleft.x + ix, topleft.y + iy);

			switch(selectTerrain)
			{
			    case 0: SetGroundToTile(newtile, Maps::Ground::WATER);	break;
			    case 1: SetGroundToTile(newtile, Maps::Ground::GRASS);	break;
			    case 2: SetGroundToTile(newtile, Maps::Ground::SNOW);	break;
			    case 3: SetGroundToTile(newtile, Maps::Ground::SWAMP);	break;
			    case 4: SetGroundToTile(newtile, Maps::Ground::LAVA);	break;
			    case 5: SetGroundToTile(newtile, Maps::Ground::DESERT);	break;
			    case 6: SetGroundToTile(newtile, Maps::Ground::DIRT);	break;
			    case 7: SetGroundToTile(newtile, Maps::Ground::WASTELAND);	break;
			    case 8: SetGroundToTile(newtile, Maps::Ground::BEACH);	break;

			    default: break;
			}

			newtile.RedrawTile(display);
			newtile.RedrawBottom(display);
			newtile.RedrawTop(display);
		    }
		}

		// modify single tiles
		for(int ii = 0; ii < world.w() * world.h(); ++ii) ModifySingleTile(world.GetTiles(ii));

		// modify all tiles abroad
		for(int ii = 0; ii < world.w() * world.h(); ++ii) ModifyTileAbroad(world.GetTiles(ii));
        
		sizeCursor.Show();
		cursor.Show();

		display.Flip();

		// wait
		while(le.HandleEvents() && le.MousePressLeft());

		radar.Generate();
		radar.RedrawArea();
		radar.RedrawCursor();
		display.Flip();
	    }
	    else
	    if(le.MousePressRight())
	    {
		if(btnSelectInfo.isPressed())
		{
		    if(IS_DEVEL())
		    {
			tile.DebugInfo();

			const u16 around = Maps::GetDirectionAroundGround(tile.GetIndex(), tile.GetGround());
			if(Direction::TOP_LEFT & around) VERBOSE("TOP_LEFT");
			if(Direction::TOP & around) VERBOSE("TOP");
			if(Direction::TOP_RIGHT & around) VERBOSE("TOP_RIGHT");
			if(Direction::RIGHT & around) VERBOSE("RIGHT");
			if(Direction::BOTTOM_RIGHT & around) VERBOSE("BOTTOM_RIGHT");
			if(Direction::BOTTOM & around) VERBOSE("BOTTOM");
			if(Direction::BOTTOM_LEFT & around) VERBOSE("BOTTOM_LEFT");
			if(Direction::LEFT & around) VERBOSE("LEFT");

			// wait
			while(le.HandleEvents() && le.MousePressRight());
		    }
		    else
		    {
			//const std::string & info = (MP2::OBJ_ZERO == object || MP2::OBJ_EVENT == object ?
			//Maps::Ground::String(tile.GetGround()) : MP2::StringObject(object));

			//Dialog::QuickInfo(info);
		    }
		}
	    }
	// end cursor over game area
	}


	// draw push buttons
	le.MousePressLeft(btnLeftTopScroll) ? btnLeftTopScroll.PressDraw() : btnLeftTopScroll.ReleaseDraw();
	le.MousePressLeft(btnRightTopScroll) ? btnRightTopScroll.PressDraw() : btnRightTopScroll.ReleaseDraw();
	le.MousePressLeft(btnTopScroll) ? btnTopScroll.PressDraw() : btnTopScroll.ReleaseDraw();
	le.MousePressLeft(btnLeftBottomScroll) ? btnLeftBottomScroll.PressDraw() : btnLeftBottomScroll.ReleaseDraw();
	le.MousePressLeft(btnLeftScroll) ? btnLeftScroll.PressDraw() : btnLeftScroll.ReleaseDraw();
	le.MousePressLeft(btnRightScroll) ? btnRightScroll.PressDraw() : btnRightScroll.ReleaseDraw();
	le.MousePressLeft(btnRightBottomScroll) ? btnRightBottomScroll.PressDraw() : btnRightBottomScroll.ReleaseDraw();
	le.MousePressLeft(btnBottomScroll) ? btnBottomScroll.PressDraw() : btnBottomScroll.ReleaseDraw();

	le.MousePressLeft(btnZoom) ? btnZoom.PressDraw() : btnZoom.ReleaseDraw();
	le.MousePressLeft(btnUndo) ? btnUndo.PressDraw() : btnUndo.ReleaseDraw();
	le.MousePressLeft(btnNew) ? btnNew.PressDraw() : btnNew.ReleaseDraw();
	le.MousePressLeft(btnSpec) ? btnSpec.PressDraw() : btnSpec.ReleaseDraw();
	le.MousePressLeft(btnFile) ? btnFile.PressDraw() : btnFile.ReleaseDraw();
	le.MousePressLeft(btnSystem) ? btnSystem.PressDraw() : btnSystem.ReleaseDraw();


	// click control button
	if(le.MouseClickLeft(btnSelectGround) ||
	    le.MouseClickLeft(btnSelectObject) ||
	    le.MouseClickLeft(btnSelectInfo) ||
	    le.MouseClickLeft(btnSelectRiver) ||
	    le.MouseClickLeft(btnSelectRoad) ||
	    le.MouseClickLeft(btnSelectClear))
	{
	    cursor.Hide();

	    if(btnSelectGround.isPressed()){ btnSelectGround.Release(); btnSelectGround.Draw(); }
	    else
	    if(btnSelectObject.isPressed()){ btnSelectObject.Release(); btnSelectObject.Draw(); }
	    else
	    if(btnSelectInfo.isPressed()){ btnSelectInfo.Release(); btnSelectInfo.Draw(); }
	    else
	    if(btnSelectRiver.isPressed()){ btnSelectRiver.Release(); btnSelectRiver.Draw(); }
	    else
	    if(btnSelectRoad.isPressed()){ btnSelectRoad.Release(); btnSelectRoad.Draw(); }
	    else
	    if(btnSelectClear.isPressed()){ btnSelectClear.Release(); btnSelectClear.Draw(); }

	    if(le.MouseCursor(btnSelectGround))
	    {
		selectTerrain = 0;

		btnSizeSmall.SetDisable(false);
		btnSizeMedium.SetDisable(false);
		btnSizeLarge.SetDisable(false);
		btnSizeManual.SetDisable(false);

		btnSelectGround.Press();
		btnSelectGround.Draw();

		display.Blit(spritePanelGround, dstPanel);
		DEBUG(DBG_GAME , DBG_INFO, "Game::Editor::StartGame: select Terrain Mode");
		selectTerrainCursor.Move(rectTerrainWater.x - 1, rectTerrainWater.y - 1);
	    }
	    else
	    if(le.MouseCursor(btnSelectObject))
	    {
		selectObject = 0;

		btnSizeSmall.SetDisable(true);
		btnSizeMedium.SetDisable(true);
		btnSizeLarge.SetDisable(true);
		btnSizeManual.SetDisable(true);

		btnSelectObject.Press();
		btnSelectObject.Draw();

		display.Blit(spritePanelObject, dstPanel);
		DEBUG(DBG_GAME , DBG_INFO, "Game::Editor::StartGame: select Object Mode");
		selectObjectCursor.Move(rectObjectWater.x - 1, rectObjectWater.y - 1);
	    }
	    else
	    if(le.MouseCursor(btnSelectInfo))
	    {
		btnSizeSmall.Release();
		btnSizeMedium.Release();
		btnSizeLarge.Release();
		btnSizeManual.Release();

		btnSizeSmall.Press();
		sizeCursor.ModifySize(1, 1);

		btnSizeSmall.SetDisable(true);
		btnSizeMedium.SetDisable(true);
		btnSizeLarge.SetDisable(true);
		btnSizeManual.SetDisable(true);

		btnSelectInfo.Press();
		btnSelectInfo.Draw();

		display.Blit(spritePanelInfo, dstPanel);
		DEBUG(DBG_GAME , DBG_INFO, "Game::Editor::StartGame: select Detail Mode");
	    }
	    else
	    if(le.MouseCursor(btnSelectRiver))
	    {
		btnSizeSmall.SetDisable(true);
		btnSizeMedium.SetDisable(true);
		btnSizeLarge.SetDisable(true);
		btnSizeManual.SetDisable(true);

		btnSelectRiver.Press();
		btnSelectRiver.Draw();

		display.Blit(spritePanelRiver, dstPanel);
		DEBUG(DBG_GAME , DBG_INFO, "Game::Editor::StartGame: select Stream Mode");
	    }
	    else
	    if(le.MouseCursor(btnSelectRoad))
	    {
		btnSizeSmall.SetDisable(true);
		btnSizeMedium.SetDisable(true);
		btnSizeLarge.SetDisable(true);
		btnSizeManual.SetDisable(true);

		btnSelectRoad.Press();
		btnSelectRoad.Draw();

		display.Blit(spritePanelRoad, dstPanel);		
		DEBUG(DBG_GAME , DBG_INFO, "Game::Editor::StartGame: select Road Mode");
	    }
	    else
	    if(le.MouseCursor(btnSelectClear))
	    {
		btnSizeSmall.SetDisable(false);
		btnSizeMedium.SetDisable(false);
		btnSizeLarge.SetDisable(false);
		btnSizeManual.SetDisable(false);

		btnSelectClear.Press();
		btnSelectClear.Draw();

		display.Blit(spritePanelClear, dstPanel);
		DEBUG(DBG_GAME , DBG_INFO, "Game::Editor::StartGame: select Erase Mode");
	    }
		

	    if(btnSizeSmall.isEnable()) btnSizeSmall.isPressed() ? btnSizeSmall.Press() : btnSizeSmall.Release();
	    if(btnSizeMedium.isEnable()) btnSizeMedium.isPressed() ? btnSizeMedium.Press() : btnSizeMedium.Release();
	    if(btnSizeLarge.isEnable()) btnSizeLarge.isPressed() ? btnSizeLarge.Press() : btnSizeLarge.Release();
	    if(btnSizeManual.isEnable()) btnSizeManual.isPressed() ? btnSizeManual.Press() : btnSizeManual.Release();

	    if(btnSizeSmall.isEnable()) btnSizeSmall.Draw();
	    if(btnSizeMedium.isEnable()) btnSizeMedium.Draw();
	    if(btnSizeLarge.isEnable()) btnSizeLarge.Draw();
	    if(btnSizeManual.isEnable()) btnSizeManual.Draw();

	    cursor.Show();
	    display.Flip();
	}

	// click select size button
	if((btnSizeSmall.isEnable() && le.MouseClickLeft(btnSizeSmall)) ||
	   (btnSizeMedium.isEnable() && le.MouseClickLeft(btnSizeMedium)) ||
	   (btnSizeLarge.isEnable() && le.MouseClickLeft(btnSizeLarge)) ||
	   (btnSizeManual.isEnable() && le.MouseClickLeft(btnSizeManual)))
	{
	    cursor.Hide();

	    btnSizeSmall.Release();
	    btnSizeMedium.Release();
	    btnSizeLarge.Release();
	    btnSizeManual.Release();

	    if(le.MouseCursor(btnSizeSmall)){ btnSizeSmall.Press(); sizeCursor.ModifySize(1, 1); }
	    else
	    if(le.MouseCursor(btnSizeMedium)){ btnSizeMedium.Press(); sizeCursor.ModifySize(2, 2); }
	    else
	    if(le.MouseCursor(btnSizeLarge)){ btnSizeLarge.Press(); sizeCursor.ModifySize(4, 4); }
	    else
	    if(le.MouseCursor(btnSizeManual)){ btnSizeManual.Press(); sizeCursor.ModifySize(2, 2); }

	    btnSizeSmall.Draw();
	    btnSizeMedium.Draw();
	    btnSizeLarge.Draw();
	    btnSizeManual.Draw();
		
	    cursor.Show();
	    display.Flip();
	}

	// click select terrain
	if(btnSelectGround.isPressed() && le.MouseClickLeft(rectTerrainWater))
	{
	    selectTerrain = 0;
	    cursor.Hide();
	    selectTerrainCursor.Move(rectTerrainWater.x - 1, rectTerrainWater.y - 1);
	    cursor.Show();
	    display.Flip();
	    DEBUG(DBG_GAME , DBG_INFO, "Game::Editor::StartGame: select terrain: water");
	}
	else
	if(btnSelectGround.isPressed() && le.MouseClickLeft(rectTerrainGrass))
	{
	    selectTerrain = 1;
	    cursor.Hide();
	    selectTerrainCursor.Move(rectTerrainGrass.x - 1, rectTerrainGrass.y - 1);
	    cursor.Show();
	    display.Flip();
	    DEBUG(DBG_GAME , DBG_INFO, "Game::Editor::StartGame: select terrain: grass");
	}
	else
	if(btnSelectGround.isPressed() && le.MouseClickLeft(rectTerrainSnow))
	{
	    selectTerrain = 2;
	    cursor.Hide();
	    selectTerrainCursor.Move(rectTerrainSnow.x - 1, rectTerrainSnow.y - 1);
	    cursor.Show();
	    display.Flip();
	    DEBUG(DBG_GAME , DBG_INFO, "Game::Editor::StartGame: select terrain: snow");
	}
	else
	if(btnSelectGround.isPressed() && le.MouseClickLeft(rectTerrainSwamp))
	{
	    selectTerrain = 3;
	    cursor.Hide();
	    selectTerrainCursor.Move(rectTerrainSwamp.x - 1, rectTerrainSwamp.y - 1);
	    cursor.Show();
	    display.Flip();
	    DEBUG(DBG_GAME , DBG_INFO, "Game::Editor::StartGame: select terrain: swamp");
	}
	else
	if(btnSelectGround.isPressed() && le.MouseClickLeft(rectTerrainLava))
	{
	    selectTerrain = 4;
	    cursor.Hide();
	    selectTerrainCursor.Move(rectTerrainLava.x - 1, rectTerrainLava.y - 1);
	    cursor.Show();
	    display.Flip();
	    DEBUG(DBG_GAME , DBG_INFO, "Game::Editor::StartGame: select terrain: lava");
	}
	else
	if(btnSelectGround.isPressed() && le.MouseClickLeft(rectTerrainBeach))
	{
	    selectTerrain = 5;
	    cursor.Hide();
	    selectTerrainCursor.Move(rectTerrainBeach.x - 1, rectTerrainBeach.y - 1);
	    cursor.Show();
	    display.Flip();
	    DEBUG(DBG_GAME , DBG_INFO, "Game::Editor::StartGame: select terrain: beach");
	}
	else
	if(btnSelectGround.isPressed() && le.MouseClickLeft(rectTerrainDirt))
	{
	    selectTerrain = 6;
	    cursor.Hide();
	    selectTerrainCursor.Move(rectTerrainDirt.x - 1, rectTerrainDirt.y - 1);
	    cursor.Show();
	    display.Flip();
	    DEBUG(DBG_GAME , DBG_INFO, "Game::Editor::StartGame: select terrain: dirt");
	}
	else
	if(btnSelectGround.isPressed() && le.MouseClickLeft(rectTerrainWasteland))
	{
	    selectTerrain = 7;
	    cursor.Hide();
	    selectTerrainCursor.Move(rectTerrainWasteland.x - 1, rectTerrainWasteland.y - 1);
	    cursor.Show();
	    display.Flip(); 
	    DEBUG(DBG_GAME , DBG_INFO, "Game::Editor::StartGame: select terrain: wasteland");
	}
	else
	if(btnSelectGround.isPressed() && le.MouseClickLeft(rectTerrainDesert))
	{
	    selectTerrain = 8;
	    cursor.Hide();
	    selectTerrainCursor.Move(rectTerrainDesert.x - 1, rectTerrainDesert.y - 1);
	    cursor.Show();
	    display.Flip();
	    DEBUG(DBG_GAME , DBG_INFO, "Game::Editor::StartGame: select terrain: desert");
	}

	// click select object
	if(btnSelectObject.isPressed() && le.MouseClickLeft(rectObjectWater))
	{
	    selectObject = 0;
	    cursor.Hide();
	    selectObjectCursor.Move(rectObjectWater.x - 1, rectObjectWater.y - 1);
	    cursor.Show();
	    display.Flip();
	    DEBUG(DBG_GAME , DBG_INFO, "Game::Editor::StartGame: select object: water");
	}
	else
	if(btnSelectObject.isPressed() && le.MouseClickLeft(rectObjectGrass))
	{
	    selectObject = 1;
	    cursor.Hide();
	    selectObjectCursor.Move(rectObjectGrass.x - 1, rectObjectGrass.y - 1);
	    cursor.Show();
	    display.Flip();
	    DEBUG(DBG_GAME , DBG_INFO, "Game::Editor::StartGame: select object: grass");
	}
	else
	if(btnSelectObject.isPressed() && le.MouseClickLeft(rectObjectSnow))
	{
	    selectObject = 2;
	    cursor.Hide();
	    selectObjectCursor.Move(rectObjectSnow.x - 1, rectObjectSnow.y - 1);
	    cursor.Show();
	    display.Flip();
	    DEBUG(DBG_GAME , DBG_INFO, "Game::Editor::StartGame: select object: snow");
	}
	else
	if(btnSelectObject.isPressed() && le.MouseClickLeft(rectObjectSwamp))
	{
	    selectObject = 3;
	    cursor.Hide();
	    selectObjectCursor.Move(rectObjectSwamp.x - 1, rectObjectSwamp.y - 1);
	    cursor.Show();
	    display.Flip();
	    DEBUG(DBG_GAME , DBG_INFO, "Game::Editor::StartGame: select object: swamp");
	}
	else
	if(btnSelectObject.isPressed() && le.MouseClickLeft(rectObjectLava))
	{
	    selectObject = 4;
	    cursor.Hide();
	    selectObjectCursor.Move(rectObjectLava.x - 1, rectObjectLava.y - 1);
	    cursor.Show();
	    display.Flip();
	    DEBUG(DBG_GAME , DBG_INFO, "Game::Editor::StartGame: select object: lava");
	}
	else
	if(btnSelectObject.isPressed() && le.MouseClickLeft(rectObjectDesert))
	{
	    selectObject = 5;
	    cursor.Hide();
	    selectObjectCursor.Move(rectObjectDesert.x - 1, rectObjectDesert.y - 1);
	    cursor.Show();
	    display.Flip();
	    DEBUG(DBG_GAME , DBG_INFO, "Game::Editor::StartGame: select object: desert");
	}
	else
	if(btnSelectObject.isPressed() && le.MouseClickLeft(rectObjectDirt))
	{
	    selectObject = 6;
	    cursor.Hide();
	    selectObjectCursor.Move(rectObjectDirt.x - 1, rectObjectDirt.y - 1);
	    cursor.Show();
	    display.Flip();
	    DEBUG(DBG_GAME , DBG_INFO, "Game::Editor::StartGame: select object: dirt");
	}
	else
	if(btnSelectObject.isPressed() && le.MouseClickLeft(rectObjectWasteland))
	{
	    selectObject = 7;
	    cursor.Hide();
	    selectObjectCursor.Move(rectObjectWasteland.x - 1, rectObjectWasteland.y - 1);
	    cursor.Show();
	    display.Flip();
	    DEBUG(DBG_GAME , DBG_INFO, "Game::Editor::StartGame: select object: wasteland");
	}
	else
	if(btnSelectObject.isPressed() && le.MouseClickLeft(rectObjectBeach))
	{
	    selectObject = 8;
	    cursor.Hide();
	    selectObjectCursor.Move(rectObjectBeach.x - 1, rectObjectBeach.y - 1);
	    cursor.Show();
	    display.Flip();
	    DEBUG(DBG_GAME , DBG_INFO, "Game::Editor::StartGame: select object: beach");
	}
	else
	if(btnSelectObject.isPressed() && le.MouseClickLeft(rectObjectTown))
	{
	    selectObject = 9;
	    cursor.Hide();
	    selectObjectCursor.Move(rectObjectTown.x - 1, rectObjectTown.y - 1);
	    cursor.Show();
	    display.Flip();
	    DEBUG(DBG_GAME , DBG_INFO, "Game::Editor::StartGame: select object: town");
	}
	else
	if(btnSelectObject.isPressed() && le.MouseClickLeft(rectObjectMonster))
	{
	    selectObject = 10;
	    cursor.Hide();
	    selectObjectCursor.Move(rectObjectMonster.x - 1, rectObjectMonster.y - 1);
	    cursor.Show();
	    display.Flip();
	    DEBUG(DBG_GAME , DBG_INFO, "Game::Editor::StartGame: select object: monster");
	}
	else
	if(btnSelectObject.isPressed() && le.MouseClickLeft(rectObjectHero))
	{
	    selectObject = 11;
	    cursor.Hide();
	    selectObjectCursor.Move(rectObjectHero.x - 1, rectObjectHero.y - 1);
	    cursor.Show();
	    display.Flip();
	    DEBUG(DBG_GAME , DBG_INFO, "Game::Editor::StartGame: select object: hero");
	}
	else
	if(btnSelectObject.isPressed() && le.MouseClickLeft(rectObjectArtifact))
	{
	    selectObject = 12;
	    cursor.Hide();
	    selectObjectCursor.Move(rectObjectArtifact.x - 1, rectObjectArtifact.y - 1);
	    cursor.Show();
	    display.Flip();
	    DEBUG(DBG_GAME , DBG_INFO, "Game::Editor::StartGame: select object: artifact");
	}
	else
	if(btnSelectObject.isPressed() && le.MouseClickLeft(rectObjectResource))
	{
	    selectObject = 13;
	    cursor.Hide();
	    selectObjectCursor.Move(rectObjectResource.x - 1, rectObjectResource.y - 1);
	    cursor.Show();
	    display.Flip();
	    DEBUG(DBG_GAME , DBG_INFO, "Game::Editor::StartGame: select object: resource");
	}

	// button click
	if(le.MouseClickLeft(btnZoom))
	{
	    VERBOSE("Game::Editor::StartGame: FIXME: click button Zoom");
	}
	if(le.MouseClickLeft(btnUndo))
	{
	    VERBOSE("Game::Editor::StartGame: FIXME: click button Undo");
	}
	if(le.MouseClickLeft(btnNew))
	{
	    return EDITNEWMAP;
	}
	if(le.MouseClickLeft(btnSpec))
	{
	    VERBOSE("Game::Editor::StartGame: FIXME: click button Spec");
	}
	if(le.MouseClickLeft(btnFile))
	{
	    switch(Dialog::FileOptions())
	    {
		case Game::NEWGAME:	return EDITNEWMAP;
		case Game::LOADGAME:	return EDITLOADMAP;
		case Game::SAVEGAME:	return EDITSAVEMAP;
		case Game::QUITGAME:	return QUITGAME;
		
		default: break;
	    }
	}
	if(le.MouseClickLeft(btnSystem))
	{
	    VERBOSE("Game::Editor::StartGame: FIXME: click button Options");
	}

	// press right info
	if(le.MousePressRight(btnZoom))
	    Dialog::Message(_("Magnify"), _("Change between zoom and normal view."), Font::BIG);
	else
	if(le.MousePressRight(btnUndo))
	    Dialog::Message(_("Undo"), _("Undo your last action. Press againt to redo the action."), Font::BIG);
	else
	if(le.MousePressRight(btnNew))
	    Dialog::Message(_("New"), _("Start a new map from scratch."), Font::BIG);
	else
	if(le.MousePressRight(btnSpec))
	    Dialog::Message(_("Specifications"), _("Edit maps title, description, and other general information."), Font::BIG);
	else
	if(le.MousePressRight(btnFile))
	    Dialog::Message(_("File Options"), _("Open the file options menu, where you can save or load maps, or quit out of the editor."), Font::BIG);
	else
	if(le.MousePressRight(btnSystem))
	    Dialog::Message(_("System Options"), _("View the editor system options, which let you customize the editor."), Font::BIG);
	else
	if(le.MousePressRight(btnSelectGround))
	    Dialog::Message(_("Terrain Mode"), _("Used to draw the underlying grass, dirt, water, etc. on the map."), Font::BIG);
	else
	if(le.MousePressRight(btnSelectObject))
	    Dialog::Message(_("Object Mode"), _("Used to place objects (mountains, trees, treasure, etc.) on the map."), Font::BIG);
	else
	if(le.MousePressRight(btnSelectInfo))
	    Dialog::Message(_("Detail Mode"), _("Used for special editing of monsters, heroes and towns."), Font::BIG);
	else
	if(le.MousePressRight(btnSelectRiver))
	    Dialog::Message(_("Stream Mode"), _("Allows you to draw streams by clicking and dragging."), Font::BIG);
	else
	if(le.MousePressRight(btnSelectRoad))
	    Dialog::Message(_("Road Mode"), _("Allows you to draw roads by clicking and dragging."), Font::BIG);
	else
	if(le.MousePressRight(btnSelectClear))
	    Dialog::Message(_("Erase Mode"), _("Used to erase objects of the map."), Font::BIG);
	else
	if(btnSelectGround.isPressed() && le.MousePressRight(rectTerrainWater))
	    Dialog::Message(_("Water"), _("Traversable only by boat."), Font::BIG);
	else
	if(btnSelectGround.isPressed() && le.MousePressRight(rectTerrainGrass))
	    Dialog::Message(_("Grass"), _("No special modifiers."), Font::BIG);
	else
	if(btnSelectGround.isPressed() && le.MousePressRight(rectTerrainSnow))
	    Dialog::Message(_("Snow"), _("Cost 1.5 times normal movement for all heroes. (Pathfinding reduces or eliminates the penalty.)"), Font::BIG);
	else
	if(btnSelectGround.isPressed() && le.MousePressRight(rectTerrainSwamp))
	    Dialog::Message(_("Swamp"), _("Cost 1.75 times normal movement for all heroes. (Pathfinding reduces or eliminates the penalty.)"), Font::BIG);
	else
	if(btnSelectGround.isPressed() && le.MousePressRight(rectTerrainLava))
	    Dialog::Message(_("Lava"), _("No special modifiers."), Font::BIG);
	else
	if(btnSelectGround.isPressed() && le.MousePressRight(rectTerrainBeach))
	    Dialog::Message(_("Beach"), _("Cost 1.25 times normal movement for all heroes. (Pathfinding reduces or eliminates the penalty.)"), Font::BIG);
	else
	if(btnSelectGround.isPressed() && le.MousePressRight(rectTerrainDirt))
	    Dialog::Message(_("Dirt"), _("No special modifiers."), Font::BIG);
	else
	if(btnSelectGround.isPressed() && le.MousePressRight(rectTerrainWasteland))
	    Dialog::Message(_("Wasteland"), _("Cost 1.25 times normal movement for all heroes. (Pathfinding reduces or eliminates the penalty.)"), Font::BIG);
	else
	if(btnSelectGround.isPressed() && le.MousePressRight(rectTerrainDesert))
	    Dialog::Message(_("Desert"), _("Cost 2 times normal movement for all heroes. (Pathfinding reduces or eliminates the penalty.)"), Font::BIG);
	else
	if(btnSelectObject.isPressed() && le.MousePressRight(rectObjectWater))
	    Dialog::Message(_("Water Objects"), _("Used to select objects most appropriate for use on water."), Font::BIG);
	else
	if(btnSelectObject.isPressed() && le.MousePressRight(rectObjectGrass))
	    Dialog::Message(_("Grass Objects"), _("Used to select objects most appropriate for use on grass."), Font::BIG);
	else
	if(btnSelectObject.isPressed() && le.MousePressRight(rectObjectSnow))
	    Dialog::Message(_("Snow Objects"), _("Used to select objects most appropriate for use on snow."), Font::BIG);
	else
	if(btnSelectObject.isPressed() && le.MousePressRight(rectObjectSwamp))
	    Dialog::Message(_("Swamp Objects"), _("Used to select objects most appropriate for use on swamp."), Font::BIG);
	else
	if(btnSelectObject.isPressed() && le.MousePressRight(rectObjectLava))
	    Dialog::Message(_("Lava Objects"), _("Used to select objects most appropriate for use on lava."), Font::BIG);
	else
	if(btnSelectObject.isPressed() && le.MousePressRight(rectObjectDesert))
	    Dialog::Message(_("Desert Objects"), _("Used to select objects most appropriate for use on desert."), Font::BIG);
	else
	if(btnSelectObject.isPressed() && le.MousePressRight(rectObjectDirt))
	    Dialog::Message(_("Dirt Objects"), _("Used to select objects most appropriate for use on dirt."), Font::BIG);
	else
	if(btnSelectObject.isPressed() && le.MousePressRight(rectObjectWasteland))
	    Dialog::Message(_("Wasteland Objects"), _("Used to select objects most appropriate for use on wasteland."), Font::BIG);
	else
	if(btnSelectObject.isPressed() && le.MousePressRight(rectObjectBeach))
	    Dialog::Message(_("Beach Objects"), _("Used to select objects most appropriate for use on beach."), Font::BIG);
	else
	if(btnSelectObject.isPressed() && le.MousePressRight(rectObjectTown))
	    Dialog::Message(_("Towns"), _("Used to place a town or castle."), Font::BIG);
	else
	if(btnSelectObject.isPressed() && le.MousePressRight(rectObjectMonster))
	    Dialog::Message(_("Monsters"), _("Used to place a monster group."), Font::BIG);
	else
	if(btnSelectObject.isPressed() && le.MousePressRight(rectObjectHero))
	    Dialog::Message(_("Heroes"), _("Used to place a hero."), Font::BIG);
	else
	if(btnSelectObject.isPressed() && le.MousePressRight(rectObjectArtifact))
	    Dialog::Message(_("Artifact"), _("Used to place an artifact."), Font::BIG);
	else
	if(btnSelectObject.isPressed() && le.MousePressRight(rectObjectResource))
	    Dialog::Message(_("Treasures"), _("Used to place a resource or treasure."), Font::BIG);

	if(areaMaps.NeedScroll())
	{
	    cursor.Hide();
	    sizeCursor.Hide();
	    cursor.SetThemes(areaMaps.GetScrollCursor());
	    areaMaps.Scroll();
	    //I.Scroll(scrollDir);
	    areaMaps.Redraw(display, LEVEL_ALL);
	    radar.RedrawCursor();
	    cursor.Show();
	    display.Flip();
	}
    }

    return QUITGAME;
}

void Game::Editor::ModifySingleTile(Maps::Tiles & tile)
{
    //u8 count = Maps::GetCountAroundGround(tile.GetIndex(), tile.GetGround());
    const u16 center = tile.GetIndex();
    const Maps::Ground::ground_t ground = tile.GetGround();
    const u16 max = Maps::GetMaxGroundAround(center);
    Display & display = Display::Get();

    if(max & ground) return;

    if((ground == world.GetTiles(Maps::GetDirectionIndex(center, Direction::TOP)).GetGround() &&
	ground == world.GetTiles(Maps::GetDirectionIndex(center, Direction::LEFT)).GetGround()) ||
       (ground == world.GetTiles(Maps::GetDirectionIndex(center, Direction::TOP)).GetGround() &&
	ground == world.GetTiles(Maps::GetDirectionIndex(center, Direction::RIGHT)).GetGround()) ||
       (ground == world.GetTiles(Maps::GetDirectionIndex(center, Direction::BOTTOM)).GetGround() &&
	ground == world.GetTiles(Maps::GetDirectionIndex(center, Direction::LEFT)).GetGround()) ||
       (ground == world.GetTiles(Maps::GetDirectionIndex(center, Direction::BOTTOM)).GetGround() &&
	ground == world.GetTiles(Maps::GetDirectionIndex(center, Direction::RIGHT)).GetGround())) return;

    u16 index = 0;

    if(max & Maps::Ground::DESERT)	index = 300;
    else
    if(max & Maps::Ground::SNOW)	index = 130;
    else
    if(max & Maps::Ground::SWAMP)	index = 184;
    else
    if(max & Maps::Ground::WASTELAND)	index = 399;
    else
    if(max & Maps::Ground::BEACH)	index = 415;
    else
    if(max & Maps::Ground::LAVA)	index = 246;
    else
    if(max & Maps::Ground::DIRT)	index = 337;
    else
    if(max & Maps::Ground::GRASS)	index =  68;
    else
    if(max & Maps::Ground::WATER)	index =  16;

    if(index)
    {
	tile.SetTile(Rand::Get(index, index + 7), 0);
        tile.RedrawTile(display);
        tile.RedrawBottom(display);
    	tile.RedrawTop(display);
    }
}

void Game::Editor::ModifyTileAbroad(Maps::Tiles & tile)
{
    const u16 center = tile.GetIndex();
    Display & display = Display::Get();

    // fix
    if(Maps::Ground::WATER != tile.GetGround()) return;

    for(Direction::vector_t direct = Direction::TOP_LEFT; direct != Direction::CENTER; ++direct)
    {
	if(Maps::isValidDirection(center, direct))
	{
	    const Maps::Tiles & opposition = world.GetTiles(Maps::GetDirectionIndex(center, direct));
	    u16 index = 0;

	    // start index sprite
	    switch(opposition.GetGround())
	    {
		case Maps::Ground::DESERT:
		case Maps::Ground::SNOW:
		case Maps::Ground::SWAMP:
		case Maps::Ground::WASTELAND:
		case Maps::Ground::BEACH:
		case Maps::Ground::LAVA:
		case Maps::Ground::DIRT:
		case Maps::Ground::GRASS:	index = 0; break;

		case Maps::Ground::WATER:
		default: continue;
	    }

	    const u16 around = Maps::GetDirectionAroundGround(center, tile.GetGround());

	    // normal: 0, vertical: 1, horizontal: 2, any: 3
	    bool fix = false;
	    u8 revert = 0;

	    // sprite small corner
	    if(around == (DIRECTION_ALL & (~(Direction::TOP_RIGHT | Direction::CENTER))))
	    { fix = true; index += 12; revert = 0; }
	    else
	    if(around == (DIRECTION_ALL & (~(Direction::TOP_LEFT | Direction::CENTER))))
	    { fix = true; index += 12; revert = 2; }
	    else
	    if(around == (DIRECTION_ALL & (~(Direction::BOTTOM_RIGHT | Direction::CENTER))))
	    { fix = true; index += 12; revert = 1; }
	    else
	    if(around == (DIRECTION_ALL & (~(Direction::BOTTOM_LEFT | Direction::CENTER))))
	    { fix = true; index += 12; revert = 3; }
	    else
	    // sprite row
	    if(around & (DIRECTION_CENTER_ROW | DIRECTION_BOTTOM_ROW) &&
        	!(around & (Direction::TOP)))
            { fix = true; index += 0; revert = 0; }
            else
            if(around & (DIRECTION_CENTER_ROW | DIRECTION_TOP_ROW) &&
        	!(around & (Direction::BOTTOM)))
    	    { fix = true; index += 0; revert = 1; }
            else
	    // sprite col
            if(around & (DIRECTION_CENTER_COL | DIRECTION_LEFT_COL) &&
        	!(around & (Direction::RIGHT)))
            { fix = true; index += 8; revert = 0; }
    	    else
            if(around & (DIRECTION_CENTER_COL | DIRECTION_RIGHT_COL) &&
        	!(around & (Direction::LEFT)))
    	    { fix = true; index += 8; revert = 2; }
	    // sprite small corner
	    if(around & (Direction::CENTER | Direction::LEFT | Direction::BOTTOM_LEFT | Direction::BOTTOM) &&
		!(around & (Direction::TOP | Direction::TOP_RIGHT | Direction::RIGHT)))
    	    { fix = true; index += 4; revert = 0; }
	    else
	    if(around & (Direction::CENTER | Direction::RIGHT | Direction::BOTTOM_RIGHT | Direction::BOTTOM) &&
		!(around & (Direction::TOP | Direction::TOP_LEFT | Direction::LEFT)))
    	    { fix = true; index += 4; revert = 2; }
	    else
	    if(around & (Direction::CENTER | Direction::LEFT | Direction::TOP_LEFT | Direction::TOP) &&
		!(around & (Direction::BOTTOM | Direction::BOTTOM_RIGHT | Direction::RIGHT)))
    	    { fix = true; index += 4; revert = 1; }
	    else
            if(around & (Direction::CENTER | Direction::RIGHT | Direction::TOP_RIGHT | Direction::TOP) &&
        	!(around & (Direction::BOTTOM | Direction::BOTTOM_LEFT | Direction::LEFT)))
    	    { fix = true; index += 4; revert = 3; }

	    // fix random
	    if(fix)
	    {
		tile.SetTile(Rand::Get(index, index + 3), revert);
                tile.RedrawTile(display);
                tile.RedrawBottom(display);
    		tile.RedrawTop(display);
    	    }
    	}
    }
}

/* set ground to tile */
void Game::Editor::SetGroundToTile(Maps::Tiles & tile, const Maps::Ground::ground_t ground)
{
    const u16 around = Maps::GetDirectionAroundGround(tile.GetIndex(), ground);

    // simply set
    if(ground == around)
    {
	u16 index_ground = 0;

	switch(ground)
	{
	    case Maps::Ground::WATER:		tile.SetTile(Rand::Get(16, 19), 0); return;
	    case Maps::Ground::GRASS:		index_ground =  68; break;
	    case Maps::Ground::SNOW:		index_ground = 130; break;
	    case Maps::Ground::SWAMP:		index_ground = 184; break;
	    case Maps::Ground::LAVA:		index_ground = 246; break;
	    case Maps::Ground::DESERT:		index_ground = 300; break;
	    case Maps::Ground::DIRT:		index_ground = 337; break;
	    case Maps::Ground::WASTELAND:	index_ground = 399; break;
	    case Maps::Ground::BEACH:		index_ground = 415; break;	
	    default: break;
	}

	switch(Rand::Get(1, 7))
	{
	    // 85% simple ground
    	    case 1:
    	    case 2:
    	    case 3:
    	    case 4:
    	    case 5:
    	    case 6:
		tile.SetTile(Rand::Get(index_ground, index_ground + 7), 0);
		break;

    	    // 15% extended ground
    	    default:
		tile.SetTile(Rand::Get(index_ground + 8, index_ground + 15), 0);
		break;
	}
    }
    else
    {
    }
}

GroundIndexAndRotate Game::Editor::GetTileWithCorner(u16 around, const Maps::Ground::ground_t ground)
{
    GroundIndexAndRotate result;

    switch(ground)
    {
	case Maps::Ground::WATER:
	default: break;
    }

    return result;
}

#endif
