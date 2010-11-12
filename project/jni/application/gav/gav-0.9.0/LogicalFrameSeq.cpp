/* -*- C++ -*- */
/*
  GAV - Gpl Arcade Volleyball
  
  Copyright (C) 2002
  GAV team (http://sourceforge.net/projects/gav/)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "LogicalFrameSeq.h"
#include "GameRenderer.h"
#include "globals.h"

using namespace std;

/*
  start - the starting width
  zoom - the zoom factor calculated from background ratio
  step - the new width must be a 'step' multiple

  returns the zoom factor nearest to 'zoom', in order to obtain a 'step'
  multiple.
 */
double exactScaleFactor(int startw, double zoom, int step) {
  if ( step == 1 ) return zoom;
  double corr = 0.0;
  double ratio = 1/zoom;
  double fract = (double) startw / ratio;
  int temp = (int) round(fract / step);
  int adjustedResize = temp * step;
  //  printf("W: %d NP: %d\n", startw, temp);
  
  double adjustedRatio = (double) startw / (double) adjustedResize;

  /*
  int tmp2 = (int) (startw / adjustedRatio);
  //  printf ("ar: %g  tmp2: %d\n", adjustedRatio, tmp2);
  if ( tmp2 < adjustedResize )
    corr = 0.5;
  */

  return (1/adjustedRatio + (corr/(double) startw));
}


void LogicalFrameSeq::blit(int idx, SDL_Surface *dest,
			   SDL_Rect *rect) {
  gameRenderer->display(dest, rect, _actualFrameSeq, idx);
}
