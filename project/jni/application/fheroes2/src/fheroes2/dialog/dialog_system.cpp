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

#include "agg.h"
#include "button.h"
#include "cursor.h"
#include "settings.h"
#include "background.h"
#include "dialog.h"

namespace Dialog
{
    void DrawSystemInfo(const Point & dst);
}

/* return 0x01 - change speed, 0x02 - change sound, 0x04 - change music, 0x08 - change interface, 0x10 - change scroll  */
u8 Dialog::SystemOptions(void)
{
    // FIXME: QVGA version
    if(Settings::Get().QVGA())
    {
       Dialog::Message("", _("For the QVGA version is not available."), Font::SMALL, Dialog::OK);
       return 0;
    }

    Display & display = Display::Get();
    Settings & conf = Settings::Get();

    // preload
    const ICN::icn_t spanbkg = conf.EvilInterface() ? ICN::SPANBKGE : ICN::SPANBKG;
    const ICN::icn_t spanbtn = conf.EvilInterface() ? ICN::SPANBTNE : ICN::SPANBTN;

    AGG::PreloadObject(spanbkg);
    AGG::PreloadObject(spanbtn);
    AGG::PreloadObject(ICN::SPANEL);

    // cursor
    Cursor & cursor = Cursor::Get();
    const Cursor::themes_t oldcursor = cursor.Themes();
    cursor.Hide();
    cursor.SetThemes(cursor.POINTER);

    // image box
    const Sprite &box = AGG::GetICN(spanbkg, 0);

    Rect rb((display.w() - box.w()) / 2, (display.h() - box.h()) / 2, box.w(), box.h());
    Background back(rb);
    back.Save();
    display.Blit(box, rb.x, rb.y);

    const Rect rect1(rb.x + 36,  rb.y + 47,  64, 64);
    const Rect rect2(rb.x + 128, rb.y + 47,  64, 64);
    //const Rect rect3(rb.x + 220, rb.y + 47,  64, 64);
    const Rect rect4(rb.x + 36,  rb.y + 157, 64, 64);
    const Rect rect5(rb.x + 128, rb.y + 157, 64, 64);
    const Rect rect6(rb.x + 220, rb.y + 157, 64, 64);
    const Rect rect7(rb.x + 36,  rb.y + 267, 64, 64);
    //const Rect rect8(rb.x + 128, rb.y + 267, 64, 64);
    //const Rect rect9(rb.x + 220, rb.y + 267, 64, 64);

    Surface back2(rb.w, rb.h);
    back2.Blit(display, rb, 0, 0);

    DrawSystemInfo(rb);

    LocalEvent & le = LocalEvent::Get();

    Button buttonOk(rb.x + 113, rb.y + 362, spanbtn, 0, 1);

    buttonOk.Draw();

    cursor.Show();
    display.Flip();

    u8 result = 0;

    // dialog menu loop
    while(le.HandleEvents())
    {
        le.MousePressLeft(buttonOk) ? buttonOk.PressDraw() : buttonOk.ReleaseDraw();

        if(le.MouseClickLeft(buttonOk) || Game::HotKeyPress(Game::EVENT_DEFAULT_READY)){ break; }
        if(Game::HotKeyPress(Game::EVENT_DEFAULT_EXIT)){ result = false; break; }

        // set sound volume
        if(conf.Sound() && le.MouseClickLeft(rect1))
        {
    	    conf.SetSoundVolume(10 > conf.SoundVolume() ? conf.SoundVolume() + 1 : 0);
    	    cursor.Hide();
    	    display.Blit(back2, rb);
	    DrawSystemInfo(rb);
    	    cursor.Show();
    	    display.Flip();
    	    Game::EnvironmentSoundMixer();
    	}

        // set music volume
        if(conf.Music() && le.MouseClickLeft(rect2))
        {
    	    conf.SetMusicVolume(10 > conf.MusicVolume() ? conf.MusicVolume() + 1 : 0);
    	    cursor.Hide();
    	    display.Blit(back2, rb);
	    DrawSystemInfo(rb);
    	    cursor.Show();
    	    display.Flip();
    	    Music::Volume(Mixer::MaxVolume() * conf.MusicVolume() / 10);
    	}

        // set hero speed
        if(le.MouseClickLeft(rect4))
        {
    	    conf.SetHeroesMoveSpeed(10 > conf.HeroesMoveSpeed() ? conf.HeroesMoveSpeed() + 1 : 0);
    	    result |= 0x01;
    	    cursor.Hide();
    	    display.Blit(back2, rb);
	    DrawSystemInfo(rb);
    	    cursor.Show();
    	    display.Flip();
	    Game::UpdateHeroesMoveSpeed();
    	}

        // set ai speed
        if(le.MouseClickLeft(rect5))
        {
    	    conf.SetAIMoveSpeed(10 > conf.AIMoveSpeed() ? conf.AIMoveSpeed() + 1 : 0);
    	    result |= 0x01;
    	    cursor.Hide();
    	    display.Blit(back2, rb);
	    DrawSystemInfo(rb);
    	    cursor.Show();
    	    display.Flip();
	    Game::UpdateHeroesMoveSpeed();
    	}

        // set scroll speed
        if(le.MouseClickLeft(rect6))
        {
    	    conf.SetScrollSpeed(SCROLL_FAST2 > conf.ScrollSpeed() ? conf.ScrollSpeed() << 1 : SCROLL_SLOW);
    	    result |= 0x10;
    	    cursor.Hide();
    	    display.Blit(back2, rb);
	    DrawSystemInfo(rb);
    	    cursor.Show();
    	    display.Flip();
    	}

        // set interface
        if(le.MouseClickLeft(rect7))
        {
    	    conf.SetEvilInterface(!conf.EvilInterface());
    	    result |= 0x08;
    	    cursor.Hide();
    	    display.Blit(back2, rb);
	    DrawSystemInfo(rb);
    	    cursor.Show();
    	    display.Flip();
    	}
    }

    // restore background
    cursor.Hide();
    back.Restore();
    cursor.SetThemes(oldcursor);
    cursor.Show();
    display.Flip();

    return result;
}

void Dialog::DrawSystemInfo(const Point & dst)
{
    Display & display = Display::Get();
    Settings & conf = Settings::Get();

    std::string str;
    Text text;

    Surface black(65, 65);
    black.Fill(0, 0, 0);

    // sound
    const Sprite & sprite1 = AGG::GetICN(ICN::SPANEL, conf.Sound() ? 1 : 0);
    const Rect rect1(dst.x + 36, dst.y + 47, sprite1.w(), sprite1.h());
    display.Blit(sprite1, rect1);
    str.clear();
    str = _("sound");
    str += " ";
    if(conf.Sound() && conf.SoundVolume())
	String::AddInt(str, conf.SoundVolume());
    else
	str += _("off");
    text.Set(str, Font::SMALL);
    text.Blit(rect1.x + (rect1.w - text.w()) / 2, rect1.y + rect1.h + 5);

    // music
    const Sprite & sprite2 = AGG::GetICN(ICN::SPANEL, conf.Music() ? 3 : 2);
    const Rect rect2(dst.x + 128, dst.y + 47, sprite2.w(), sprite2.h());
    display.Blit(sprite2, rect2);
    str.clear();
    str = _("music");
    str += " ";
    if(conf.Music() && conf.MusicVolume())
	String::AddInt(str, conf.MusicVolume());
    else
	str += _("off");
    text.Set(str);
    text.Blit(rect2.x + (rect2.w - text.w()) / 2, rect2.y + rect2.h + 5);

    // unused
    const Sprite & sprite3 = AGG::GetICN(ICN::SPANEL, 17);
    const Rect rect3(dst.x + 220, dst.y + 47, sprite3.w(), sprite3.h());
    display.Blit(black, rect3);
    str.clear();
    str = "unused";
    text.Set(str);
    text.Blit(rect3.x + (rect3.w - text.w()) / 2, rect3.y + rect3.h + 5);

    // hero move speed
    const u8 is4 = conf.HeroesMoveSpeed() ? (conf.HeroesMoveSpeed() < 9 ? (conf.HeroesMoveSpeed() < 7 ? (conf.HeroesMoveSpeed() < 4 ? 4 : 5) : 6) : 7) : 9;
    const Sprite & sprite4 = AGG::GetICN(ICN::SPANEL, is4);
    const Rect rect4(dst.x + 36, dst.y + 157, sprite4.w(), sprite4.h());
    display.Blit(sprite4, rect4);
    str.clear();
    str = _("hero speed");
    str += " ";
    if(conf.HeroesMoveSpeed())
	String::AddInt(str, conf.HeroesMoveSpeed());
    else
	str += _("off");
    text.Set(str);
    text.Blit(rect4.x + (rect4.w - text.w()) / 2, rect4.y + rect4.h + 5);

    // ai move speed
    const u8 is5 = conf.AIMoveSpeed() ? (conf.AIMoveSpeed() < 9 ? (conf.AIMoveSpeed() < 7 ? (conf.AIMoveSpeed() < 4 ? 4 : 5) : 6) : 7) : 9;
    const Sprite & sprite5 = AGG::GetICN(ICN::SPANEL, is5);
    const Rect rect5(dst.x + 128, dst.y + 157, sprite5.w(), sprite5.h());
    display.Blit(sprite5, rect5);
    str.clear();
    str = _("ai speed");
    str += " ";
    if(conf.AIMoveSpeed())
	String::AddInt(str, conf.AIMoveSpeed());
    else
	str += _("off");
    text.Set(str);
    text.Blit(rect5.x + (rect5.w - text.w()) / 2, rect5.y + rect5.h + 5);

    // scroll speed
    const u8 is6 = (conf.ScrollSpeed() < SCROLL_FAST2 ? (conf.ScrollSpeed() < SCROLL_FAST1 ? (conf.ScrollSpeed() < SCROLL_NORMAL ? 4 : 5) : 6) : 7);
    const Sprite & sprite6 = AGG::GetICN(ICN::SPANEL, is6);
    const Rect rect6(dst.x + 220, dst.y + 157, sprite6.w(), sprite6.h());
    display.Blit(sprite6, rect6);
    str.clear();
    str = _("scroll speed");
    str += " ";
    String::AddInt(str, conf.ScrollSpeed());
    text.Set(str);
    text.Blit(rect6.x + (rect6.w - text.w()) / 2, rect6.y + rect6.h + 5);

    // interface
    const Sprite & sprite7 = AGG::GetICN(ICN::SPANEL, (conf.EvilInterface() ? 17 : 16));
    const Rect rect7(dst.x + 36, dst.y + 267, sprite7.w(), sprite7.h());
    display.Blit(sprite7, rect7);
    str.clear();
    str = _("Interface");
    str += ": ";
    if(conf.EvilInterface())
	str += _("Evil");
    else
	str += _("Good");
    text.Set(str);
    text.Blit(rect7.x + (rect7.w - text.w()) / 2, rect7.y + rect7.h + 5);

    // unused
    const Sprite & sprite8 = AGG::GetICN(ICN::SPANEL, 17);
    const Rect rect8(dst.x + 128, dst.y + 267, sprite8.w(), sprite8.h());
    display.Blit(black, rect8);
    str.clear();
    str = "unused";
    text.Set(str);
    text.Blit(rect8.x + (rect8.w - text.w()) / 2, rect8.y + rect8.h + 5);

    // unused
    const Sprite & sprite9 = AGG::GetICN(ICN::SPANEL, 17);
    const Rect rect9(dst.x + 220, dst.y + 267, sprite9.w(), sprite9.h());
    display.Blit(black, rect9);
    str.clear();
    str = "unused";
    text.Set(str);
    text.Blit(rect9.x + (rect9.w - text.w()) / 2, rect9.y + rect9.h + 5);
}
