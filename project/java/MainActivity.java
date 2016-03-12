/*
Simple DirectMedia Layer
Java source code (C) 2009-2014 Sergii Pylypenko

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
import android.app.Service;
import android.content.Context;
import android.os.Bundle;
import android.os.IBinder;
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
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;
import java.util.zip.CheckedInputStream;
import java.util.zip.CRC32;
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
import android.util.DisplayMetrics;
import android.text.InputType;
import android.util.Log;
import android.view.Surface;
import android.app.ProgressDialog;
import android.app.KeyguardManager;
import android.view.ViewTreeObserver;
import android.graphics.Rect;
import android.view.InputDevice;
import android.inputmethodservice.KeyboardView;
import android.inputmethodservice.Keyboard;
import android.app.Notification;
import android.app.PendingIntent;
import java.util.TreeSet;
import android.app.UiModeManager;

public class MainActivity extends Activity
{
	@Override
	protected void onCreate(Bundle savedInstanceState)
	{
		super.onCreate(savedInstanceState);

		instance = this;
		// fullscreen mode
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
				WindowManager.LayoutParams.FLAG_FULLSCREEN);
		if(Globals.InhibitSuspend)
			getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
					WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

		Log.i("SDL", "libSDL: Creating startup screen");
		_layout = new LinearLayout(this);
		_layout.setOrientation(LinearLayout.VERTICAL);
		_layout.setLayoutParams(new LinearLayout.LayoutParams(ViewGroup.LayoutParams.FILL_PARENT, ViewGroup.LayoutParams.FILL_PARENT));
		_layout2 = new LinearLayout(this);
		_layout2.setLayoutParams(new LinearLayout.LayoutParams(ViewGroup.LayoutParams.FILL_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT));
		loadingDialog = new ProgressDialog(this);
		loadingDialog.setMessage(getString(R.string.accessing_network));

		final Semaphore loadedLibraries = new Semaphore(0);

		if( Globals.StartupMenuButtonTimeout > 0 )
		{
			_btn = new Button(this);
			_btn.setEnabled(false);
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
						setScreenOrientation();
						SettingsMenu.showConfig(p, false);
					}
			};
			_btn.setOnClickListener(new onClickListener(this));

			_layout2.addView(_btn);
		}

		_layout.addView(_layout2);

		ImageView img = new ImageView(this);

		img.setScaleType(ImageView.ScaleType.FIT_CENTER /* FIT_XY */ );
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
							setScreenOrientation();
							loaded.release();
							loadedLibraries.release();
							if( _btn != null )
								_btn.setEnabled(true);
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
		if( Globals.CreateService )
		{
			Log.v("SDL", "Starting dummy service - displaying notification");
			Intent intent = new Intent(this, DummyService.class);
			startService(intent);
		}
		cloudSave = new CloudSave(this);
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
			//Get the display so we can know the screen size
			Display display = getWindowManager().getDefaultDisplay();
			int width = display.getWidth();
			int height = display.getHeight();
			Parent._tv = new TextView(Parent);
			Parent._tv.setMaxLines(2); // To show some long texts on smaller devices
			Parent._tv.setMinLines(2); // Otherwise the background picture is getting resized at random, which does not look good
			Parent._tv.setText(R.string.init);
			// Padding is a good idea because if the display device is a TV the edges might be cut off
			Parent._tv.setPadding((int)(width * 0.1), (int)(height * 0.1), (int)(width * 0.1), 0);
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
		setScreenOrientation();
		updateScreenOrientation();
		DimSystemStatusBar.get().dim(_videoLayout);
		(new Thread(new Runnable()
		{
			public void run()
			{
				if( Globals.AutoDetectOrientation )
					Globals.HorizontalOrientation = isCurrentOrientationHorizontal();
				while( isCurrentOrientationHorizontal() != Globals.HorizontalOrientation ||
						((KeyguardManager)getSystemService(Context.KEYGUARD_SERVICE)).inKeyguardRestrictedInputMode() )
				{
					Log.d("SDL", "libSDL: Waiting for screen orientation to change to " + (Globals.HorizontalOrientation ? "landscape" : "portrait") + ", and for disabling lockscreen mode");
					try {
						Thread.sleep(500);
					} catch( Exception e ) {}
					if( _isPaused )
					{
						Log.i("SDL", "libSDL: Application paused, cancelling SDL initialization until it will be brought to foreground");
						return;
					}
					DimSystemStatusBar.get().dim(_videoLayout);
				}
				runOnUiThread(new Runnable()
				{
					public void run()
					{
						// Hide navigation buttons, and sleep a bit so OS will process the event.
						// Do not check the display size in a loop - we may have several displays of different sizes,
						// so app may stuck in infinite loop
						DisplayMetrics dm = new DisplayMetrics();
						getWindowManager().getDefaultDisplay().getMetrics(dm);
						if( android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.KITKAT && Globals.ImmersiveMode &&
							(_videoLayout.getHeight() != dm.widthPixels || _videoLayout.getWidth() != dm.heightPixels) )
						{
							DimSystemStatusBar.get().dim(_videoLayout);
							try {
								Thread.sleep(300);
							} catch( Exception e ) {}
						}
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
		DimSystemStatusBar.get().dim(_videoLayout);
		_videoLayout.removeView(_layout);
		if( _ad.getView() != null )
			_videoLayout.removeView(_ad.getView());
		_layout = null;
		_layout2 = null;
		_btn = null;
		_tv = null;
		_inputManager = (InputMethodManager) getSystemService(Context.INPUT_METHOD_SERVICE);
		_videoLayout = new FrameLayout(this);
		SetLayerType.get().setLayerType(_videoLayout);
		setContentView(_videoLayout);
		mGLView = new DemoGLSurfaceView(this);
		SetLayerType.get().setLayerType(mGLView);
		FrameLayout.LayoutParams margin = new FrameLayout.LayoutParams(FrameLayout.LayoutParams.MATCH_PARENT, FrameLayout.LayoutParams.MATCH_PARENT);
		// Add TV screen borders, if needed
		if( Globals.TvBorders )
			margin.setMargins(  getResources().getDimensionPixelOffset(R.dimen.screen_border_horizontal),
								getResources().getDimensionPixelOffset(R.dimen.screen_border_vertical),
								getResources().getDimensionPixelOffset(R.dimen.screen_border_horizontal),
								getResources().getDimensionPixelOffset(R.dimen.screen_border_vertical));
		_videoLayout.addView(mGLView, margin);
		mGLView.setFocusableInTouchMode(true);
		mGLView.setFocusable(true);
		mGLView.requestFocus();
		if( _ad.getView() != null )
		{
			_videoLayout.addView(_ad.getView());
			_ad.getView().setLayoutParams(new FrameLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT, Gravity.TOP | Gravity.RIGHT));
		}
		DimSystemStatusBar.get().dim(_videoLayout);
		//DimSystemStatusBar.get().dim(mGLView);

		Rect r = new Rect();
		_videoLayout.getWindowVisibleDisplayFrame(r);
		mGLView.nativeScreenVisibleRect(r.left, r.top, r.right, r.bottom);
		_videoLayout.getViewTreeObserver().addOnGlobalLayoutListener(new ViewTreeObserver.OnGlobalLayoutListener()
		{
			public void onGlobalLayout()
			{
				final Rect r = new Rect();
				_videoLayout.getWindowVisibleDisplayFrame(r);
				final int heightDiff = _videoLayout.getRootView().getHeight() - _videoLayout.getHeight(); // Take system bar into consideration
				final int widthDiff = _videoLayout.getRootView().getWidth() - _videoLayout.getWidth(); // Nexus 5 has system bar at the right side
				Log.v("SDL", "Main window visible region changed: " + r.left + ":" + r.top + ":" + r.width() + ":" + r.height() );
				_videoLayout.postDelayed( new Runnable()
				{
					public void run()
					{
						DimSystemStatusBar.get().dim(_videoLayout);
						mGLView.nativeScreenVisibleRect(r.left + widthDiff, r.top + heightDiff, r.width(), r.height());
					}
				}, 300 );
				_videoLayout.postDelayed( new Runnable()
				{
					public void run()
					{
						DimSystemStatusBar.get().dim(_videoLayout);
						mGLView.nativeScreenVisibleRect(r.left + widthDiff, r.top + heightDiff, r.width(), r.height());
					}
				}, 600 );
			}
		});
	}

	@Override
	protected void onPause() {
		if( downloader != null )
		{
			synchronized( downloader )
			{
				downloader.setStatusField(null);
			}
		}
		_isPaused = true;
		if( mGLView != null )
			mGLView.onPause();
		//if( _ad.getView() != null )
		//	_ad.getView().onPause();
		super.onPause();
	}

	@Override
	protected void onResume() {
		super.onResume();
		if( mGLView != null )
		{
			DimSystemStatusBar.get().dim(_videoLayout);
			//DimSystemStatusBar.get().dim(mGLView);
			mGLView.onResume();
		}
		else
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
		_isPaused = false;
		// Nvidia is too smart to use Samsung's stylus API, obviously they need their own method to enable hover events
		Intent i = new Intent("com.nvidia.intent.action.ENABLE_STYLUS");
		i.putExtra("package", getPackageName());
		sendBroadcast(i);
	}

	@Override
	public void onWindowFocusChanged (boolean hasFocus) {
		super.onWindowFocusChanged(hasFocus);
		Log.i("SDL", "libSDL: onWindowFocusChanged: " + hasFocus + " - sending onPause/onResume");
		if (hasFocus == false)
			onPause();
		else
			onResume();
	}
	
	public boolean isPaused()
	{
		return _isPaused;
	}

	@Override
	protected void onDestroy()
	{
		if( downloader != null )
		{
			synchronized( downloader )
			{
				downloader.setStatusField(null);
			}
		}
		if( mGLView != null )
			mGLView.exitApp();
		super.onDestroy();
		try{
			Thread.sleep(2000); // The event is sent asynchronously, allow app to save it's state, and call exit() itself.
		} catch (InterruptedException e) {}
		System.exit(0);
	}

	@Override
	protected void onStart() {
		super.onStart();
		cloudSave.onStart();
	}

	@Override
	protected void onStop() {
		super.onStart();
		cloudSave.onStop();
	}

	@Override
	public void onActivityResult(int request, int response, Intent data) {
		super.onActivityResult(request, response, data);
		cloudSave.onActivityResult(request, response, data);
	}

	private int TextInputKeyboardList[][] =
	{
		{ 0, R.xml.qwerty, R.xml.c64, R.xml.amiga, R.xml.atari800 },
		{ 0, R.xml.qwerty_shift, R.xml.c64, R.xml.amiga_shift, R.xml.atari800 },
		{ 0, R.xml.qwerty_alt, R.xml.c64, R.xml.amiga_alt, R.xml.atari800 },
		{ 0, R.xml.qwerty_alt_shift, R.xml.c64, R.xml.amiga_alt_shift, R.xml.atari800 }
	};

	public void showScreenKeyboardWithoutTextInputField(final int keyboard)
	{
		if( !keyboardWithoutTextInputShown )
		{
			keyboardWithoutTextInputShown = true;
			runOnUiThread(new Runnable()
			{
				public void run()
				{
					if (keyboard == 0)
					{
						_inputManager.toggleSoftInput(InputMethodManager.SHOW_FORCED, 0);
						_inputManager.showSoftInput(mGLView, InputMethodManager.SHOW_FORCED);
						getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_VISIBLE);
					}
					else
					{
						if( _screenKeyboard != null )
							return;
						class BuiltInKeyboardView extends KeyboardView
						{
							public boolean shift = false;
							public boolean alt = false;
							public TreeSet<Integer> stickyKeys = new TreeSet<Integer>();
							public BuiltInKeyboardView(Context context, android.util.AttributeSet attrs)
							{
								super(context, attrs);
							}
							public boolean dispatchTouchEvent(final MotionEvent ev)
							{
								if( ev.getY() < getTop() )
									return false;
								if( ev.getAction() == MotionEvent.ACTION_DOWN || ev.getAction() == MotionEvent.ACTION_UP || ev.getAction() == MotionEvent.ACTION_MOVE )
								{
									// Convert pointer coords, this will lose multitiouch data, however KeyboardView does not support multitouch anyway
									MotionEvent converted = MotionEvent.obtain(ev.getDownTime(), ev.getEventTime(), ev.getAction(), ev.getX(), ev.getY() - (float)getTop(), ev.getMetaState());
									return super.dispatchTouchEvent(converted);
								}
								return false;
							}
							public boolean onKeyDown(int key, final KeyEvent event)
							{
								return false;
							}
							public boolean onKeyUp(int key, final KeyEvent event)
							{
								return false;
							}
							public void ChangeKeyboard()
							{
								int idx = (shift ? 1 : 0) + (alt ? 2 : 0);
								setKeyboard(new Keyboard(MainActivity.this, TextInputKeyboardList[idx][keyboard]));
								setPreviewEnabled(false);
								setProximityCorrectionEnabled(false);
								for (Keyboard.Key k: getKeyboard().getKeys())
								{
									if (stickyKeys.contains(k.codes[0]))
									{
										k.on = true;
										invalidateAllKeys();
									}
								}
							}
						}
						final BuiltInKeyboardView builtinKeyboard = new BuiltInKeyboardView(MainActivity.this, null);
						builtinKeyboard.ChangeKeyboard();
						builtinKeyboard.setOnKeyboardActionListener(new KeyboardView.OnKeyboardActionListener()
						{
							public void onPress(int key)
							{
								if (key == KeyEvent.KEYCODE_BACK)
									return;
								if (key < 0)
									return;
								for (Keyboard.Key k: builtinKeyboard.getKeyboard().getKeys())
								{
									if (k.sticky && key == k.codes[0])
										return;
								}
								if (key > 100000)
								{
									key -= 100000;
									MainActivity.this.onKeyDown(KeyEvent.KEYCODE_SHIFT_LEFT, new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_SHIFT_LEFT));
								}
								MainActivity.this.onKeyDown(key, new KeyEvent(KeyEvent.ACTION_DOWN, key));
							}
							public void onRelease(int key)
							{
								if (key == KeyEvent.KEYCODE_BACK)
								{
									builtinKeyboard.setOnKeyboardActionListener(null);
									showScreenKeyboardWithoutTextInputField(0); // Hide keyboard
									return;
								}
								if (key == Keyboard.KEYCODE_SHIFT)
								{
									builtinKeyboard.shift = ! builtinKeyboard.shift;
									if (builtinKeyboard.shift && !builtinKeyboard.alt)
										MainActivity.this.onKeyDown(KeyEvent.KEYCODE_SHIFT_LEFT, new KeyEvent(KeyEvent.ACTION_DOWN, KeyEvent.KEYCODE_SHIFT_LEFT));
									else
										MainActivity.this.onKeyUp(KeyEvent.KEYCODE_SHIFT_LEFT, new KeyEvent(KeyEvent.ACTION_UP, KeyEvent.KEYCODE_SHIFT_LEFT));
									builtinKeyboard.ChangeKeyboard();
									return;
								}
								if (key == Keyboard.KEYCODE_ALT)
								{
									builtinKeyboard.alt = ! builtinKeyboard.alt;
									if (builtinKeyboard.alt)
										MainActivity.this.onKeyUp(KeyEvent.KEYCODE_SHIFT_LEFT, new KeyEvent(KeyEvent.ACTION_UP, KeyEvent.KEYCODE_SHIFT_LEFT));
									else
										builtinKeyboard.shift = false;
									builtinKeyboard.ChangeKeyboard();
									return;
								}
								if (key < 0)
									return;
								for (Keyboard.Key k: builtinKeyboard.getKeyboard().getKeys())
								{
									if (k.sticky && key == k.codes[0])
									{
										if (k.on)
										{
											builtinKeyboard.stickyKeys.add(key);
											MainActivity.this.onKeyDown(key, new KeyEvent(KeyEvent.ACTION_DOWN, key));
										}
										else
										{
											builtinKeyboard.stickyKeys.remove(key);
											MainActivity.this.onKeyUp(key, new KeyEvent(KeyEvent.ACTION_UP, key));
										}
										return;
									}
								}

								boolean shifted = false;
								if (key > 100000)
								{
									key -= 100000;
									shifted = true;
								}

								MainActivity.this.onKeyUp(key, new KeyEvent(KeyEvent.ACTION_UP, key));

								if (shifted)
								{
									MainActivity.this.onKeyUp(KeyEvent.KEYCODE_SHIFT_LEFT, new KeyEvent(KeyEvent.ACTION_UP, KeyEvent.KEYCODE_SHIFT_LEFT));
									builtinKeyboard.stickyKeys.remove(KeyEvent.KEYCODE_SHIFT_LEFT);
									for (Keyboard.Key k: builtinKeyboard.getKeyboard().getKeys())
									{
										if (k.sticky && k.codes[0] == KeyEvent.KEYCODE_SHIFT_LEFT && k.on)
										{
											k.on = false;
											builtinKeyboard.invalidateAllKeys();
										}
									}
								}
							}
							public void onText(CharSequence p1) {}
							public void swipeLeft() {}
							public void swipeRight() {}
							public void swipeDown() {}
							public void swipeUp() {}
							public void onKey(int p1, int[] p2) {}
						});
						_screenKeyboard = builtinKeyboard;
						FrameLayout.LayoutParams layout = new FrameLayout.LayoutParams(FrameLayout.LayoutParams.MATCH_PARENT, FrameLayout.LayoutParams.WRAP_CONTENT, Gravity.BOTTOM);
						_videoLayout.addView(_screenKeyboard, layout);
					}
				}
			});
		}
		else
		{
			keyboardWithoutTextInputShown = false;
			runOnUiThread(new Runnable()
			{
				public void run()
				{
					if( _screenKeyboard != null )
					{
						_videoLayout.removeView(_screenKeyboard);
						_screenKeyboard = null;
					}
					getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_HIDDEN);
					_inputManager.hideSoftInputFromWindow(mGLView.getWindowToken(), 0);
					DimSystemStatusBar.get().dim(_videoLayout);
					//DimSystemStatusBar.get().dim(mGLView);
				}
			});
		}
		mGLView.nativeScreenKeyboardShown( keyboardWithoutTextInputShown ? 1 : 0 );
	}

	public void showScreenKeyboard(final String oldText)
	{
		if(Globals.CompatibilityHacksTextInputEmulatesHwKeyboard)
		{
			showScreenKeyboardWithoutTextInputField(Globals.TextInputKeyboard);
			return;
		}
		if(_screenKeyboard != null)
			return;
		class simpleKeyListener implements OnKeyListener
		{
			MainActivity _parent;
			simpleKeyListener(MainActivity parent) { _parent = parent; };
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
					_parent.hideScreenKeyboard();
					return true;
				}
				/*
				if (keyCode == KeyEvent.KEYCODE_DEL || keyCode == KeyEvent.KEYCODE_CLEAR)
				{
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
				*/
				//Log.i("SDL", "Key " + keyCode + " flags " + event.getFlags() + " action " + event.getAction());
				return false;
			}
		};
		EditText screenKeyboard = new EditText(this);
		// This code does not work
		/*
		screenKeyboard.setMaxLines(100);
		ViewGroup.LayoutParams layout = _screenKeyboard.getLayoutParams();
		if( layout != null )
		{
			layout.width = ViewGroup.LayoutParams.FILL_PARENT;
			layout.height = ViewGroup.LayoutParams.FILL_PARENT;
			screenKeyboard.setLayoutParams(layout);
		}
		screenKeyboard.setGravity(android.view.Gravity.BOTTOM | android.view.Gravity.LEFT);
		*/
		String hint = _screenKeyboardHintMessage;
		screenKeyboard.setHint(hint != null ? hint : getString(R.string.text_edit_click_here));
		screenKeyboard.setText(oldText);
		screenKeyboard.setSelection(screenKeyboard.getText().length());
		screenKeyboard.setOnKeyListener(new simpleKeyListener(this));
		screenKeyboard.setBackgroundColor(Color.BLACK); // Full opaque - do not show semi-transparent edit box, it's confusing
		screenKeyboard.setTextColor(Color.WHITE); // Just to be sure about gamma
		if( isRunningOnOUYA() && Globals.TvBorders )
			screenKeyboard.setPadding(100, 100, 100, 100); // Bad bad HDMI TVs all have cropped borders
		_screenKeyboard = screenKeyboard;
		_videoLayout.addView(_screenKeyboard);
		//_screenKeyboard.setKeyListener(new TextKeyListener(TextKeyListener.Capitalize.NONE, false));
		screenKeyboard.setInputType(InputType.TYPE_CLASS_TEXT);
		screenKeyboard.setFocusableInTouchMode(true);
		screenKeyboard.setFocusable(true);
		screenKeyboard.requestFocus();
		_inputManager.showSoftInput(screenKeyboard, InputMethodManager.SHOW_IMPLICIT);
		getWindow().setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_STATE_VISIBLE);
		// Hack to try to force on-screen keyboard
		final EditText keyboard = screenKeyboard;
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

	public void hideScreenKeyboard()
	{
		if( keyboardWithoutTextInputShown )
			showScreenKeyboardWithoutTextInputField(Globals.TextInputKeyboard);

		if(_screenKeyboard == null || ! (_screenKeyboard instanceof EditText))
			return;

		synchronized(textInput)
		{
			String text = ((EditText)_screenKeyboard).getText().toString();
			for(int i = 0; i < text.length(); i++)
			{
				DemoRenderer.nativeTextInput( (int)text.charAt(i), (int)text.codePointAt(i) );
			}
		}
		DemoRenderer.nativeTextInputFinished();
		_inputManager.hideSoftInputFromWindow(_screenKeyboard.getWindowToken(), 0);
		_videoLayout.removeView(_screenKeyboard);
		_screenKeyboard = null;
		mGLView.setFocusableInTouchMode(true);
		mGLView.setFocusable(true);
		mGLView.requestFocus();
		DimSystemStatusBar.get().dim(_videoLayout);

		_videoLayout.postDelayed( new Runnable()
		{
			public void run()
			{
				DimSystemStatusBar.get().dim(_videoLayout);
			}
		}, 500 );
	};

	public boolean isScreenKeyboardShown()
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
				if( _screenKeyboard != null && _screenKeyboard instanceof EditText )
				{
					String hint = _screenKeyboardHintMessage;
					((EditText)_screenKeyboard).setHint(hint != null ? hint : getString(R.string.text_edit_click_here));
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
	}
	public void requestNewAdvertisement()
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
	}

	@Override
	public boolean onKeyDown(int keyCode, final KeyEvent event)
	{
		if( keyCode == KeyEvent.KEYCODE_BACK )
		{
			if( (event.getSource() & InputDevice.SOURCE_MOUSE) == InputDevice.SOURCE_MOUSE )
			{
				// Stupid Samsung and stupid Acer remaps right mouse button to BACK key
				DemoGLSurfaceView.nativeMouseButtonsPressed(2, 1);
				return true;
			}
			else if( keyboardWithoutTextInputShown )
			{
				return true;
			}
		}
		if( _screenKeyboard != null && _screenKeyboard.onKeyDown(keyCode, event) )
			return true;

		if( mGLView != null )
		{
			if( mGLView.nativeKey( keyCode, 1, event.getUnicodeChar() ) == 0 )
				return super.onKeyDown(keyCode, event);
		}
		else
		if( keyListener != null )
		{
			keyListener.onKeyEvent(keyCode);
		}
		else
		if( _btn != null )
			return _btn.onKeyDown(keyCode, event);
		return true;
	}
	
	@Override
	public boolean onKeyUp(int keyCode, final KeyEvent event)
	{
		if( keyCode == KeyEvent.KEYCODE_BACK )
		{
			if( (event.getSource() & InputDevice.SOURCE_MOUSE) == InputDevice.SOURCE_MOUSE )
			{
				// Stupid Samsung and stupid Acer remaps right mouse button to BACK key
				DemoGLSurfaceView.nativeMouseButtonsPressed(2, 0);
				return true;
			}
			else if( keyboardWithoutTextInputShown )
			{
				showScreenKeyboardWithoutTextInputField(0); // Hide keyboard
				return true;
			}
		}
		if( _screenKeyboard != null && _screenKeyboard.onKeyUp(keyCode, event) )
			return true;

		if( mGLView != null )
		{
			if( mGLView.nativeKey( keyCode, 0, event.getUnicodeChar() ) == 0 )
				return super.onKeyUp(keyCode, event);
			if( keyCode == KeyEvent.KEYCODE_BACK || keyCode == KeyEvent.KEYCODE_MENU )
			{
				DimSystemStatusBar.get().dim(_videoLayout);
				//DimSystemStatusBar.get().dim(mGLView);
			}
		}
		else
		if( _btn != null )
			return _btn.onKeyUp(keyCode, event);
		return true;
	}

	@Override
	public boolean onKeyMultiple(int keyCode, int repeatCount, final KeyEvent event)
	{
		if( _screenKeyboard != null )
		{
			_screenKeyboard.onKeyMultiple(keyCode, repeatCount, event);
			return true;
		}
		else if( mGLView != null && event.getCharacters() != null )
		{
			// International text input
			for(int i = 0; i < event.getCharacters().length(); i++ )
			{
				mGLView.nativeKey( event.getKeyCode(), 1, event.getCharacters().codePointAt(i) );
				mGLView.nativeKey( event.getKeyCode(), 0, event.getCharacters().codePointAt(i) );
			}
			return true;
		}
		return false;
	}

	@Override
	public boolean onKeyLongPress (int keyCode, KeyEvent event)
	{
		if( _screenKeyboard != null )
		{
			_screenKeyboard.onKeyLongPress(keyCode, event);
			return true;
		}
		return false;
	}

	@Override
	public boolean dispatchTouchEvent(final MotionEvent ev)
	{
		//Log.i("SDL", "dispatchTouchEvent: " + ev.getAction() + " coords " + ev.getX() + ":" + ev.getY() );
		if(_screenKeyboard != null && _screenKeyboard.dispatchTouchEvent(ev))
			return true;

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
		if( touchListener != null )
			touchListener.onTouchEvent(ev);
		return true;
	}
	
	@Override
	public boolean dispatchGenericMotionEvent (MotionEvent ev)
	{
		//Log.i("SDL", "dispatchGenericMotionEvent: " + ev.getAction() + " coords " + ev.getX() + ":" + ev.getY() );
		// This code fails to run for Android 1.6, so there will be no generic motion event for Andorid screen keyboard
		/*
		if(_screenKeyboard != null)
			_screenKeyboard.dispatchGenericMotionEvent(ev);
		else
		*/
		if(mGLView != null)
			mGLView.onGenericMotionEvent(ev);
		return true;
	}

	//private Configuration oldConfig = null;
	@Override
	public void onConfigurationChanged(Configuration newConfig)
	{
		super.onConfigurationChanged(newConfig);
		updateScreenOrientation();
		/*
		if (oldConfig != null)
		{
			int diff = newConfig.diff(oldConfig);
			Log.i("SDL", "onConfigurationChanged(): " + " diff " + diff +
					((diff & ActivityInfo.CONFIG_ORIENTATION) == ActivityInfo.CONFIG_ORIENTATION ? " orientation" : "") +
					((diff & ActivityInfo.CONFIG_SCREEN_SIZE) == ActivityInfo.CONFIG_SCREEN_SIZE ? " screen size" : "") +
					((diff & ActivityInfo.CONFIG_SMALLEST_SCREEN_SIZE) == ActivityInfo.CONFIG_SMALLEST_SCREEN_SIZE ? " smallest screen size" : "") +
					" " + newConfig.toString());
		}
		oldConfig = new Configuration(newConfig);
		*/
	}

	public void updateScreenOrientation()
	{
		int rotation = Surface.ROTATION_0;
		if( android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.FROYO )
			rotation = getWindowManager().getDefaultDisplay().getRotation();
		AccelerometerReader.gyro.invertedOrientation = ( rotation == Surface.ROTATION_180 || rotation == Surface.ROTATION_270 );
		//Log.d("SDL", "updateScreenOrientation(): screen orientation: " + rotation + " inverted " + AccelerometerReader.gyro.invertedOrientation);
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

	@Override
	public void onNewIntent(Intent i)
	{
		Log.i("SDL", "onNewIntent(): " + i.toString());
		super.onNewIntent(i);
		setIntent(i);
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
					if( entry == null )
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

		String [] binaryZipNames = { "binaries-" + android.os.Build.CPU_ABI + ".zip", "binaries-" + android.os.Build.CPU_ABI2 + ".zip", "binaries.zip" };
		if ( android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.JELLY_BEAN )
			binaryZipNames = new String[] { "binaries-" + android.os.Build.CPU_ABI + "-pie.zip", "binaries-" + android.os.Build.CPU_ABI2 + "-pie.zip", "binaries-" + android.os.Build.CPU_ABI + ".zip", "binaries-" + android.os.Build.CPU_ABI2 + ".zip", "binaries.zip" };
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
				break;
			}
			catch ( Exception eee )
			{
				//Log.i("SDL", "libSDL: Error: " + eee.toString());
			}
		}
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
		Log.v("SDL", "libSDL: loaded all libraries");
		ApplicationLibraryLoaded = true;
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
		UiModeManager uiModeManager = (UiModeManager) getSystemService(UI_MODE_SERVICE);
		return (uiModeManager.getCurrentModeType() == Configuration.UI_MODE_TYPE_TELEVISION) || Globals.OuyaEmulation;
	}

	public boolean isCurrentOrientationHorizontal()
	{
		if (Globals.AutoDetectOrientation)
		{
			// Less reliable way to detect orientation, but works with multiwindow
			View topView = getWindow().peekDecorView();
			if (topView != null)
			{
				//Log.d("SDL", "isCurrentOrientationHorizontal(): decorview: " + topView.getWidth() + "x" + topView.getHeight());
				return topView.getWidth() >= topView.getHeight();
			}
		}
		Display getOrient = getWindowManager().getDefaultDisplay();
		return getOrient.getWidth() >= getOrient.getHeight();
	}

	void setScreenOrientation()
	{
		if( !Globals.AutoDetectOrientation && getIntent().getBooleanExtra(RestartMainActivity.ACTIVITY_AUTODETECT_SCREEN_ORIENTATION, false) )
			Globals.AutoDetectOrientation = true;
		if( Globals.AutoDetectOrientation )
		{
			if( android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.JELLY_BEAN_MR2 )
				setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_FULL_USER);
			else
				setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_USER);
			return;
		}
		if( android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.GINGERBREAD )
			setRequestedOrientation(Globals.HorizontalOrientation ? ActivityInfo.SCREEN_ORIENTATION_SENSOR_LANDSCAPE : ActivityInfo.SCREEN_ORIENTATION_SENSOR_PORTRAIT);
		else
			setRequestedOrientation(Globals.HorizontalOrientation ? ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE : ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
	}

	public FrameLayout getVideoLayout() { return _videoLayout; }

	DemoGLSurfaceView mGLView = null;
	private static AudioThread mAudioThread = null;
	private static DataDownloader downloader = null;

	private TextView _tv = null;
	private Button _btn = null;
	private LinearLayout _layout = null;
	private LinearLayout _layout2 = null;
	private Advertisement _ad = null;
	public CloudSave cloudSave = null;
	public ProgressDialog loadingDialog = null;

	FrameLayout _videoLayout = null;
	private View _screenKeyboard = null;
	private String _screenKeyboardHintMessage = null;
	static boolean keyboardWithoutTextInputShown = false;
	private boolean sdlInited = false;
	public static boolean ApplicationLibraryLoaded = false;

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
			if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.KITKAT && Globals.ImmersiveMode)
				// Immersive mode, I already hear curses when system bar reappears mid-game from the slightest swipe at the bottom of the screen
				view.setSystemUiVisibility(android.view.View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY | android.view.View.SYSTEM_UI_FLAG_HIDE_NAVIGATION | android.view.View.SYSTEM_UI_FLAG_FULLSCREEN);
			else
				view.setSystemUiVisibility(android.view.View.SYSTEM_UI_FLAG_LOW_PROFILE);
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
