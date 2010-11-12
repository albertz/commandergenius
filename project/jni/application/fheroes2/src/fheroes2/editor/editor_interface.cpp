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

#include "agg.h"
#include "settings.h"
#include "interface_gamearea.h"
#include "world.h"
#include "editor_interface.h"

EditorInterface::EditorInterface()
{
    btnLeftTopScroll.SetSprite(ICN::ESCROLL, 12, 13);
    btnRightTopScroll.SetSprite(ICN::ESCROLL, 14, 15);
    btnTopScroll.SetSprite(ICN::ESCROLL, 4, 5);
    btnLeftBottomScroll.SetSprite(ICN::ESCROLL, 18, 19);
    btnLeftScroll.SetSprite(ICN::ESCROLL, 8, 9);
    btnRightScroll.SetSprite(ICN::ESCROLL, 10, 11);
    btnRightBottomScroll.SetSprite(ICN::ESCROLL, 16, 17);
    btnBottomScroll.SetSprite(ICN::ESCROLL, 6, 7);
    btnSelectGround.SetSprite(ICN::EDITBTNS, 0, 1);
    btnSelectObject.SetSprite(ICN::EDITBTNS, 2, 3);
    btnSelectInfo.SetSprite(ICN::EDITBTNS, 4, 5);
    btnSelectRiver.SetSprite(ICN::EDITBTNS, 6, 7);
    btnSelectRoad.SetSprite(ICN::EDITBTNS, 8, 9);
    btnSelectClear.SetSprite(ICN::EDITBTNS, 10, 11);
    btnSizeSmall.SetSprite(ICN::EDITBTNS, 24, 25);
    btnSizeMedium.SetSprite(ICN::EDITBTNS, 26, 27);
    btnSizeLarge.SetSprite(ICN::EDITBTNS, 28, 29);
    btnSizeManual.SetSprite(ICN::EDITBTNS, 30, 31);
    btnZoom.SetSprite(ICN::EDITBTNS, 12, 13);
    btnUndo.SetSprite(ICN::EDITBTNS, 14, 15);
    btnNew.SetSprite(ICN::EDITBTNS, 16, 17);
    btnSpec.SetSprite(ICN::EDITBTNS, 18, 19);
    btnFile.SetSprite(ICN::EDITBTNS, 20, 21);
    btnSystem.SetSprite(ICN::EDITBTNS, 22, 23);

    split_h.SetSprite(AGG::GetICN(ICN::ESCROLL, 2));
    split_v.SetSprite(AGG::GetICN(ICN::ESCROLL, 3));
}

EditorInterface & EditorInterface::Get(void)
{
    static EditorInterface einterface;

    return einterface;
}

void EditorInterface::Draw(void)
{
    Display & display = Display::Get();
    Interface::GameArea & areaMaps = Interface::GameArea::Get();
    const Rect & areaPos = areaMaps.GetArea();

    display.Fill(0x00, 0x00, 0x00);

    Rect src_rt;
    Point dst_pt;

    const Sprite & spritePanelGround = AGG::GetICN(ICN::EDITPANL, 0);
    const Sprite & spriteBac = AGG::GetICN(ICN::STONBACK, 0);

    // bottom scroll bar indicator
    const Sprite & spriteBottomBar(AGG::GetICN(ICN::ESCROLL, 0));
    dst_pt.x = BORDERWIDTH * 2;
    dst_pt.y = BORDERWIDTH + areaPos.h;
    src_rt.x = 0;
    src_rt.y = 0;
    src_rt.w = TILEWIDTH;
    src_rt.h = BORDERWIDTH;
    display.Blit(spriteBottomBar, src_rt, dst_pt);
    src_rt.x = TILEWIDTH;
    for(u16 ii = 0; ii < (areaMaps.GetRectMaps().w - 4); ++ii)
    {
	dst_pt.x += TILEWIDTH;
	display.Blit(spriteBottomBar, src_rt, dst_pt);
    }
    dst_pt.x += TILEWIDTH;
    src_rt.x = spriteBottomBar.w() - TILEWIDTH;
    display.Blit(spriteBottomBar, src_rt, dst_pt);
    // left scroll bar indicator
    const Sprite & spriteLeftBar(AGG::GetICN(ICN::ESCROLL, 1));
    dst_pt.x = BORDERWIDTH + areaPos.w;
    dst_pt.y = BORDERWIDTH * 2;
    src_rt.x = 0;
    src_rt.y = 0;
    src_rt.w = BORDERWIDTH;
    src_rt.h = TILEWIDTH;
    display.Blit(spriteLeftBar, src_rt, dst_pt);
    src_rt.y = TILEWIDTH;
    for(u16 ii = 0; ii < (areaMaps.GetRectMaps().h - 4); ++ii)
    {
	dst_pt.y += TILEWIDTH;
	display.Blit(spriteLeftBar, src_rt, dst_pt);
    }
    dst_pt.y += TILEWIDTH;
    src_rt.y = spriteLeftBar.h() - TILEWIDTH;
    display.Blit(spriteLeftBar,src_rt, dst_pt);

    // draw EditorInterface
    const Sprite & spriteAdv = AGG::GetICN(ICN::ADVBORD, 0);

    // left top static border
    src_rt.x = spriteAdv.w() - BORDERWIDTH - RADARWIDTH;
    src_rt.y = 0;
    src_rt.w = BORDERWIDTH + RADARWIDTH;
    src_rt.h = BORDERWIDTH;
    dst_pt.x = 2 * BORDERWIDTH + areaPos.w;
    dst_pt.y = 0;
    display.Blit(spriteAdv, src_rt, dst_pt);
    // left bottom static border
    src_rt.x = spriteAdv.w() - BORDERWIDTH - RADARWIDTH;
    src_rt.y = spriteAdv.h() - BORDERWIDTH;
    src_rt.w = BORDERWIDTH + RADARWIDTH;
    src_rt.h = BORDERWIDTH;
    dst_pt.x = 2 * BORDERWIDTH + areaPos.w;
    dst_pt.y = BORDERWIDTH + areaPos.h;
    display.Blit(spriteAdv, src_rt, dst_pt);
    // left static border
    src_rt.x = spriteAdv.w() - BORDERWIDTH;
    src_rt.y = 0;
    src_rt.w = BORDERWIDTH;
    src_rt.h = 250;
    dst_pt.x = display.w() - BORDERWIDTH;
    dst_pt.y = 0;
    display.Blit(spriteAdv, src_rt, dst_pt);
    //
    src_rt.y = 250;
    src_rt.h = TILEWIDTH;
    dst_pt.y = 250;
    //
    u8 var1 = 4 + (display.h() - 480) / TILEWIDTH;

    for(u8 ii = 0; ii < var1; ++ii)
    {
	display.Blit(spriteAdv, src_rt, dst_pt);
	dst_pt.y += TILEWIDTH;
    }
    //
    src_rt.y = spriteAdv.h() - 102;
    src_rt.h = 102;
    dst_pt.y = display.h() - 102;
    display.Blit(spriteAdv, src_rt, dst_pt);


    Point pt_rts, pt_tsc, pt_lbs, pt_lsc, pt_rsc, pt_rbs, pt_bsc, pt_sgr, pt_sob, pt_sin, pt_sri, pt_sro, pt_scl;
    Point pt_ssm, pt_sme, pt_sla, pt_smn, pt_zoo, pt_und, pt_new, pt_spe, pt_fil, pt_sys;

    // btn right top scroll
    pt_rts.x = BORDERWIDTH + areaPos.w;
    pt_rts.y = 0;
    // btn top scroll
    pt_tsc.x = pt_rts.x;
    pt_tsc.y = BORDERWIDTH;
    // btn left bottom scroll
    pt_lbs.x = 0;
    pt_lbs.y = BORDERWIDTH + areaPos.h;
    // btn left scroll
    pt_lsc.x = BORDERWIDTH;
    pt_lsc.y = pt_lbs.y;
    // btn right scroll
    pt_rsc.x = 2 * BORDERWIDTH + (areaMaps.GetRectMaps().w - 2) * TILEWIDTH;
    pt_rsc.y = pt_lbs.y;
    // btn right bottom scroll
    pt_rbs.x = BORDERWIDTH + areaPos.w;
    pt_rbs.y = pt_lbs.y;
    // btn bottom scroll
    pt_bsc.x = pt_rbs.x;
    pt_bsc.y = areaPos.h;
    // btn select ground
    pt_sgr.x = display.w() - BORDERWIDTH - RADARWIDTH;
    pt_sgr.y = BORDERWIDTH + RADARWIDTH;
    // btn select object
    pt_sob.x = pt_sgr.x + btnSelectGround.w;
    pt_sob.y = pt_sgr.y;
    // btn select info
    pt_sin.x = pt_sob.x + btnSelectObject.w;
    pt_sin.y = pt_sgr.y;
    // btn select river
    pt_sri.x = pt_sgr.x;
    pt_sri.y = pt_sgr.y + btnSelectGround.h;
    // btn select road
    pt_sro.x = pt_sgr.x + btnSelectRiver.w;
    pt_sro.y = pt_sri.y;
    // btn select clear
    pt_scl.x = pt_sro.x + btnSelectRoad.w;
    pt_scl.y = pt_sri.y;
    // btn size small
    pt_ssm.x = pt_sgr.x + 14;
    pt_ssm.y = pt_sri.y + btnSelectRiver.h + 127;
    // btn size medium
    pt_sme.x = pt_sgr.x + 44;
    pt_sme.y = pt_ssm.y;
    // btn size large
    pt_sla.x = pt_sgr.x + 74;
    pt_sla.y = pt_ssm.y;
    // btn size manual
    pt_smn.x = pt_sgr.x + 104;
    pt_smn.y = pt_ssm.y;
    // btn zoom
    pt_zoo.x = pt_sgr.x;
    pt_zoo.y = pt_sri.y + btnSelectRiver.h + spritePanelGround.h();
    // btn undo
    pt_und.x = pt_zoo.x + btnZoom.w;
    pt_und.y = pt_zoo.y;
    // btn new
    pt_new.x = pt_und.x + btnUndo.w;
    pt_new.y = pt_zoo.y;
    // btn spec
    pt_spe.x = pt_zoo.x;
    pt_spe.y = pt_zoo.y + btnZoom.h;
    // btn file
    pt_fil.x = pt_spe.x + btnSpec.w;
    pt_fil.y = pt_spe.y;
    // btn system
    pt_sys.x = pt_fil.x + btnFile.w;
    pt_sys.y = pt_spe.y;
    
    btnLeftTopScroll.SetPos(0, 0);
    btnRightTopScroll.SetPos(pt_rts);
    btnTopScroll.SetPos(pt_tsc);
    btnLeftBottomScroll.SetPos(pt_lbs);
    btnLeftScroll.SetPos(pt_lsc);
    btnRightScroll.SetPos(pt_rsc);
    btnRightBottomScroll.SetPos(pt_rbs);
    btnBottomScroll.SetPos(pt_bsc);
    btnSelectGround.SetPos(pt_sgr);
    btnSelectObject.SetPos(pt_sob);
    btnSelectInfo.SetPos(pt_sin);
    btnSelectRiver.SetPos(pt_sri);
    btnSelectRoad.SetPos(pt_sro);
    btnSelectClear.SetPos(pt_scl);
    btnSizeSmall.SetPos(pt_ssm);
    btnSizeMedium.SetPos(pt_sme);
    btnSizeLarge.SetPos(pt_sla);
    btnSizeManual.SetPos(pt_smn);
    btnZoom.SetPos(pt_zoo);
    btnUndo.SetPos(pt_und);
    btnNew.SetPos(pt_new);
    btnSpec.SetPos(pt_spe);
    btnFile.SetPos(pt_fil);
    btnSystem.SetPos(pt_sys);

    // bottom static
    var1 = (display.h() - 480) / TILEWIDTH - 2;

    src_rt.x = 0;
    src_rt.y = 0;
    src_rt.w = spriteBac.w();
    src_rt.h = TILEWIDTH;
    dst_pt.x = btnSpec.x;
    dst_pt.y = btnSpec.y + btnSpec.h;
    if(var1) display.Blit(spriteBac, src_rt, dst_pt);
    src_rt.y = TILEWIDTH;
    dst_pt.y += TILEWIDTH;
    for(u8 ii = 0; ii < var1; ++ii)
    {
	display.Blit(spriteBac, src_rt, dst_pt);
	dst_pt.y += TILEWIDTH;
    }
    src_rt.y = spriteBac.h() - TILEWIDTH;
    if(var1) display.Blit(spriteBac, src_rt, dst_pt);

/*
    const Point dstPanel(btnSelectRiver.x, btnSelectRiver.y + btnSelectRiver.h);
    const Sprite & spritePanelGround = AGG::GetICN(ICN::EDITPANL, 0);
    const Sprite & spritePanelObject = AGG::GetICN(ICN::EDITPANL, 1);
    const Sprite & spritePanelInfo = AGG::GetICN(ICN::EDITPANL, 2);
    const Sprite & spritePanelRiver = AGG::GetICN(ICN::EDITPANL, 3);
    const Sprite & spritePanelRoad = AGG::GetICN(ICN::EDITPANL, 4);
    const Sprite & spritePanelClear = AGG::GetICN(ICN::EDITPANL, 5);
*/

    btnLeftTopScroll.Draw();
    btnRightTopScroll.Draw();
    btnTopScroll.Draw();
    btnLeftBottomScroll.Draw();
    btnLeftScroll.Draw();
    btnRightScroll.Draw();
    btnRightBottomScroll.Draw();
    btnBottomScroll.Draw();
    btnZoom.Draw();
    btnUndo.Draw();
    btnNew.Draw();
    btnSpec.Draw();
    btnFile.Draw();
    btnSystem.Draw();
    btnSelectObject.Draw();
    btnSelectInfo.Draw();
    btnSelectRiver.Draw();
    btnSelectRoad.Draw();
    btnSelectClear.Draw();
    btnSelectGround.Draw();
    btnSizeSmall.Draw();
    btnSizeLarge.Draw();
    btnSizeManual.Draw();
    btnSizeMedium.Press();
    btnSizeMedium.Draw();

    DrawTopNumberCell();
    DrawLeftNumberCell();

    split_h.SetArea(Rect(2 * BORDERWIDTH + 3, display.h() - BORDERWIDTH + 4, (areaMaps.GetRectMaps().w - 2) * TILEWIDTH - 6, BORDERWIDTH - 8));
    split_h.SetOrientation(Splitter::HORIZONTAL);

    split_v.SetArea(Rect(BORDERWIDTH + areaPos.w + 4, 2 * BORDERWIDTH + 3, BORDERWIDTH - 8, (areaMaps.GetRectMaps().h - 2) * TILEWIDTH - 6));
    split_v.SetOrientation(Splitter::VERTICAL);

    split_h.SetRange(0, world.w() - areaMaps.GetRectMaps().w);
    split_v.SetRange(0, world.h() - areaMaps.GetRectMaps().h);

    split_h.Move(areaMaps.GetRectMaps().x);
    split_v.Move(areaMaps.GetRectMaps().y);
}

void EditorInterface::Scroll(const u8 scroll)
{
    if(scroll & SCROLL_LEFT)
    {
	split_h.Backward();
	DrawTopNumberCell();
    }
    else
    if(scroll & SCROLL_RIGHT)
    {
	split_h.Forward();
	DrawTopNumberCell();
    }
    
    if(scroll & SCROLL_TOP)
    {
	split_v.Backward();
	DrawLeftNumberCell();
    }
    else
    if(scroll & SCROLL_BOTTOM)
    {
	split_v.Forward();
	DrawLeftNumberCell();
    }
}

void EditorInterface::DrawTopNumberCell(void)
{
    const Rect & area = Interface::GameArea::Get().GetRectMaps();
    Point dst_pt;

    // top number cell
    for(u16 ii = 0; ii < area.w - 1; ++ii)
    {
	dst_pt.x = BORDERWIDTH + ii * TILEWIDTH;
	dst_pt.y = 0;

	Display::Get().Blit(AGG::GetICN(ICN::EDITBTNS, 34), dst_pt);

	std::string number;
	String::AddInt(number, area.x + ii);

	Text text(number, Font::SMALL);
	text.Blit(2 * BORDERWIDTH + ii * TILEWIDTH - text.w() / 2, 2);
    }
}

void EditorInterface::DrawLeftNumberCell(void)
{
    const Rect & area = Interface::GameArea::Get().GetRectMaps();
    Point dst_pt;

    // left number cell
    for(u16 ii = 0; ii < area.h - 1; ++ii)
    {
	dst_pt.x = 0;
	dst_pt.y = BORDERWIDTH + ii * TILEWIDTH;

	Display::Get().Blit(AGG::GetICN(ICN::EDITBTNS, 33), dst_pt);

	std::string number;
	String::AddInt(number, area.y + ii);

	Text text(number, Font::SMALL);
 	text.Blit(BORDERWIDTH / 2 - text.w() / 2 - 1, BORDERWIDTH + ii * TILEWIDTH + BORDERWIDTH - 5);
    }
}

#endif
