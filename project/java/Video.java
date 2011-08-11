/*ACTION_HOVER_MOVE
Simple DirectMedia Layer
Java source code (C) 2009-2011 Sergii Pylypenko
  
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

import javax.microedition.khronos.opengles.GL10;
import javax.microedition.khronos.opengles.GL11;
import javax.microedition.khronos.opengles.GL11Ext;

import javax.microedition.khronos.egl.EGL10;
import javax.microedition.khronos.egl.EGL11;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.egl.EGLContext;
import javax.microedition.khronos.egl.EGLDisplay;
import javax.microedition.khronos.egl.EGLSurface;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.KeyEvent;
import android.view.Window;
import android.view.WindowManager;
import android.os.Environment;
import java.io.File;
import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;
import android.content.res.Resources;
import android.content.res.AssetManager;

import android.widget.TextView;
import java.lang.Thread;
import java.util.concurrent.locks.ReentrantLock;
import android.os.Build;
import java.lang.reflect.Method;
import java.util.LinkedList;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;


class Mouse
{
	public static final int LEFT_CLICK_NORMAL = 0;
	public static final int LEFT_CLICK_NEAR_CURSOR = 1;
	public static final int LEFT_CLICK_WITH_MULTITOUCH = 2;
	public static final int LEFT_CLICK_WITH_PRESSURE = 3;
	public static final int LEFT_CLICK_WITH_KEY = 4;
	public static final int LEFT_CLICK_WITH_TIMEOUT = 5;
	public static final int LEFT_CLICK_WITH_TAP = 6;
	public static final int LEFT_CLICK_WITH_TAP_OR_TIMEOUT = 7;
	
	public static final int RIGHT_CLICK_NONE = 0;
	public static final int RIGHT_CLICK_WITH_MULTITOUCH = 1;
	public static final int RIGHT_CLICK_WITH_PRESSURE = 2;
	public static final int RIGHT_CLICK_WITH_KEY = 3;
	public static final int RIGHT_CLICK_WITH_TIMEOUT = 4;

	public static final int SDL_FINGER_DOWN = 0;
	public static final int SDL_FINGER_UP = 1;
	public static final int SDL_FINGER_MOVE = 2;
	public static final int SDL_FINGER_HOVER = 3;

}

abstract class DifferentTouchInput
{
	public static DifferentTouchInput getInstance()
	{
		boolean multiTouchAvailable1 = false;
		boolean multiTouchAvailable2 = false;
		// Not checking for getX(int), getY(int) etc 'cause I'm lazy
		Method methods [] = MotionEvent.class.getDeclaredMethods();
		for(Method m: methods) 
		{
			if( m.getName().equals("getPointerCount") )
				multiTouchAvailable1 = true;
			if( m.getName().equals("getPointerId") )
				multiTouchAvailable2 = true;
		}

		if (multiTouchAvailable1 && multiTouchAvailable2)
			return MultiTouchInput.Holder.sInstance;
		else
			return SingleTouchInput.Holder.sInstance;
	}
	public abstract void process(final MotionEvent event);
	private static class SingleTouchInput extends DifferentTouchInput
	{
		private static class Holder
		{
			private static final SingleTouchInput sInstance = new SingleTouchInput();
		}
		public void process(final MotionEvent event)
		{
			int action = -1;
			if( event.getAction() == MotionEvent.ACTION_DOWN )
				action = Mouse.SDL_FINGER_DOWN;
			if( event.getAction() == MotionEvent.ACTION_UP )
				action = Mouse.SDL_FINGER_UP;
			if( event.getAction() == MotionEvent.ACTION_MOVE )
				action = Mouse.SDL_FINGER_MOVE;
			if ( action >= 0 )
				DemoGLSurfaceView.nativeMouse( (int)event.getX(), (int)event.getY(), action, 0, 
												(int)(event.getPressure() * 1000.0),
												(int)(event.getSize() * 1000.0) );
		}
	}
	private static class MultiTouchInput extends DifferentTouchInput
	{
		
		private static final int touchEventMax = 16; // Max multitouch pointers

		private class touchEvent
		{
			public boolean down = false;
			public int x = 0;
			public int y = 0;
			public int pressure = 0;
			public int size = 0;
		}
		
		private touchEvent touchEvents[];
		
		MultiTouchInput()
		{
			touchEvents = new touchEvent[touchEventMax];
			for( int i = 0; i < touchEventMax; i++ )
				touchEvents[i] = new touchEvent();
		}
		
		private static class Holder
		{
			private static final MultiTouchInput sInstance = new MultiTouchInput();
		}

		public void process(final MotionEvent event)
		{
			int action = -1;

			//System.out.println("Got motion event, type " + (int)(event.getAction()) + " X " + (int)event.getX() + " Y " + (int)event.getY());
			if( event.getAction() == MotionEvent.ACTION_UP )
			{
				action = Mouse.SDL_FINGER_UP;
				for( int i = 0; i < touchEventMax; i++ )
				{
					if( touchEvents[i].down )
					{
						touchEvents[i].down = false;
						DemoGLSurfaceView.nativeMouse( touchEvents[i].x, touchEvents[i].y, action, i, touchEvents[i].pressure, touchEvents[i].size );
					}
				}
			}
			if( event.getAction() == MotionEvent.ACTION_DOWN )
			{
				action = Mouse.SDL_FINGER_DOWN;
				for( int i = 0; i < event.getPointerCount(); i++ )
				{
					int id = event.getPointerId(i);
					if( id >= touchEventMax )
						id = touchEventMax-1;
					touchEvents[id].down = true;
					touchEvents[id].x = (int)event.getX(i);
					touchEvents[id].y = (int)event.getY(i);
					touchEvents[id].pressure = (int)(event.getPressure(i) * 1000.0);
					touchEvents[id].size = (int)(event.getSize(i) * 1000.0);
					DemoGLSurfaceView.nativeMouse( touchEvents[id].x, touchEvents[id].y, action, id, touchEvents[id].pressure, touchEvents[id].size );
				}
			}
			if( event.getAction() == MotionEvent.ACTION_MOVE )
			{
				/*
				String s = "MOVE: ptrs " + event.getPointerCount();
				for( int i = 0 ; i < event.getPointerCount(); i++ )
				{
					s += " p" + event.getPointerId(i) + "=" + (int)event.getX(i) + ":" + (int)event.getY(i);
				}
				System.out.println(s);
				*/

				for( int id = 0; id < touchEventMax; id++ )
				{
					int ii;
					for( ii = 0; ii < event.getPointerCount(); ii++ )
					{
						if( id == event.getPointerId(ii) )
							break;
					}
					if( ii >= event.getPointerCount() )
					{
						// Up event
						if( touchEvents[id].down )
						{
							action = Mouse.SDL_FINGER_UP;
							touchEvents[id].down = false;
							DemoGLSurfaceView.nativeMouse( touchEvents[id].x, touchEvents[id].y, action, id, touchEvents[id].pressure, touchEvents[id].size );
						}
					}
					else
					{
						if( touchEvents[id].down )
							action = Mouse.SDL_FINGER_MOVE;
						else
							action = Mouse.SDL_FINGER_DOWN;
						touchEvents[id].down = true;
						touchEvents[id].x = (int)event.getX(ii);
						touchEvents[id].y = (int)event.getY(ii);
						touchEvents[id].pressure = (int)(event.getPressure(ii) * 1000.0);
						touchEvents[id].size = (int)(event.getSize(ii) * 1000.0);
						DemoGLSurfaceView.nativeMouse( touchEvents[id].x, touchEvents[id].y, action, id, touchEvents[id].pressure, touchEvents[id].size );
					}
				}
			}
			if( event.getAction() == MotionEvent.ACTION_HOVER_MOVE ) // Support bluetooth/USB mouse - available since Android 3.1
			{
				// TODO: it is possible that multiple pointers return that event, but we're handling only pointer #0
				if( touchEvents[0].down )
					action = Mouse.SDL_FINGER_UP;
				else
					action = Mouse.SDL_FINGER_HOVER;
				touchEvents[0].down = false;
				touchEvents[0].x = (int)event.getX();
				touchEvents[0].y = (int)event.getY();
				touchEvents[0].pressure = 0;
				touchEvents[0].size = 0;
				DemoGLSurfaceView.nativeMouse( touchEvents[0].x, touchEvents[0].y, action, 0, touchEvents[0].pressure, touchEvents[0].size );
			}
		}
	}
}


class DemoRenderer extends GLSurfaceView_SDL.Renderer
{
	public DemoRenderer(MainActivity _context)
	{
		context = _context;
		// Froyo does not flood touch events, and syncs to the screen update,
		// so we should not use event rate limiter, or we'll get some multitouch events largely outdated
		// Another test on Tegra development board shows that with USB mouse FPS drops in half
		// when mouse is moved, with and without ratelimiter
		if( android.os.Build.VERSION.SDK_INT < android.os.Build.VERSION_CODES.FROYO )
			mRatelimitTouchEvents = true;
		System.out.println("libSDL: DemoRenderer: RatelimitTouchEvents " + mRatelimitTouchEvents );
	}
	
	public void onSurfaceCreated(GL10 gl, EGLConfig config) {
		System.out.println("libSDL: DemoRenderer.onSurfaceCreated(): paused " + mPaused + " mFirstTimeStart " + mFirstTimeStart );
		mGlSurfaceCreated = true;
		if( ! mPaused && ! mFirstTimeStart )
			nativeGlContextRecreated();
		mFirstTimeStart = false;
	}

	public void onSurfaceChanged(GL10 gl, int w, int h) {
		mWidth = w;
		mHeight = h;
		nativeResize(w, h, Globals.KeepAspectRatio ? 1 : 0);
	}
	
	public void onSurfaceDestroyed() {
		mGlSurfaceCreated = false;
		mGlContextLost = true;
		nativeGlContextLost();
	};

	public void onDrawFrame(GL10 gl) {

		nativeInitJavaCallbacks();
		
		// Make main thread priority lower so audio thread won't get underrun
		// Thread.currentThread().setPriority((Thread.currentThread().getPriority() + Thread.MIN_PRIORITY)/2);
		
		mGlContextLost = false;

		String libs[] = { "application", "sdl_main" };
		try
		{
			for(String l : libs)
			{
				System.loadLibrary(l);
			}
		}
		catch ( UnsatisfiedLinkError e )
		{
			System.out.println("libSDL: error loading lib: " + e.toString());
			try
			{
				for(String l : libs)
				{
					String libname = System.mapLibraryName(l);
					File libpath = new File(context.getCacheDir(), libname);
					System.out.println("libSDL: loading lib " + libpath.getPath());
					System.load(libpath.getPath());
					libpath.delete();
				}
			}
			catch ( UnsatisfiedLinkError ee )
			{
				System.out.println("libSDL: error loading lib: " + ee.toString());
			}
		}

		Settings.Apply(context);
		accelerometer = new AccelerometerReader(context);
		// Tweak video thread priority, if user selected big audio buffer
		if(Globals.AudioBufferConfig >= 2)
			Thread.currentThread().setPriority( (Thread.NORM_PRIORITY + Thread.MIN_PRIORITY) / 2 ); // Lower than normal
		 // Calls main() and never returns, hehe - we'll call eglSwapBuffers() from native code
		nativeInit( Globals.DataDir,
					Globals.CommandLine,
					( (Globals.SwVideoMode && Globals.MultiThreadedVideo) || Globals.CompatibilityHacks ) ? 1 : 0 );
		System.exit(0); // The main() returns here - I don't bother with deinit stuff, just terminate process
	}

	public int swapBuffers() // Called from native code
	{
		if( mRatelimitTouchEvents )
		{
			synchronized(this)
			{
				this.notify();
			}
		}
		if( ! super.SwapBuffers() && Globals.NonBlockingSwapBuffers )
			return 0;
		if(mGlContextLost) {
			mGlContextLost = false;
			Settings.SetupTouchscreenKeyboardGraphics(context); // Reload on-screen buttons graphics
		}
		
		return 1;
	}

	public void showScreenKeyboard(final String oldText, int sendBackspace) // Called from native code
	{
		class Callback implements Runnable
		{
			public MainActivity parent;
			public String oldText;
			public boolean sendBackspace;
			public void run()
			{
				parent.showScreenKeyboard(oldText, sendBackspace);
			}
		}
		Callback cb = new Callback();
		cb.parent = context;
		cb.oldText = oldText;
		cb.sendBackspace = (sendBackspace != 0);
		context.runOnUiThread(cb);
	}

	public void exitApp()
	{
		 nativeDone();
	};

	private int PowerOf2(int i)
	{
		int value = 1;
		while (value < i)
			value <<= 1;
		return value;
	}
	public void DrawLogo(GL10 gl)
	{
		BitmapDrawable bmp = null;
		try
		{
			bmp = new BitmapDrawable(context.getAssets().open("logo.png"));
		}
		catch(Exception e)
		{
			bmp = new BitmapDrawable(context.getResources().openRawResource(R.drawable.publisherlogo));
		}
		int width = bmp.getBitmap().getWidth();
		int height = bmp.getBitmap().getHeight();
		ByteBuffer byteBuffer = ByteBuffer.allocateDirect(4 * width * height);
		byteBuffer.order(ByteOrder.BIG_ENDIAN);
		bmp.getBitmap().copyPixelsToBuffer(byteBuffer);
		byteBuffer.position(0);

		gl.glViewport(0, 0, mWidth, mHeight);
		gl.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		gl.glClear(gl.GL_COLOR_BUFFER_BIT | gl.GL_DEPTH_BUFFER_BIT);
		gl.glColor4x(0x10000, 0x10000, 0x10000, 0x10000);
		gl.glPixelStorei(gl.GL_UNPACK_ALIGNMENT, 1);
		gl.glEnable(GL10.GL_TEXTURE_2D);
		int textureName = -1;
		int mTextureNameWorkspace[] = new int[1];
		int mCropWorkspace[] = new int[4];
		gl.glGenTextures(1, mTextureNameWorkspace, 0);
		textureName = mTextureNameWorkspace[0];
		gl.glBindTexture(GL10.GL_TEXTURE_2D, textureName);
		gl.glActiveTexture(textureName);
		gl.glClientActiveTexture(textureName);
		gl.glTexImage2D(GL10.GL_TEXTURE_2D, 0, GL10.GL_RGBA,
				PowerOf2(width), PowerOf2(height), 0,
				GL10.GL_RGBA, GL10.GL_UNSIGNED_BYTE, null);
		gl.glTexSubImage2D(GL10.GL_TEXTURE_2D, 0, 0, 0,
				width, height,
				GL10.GL_RGBA, GL10.GL_UNSIGNED_BYTE, byteBuffer);
		mCropWorkspace[0] = 0; // u
		mCropWorkspace[1] = height; // v
		mCropWorkspace[2] = width;
		mCropWorkspace[3] = -height;
		((GL11) gl).glTexParameteriv(GL10.GL_TEXTURE_2D,
				GL11Ext.GL_TEXTURE_CROP_RECT_OES, mCropWorkspace, 0);
		((GL11Ext) gl).glDrawTexiOES(0, -mHeight, 0, mWidth, mHeight);
		gl.glActiveTexture(0);
		gl.glClientActiveTexture(0);
		gl.glBindTexture(GL10.GL_TEXTURE_2D, 0);
		gl.glDeleteTextures(1, mTextureNameWorkspace, 0);

		gl.glFlush();
	}


	private native void nativeInitJavaCallbacks();
	private native void nativeInit(String CurrentPath, String CommandLine, int multiThreadedVideo);
	private native void nativeResize(int w, int h, int keepAspectRatio);
	private native void nativeDone();
	private native void nativeGlContextLost();
	public native void nativeGlContextRecreated();
	public static native void nativeTextInput( int ascii, int unicode );
	public static native void nativeTextInputFinished();

	private MainActivity context = null;
	public AccelerometerReader accelerometer = null;
	
	private EGL10 mEgl = null;
	private EGLDisplay mEglDisplay = null;
	private EGLSurface mEglSurface = null;
	private EGLContext mEglContext = null;
	private boolean mGlContextLost = false;
	public boolean mGlSurfaceCreated = false;
	public boolean mPaused = false;
	private boolean mFirstTimeStart = true;
	public int mWidth = 0;
	public int mHeight = 0;
	public boolean mRatelimitTouchEvents = false;
}

class DemoGLSurfaceView extends GLSurfaceView_SDL {
	public DemoGLSurfaceView(MainActivity context) {
		super(context);
		mParent = context;
		touchInput = DifferentTouchInput.getInstance();
		setEGLConfigChooser(Globals.VideoDepthBpp, Globals.NeedDepthBuffer, Globals.NeedStencilBuffer, Globals.NeedGles2);
		mRenderer = new DemoRenderer(context);
		setRenderer(mRenderer);
	}

	@Override
	public boolean onTouchEvent(final MotionEvent event) 
	{
		touchInput.process(event);
		// Wait a bit, and try to synchronize to app framerate, or event thread will eat all CPU and we'll lose FPS
		if(( event.getAction() == MotionEvent.ACTION_MOVE ||
			event.getAction() == MotionEvent.ACTION_HOVER_MOVE) &&
			mRenderer.mRatelimitTouchEvents )
		{
			synchronized(mRenderer)
			{
				try
				{
					mRenderer.wait(300L);
				} catch (InterruptedException e) { }
			}
		}
		return true;
	};

	@Override
	public boolean onGenericMotionEvent (final MotionEvent ev)
	{
		return onTouchEvent(ev);
	}

	public void exitApp() {
		mRenderer.exitApp();
	};

	@Override
	public void onPause() {
		mRenderer.accelerometer.stop();
		super.onPause();
		mRenderer.mPaused = true;
	};
	
	public boolean isPaused() {
		return mRenderer.mPaused;
	}

	@Override
	public void onResume() {
		super.onResume();
		mRenderer.mPaused = false;
		System.out.println("libSDL: DemoGLSurfaceView.onResume(): mRenderer.mGlSurfaceCreated " + mRenderer.mGlSurfaceCreated + " mRenderer.mPaused " + mRenderer.mPaused);
		if( mRenderer.mGlSurfaceCreated && ! mRenderer.mPaused || Globals.NonBlockingSwapBuffers )
			mRenderer.nativeGlContextRecreated();
		mRenderer.accelerometer.start();
	};

	// This seems like redundant code - it handled in MainActivity.java
	@Override
	public boolean onKeyDown(int keyCode, final KeyEvent event) {
		if( nativeKey( keyCode, 1 ) == 0 )
				return super.onKeyDown(keyCode, event);
		return true;
	}
	
	@Override
	public boolean onKeyUp(int keyCode, final KeyEvent event) {
		if( nativeKey( keyCode, 0 ) == 0 )
				return super.onKeyUp(keyCode, event);
		return true;
	}

	DemoRenderer mRenderer;
	MainActivity mParent;
	DifferentTouchInput touchInput = null;

	public static native void nativeMouse( int x, int y, int action, int pointerId, int pressure, int radius );
	public static native int nativeKey( int keyCode, int down );
	public static native void initJavaCallbacks();

}


