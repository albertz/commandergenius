/*
Simple DirectMedia Layer
Java source code (C) 2009-2012 Sergii Pylypenko
  
This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:
  
1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required. 
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

package net.sourceforge.clonekeenplus;

import android.app.Activity;
import android.content.Context;
import android.view.MotionEvent;
import android.view.KeyEvent;
import android.view.Window;
import android.view.WindowManager;
import android.widget.TextView;
import android.view.View;

import com.google.ads.*; // Copy GoogleAdMobAdsSdk.jar to the directory project/libs

class Advertisement
{
	private AdView ad;
	MainActivity parent;

	public Advertisement(MainActivity p)
	{
		parent = p;
		AdSize adSize = AdSize.BANNER;
		if( Globals.AdmobBannerSize.equals("BANNER") )
			adSize = AdSize.BANNER;
		else if( Globals.AdmobBannerSize.equals("IAB_BANNER") )
			adSize = AdSize.IAB_BANNER;
		else if( Globals.AdmobBannerSize.equals("IAB_LEADERBOARD") )
			adSize = AdSize.IAB_LEADERBOARD;
		else if( Globals.AdmobBannerSize.equals("IAB_MRECT") )
			adSize = AdSize.IAB_MRECT;
		else if( Globals.AdmobBannerSize.equals("IAB_WIDE_SKYSCRAPER") )
			adSize = AdSize.IAB_WIDE_SKYSCRAPER;
		else if( Globals.AdmobBannerSize.equals("SMART_BANNER") )
			adSize = AdSize.SMART_BANNER;
		ad = new AdView(parent, adSize, Globals.AdmobPublisherId);
		AdRequest adRequest = new AdRequest();
		adRequest.addTestDevice(AdRequest.TEST_EMULATOR); // Copy GoogleAdMobAdsSdk.jar to the directory project/libs
		adRequest.addTestDevice(Globals.AdmobTestDeviceId);
		ad.loadAd(adRequest);
	}

	public View getView()
	{
		return ad;
	}
}
