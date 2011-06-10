/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2011 Sam Lantinga
    Java source code (C) 2009-2011 Sergii Pylypenko

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.
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
				action = 0;
			if( event.getAction() == MotionEvent.ACTION_UP )
				action = 1;
			if( event.getAction() == MotionEvent.ACTION_MOVE )
				action = 2;
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

		static final int SDL_FINGER_DOWN = 0;
		static final int SDL_FINGER_UP = 1;
		static final int SDL_FINGER_MOVE = 2;

		public void process(final MotionEvent event)
		{
			int action = -1;

			//System.out.println("Got motion event, type " + (int)(event.getAction()) + " X " + (int)event.getX() + " Y " + (int)event.getY());
			if( event.getAction() == MotionEvent.ACTION_UP )
			{
				action = SDL_FINGER_UP;
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
				action = SDL_FINGER_DOWN;
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
				for( int i = 0; i < touchEventMax; i++ )
				{
					int ii;
					for( ii = 0; ii < event.getPointerCount(); ii++ )
					{
						if( i == event.getPointerId(ii) )
							break;
					}
					if( ii >= event.getPointerCount() )
					{
						// Up event
						if( touchEvents[i].down )
						{
							action = SDL_FINGER_UP;
							touchEvents[i].down = false;
							DemoGLSurfaceView.nativeMouse( touchEvents[i].x, touchEvents[i].y, action, i, touchEvents[i].pressure, touchEvents[i].size );
						}
					}
					else
					{
						if( touchEvents[i].down )
							action = SDL_FINGER_MOVE;
						else
							action = SDL_FINGER_DOWN;
						touchEvents[i].down = true;
						touchEvents[i].x = (int)event.getX(ii);
						touchEvents[i].y = (int)event.getY(ii);
						touchEvents[i].pressure = (int)(event.getPressure(ii) * 1000.0);
						touchEvents[i].size = (int)(event.getSize(ii) * 1000.0);
						DemoGLSurfaceView.nativeMouse( touchEvents[i].x, touchEvents[i].y, action, i, touchEvents[i].pressure, touchEvents[i].size );
					}
				}
			}
			if( event.getAction() == MotionEvent.ACTION_HOVER_MOVE ) // Support bluetooth/USB mouse - available since Android 3.1
			{
				// TODO: it is possible that multiple pointers return that event, but we're handling only pointer #0
				// TODO: need to check this on a device, the emulator does not return such event
				if( touchEvents[0].down )
					action = SDL_FINGER_UP;
				else
					action = SDL_FINGER_MOVE;
				action = 2;
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

		// ----- VCMI hack -----
		try
		{
			File libpath = new File(context.getFilesDir(), "libvcmi.so");
			System.load(libpath.getPath());
		}
		catch ( UnsatisfiedLinkError eee )
		{
			System.out.println("libSDL: error loading lib: " + eee.toString());
		}
		// ----- VCMI hack -----
		
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
		nativeInit( Globals.DataDir,
					Globals.CommandLine,
					( Globals.SwVideoMode && Globals.MultiThreadedVideo ) ? 1 : 0 ); // Calls main() and never returns, hehe - we'll call eglSwapBuffers() from native code
		System.exit(0); // The main() returns here - I don't bother with deinit stuff, just terminate process
	}

	public int swapBuffers() // Called from native code
	{
		synchronized(this) {
			this.notify();
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
	private AccelerometerReader accelerometer = null;
	
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
}

class DemoGLSurfaceView extends GLSurfaceView_SDL {
	public DemoGLSurfaceView(MainActivity context) {
		super(context);
		mParent = context;
		touchInput = DifferentTouchInput.getInstance();
		setEGLConfigChooser(Globals.NeedDepthBuffer);
		mRenderer = new DemoRenderer(context);
		setRenderer(mRenderer);
	}

	@Override
	public boolean onTouchEvent(final MotionEvent event) 
	{
		touchInput.process(event);
		// Wait a bit, and try to synchronize to app framerate, or event thread will eat all CPU and we'll lose FPS
		if( event.getAction() == MotionEvent.ACTION_MOVE ) {
			synchronized(mRenderer) {
				try {
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


