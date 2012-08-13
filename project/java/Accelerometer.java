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
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.KeyEvent;
import android.view.Window;
import android.view.WindowManager;
import android.os.Vibrator;
import android.hardware.SensorManager;
import android.hardware.SensorEventListener;
import android.hardware.Sensor;
import android.hardware.SensorEvent;

import android.widget.TextView;


class AccelerometerReader implements SensorEventListener {

	private SensorManager _manager = null;

	public AccelerometerReader(Activity context)
	{
		System.out.println("libSDL: accelerometer start required: " + String.valueOf(Globals.UseAccelerometerAsArrowKeys));
		_manager = (SensorManager) context.getSystemService(Context.SENSOR_SERVICE);
		start();
	}
	
	public synchronized void stop()
	{
		if( _manager != null )
		{
			System.out.println("libSDL: stopping accelerometer");
			_manager.unregisterListener(this);
		}
	}

	public synchronized void start()
	{
		if( Globals.UseAccelerometerAsArrowKeys )
		{
			if( _manager != null )
			{
				System.out.println("libSDL: starting accelerometer");
				// TODO: orientation allows for 3rd axis - azimuth, but it will be way too hard to the user
				// if( ! _manager.registerListener(this, _manager.getDefaultSensor(Sensor.TYPE_ORIENTATION), SensorManager.SENSOR_DELAY_GAME) )
				_manager.registerListener(this, _manager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER), SensorManager.SENSOR_DELAY_GAME);
			}
		}
	}

	public synchronized void onSensorChanged(SensorEvent event) {

		if (event.sensor.getType() == Sensor.TYPE_ACCELEROMETER) 
		{
			if( Globals.HorizontalOrientation )
				nativeAccelerometer(event.values[1], -event.values[0], event.values[2]);
			else
				nativeAccelerometer(event.values[0], event.values[1], event.values[2]); // TODO: not tested!
		}
		else
		{
			if( Globals.HorizontalOrientation )
				nativeOrientation(event.values[1], -event.values[2], event.values[0]);
			else
				nativeOrientation(event.values[2], event.values[1], event.values[0]);
		}
		
	}

	public synchronized void onAccuracyChanged(Sensor s, int a) {
	}

	private native void nativeAccelerometer(float accX, float accY, float accZ);
	private native void nativeOrientation(float accX, float accY, float accZ);
}


