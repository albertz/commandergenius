/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This is a small port of the "San Angeles Observation" demo
 * program for OpenGL ES 1.x. For more details, see:
 *
 *    http://jet.ro/visuals/san-angeles-observation/
 *
 * This program demonstrates how to use a GLSurfaceView from Java
 * along with native OpenGL calls to perform frame rendering.
 *
 * Touching the screen will start/stop the animation.
 *
 * Note that the demo runs much faster on the emulator than on
 * real devices, this is mainly due to the following facts:
 *
 * - the demo sends bazillions of polygons to OpenGL without
 *   even trying to do culling. Most of them are clearly out
 *   of view.
 *
 * - on a real device, the GPU bus is the real bottleneck
 *   that prevent the demo from getting acceptable performance.
 *
 * - the software OpenGL engine used in the emulator uses
 *   the system bus instead, and its code rocks :-)
 *
 * Fixing the program to send less polygons to the GPU is left
 * as an exercise to the reader. As always, patches welcomed :-)
 */
package de.schwardtnet.alienblaster;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.app.Activity;
import android.content.Context;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.KeyEvent;

class LoadLibrary {
    public LoadLibrary() {}
    static {
        System.loadLibrary("alienblaster");
    }
}

class DemoRenderer implements GLSurfaceView.Renderer {

    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        nativeInit();
    }

    public void onSurfaceChanged(GL10 gl, int w, int h) {
        //gl.glViewport(0, 0, w, h);
        nativeResize(w, h);
    }

    public void onDrawFrame(GL10 gl) {
        nativeRender();
    }

    public void exitApp() {
         nativeDone();
    };

    private static native void nativeInit();
    private static native void nativeResize(int w, int h);
    private static native void nativeRender();
    private static native void nativeDone();
}

class DemoGLSurfaceView extends GLSurfaceView {
    public DemoGLSurfaceView(Activity context) {
        super(context);
        mParent = context;
        mRenderer = new DemoRenderer();
        setRenderer(mRenderer);
    }

    @Override
    public boolean onTouchEvent(final MotionEvent event) {
        int action = -1;
        if( event.getAction() == MotionEvent.ACTION_DOWN )
        	action = 0;
        if( event.getAction() == MotionEvent.ACTION_UP )
        	action = 1;
        if( event.getAction() == MotionEvent.ACTION_MOVE )
        	action = 2;
        if (  action >= 0 ) {
            nativeMouse( (int)event.getX(), (int)event.getY(), action );
        }
        return true;
    }
	
	@Override
	public boolean onKeyDown(int keyCode, final KeyEvent event) {
         if(keyCode == KeyEvent.KEYCODE_BACK) {
            mParent.finish();
         } else {
           nativeKey( keyCode, 1 );
         }
         return true;
     }
	
	@Override
	public boolean onKeyUp(int keyCode, final KeyEvent event) {
         nativeKey( keyCode, 0 );
         return true;
     }
     
     public void exitApp() {
         mRenderer.exitApp();
     };

    DemoRenderer mRenderer;
    Activity mParent;

    private static native void nativeMouse( int x, int y, int action );
    private static native void nativeKey( int keyCode, int down );
}

public class DemoActivity extends Activity {
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mLoadLibraryStub = new LoadLibrary();
        mGLView = new DemoGLSurfaceView(this);
        setContentView(mGLView);
        // Receive keyboard events
        mGLView.setFocusableInTouchMode(true);
        mGLView.setFocusable(true);
        mGLView.requestFocus();
    }

    @Override
    protected void onPause() {
        super.onPause();
        mGLView.onPause();
    }

    @Override
    protected void onResume() {
        super.onResume();
        mGLView.onResume();
    }

    @Override
    protected void onStop() {
        super.onStop();
        mGLView.exitApp();
        finish();
    }

    private DemoGLSurfaceView mGLView;
    private LoadLibrary mLoadLibraryStub;
}
