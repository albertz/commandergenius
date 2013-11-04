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

import org.libsdl.app.SDLActivity;
import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.KeyEvent;
import android.view.Window;
import android.view.WindowManager;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;
import android.widget.EditText;
import android.text.Editable;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.FrameLayout;
import android.graphics.drawable.Drawable;
import android.graphics.Color;
import android.content.res.Configuration;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.PendingIntent;
import android.content.Intent;
import android.view.View.OnKeyListener;
import android.view.MenuItem;
import android.view.Menu;
import android.view.Gravity;
import android.text.method.TextKeyListener;
import java.util.LinkedList;
import java.io.SequenceInputStream;
import java.io.BufferedInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.FileOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.util.zip.*;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;
import java.util.Set;
import android.text.SpannedString;
import java.io.BufferedReader;
import java.io.BufferedInputStream;
import java.io.InputStreamReader;
import android.view.inputmethod.InputMethodManager;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;
import java.util.concurrent.Semaphore;
import android.content.pm.ActivityInfo;
import android.view.Display;
import android.text.InputType;
import android.util.Log;

/* 
 * A sample wrapper class that just calls SDLActivity 
 */ 

public class MainActivity extends SDLActivity 
{ 

    // Load the .so
    static {
        System.loadLibrary("sdl_native_helpers");
        System.loadLibrary("sdl2_image");
        System.loadLibrary("application");
        System.loadLibrary("sdl_main");
    }

	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		Settings.Load(this);
		Settings.Apply(this);

		setRequestedOrientation(Globals.HorizontalOrientation ? ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE : ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);

		instance = this;

		// fullscreen mode
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
				WindowManager.LayoutParams.FLAG_FULLSCREEN);

		if(Globals.InhibitSuspend)
			getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
					WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);


		super.onCreate(savedInstanceState);

		Log.i("SDL", "libSDL: Creating startup screen");
		_layout = new LinearLayout(this);		_layout.setOrientation(LinearLayout.VERTICAL);
		_layout.setLayoutParams(new LinearLayout.LayoutParams(ViewGroup.LayoutParams.FILL_PARENT, ViewGroup.LayoutParams.FILL_PARENT));
		_layout2 = new LinearLayout(this);
		_layout2.setLayoutParams(new LinearLayout.LayoutParams(ViewGroup.LayoutParams.FILL_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));

		final Semaphore loadedLibraries = new Semaphore(0);

		if( Globals.StartupMenuButtonTimeout > 0 )
		{
			_btn = new Button(this);
			_btn.setLayoutParams(new ViewGroup.LayoutParams(ViewGroup.LayoutParams.FILL_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
			_btn.setText(getResources().getString(R.string.device_change_cfg));
			class onClickListener implements View.OnClickListener
			{
					public MainActivity p;
					onClickListener( MainActivity _p ) { p = _p; }
					public void onClick(View v)
					{
						setUpStatusLabel();
						Log.i("SDL", "libSDL: User clicked change phone config button");
						loadedLibraries.acquireUninterruptibly();
						SettingsMenu.showConfig(p, false);
					}
			};
			_btn.setOnClickListener(new onClickListener(this));

			_layout2.addView(_btn);
		}

		_layout.addView(_layout2);

		ImageView img = new ImageView(this);

		img.setScaleType(ImageView.ScaleType.FIT_CENTER  ); // FIT_XY 
		try
		{
			img.setImageDrawable(Drawable.createFromStream(getAssets().open("logo.png"), "logo.png"));
		}
		catch(Exception e)
		{
			img.setImageResource(R.drawable.publisherlogo);
		}
		img.setLayoutParams(new ViewGroup.LayoutParams(ViewGroup.LayoutParams.FILL_PARENT, ViewGroup.LayoutParams.FILL_PARENT));
		_layout.addView(img);
		
		//_videoLayout = new FrameLayout(this);
		//_videoLayout.addView(_layout);
		mLayout.addView(_layout);

	        setContentView(mLayout);

		_ad = new Advertisement(this);
		if( _ad.getView() != null )
		{
			mLayout.addView(_ad.getView());
			_ad.getView().setLayoutParams(new FrameLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT, Gravity.BOTTOM | Gravity.RIGHT));
		}
		
//		setContentView(_videoLayout);

		class Callback implements Runnable
		{
			MainActivity p;
			Callback( MainActivity _p ) { p = _p; }
			public void run()
			{
				try {
					Thread.sleep(200);
				} catch( InterruptedException e ) {};

				if(p.mAudioThread == null)
				{
					Log.i("SDL", "libSDL: Loading libraries");
					p.LoadLibraries();
					p.mAudioThread = new AudioThread(p);
					Log.i("SDL", "libSDL: Loading settings");
					final Semaphore loaded = new Semaphore(0);
					class Callback2 implements Runnable
					{
						public MainActivity Parent;
						public void run()
						{
							Settings.Load(Parent);
							loaded.release();
							loadedLibraries.release();
						}
					}
					Callback2 cb = new Callback2();
					cb.Parent = p;
					p.runOnUiThread(cb);
					loaded.acquireUninterruptibly();
					if(!Globals.CompatibilityHacksStaticInit)
						p.LoadApplicationLibrary(p);
				}

				if( !Settings.settingsChanged )
				{
					if( Globals.StartupMenuButtonTimeout > 0 )
					{
						Log.i("SDL", "libSDL: " + String.valueOf(Globals.StartupMenuButtonTimeout) + "-msec timeout in startup screen");
						try {
							Thread.sleep(Globals.StartupMenuButtonTimeout);
						} catch( InterruptedException e ) {};
					}
					if( Settings.settingsChanged )
						return;
					Log.i("SDL", "libSDL: Timeout reached in startup screen, process with downloader");
					p.startDownloader();
				}

				
			}
		};
		(new Thread(new Callback(this))).start();
	}
	public void setUpStatusLabel()
	{
		MainActivity Parent = this; // Too lazy to rename
		if( Parent._btn != null )
		{
			Parent._layout2.removeView(Parent._btn);
			Parent._btn = null;
		}
		if( Parent._tv == null )
		{
			Parent._tv = new TextView(Parent);
			Parent._tv.setMaxLines(2); // To show some long texts on smaller devices
			Parent._tv.setMinLines(2); // Otherwise the background picture is getting resized at random, which does not look good
			Parent._tv.setText(R.string.init);
			Parent._layout2.addView(Parent._tv);
		}
	}

	public void startDownloader()
	{
		Log.i("SDL", "libSDL: Starting data downloader");
		class Callback implements Runnable
		{
			public MainActivity Parent;
			public void run()
			{
				setUpStatusLabel();
				Log.i("SDL", "libSDL: Starting downloader");
				if( Parent.downloader == null )
					Parent.downloader = new DataDownloader(Parent, Parent._tv);
			}
		}
		Callback cb = new Callback();
		cb.Parent = this;
		this.runOnUiThread(cb);
	}

	public void initSDL()
	{
		(new Thread(new Runnable()
		{
			public void run()
			{
				//int tries = 30;
				while( isCurrentOrientationHorizontal() != Globals.HorizontalOrientation )
				{
					Log.i("SDL", "libSDL: Waiting for screen orientation to change - the device is probably in the lockscreen mode");
					try {
						Thread.sleep(500);
					} catch( Exception e ) {}
					/*
					tries--;
					if( tries <= 0 )
					{
						Log.i("SDL", "libSDL: Giving up waiting for screen orientation change");
						break;
					}
					*/
					if( _isPaused )
					{
						Log.i("SDL", "libSDL: Application paused, cancelling SDL initialization until it will be brought to foreground");
						return;
					}
				}
				runOnUiThread(new Runnable()
				{
					public void run()
					{
						initSDLInternal();
					}
				});
			}
		})).start();
	}

	private void initSDLInternal()
	{
		if(sdlInited)
			return;
		Log.i("SDL", "libSDL: Initializing video and SDL application");
		
		sdlInited = true;

		mLayout.removeView(_layout);

		if( _ad.getView() != null )
			mLayout.removeView(_ad.getView());
		_layout = null;
		_layout2 = null;
		_btn = null;
		_tv = null;
		//_inputManager = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
		/*_videoLayout = new FrameLayout(this);
		SetLayerType.get().setLayerType(_videoLayout);
		setContentView(_videoLayout);*/


		/*mGLView = new DemoGLSurfaceView(this);
		SetLayerType.get().setLayerType(mGLView);
		mLayout.addView(mGLView);*/
		//mGLView.setFocusableInTouchMode(true);
		//mGLView.setFocusable(true);
		//mGLView.requestFocus();
		/*if( _ad.getView() != null )
		{
			_videoLayout.addView(_ad.getView());
			_ad.getView().setLayoutParams(new FrameLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT, Gravity.TOP | Gravity.RIGHT));
		}*/
		// Receive keyboard events
		/*DimSystemStatusBar.get().dim(_videoLayout);*/
		//DimSystemStatusBar.get().dim(mGLView);
	}

	@Override
	protected void onPause() {
		/*if( downloader != null )
		{
			synchronized( downloader )
			{
				downloader.setStatusField(null);
			}
		}
		_isPaused = true;
		if( mGLView != null )
			mGLView.onPause();*/
		//if( _ad.getView() != null )
		//	_ad.getView().onPause();
		super.onPause();
	}

	@Override
	protected void onResume() {
		Settings.Apply(this);
		super.onResume();		
		/*if( mGLView != null )
		{
			mGLView.onResume();
			DimSystemStatusBar.get().dim(_videoLayout);
			DimSystemStatusBar.get().dim(mGLView);
		}
		if( downloader != null )
		{
			synchronized( downloader )
			{
				downloader.setStatusField(_tv);
				if( downloader.DownloadComplete )
				{
					initSDL();
				}
			}
		}
		//if( _ad.getView() != null )
		//	_ad.getView().onResume();
		_isPaused = false;*/
	}

	@Override
	public void onWindowFocusChanged (boolean hasFocus) {
		super.onWindowFocusChanged(hasFocus);
		Log.i("SDL", "libSDL: onWindowFocusChanged: " + hasFocus + " - sending onPause/onResume");
		/*if (hasFocus == false)
			onPause();
		else
			onResume();*/
		/*
		if (hasFocus == false) {
			synchronized(textInput) {
				// Send 'SDLK_PAUSE' (to enter pause mode) to native code:
				DemoRenderer.nativeTextInput( 19, 19 );
			}
		}
		*/
	}
	
	public boolean isPaused()
	{
		return _isPaused;
	}

	@Override
	protected void onDestroy()
	{
		/*if( downloader != null )
		{
			synchronized( downloader )
			{
				downloader.setStatusField(null);
			}
		}
		if( mGLView != null )
			mGLView.exitApp();*/
		super.onDestroy();
/*		try{
			Thread.sleep(2000); // The event is sent asynchronously, allow app to save it's state, and call exit() itself.
		} catch (InterruptedException e) {}
		System.exit(0);*/
	}

	public void showScreenKeyboardWithoutTextInputField()
	{
		//_inputManager.toggleSoftInput(InputMethodManager.SHOW_FORCED, 0);
		//_inputManager.showSoftInput(mGLView, InputMethodManager.SHOW_FORCED);
		//getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_VISIBLE);
	}

	public void showScreenKeyboard(final String oldText, boolean sendBackspace)
	{
		if(Globals.CompatibilityHacksTextInputEmulatesHwKeyboard)
		{
			showScreenKeyboardWithoutTextInputField();
			return;
		}
		if(_screenKeyboard != null)
			return;
		class simpleKeyListener implements OnKeyListener
		{
			MainActivity _parent;
			boolean sendBackspace;
			simpleKeyListener(MainActivity parent, boolean sendBackspace) { _parent = parent; this.sendBackspace = sendBackspace; };
			public boolean onKey(View v, int keyCode, KeyEvent event)
			{
				if ((event.getAction() == KeyEvent.ACTION_UP) && (
					keyCode == KeyEvent.KEYCODE_ENTER ||
					keyCode == KeyEvent.KEYCODE_BACK ||
					keyCode == KeyEvent.KEYCODE_MENU ||
					keyCode == KeyEvent.KEYCODE_BUTTON_A ||
					keyCode == KeyEvent.KEYCODE_BUTTON_B ||
					keyCode == KeyEvent.KEYCODE_BUTTON_X ||
					keyCode == KeyEvent.KEYCODE_BUTTON_Y ||
					keyCode == KeyEvent.KEYCODE_BUTTON_1 ||
					keyCode == KeyEvent.KEYCODE_BUTTON_2 ||
					keyCode == KeyEvent.KEYCODE_BUTTON_3 ||
					keyCode == KeyEvent.KEYCODE_BUTTON_4 ))
				{
					//_parent.hideScreenKeyboard();
					return true;
				}
				if (keyCode == KeyEvent.KEYCODE_DEL || keyCode == KeyEvent.KEYCODE_CLEAR)
				{
					if (sendBackspace && event.getAction() == KeyEvent.ACTION_UP)
					{
						synchronized(textInput) {
							//DemoRenderer.nativeTextInput( 8, 0 ); // Send backspace to native code
						}
					}
					// EditText deletes two characters at a time, here's a hacky fix
					if (event.getAction() == KeyEvent.ACTION_DOWN && (event.getFlags() | KeyEvent.FLAG_SOFT_KEYBOARD) != 0)
					{
						EditText t = (EditText) v;
						int start = t.getSelectionStart();  //get cursor starting position
						int end = t.getSelectionEnd();      //get cursor ending position
						if ( start < 0 )
							return true;
						if ( end < 0 || end == start )
						{
							start --;
							if ( start < 0 )
								return true;
							end = start + 1;
						}
						t.setText(t.getText().toString().substring(0, start) + t.getText().toString().substring(end));
						t.setSelection(start);
						return true;
					}
				}
				//Log.i("SDL", "Key " + keyCode + " flags " + event.getFlags() + " action " + event.getAction());
				return false;
			}
		};
		_screenKeyboard = new EditText(this);
		// This code does not work
		/*
		_screenKeyboard.setMaxLines(100);
		ViewGroup.LayoutParams layout = _screenKeyboard.getLayoutParams();
		if( layout != null )
		{
			layout.width = ViewGroup.LayoutParams.FILL_PARENT;
			layout.height = ViewGroup.LayoutParams.FILL_PARENT;
			_screenKeyboard.setLayoutParams(layout);
		}
		_screenKeyboard.setGravity(android.view.Gravity.BOTTOM | android.view.Gravity.LEFT);
		*/
		String hint = _screenKeyboardHintMessage;
		_screenKeyboard.setHint(hint != null ? hint : getString(R.string.text_edit_click_here));
		_screenKeyboard.setText(oldText);
		_screenKeyboard.setSelection(_screenKeyboard.getText().length());
		_screenKeyboard.setOnKeyListener(new simpleKeyListener(this, sendBackspace));
		_screenKeyboard.setBackgroundColor(Color.BLACK); // Full opaque - do not show semi-transparent edit box, it's confusing
		_screenKeyboard.setTextColor(Color.WHITE); // Just to be sure about gamma
		if( isRunningOnOUYA() )
			_screenKeyboard.setPadding(100, 100, 100, 100); // Bad bad HDMI TVs all have cropped borders
		//_videoLayout.addView(_screenKeyboard);
		//_screenKeyboard.setKeyListener(new TextKeyListener(TextKeyListener.Capitalize.NONE, false));
		_screenKeyboard.setInputType(InputType.TYPE_CLASS_TEXT);
		_screenKeyboard.setFocusableInTouchMode(true);
		_screenKeyboard.setFocusable(true);
		_screenKeyboard.requestFocus();
		_inputManager.showSoftInput(_screenKeyboard, InputMethodManager.SHOW_IMPLICIT);
		getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_VISIBLE);
		// Hack to try to force on-screen keyboard
		final EditText keyboard = _screenKeyboard;
		keyboard.postDelayed( new Runnable()
			{
				public void run()
				{
					keyboard.requestFocus();
					//_inputManager.toggleSoftInput(InputMethodManager.SHOW_FORCED, 0);
					_inputManager.showSoftInput(keyboard, InputMethodManager.SHOW_FORCED);
					// Hack from Stackoverflow, to force text input on Ouya
					keyboard.dispatchTouchEvent(MotionEvent.obtain(SystemClock.uptimeMillis(), SystemClock.uptimeMillis(), MotionEvent.ACTION_DOWN , 0, 0, 0));
					keyboard.dispatchTouchEvent(MotionEvent.obtain(SystemClock.uptimeMillis(), SystemClock.uptimeMillis(), MotionEvent.ACTION_UP , 0, 0, 0));
					keyboard.postDelayed( new Runnable()
					{
						public void run()
						{
							keyboard.setSelection(keyboard.getText().length());
						}
					}, 100 );
				}
			}, 500 );
	};

	/*public void hideScreenKeyboard()
	{
		if(_screenKeyboard == null)
			return;

		synchronized(textInput)
		{
			String text = _screenKeyboard.getText().toString();*/
			/*for(int i = 0; i < text.length(); i++)
			{
				DemoRenderer.nativeTextInput( (int)text.charAt(i), (int)text.codePointAt(i) );
			}*/
		/*}
		//DemoRenderer.nativeTextInputFinished();
		_inputManager.hideSoftInputFromWindow(_screenKeyboard.getWindowToken(), 0);
		mLayout.removeView(_screenKeyboard);
		_screenKeyboard = null;
		mGLView.setFocusableInTouchMode(true);
		mGLView.setFocusable(true);
		mGLView.requestFocus();
	};*/

	/*public boolean isScreenKeyboardShown()
	{
		return _screenKeyboard != null;
	};
	
	public void setScreenKeyboardHintMessage(String s)
	{
		_screenKeyboardHintMessage = s;
		//Log.i("SDL", "setScreenKeyboardHintMessage: " + (_screenKeyboardHintMessage != null ? _screenKeyboardHintMessage : getString(R.string.text_edit_click_here)));
		runOnUiThread(new Runnable()
		{
			public void run()
			{
				if( _screenKeyboard != null )
				{
					String hint = _screenKeyboardHintMessage;
					_screenKeyboard.setHint(hint != null ? hint : getString(R.string.text_edit_click_here));
				}
			}
		} );
	}

	final static int ADVERTISEMENT_POSITION_RIGHT = -1;
	final static int ADVERTISEMENT_POSITION_BOTTOM = -1;
	final static int ADVERTISEMENT_POSITION_CENTER = -2;

	public void setAdvertisementPosition(int x, int y)
	{
		
		if( _ad.getView() != null )
		{
			final FrameLayout.LayoutParams layout = new FrameLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT);
			layout.gravity = 0;
			layout.leftMargin = 0;
			layout.topMargin = 0;
			if( x == ADVERTISEMENT_POSITION_RIGHT )
				layout.gravity |= Gravity.RIGHT;
			else if ( x == ADVERTISEMENT_POSITION_CENTER )
				layout.gravity |= Gravity.CENTER_HORIZONTAL;
			else
			{
				layout.gravity |= Gravity.LEFT;
				layout.leftMargin = x;
			}
			if( y == ADVERTISEMENT_POSITION_BOTTOM )
				layout.gravity |= Gravity.BOTTOM;
			else if ( x == ADVERTISEMENT_POSITION_CENTER )
				layout.gravity |= Gravity.CENTER_VERTICAL;
			else
			{
				layout.gravity |= Gravity.TOP;
				layout.topMargin = y;
			}
			class Callback implements Runnable
			{
				public void run()
				{
					_ad.getView().setLayoutParams(layout);
				}
			};
			runOnUiThread(new Callback());
		}
	}
	public void setAdvertisementVisible(final int visible)
	{
		if( _ad.getView() != null )
		{
			class Callback implements Runnable
			{
				public void run()
				{
					if( visible == 0 )
						_ad.getView().setVisibility(View.GONE);
					else
						_ad.getView().setVisibility(View.VISIBLE);
				}
			}
			runOnUiThread(new Callback());
		}
	}

	public void getAdvertisementParams(int params[])
	{
		for( int i = 0; i < 5; i++ )
			params[i] = 0;
		if( _ad.getView() != null )
		{
			params[0] = (_ad.getView().getVisibility() == View.VISIBLE) ? 1 : 0;
			FrameLayout.LayoutParams layout = (FrameLayout.LayoutParams) _ad.getView().getLayoutParams();
			params[1] = layout.leftMargin;
			params[2] = layout.topMargin;
			params[3] = _ad.getView().getMeasuredWidth();
			params[4] = _ad.getView().getMeasuredHeight();
		}
	}*/
	/*public void requestNewAdvertisement()
	{
		if( _ad.getView() != null )
		{
			class Callback implements Runnable
			{
				public void run()
				{
					_ad.requestNewAd();
				}
			}
			runOnUiThread(new Callback());
		}
	}*/

	/*@Override
	public boolean onKeyDown(int keyCode, final KeyEvent event)
	{
		if(_screenKeyboard != null)
			_screenKeyboard.onKeyDown(keyCode, event);
		else
		if( mGLView != null )
		{
			if( mGLView.nativeKey( keyCode, 1 ) == 0 )
				return super.onKeyDown(keyCode, event);
		}*/
		/*
		else
		if( keyCode == KeyEvent.KEYCODE_BACK && downloader != null )
		{
			if( downloader.DownloadFailed )
				System.exit(1);
			if( !downloader.DownloadComplete )
				onStop();
		}
		*/
		/*else
		if( keyListener != null )
		{
			keyListener.onKeyEvent(keyCode);
		}
		return true;*/
	//}
	
	/*@Override
	public boolean onKeyUp(int keyCode, final KeyEvent event)
	{
		if(_screenKeyboard != null)
			_screenKeyboard.onKeyUp(keyCode, event);
		else
		if( mGLView != null )
		{
			if( mGLView.nativeKey( keyCode, 0 ) == 0 )
				return super.onKeyUp(keyCode, event);
			if( keyCode == KeyEvent.KEYCODE_BACK || keyCode == KeyEvent.KEYCODE_MENU )
			{
				//DimSystemStatusBar.get().dim(_videoLayout);
				DimSystemStatusBar.get().dim(mGLView);
			}
		}
		return true;
	}*/

	/*@Override
	public boolean dispatchTouchEvent(final MotionEvent ev)
	{*/
		//Log.i("SDL", "dispatchTouchEvent: " + ev.getAction() + " coords " + ev.getX() + ":" + ev.getY() );
		/*if(_screenKeyboard != null)
			_screenKeyboard.dispatchTouchEvent(ev);
		else
		if( _ad.getView() != null && // User clicked the advertisement, ignore when user moved finger from game screen to advertisement or touches screen with several fingers
			((ev.getAction() & MotionEvent.ACTION_MASK) == MotionEvent.ACTION_DOWN ||
			(ev.getAction() & MotionEvent.ACTION_MASK) == MotionEvent.ACTION_UP) &&
			_ad.getView().getLeft() <= (int)ev.getX() &&
			_ad.getView().getRight() > (int)ev.getX() &&
			_ad.getView().getTop() <= (int)ev.getY() &&
			_ad.getView().getBottom() > (int)ev.getY() )
			return super.dispatchTouchEvent(ev);
		else
		if(mGLView != null)
			mGLView.onTouchEvent(ev);
		else
		if( _btn != null )
			return _btn.dispatchTouchEvent(ev);
		else
		/*if( touchListener != null )
			touchListener.onTouchEvent(ev);*/
/*
		return true;
	}*/
	
/*	@Override
	public boolean dispatchGenericMotionEvent (MotionEvent ev)
	{*/
		//Log.i("SDL", "dispatchGenericMotionEvent: " + ev.getAction() + " coords " + ev.getX() + ":" + ev.getY() );
		// This code fails to run for Android 1.6, so there will be no generic motion event for Andorid screen keyboard
		/*
		if(_screenKeyboard != null)
			_screenKeyboard.dispatchGenericMotionEvent(ev);
		else
		*/
		/*if(mGLView != null)
			mGLView.onGenericMotionEvent(ev);
		return true;*/
//	}

	@Override
	public void onConfigurationChanged(Configuration newConfig)
	{
		super.onConfigurationChanged(newConfig);
		// Do nothing here
	}
	
	public void setText(final String t)
	{
		class Callback implements Runnable
		{
			MainActivity Parent;
			public SpannedString text;
			public void run()
			{
				Parent.setUpStatusLabel();
				if(Parent._tv != null)
					Parent._tv.setText(text);
			}
		}
		Callback cb = new Callback();
		cb.text = new SpannedString(t);
		cb.Parent = this;
		this.runOnUiThread(cb);
	}

	public void showTaskbarNotification()
	{
		showTaskbarNotification("SDL application paused", "SDL application", "Application is paused, click to activate");
	}

	// Stolen from SDL port by Mamaich
	public void showTaskbarNotification(String text0, String text1, String text2)
	{
		NotificationManager NotificationManager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
		Intent intent = new Intent(this, MainActivity.class);
		PendingIntent pendingIntent = PendingIntent.getActivity(this, 0, intent, Intent.FLAG_ACTIVITY_NEW_TASK);
		Notification n = new Notification(R.drawable.icon, text0, System.currentTimeMillis());
		n.setLatestEventInfo(this, text1, text2, pendingIntent);
		NotificationManager.notify(NOTIFY_ID, n);
	}

	public void hideTaskbarNotification()
	{
		NotificationManager NotificationManager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
		NotificationManager.cancel(NOTIFY_ID);
	}
	
	public void LoadLibraries()
	{
		try
		{
			if(Globals.NeedGles2)
				System.loadLibrary("GLESv2");
			Log.i("SDL", "libSDL: loaded GLESv2 lib");
		}
		catch ( UnsatisfiedLinkError e )
		{
			Log.i("SDL", "libSDL: Cannot load GLESv2 lib");
		}

		// Load all libraries
		try
		{
			for(String l : Globals.AppLibraries)
			{
				try
				{
					String libname = System.mapLibraryName(l);
					File libpath = new File(getFilesDir().getAbsolutePath() + "/../lib/" + libname);
					Log.i("SDL", "libSDL: loading lib " + libpath.getAbsolutePath());
					System.load(libpath.getPath());
				}
				catch( UnsatisfiedLinkError e )
				{
					Log.i("SDL", "libSDL: error loading lib " + l + ": " + e.toString());
					try
					{
						String libname = System.mapLibraryName(l);
						File libpath = new File(getFilesDir().getAbsolutePath() + "/" + libname);
						Log.i("SDL", "libSDL: loading lib " + libpath.getAbsolutePath());
						System.load(libpath.getPath());
					}
					catch( UnsatisfiedLinkError ee )
					{
						Log.i("SDL", "libSDL: error loading lib " + l + ": " + ee.toString());
						System.loadLibrary(l);
					}
				}
			}
		}
		catch ( UnsatisfiedLinkError e )
		{
			try {
				Log.i("SDL", "libSDL: Extracting APP2SD-ed libs");
				
				InputStream in = null;
				try
				{
					for( int i = 0; ; i++ )
					{
						InputStream in2 = getAssets().open("bindata" + String.valueOf(i));
						if( in == null )
							in = in2;
						else
							in = new SequenceInputStream( in, in2 );
					}
				}
				catch( IOException ee ) { }

				if( in == null )
					throw new RuntimeException("libSDL: Extracting APP2SD-ed libs failed, the .apk file packaged incorrectly");

				ZipInputStream zip = new ZipInputStream(in);

				File libDir = getFilesDir();
				try {
					libDir.mkdirs();
				} catch( SecurityException ee ) { };
				
				byte[] buf = new byte[16384];
				while(true)
				{
					ZipEntry entry = null;
					entry = zip.getNextEntry();
					/*
					if( entry != null )
						Log.i("SDL", "Extracting lib " + entry.getName());
					*/
					/*if( entry == null )
					{
						Log.i("SDL", "Extracting libs finished");
						break;
					}
					if( entry.isDirectory() )
					{
						File outDir = new File( libDir.getAbsolutePath() + "/" + entry.getName() );
						if( !(outDir.exists() && outDir.isDirectory()) )
							outDir.mkdirs();
						continue;
					}

					OutputStream out = null;
					String path = libDir.getAbsolutePath() + "/" + entry.getName();
					try {
						File outDir = new File( path.substring(0, path.lastIndexOf("/") ));
						if( !(outDir.exists() && outDir.isDirectory()) )
							outDir.mkdirs();
					} catch( SecurityException eeeee ) { };

					Log.i("SDL", "Saving to file '" + path + "'");

					out = new FileOutputStream( path );
					int len = zip.read(buf);
					while (len >= 0)
					{
						if(len > 0)
							out.write(buf, 0, len);
						len = zip.read(buf);
					}

					out.flush();
					out.close();
				}

				for(String l : Globals.AppLibraries)
				{
					String libname = System.mapLibraryName(l);
					File libpath = new File(libDir, libname);
					Log.i("SDL", "libSDL: loading lib " + libpath.getPath());
					System.load(libpath.getPath());
					libpath.delete();
				}
			}
			catch ( Exception ee )
			{
				Log.i("SDL", "libSDL: Error: " + ee.toString());
			}
		}

		// ----- VCMI hack -----
		String [] binaryZipNames = { "binaries-" + android.os.Build.CPU_ABI + ".zip", "binaries.zip" };
		for(String binaryZip: binaryZipNames)
		{
			try {
				Log.i("SDL", "libSDL: Trying to extract binaries from assets " + binaryZip);
				
				InputStream in = null;
				try
				{
					for( int i = 0; ; i++ )
					{
						InputStream in2 = getAssets().open(binaryZip + String.format("%02d", i));
						if( in == null )
							in = in2;
						else
							in = new SequenceInputStream( in, in2 );
					}
				}
				catch( IOException ee )
				{
					try
					{
						if( in == null )
							in = getAssets().open(binaryZip);
					}
					catch( IOException eee ) {}
				}

				if( in == null )
					throw new RuntimeException("libSDL: Extracting binaries failed, the .apk file packaged incorrectly");

				ZipInputStream zip = new ZipInputStream(in);

				File libDir = getFilesDir();
				try {
					libDir.mkdirs();
				} catch( SecurityException ee ) { };
				
				byte[] buf = new byte[16384];
				while(true)
				{
					ZipEntry entry = null;
					entry = zip.getNextEntry();
					/*
					if( entry != null )
						Log.i("SDL", "Extracting lib " + entry.getName());
					*/
					if( entry == null )
					{
						Log.i("SDL", "Extracting binaries finished");
						break;
					}
					if( entry.isDirectory() )
					{
						File outDir = new File( libDir.getAbsolutePath() + "/" + entry.getName() );
						if( !(outDir.exists() && outDir.isDirectory()) )
							outDir.mkdirs();
						continue;
					}

					OutputStream out = null;
					String path = libDir.getAbsolutePath() + "/" + entry.getName();
					try {
						File outDir = new File( path.substring(0, path.lastIndexOf("/") ));
						if( !(outDir.exists() && outDir.isDirectory()) )
							outDir.mkdirs();
					} catch( SecurityException eeeeeee ) { };

					try {
						CheckedInputStream check = new CheckedInputStream( new FileInputStream(path), new CRC32() );
						while( check.read(buf, 0, buf.length) > 0 ) {};
						check.close();
						if( check.getChecksum().getValue() != entry.getCrc() )
						{
							File ff = new File(path);
							ff.delete();
							throw new Exception();
						}
						Log.i("SDL", "File '" + path + "' exists and passed CRC check - not overwriting it");
						continue;
					} catch( Exception eeeeee ) { }

					Log.i("SDL", "Saving to file '" + path + "'");

					out = new FileOutputStream( path );
					int len = zip.read(buf);
					while (len >= 0)
					{
						if(len > 0)
							out.write(buf, 0, len);
						len = zip.read(buf);
					}

					out.flush();
					out.close();
					Settings.nativeChmod(path, 0755);


					//String chmod[] = { "/system/bin/chmod", "0755", path };
					//Runtime.getRuntime().exec(chmod).waitFor();
				}
			}
			catch ( Exception eee )
			{
				//Log.i("SDL", "libSDL: Error: " + eee.toString());
			}
		}
		// ----- VCMI hack -----
	};

	public static void LoadApplicationLibrary(final Context context)
	{
		Settings.nativeChdir(Globals.DataDir);
		for(String l : Globals.AppMainLibraries)
		{
			try
			{
				String libname = System.mapLibraryName(l);
				File libpath = new File(context.getFilesDir().getAbsolutePath() + "/../lib/" + libname);
				Log.i("SDL", "libSDL: loading lib " + libpath.getAbsolutePath());
				System.load(libpath.getPath());
			}
			catch( UnsatisfiedLinkError e )
			{
				Log.i("SDL", "libSDL: error loading lib " + l + ": " + e.toString());
				try
				{
					String libname = System.mapLibraryName(l);
					File libpath = new File(context.getFilesDir().getAbsolutePath() + "/" + libname);
					Log.i("SDL", "libSDL: loading lib " + libpath.getAbsolutePath());
					System.load(libpath.getPath());
				}
				catch( UnsatisfiedLinkError ee )
				{
					Log.i("SDL", "libSDL: error loading lib " + l + ": " + ee.toString());
					System.loadLibrary(l);
				}
			}
		}
	}

	public int getApplicationVersion()
	{
		try {
			PackageInfo packageInfo = getPackageManager().getPackageInfo(getPackageName(), 0);
			return packageInfo.versionCode;
		} catch (PackageManager.NameNotFoundException e) {
			Log.i("SDL", "libSDL: Cannot get the version of our own package: " + e);
		}
		return 0;
	}

	public boolean isRunningOnOUYA()
	{
		try {
			PackageInfo packageInfo = getPackageManager().getPackageInfo("tv.ouya", 0);
			return true;
		} catch (PackageManager.NameNotFoundException e) {
		}
		return Globals.OuyaEmulation;
	}

	public boolean isCurrentOrientationHorizontal()
	{
		Display getOrient = getWindowManager().getDefaultDisplay();
		return getOrient.getWidth() >= getOrient.getHeight();
	}

	public ViewGroup getVideoLayout() { return mLayout; }

	static int NOTIFY_ID = 12367098; // Random ID

//	private static DemoGLSurfaceView mGLView = null;
	private static AudioThread mAudioThread = null;
	private static DataDownloader downloader = null;

	private TextView _tv = null;
	private Button _btn = null;
	private LinearLayout _layout = null;
	private LinearLayout _layout2 = null;
	private Advertisement _ad = null;

	//private FrameLayout _videoLayout = null;
	private EditText _screenKeyboard = null;
	private String _screenKeyboardHintMessage = null;
	private boolean sdlInited = false;

	public interface TouchEventsListener
	{
		public void onTouchEvent(final MotionEvent ev);
	}

	public interface KeyEventsListener
	{
		public void onKeyEvent(final int keyCode);
	}

	public TouchEventsListener touchListener = null;
	public KeyEventsListener keyListener = null;
	boolean _isPaused = false;
	private InputMethodManager _inputManager = null;

	public LinkedList<Integer> textInput = new LinkedList<Integer> ();
	public static MainActivity instance = null;
}

// *** HONEYCOMB / ICS FIX FOR FULLSCREEN MODE, by lmak ***
abstract class DimSystemStatusBar
{
	public static DimSystemStatusBar get()
	{
		if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.HONEYCOMB)
			return DimSystemStatusBarHoneycomb.Holder.sInstance;
		else
			return DimSystemStatusBarDummy.Holder.sInstance;
	}
	public abstract void dim(final View view);

	private static class DimSystemStatusBarHoneycomb extends DimSystemStatusBar
	{
		private static class Holder
		{
			private static final DimSystemStatusBarHoneycomb sInstance = new DimSystemStatusBarHoneycomb();
		}
	    public void dim(final View view)
	    {
	         /*
	         if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.ICE_CREAM_SANDWICH) {
	            // ICS has the same constant redefined with a different name.
	            hiddenStatusCode = android.view.View.SYSTEM_UI_FLAG_LOW_PROFILE;
	         }
	         */
	         view.setSystemUiVisibility(android.view.View.STATUS_BAR_HIDDEN);
	   }
	}
	private static class DimSystemStatusBarDummy extends DimSystemStatusBar
	{
		private static class Holder
		{
			private static final DimSystemStatusBarDummy sInstance = new DimSystemStatusBarDummy();
		}
		public void dim(final View view)
		{
		}
	}
}

abstract class SetLayerType
{
	public static SetLayerType get()
	{
		if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.HONEYCOMB)
			return SetLayerTypeHoneycomb.Holder.sInstance;
		else
			return SetLayerTypeDummy.Holder.sInstance;
	}
	public abstract void setLayerType(final View view);

	private static class SetLayerTypeHoneycomb extends SetLayerType
	{
		private static class Holder
		{
			private static final SetLayerTypeHoneycomb sInstance = new SetLayerTypeHoneycomb();
		}
		public void setLayerType(final View view)
		{
			view.setLayerType(android.view.View.LAYER_TYPE_NONE, null);
			//view.setLayerType(android.view.View.LAYER_TYPE_HARDWARE, null);
		}
	}
	private static class SetLayerTypeDummy extends SetLayerType
	{
		private static class Holder
		{
			private static final SetLayerTypeDummy sInstance = new SetLayerTypeDummy();
		}
		public void setLayerType(final View view)
		{
		}
	}
}
