/***************************************************************************
 *   Copyright (C) 2009 by Andrey Afletdinov                               *
 *   afletdinov@mail.dc.baikal.ru                                          *
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

#include "agg.h"
#include "engine.h"
#include "cursor.h"
#include "battle2.h"
#include "text.h"
#include "settings.h"
#include "pocketpc.h"

u16 PocketPC::GetCursorAttackDialog(const Point & dst, u8 allow)
{
    Display & display = Display::Get();
    LocalEvent & le = LocalEvent::Get();

    const Rect rt(dst.x - 24, dst.y - 24, 70, 70);

    const Sprite & sp_info = AGG::GetICN(ICN::CMSECO, 5);
    const Sprite & sp_bleft = AGG::GetICN(ICN::CMSECO, 10);
    const Sprite & sp_left = AGG::GetICN(ICN::CMSECO, 11);
    const Sprite & sp_tleft = AGG::GetICN(ICN::CMSECO, 12);
    const Sprite & sp_tright = AGG::GetICN(ICN::CMSECO, 7);
    const Sprite & sp_right = AGG::GetICN(ICN::CMSECO, 8);
    const Sprite & sp_bright = AGG::GetICN(ICN::CMSECO, 9);

    Surface shadow(rt.w, rt.h);
    shadow.SetAlpha(170);
    Background back(rt);
    back.Save();

    Cursor & cursor = Cursor::Get();
    cursor.Hide();
    cursor.SetThemes(Cursor::POINTER);

    display.Blit(shadow, rt.x, rt.y);

    const Rect rt_info(rt.x + (rt.w - sp_info.w()) / 2, rt.y + (rt.h - sp_info.h()) / 2, sp_info.w(), sp_info.h());
    display.Blit(sp_info, rt_info.x, rt_info.y);

    const Rect rt_tright(rt.x + 1, rt.y + rt.h - 1 - sp_tright.h(), sp_tright.w(), sp_tright.h());
    if(allow & Battle2::BOTTOM_LEFT) display.Blit(sp_tright, rt_tright.x, rt_tright.y);

    const Rect rt_right(rt.x + 1, rt.y + (rt.h - sp_right.h()) / 2, sp_right.w(), sp_right.h());
    if(allow & Battle2::LEFT) display.Blit(sp_right, rt_right.x, rt_right.y);

    const Rect rt_bright(rt.x + 1, rt.y + 1, sp_bright.w(), sp_bright.h());
    if(allow & Battle2::TOP_LEFT) display.Blit(sp_bright, rt_bright.x, rt_bright.y);

    const Rect rt_tleft(rt.x + rt.w - 1 - sp_tleft.w(), rt.y + rt.h - 1 - sp_tleft.h(), sp_tleft.w(), sp_tleft.h());
    if(allow & Battle2::BOTTOM_RIGHT) display.Blit(sp_tleft, rt_tleft.x, rt_tleft.y);

    const Rect rt_left(rt.x + rt.w - 1 - sp_left.w(), rt.y + (rt.h - sp_left.h()) / 2, sp_left.w(), sp_left.h());
    if(allow & Battle2::RIGHT) display.Blit(sp_left, rt_left.x, rt_left.y);

    const Rect rt_bleft(rt.x + rt.w - 1 - sp_bleft.w(), rt.y + 1, sp_bleft.w(), sp_bleft.h());
    if(allow & Battle2::TOP_RIGHT) display.Blit(sp_bleft, rt_bleft.x, rt_bleft.y);

    cursor.Show();
    display.Flip();

    while(le.HandleEvents() && !le.MouseClickLeft());

    if((allow & Battle2::BOTTOM_LEFT) && (rt_tright & le.GetMouseCursor()))	return Cursor::SWORD_TOPRIGHT;
    else
    if((allow & Battle2::LEFT) && (rt_right & le.GetMouseCursor()))		return Cursor::SWORD_RIGHT;
    else
    if((allow & Battle2::TOP_LEFT) && (rt_bright & le.GetMouseCursor()))	return Cursor::SWORD_BOTTOMRIGHT;
    else
    if((allow & Battle2::BOTTOM_RIGHT) && (rt_tleft & le.GetMouseCursor()))	return Cursor::SWORD_TOPLEFT;
    else
    if((allow & Battle2::RIGHT) && (rt_left & le.GetMouseCursor()))		return Cursor::SWORD_LEFT;
    else
    if((allow & Battle2::TOP_RIGHT) && (rt_bleft & le.GetMouseCursor()))	return Cursor::SWORD_BOTTOMLEFT;

    return Cursor::WAR_INFO;
}

void DrawWideCell(const Rect & dst)
{
    Display & display = Display::Get();
    const Sprite & sp = AGG::GetICN(ICN::EDITBTNS, 32);

    Rect rt = Rect(0, 0, 4, 16);
    display.Blit(sp, rt, dst.x, dst.y);


    if(dst.w > 8)
    {
	rt = Rect(4, 0, 4, 16);
	const u16 count = (dst.w - 4) / rt.w;
	for(u16 ii = 0; ii < count; ++ii)
	{
	    display.Blit(sp, rt, dst.x + 4 + rt.w * ii, dst.y);
	}
    }

    rt = Rect(12, 0, 4, 16);
    display.Blit(sp, rt, dst.x + dst.w - rt.w, dst.y);
}

void PocketPC::KeyboardDialog(std::string & str)
{
    Cursor & cursor = Cursor::Get();
    Display & display = Display::Get();
    LocalEvent & le = LocalEvent::Get();
    cursor.Hide();

    const u16 width = 240;
    const u16 height = 88;

    Background back;
    back.Save((display.w() - width) / 2, 0, width, height);
    const Rect & top = back.GetRect();
    display.FillRect(0, 0, 0, top);

    const Sprite & sp = AGG::GetICN(ICN::EDITBTNS, 32);

    Text tx;
    tx.Set(Font::SMALL);

    // 1
    const Rect rt_1(top.x + 2, top.y + 2, sp.w(), sp.h());
    display.Blit(sp, rt_1);
    tx.Set("1");
    tx.Blit(rt_1.x + (rt_1.w - tx.w()) / 2, rt_1.y + 2);

    const Rect rt_2(top.x + 19, top.y + 2, sp.w(), sp.h());
    display.Blit(sp, rt_2);
    tx.Set("2");
    tx.Blit(rt_2.x + (rt_2.w - tx.w()) / 2, rt_2.y + 2);

    const Rect rt_3(top.x + 36, top.y + 2, sp.w(), sp.h());
    display.Blit(sp, rt_3);
    tx.Set("3");
    tx.Blit(rt_3.x + (rt_3.w - tx.w()) / 2, rt_3.y + 2);

    const Rect rt_4(top.x + 53, top.y + 2, sp.w(), sp.h());
    display.Blit(sp, rt_4);
    tx.Set("4");
    tx.Blit(rt_4.x + (rt_4.w - tx.w()) / 2, rt_4.y + 2);

    const Rect rt_5(top.x + 70, top.y + 2, sp.w(), sp.h());
    display.Blit(sp, rt_5);
    tx.Set("5");
    tx.Blit(rt_5.x + (rt_5.w - tx.w()) / 2, rt_5.y + 2);

    const Rect rt_6(top.x + 87, top.y + 2, sp.w(), sp.h());
    display.Blit(sp, rt_6);
    tx.Set("6");
    tx.Blit(rt_6.x + (rt_6.w - tx.w()) / 2, rt_6.y + 2);

    const Rect rt_7(top.x + 104, top.y + 2, sp.w(), sp.h());
    display.Blit(sp, rt_7);
    tx.Set("7");
    tx.Blit(rt_7.x + (rt_7.w - tx.w()) / 2, rt_7.y + 2);

    const Rect rt_8(top.x + 121, top.y + 2, sp.w(), sp.h());
    display.Blit(sp, rt_8);
    tx.Set("8");
    tx.Blit(rt_8.x + (rt_8.w - tx.w()) / 2, rt_8.y + 2);

    const Rect rt_9(top.x + 138, top.y + 2, sp.w(), sp.h());
    display.Blit(sp, rt_9);
    tx.Set("9");
    tx.Blit(rt_9.x + (rt_9.w - tx.w()) / 2, rt_9.y + 2);

    const Rect rt_0(top.x + 155, top.y + 2, sp.w(), sp.h());
    display.Blit(sp, rt_0);
    tx.Set("0");
    tx.Blit(rt_0.x + (rt_0.w - tx.w()) / 2, rt_0.y + 2);

    const Rect rt_MINUS(top.x + 172, top.y + 2, sp.w(), sp.h());
    display.Blit(sp, rt_MINUS);
    tx.Set("-");
    tx.Blit(rt_MINUS.x + (rt_MINUS.w - tx.w()) / 2, rt_MINUS.y + 2);

    const Rect rt_EQUAL(top.x + 189, top.y + 2, sp.w(), sp.h());
    display.Blit(sp, rt_EQUAL);
    tx.Set("=");
    tx.Blit(rt_EQUAL.x + (rt_EQUAL.w - tx.w()) / 2, rt_EQUAL.y + 1);

    const Rect rt_BACKSPACE(top.x + 206, top.y + 2, 32, sp.h());
    DrawWideCell(rt_BACKSPACE);

    tx.Set("back");
    tx.Blit(rt_BACKSPACE.x + 2, rt_BACKSPACE.y + 1);

    // 2
    const Rect rt_EMPTY1(top.x + 2, top.y + 19, 7, sp.h());
    DrawWideCell(rt_EMPTY1);

    const Rect rt_Q(top.x + 10, top.y + 19, sp.w(), sp.h());
    display.Blit(sp, rt_Q);
    tx.Set("q");
    tx.Blit(rt_Q.x + (rt_Q.w - tx.w()) / 2, rt_Q.y + 2);

    const Rect rt_W(top.x + 27, top.y + 19, sp.w(), sp.h());
    display.Blit(sp, rt_W);
    tx.Set("w");
    tx.Blit(rt_W.x + (rt_W.w - tx.w()) / 2, rt_W.y + 2);

    const Rect rt_E(top.x + 44, top.y + 19, sp.w(), sp.h());
    display.Blit(sp, rt_E);
    tx.Set("e");
    tx.Blit(rt_E.x + (rt_E.w - tx.w()) / 2, rt_E.y + 2);

    const Rect rt_R(top.x + 61, top.y + 19, sp.w(), sp.h());
    display.Blit(sp, rt_R);
    tx.Set("r");
    tx.Blit(rt_R.x + (rt_R.w - tx.w()) / 2, rt_R.y + 2);

    const Rect rt_T(top.x + 78, top.y + 19, sp.w(), sp.h());
    display.Blit(sp, rt_T);
    tx.Set("t");
    tx.Blit(rt_T.x + (rt_T.w - tx.w()) / 2, rt_T.y + 2);

    const Rect rt_Y(top.x + 95, top.y + 19, sp.w(), sp.h());
    display.Blit(sp, rt_Y);
    tx.Set("y");
    tx.Blit(rt_Y.x + (rt_Y.w - tx.w()) / 2, rt_Y.y + 2);

    const Rect rt_U(top.x + 112, top.y + 19, sp.w(), sp.h());
    display.Blit(sp, rt_U);
    tx.Set("u");
    tx.Blit(rt_U.x + (rt_U.w - tx.w()) / 2, rt_U.y + 2);

    const Rect rt_I(top.x + 129, top.y + 19, sp.w(), sp.h());
    display.Blit(sp, rt_I);
    tx.Set("i");
    tx.Blit(rt_I.x + (rt_I.w - tx.w()) / 2, rt_I.y + 2);

    const Rect rt_O(top.x + 146, top.y + 19, sp.w(), sp.h());
    display.Blit(sp, rt_O);
    tx.Set("o");
    tx.Blit(rt_O.x + (rt_O.w - tx.w()) / 2, rt_O.y + 2);

    const Rect rt_P(top.x + 163, top.y + 19, sp.w(), sp.h());
    display.Blit(sp, rt_P);
    tx.Set("p");
    tx.Blit(rt_P.x + (rt_P.w - tx.w()) / 2, rt_P.y + 2);

    const Rect rt_LB(top.x + 180, top.y + 19, sp.w(), sp.h());
    display.Blit(sp, rt_LB);
    tx.Set("[");
    tx.Blit(rt_LB.x + (rt_LB.w - tx.w()) / 2, rt_LB.y + 2);

    const Rect rt_RB(top.x + 197, top.y + 19, sp.w(), sp.h());
    display.Blit(sp, rt_RB);
    tx.Set("]");
    tx.Blit(rt_RB.x + (rt_RB.w - tx.w()) / 2, rt_RB.y + 2);

    const Rect rt_EMPTY2(top.x + 214, top.y + 19, 24, sp.h());
    DrawWideCell(rt_EMPTY2);

    // 3
    const Rect rt_EMPTY3(top.x + 2, top.y + 36, 15, sp.h());
    DrawWideCell(rt_EMPTY3);

    const Rect rt_A(top.x + 18, top.y + 36, sp.w(), sp.h());
    display.Blit(sp, rt_A);
    tx.Set("a");
    tx.Blit(rt_A.x + (rt_A.w - tx.w()) / 2, rt_A.y + 2);

    const Rect rt_S(top.x + 35, top.y + 36, sp.w(), sp.h());
    display.Blit(sp, rt_S);
    tx.Set("s");
    tx.Blit(rt_S.x + (rt_S.w - tx.w()) / 2, rt_S.y + 2);

    const Rect rt_D(top.x + 52, top.y + 36, sp.w(), sp.h());
    display.Blit(sp, rt_D);
    tx.Set("d");
    tx.Blit(rt_D.x + (rt_D.w - tx.w()) / 2, rt_D.y + 2);

    const Rect rt_F(top.x + 69, top.y + 36, sp.w(), sp.h());
    display.Blit(sp, rt_F);
    tx.Set("f");
    tx.Blit(rt_F.x + (rt_F.w - tx.w()) / 2, rt_F.y + 2);

    const Rect rt_G(top.x + 86, top.y + 36, sp.w(), sp.h());
    display.Blit(sp, rt_G);
    tx.Set("g");
    tx.Blit(rt_G.x + (rt_G.w - tx.w()) / 2, rt_G.y + 2);

    const Rect rt_H(top.x + 103, top.y + 36, sp.w(), sp.h());
    display.Blit(sp, rt_H);
    tx.Set("h");
    tx.Blit(rt_H.x + (rt_H.w - tx.w()) / 2, rt_H.y + 2);

    const Rect rt_J(top.x + 120, top.y + 36, sp.w(), sp.h());
    display.Blit(sp, rt_J);
    tx.Set("j");
    tx.Blit(rt_J.x + (rt_J.w - tx.w()) / 2, rt_J.y + 2);

    const Rect rt_K(top.x + 137, top.y + 36, sp.w(), sp.h());
    display.Blit(sp, rt_K);
    tx.Set("k");
    tx.Blit(rt_K.x + (rt_K.w - tx.w()) / 2, rt_K.y + 2);

    const Rect rt_L(top.x + 154, top.y + 36, sp.w(), sp.h());
    display.Blit(sp, rt_L);
    tx.Set("l");
    tx.Blit(rt_L.x + (rt_L.w - tx.w()) / 2, rt_L.y + 2);

    const Rect rt_SP(top.x + 171, top.y + 36, sp.w(), sp.h());
    display.Blit(sp, rt_SP);
    tx.Set(";");
    tx.Blit(rt_SP.x + (rt_SP.w - tx.w()) / 2, rt_SP.y + 2);

    const Rect rt_CM(top.x + 188, top.y + 36, sp.w(), sp.h());
    display.Blit(sp, rt_CM);
    tx.Set("'");
    tx.Blit(rt_CM.x + (rt_CM.w - tx.w()) / 2, rt_CM.y + 2);

    const Rect rt_RETURN(top.x + 205, top.y + 36, 33, sp.h());
    DrawWideCell(rt_RETURN);

    tx.Set("rtrn");
    tx.Blit(rt_RETURN.x + (rt_RETURN.w - tx.w()) / 2, rt_RETURN.y + 2);

    // 4
    const Rect rt_EMPTY5(top.x + 2, top.y + 53, 23, sp.h());
    DrawWideCell(rt_EMPTY5);

    const Rect rt_Z(top.x + 26, top.y + 53, sp.w(), sp.h());
    display.Blit(sp, rt_Z);
    tx.Set("z");
    tx.Blit(rt_Z.x + (rt_Z.w - tx.w()) / 2, rt_Z.y + 2);

    const Rect rt_X(top.x + 43, top.y + 53, sp.w(), sp.h());
    display.Blit(sp, rt_X);
    tx.Set("x");
    tx.Blit(rt_X.x + (rt_X.w - tx.w()) / 2, rt_X.y + 2);

    const Rect rt_C(top.x + 60, top.y + 53, sp.w(), sp.h());
    display.Blit(sp, rt_C);
    tx.Set("c");
    tx.Blit(rt_C.x + (rt_C.w - tx.w()) / 2, rt_C.y + 2);

    const Rect rt_V(top.x + 77, top.y + 53, sp.w(), sp.h());
    display.Blit(sp, rt_V);
    tx.Set("v");
    tx.Blit(rt_V.x + (rt_V.w - tx.w()) / 2, rt_V.y + 2);

    const Rect rt_B(top.x + 94, top.y + 53, sp.w(), sp.h());
    display.Blit(sp, rt_B);
    tx.Set("b");
    tx.Blit(rt_B.x + (rt_B.w - tx.w()) / 2, rt_B.y + 2);

    const Rect rt_N(top.x + 111, top.y + 53, sp.w(), sp.h());
    display.Blit(sp, rt_N);
    tx.Set("n");
    tx.Blit(rt_N.x + (rt_N.w - tx.w()) / 2, rt_N.y + 2);

    const Rect rt_M(top.x + 128, top.y + 53, sp.w(), sp.h());
    display.Blit(sp, rt_M);
    tx.Set("m");
    tx.Blit(rt_M.x + (rt_M.w - tx.w()) / 2, rt_M.y + 2);

    const Rect rt_CS(top.x + 145, top.y + 53, sp.w(), sp.h());
    display.Blit(sp, rt_CS);
    tx.Set(",");
    tx.Blit(rt_CS.x + (rt_CS.w - tx.w()) / 2, rt_CS.y + 2);

    const Rect rt_DT(top.x + 162, top.y + 53, sp.w(), sp.h());
    display.Blit(sp, rt_DT);
    tx.Set(".");
    tx.Blit(rt_DT.x + (rt_DT.w - tx.w()) / 2, rt_DT.y + 2);

    const Rect rt_SL(top.x + 179, top.y + 53, sp.w(), sp.h());
    display.Blit(sp, rt_SL);
    tx.Set("/");
    tx.Blit(rt_SL.x + (rt_SL.w - tx.w()) / 2, rt_SL.y + 2);

    const Rect rt_SPACE(top.x + 196, top.y + 53, 42, sp.h());
    DrawWideCell(rt_SPACE);

    tx.Set("space");
    tx.Blit(rt_SPACE.x + (rt_SPACE.w - tx.w()) / 2, rt_SPACE.y + 2);

    Rect rectClose;

    cursor.Show();
    display.Flip();

    char ch = 0;
    bool redraw = true;

    // mainmenu loop
    while(le.HandleEvents())
    {
        if(le.MouseClickLeft(rectClose) || Game::HotKeyPress(Game::EVENT_DEFAULT_EXIT))
	{
	    str.clear();
	    break;
	}

	ch = 0;

        if(le.MouseClickLeft(rt_0))	ch = '0';
	else
        if(le.MouseClickLeft(rt_1))	ch = '1';
	else
        if(le.MouseClickLeft(rt_2))	ch = '2';
	else
        if(le.MouseClickLeft(rt_3))	ch = '3';
	else
        if(le.MouseClickLeft(rt_4))	ch = '4';
	else
        if(le.MouseClickLeft(rt_5))	ch = '5';
	else
        if(le.MouseClickLeft(rt_6))	ch = '6';
	else
        if(le.MouseClickLeft(rt_7))	ch = '7';
	else
        if(le.MouseClickLeft(rt_8))	ch = '8';
	else
        if(le.MouseClickLeft(rt_9))	ch = '9';
	else
        if(le.MouseClickLeft(rt_A))	ch = 'a';
	else
        if(le.MouseClickLeft(rt_B))	ch = 'b';
	else
        if(le.MouseClickLeft(rt_C))	ch = 'c';
	else
        if(le.MouseClickLeft(rt_D))	ch = 'd';
	else
        if(le.MouseClickLeft(rt_E))	ch = 'e';
	else
        if(le.MouseClickLeft(rt_F))	ch = 'f';
	else
        if(le.MouseClickLeft(rt_G))	ch = 'g';
	else
        if(le.MouseClickLeft(rt_H))	ch = 'h';
	else
        if(le.MouseClickLeft(rt_I))	ch = 'i';
	else
        if(le.MouseClickLeft(rt_J))	ch = 'j';
	else
        if(le.MouseClickLeft(rt_K))	ch = 'k';
	else
        if(le.MouseClickLeft(rt_L))	ch = 'l';
	else
        if(le.MouseClickLeft(rt_M))	ch = 'm';
	else
        if(le.MouseClickLeft(rt_N))	ch = 'n';
	else
        if(le.MouseClickLeft(rt_O))	ch = 'o';
	else
        if(le.MouseClickLeft(rt_P))	ch = 'p';
	else
        if(le.MouseClickLeft(rt_Q))	ch = 'q';
	else
        if(le.MouseClickLeft(rt_R))	ch = 'r';
	else
        if(le.MouseClickLeft(rt_S))	ch = 's';
	else
        if(le.MouseClickLeft(rt_T))	ch = 't';
	else
        if(le.MouseClickLeft(rt_U))	ch = 'u';
	else
        if(le.MouseClickLeft(rt_V))	ch = 'v';
	else
        if(le.MouseClickLeft(rt_W))	ch = 'w';
	else
        if(le.MouseClickLeft(rt_X))	ch = 'x';
	else
        if(le.MouseClickLeft(rt_Y))	ch = 'y';
	else
        if(le.MouseClickLeft(rt_Z))	ch = 'z';
	else
	if(le.MouseClickLeft(rt_EQUAL))	ch = '=';
	else
	if(le.MouseClickLeft(rt_MINUS))	ch = '-';
	else
	if(le.MouseClickLeft(rt_LB))	ch = '[';
	else
	if(le.MouseClickLeft(rt_RB))	ch = ']';
        else
	if(le.MouseClickLeft(rt_SP))	ch = ';';
	else
	if(le.MouseClickLeft(rt_CM))	ch = '\'';
	else
        if(le.MouseClickLeft(rt_CS))	ch = ',';
	else
        if(le.MouseClickLeft(rt_DT))	ch = '.';
	else
	if(le.MouseClickLeft(rt_SL))	ch = '/';
	else
	if(le.MouseClickLeft(rt_SPACE))	ch = 0x20;

	if(le.MouseClickLeft(rt_BACKSPACE) && str.size())
	{
	    str.resize(str.size() - 1);
	    redraw = true;
	}
	else
	if(le.MouseClickLeft(rt_RETURN))
	    break;
	else
	if(ch)
	{
	    str += ch;
	    redraw = true;
	}

	if(redraw)
	{
	    tx.Set(str);
	    if(tx.w() < top.w) 
	    {
		cursor.Hide();
		display.FillRect(0, 0, 0, Rect(top.x, top.y + top.h - 16, top.w, 16));
		tx.Blit(top.x + (top.w - tx.w()) / 2, top.y + top.h - 16 + 2);
		cursor.Show();
		display.Flip();
	    }
	    redraw = false;
	}
    }

    cursor.Hide();
    back.Restore();
    cursor.Show();
    display.Flip();
}
