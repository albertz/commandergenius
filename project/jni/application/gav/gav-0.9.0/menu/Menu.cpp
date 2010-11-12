#include "Menu.h"
#include <string.h>
#include "SoundMgr.h"

using namespace std;


void Menu::add(MenuItem * mi)
{
  if (currentItem < 0)
    currentItem = 0;
  if ((mi->getLabel()).length() > maxLabelLength)
    maxLabelLength = (mi->getLabel()).length();
  items.push_back(mi);
}


int Menu::execute(InputState *is, std::stack<Menu *> &s)
{
  SDL_Rect rect;
  const char * label;
  ScreenFont * font;
  static bool spacePressed = false;
  static bool PrevUp = false;
  static bool PrevDown = false;

  // Up  
  if (is->getKeyState()[SDLK_UP]) {
    if (!PrevUp) {
      if (currentItem > 0) {
        currentItem--;
#ifdef AUDIO
        soundMgr->playSound (SND_MENU_SELECT);
#endif
      } else {
        currentItem=(int)(items.size()-1);
#ifdef AUDIO
        soundMgr->playSound (SND_MENU_SELECT);
#endif
      }
      PrevUp = true;
    }
  } else
    PrevUp =false;

  // Down
  if (is->getKeyState()[SDLK_DOWN]) {
    if (!PrevDown) {
      if (currentItem < (int)(items.size()-1)) {
        currentItem++;
#ifdef AUDIO
        soundMgr->playSound (SND_MENU_SELECT);
#endif
      } else {
        currentItem=0;
#ifdef AUDIO
        soundMgr->playSound (SND_MENU_SELECT);
#endif
      }
      PrevDown = true;
    }
  } else
    PrevDown = false;

  // draw menu items labels
  rect.y = configuration.CEILING + configuration.env.h/100;
  for ( unsigned int it = 0; it < items.size(); it++ ) {
    label = items[it]->getLabel().c_str();
    rect.x = (configuration.env.w / 2) -
      strlen(label)*(cga->charWidth())/2; // UGLY
    if (it == (unsigned int) currentItem)
      font = cgaInv;
    else
      font = cga;
    font->printXY(screen, &rect, items[it]->getLabel().c_str());
    rect.y += font->charHeight();
  }
  
  // call the execute method of the current item, if an event occurred
  if ( (is->getKeyState()[SDLK_SPACE]) ||
       (is->getKeyState()[SDLK_RETURN]) ) {
    spacePressed = true;
  }
  if ( spacePressed && !(is->getKeyState()[SDLK_SPACE]) &&
       !(is->getKeyState()[SDLK_RETURN]) ) {
    spacePressed = false;
#ifdef AUDIO
    soundMgr->playSound(SND_MENU_ACTIVATE);
#endif // AUDIO
    return items[currentItem]->execute(s);
  }
  return 0;
}

