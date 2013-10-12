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
	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);

		/*setRequestedOrientation(Globals.HorizontalOrientation ? ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE : ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);

		instance = this;
		// fullscreen mode
		/*requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
				WindowManager.LayoutParams.FLAG_FULLSCREEN);
		if(Globals.InhibitSuspend)
			getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
					WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
*/
		Log.i("SDL", "libSDL: Creating startup screen");
/*		_layout = new LinearLayout(this);
		_layout.setOrientation(LinearLayout.VERTICAL);
		_layout.setLayoutParams(new LinearLayout.LayoutParams(ViewGroup.LayoutParams.FILL_PARENT, ViewGroup.LayoutParams.FILL_PARENT));
		_layout2 = new LinearLayout(this);
		_layout2.setLayoutParams(new LinearLayout.LayoutParams(ViewGroup.LayoutParams.FILL_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));

		final Semaphore loadedLibraries = new Semaphore(0);
*/
		if( Globals.StartupMenuButtonTimeout > 0 )
		{
			_btn = new Button(this);
		/*	_btn.setLayoutParams(new ViewGroup.LayoutParams(ViewGroup.LayoutParams.FILL_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
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

			_layout2.addView(_btn);*/
		}

/*		_layout.addView(_layout2);

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
		
		_videoLayout = new FrameLayout(this);
		_videoLayout.addView(_layout);

		_ad = new Advertisement(this);
		if( _ad.getView() != null )
		{
			_videoLayout.addView(_ad.getView());
			_ad.getView().setLayoutParams(new FrameLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT, Gravity.BOTTOM | Gravity.RIGHT));
		}
		
		setContentView(_videoLayout);

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
		(new Thread(new Callback(this))).start();*/
	}


	static int NOTIFY_ID = 12367098; // Random ID

//	private static DemoGLSurfaceView mGLView = null;
//	private static AudioThread mAudioThread = null;
//	private static DataDownloader downloader = null;

	private TextView _tv = null;
	private Button _btn = null;
	private LinearLayout _layout = null;
	private LinearLayout _layout2 = null;
//	private Advertisement _ad = null;

	private FrameLayout _videoLayout = null;
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
