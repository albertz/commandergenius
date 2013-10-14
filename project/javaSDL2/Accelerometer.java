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
import android.util.Log;
import android.widget.TextView;


class AccelerometerReader implements SensorEventListener
{

	private SensorManager _manager = null;
	public boolean openedBySDL = false;
	public static final GyroscopeListener gyro = new GyroscopeListener();

	public AccelerometerReader(Activity context)
	{
		_manager = (SensorManager) context.getSystemService(Context.SENSOR_SERVICE);
	}
	
	public synchronized void stop()
	{
		if( _manager != null )
		{
			Log.i("SDL", "libSDL: stopping accelerometer/gyroscope");
			_manager.unregisterListener(this);
			_manager.unregisterListener(gyro);
		}
	}

	public synchronized void start()
	{
		if( (Globals.UseAccelerometerAsArrowKeys || Globals.AppUsesAccelerometer) &&
			_manager != null && _manager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER) != null )
		{
			Log.i("SDL", "libSDL: starting accelerometer");
			_manager.registerListener(this, _manager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER), SensorManager.SENSOR_DELAY_GAME);
		}
		if( Globals.AppUsesGyroscope && _manager != null && _manager.getDefaultSensor(Sensor.TYPE_GYROSCOPE) != null )
		{
			Log.i("SDL", "libSDL: starting gyroscope");
			_manager.registerListener(gyro, _manager.getDefaultSensor(Sensor.TYPE_GYROSCOPE), SensorManager.SENSOR_DELAY_GAME);
		}
	}

	public void onSensorChanged(SensorEvent event)
	{
		if( Globals.HorizontalOrientation )
			nativeAccelerometer(event.values[1], -event.values[0], event.values[2]);
		else
			nativeAccelerometer(event.values[0], event.values[1], event.values[2]); // TODO: not tested!
	}
	public void onAccuracyChanged(Sensor s, int a)
	{
	}

	static class GyroscopeListener implements SensorEventListener
	{
		public float x1, x2, xc, y1, y2, yc, z1, z2, zc;
		public GyroscopeListener()
		{
		}
		public void onSensorChanged(SensorEvent event)
		{
			// TODO: vertical orientation
			//if( Globals.HorizontalOrientation )
			if( event.values[0] < x1 || event.values[0] > x2 ||
				event.values[1] < y1 || event.values[1] > y2 ||
				event.values[2] < z1 || event.values[2] > z2 )
				nativeGyroscope(event.values[0] - xc, event.values[1] - yc, event.values[2] - zc);
		}
		public void onAccuracyChanged(Sensor s, int a)
		{
		}
		public boolean available(Activity context)
		{
			SensorManager manager = (SensorManager) context.getSystemService(Context.SENSOR_SERVICE);
			return ( manager != null && manager.getDefaultSensor(Sensor.TYPE_GYROSCOPE) != null );
		}
		public void registerListener(Activity context, SensorEventListener l)
		{
			SensorManager manager = (SensorManager) context.getSystemService(Context.SENSOR_SERVICE);
			if ( manager == null && manager.getDefaultSensor(Sensor.TYPE_GYROSCOPE) == null )
				return;
			manager.registerListener(l, manager.getDefaultSensor(Sensor.TYPE_GYROSCOPE), SensorManager.SENSOR_DELAY_GAME);
		}
		public void unregisterListener(Activity context,SensorEventListener l)
		{
			SensorManager manager = (SensorManager) context.getSystemService(Context.SENSOR_SERVICE);
			if ( manager == null )
				return;
			manager.unregisterListener(l);
		}
	}

	private static native void nativeAccelerometer(float accX, float accY, float accZ);
	private static native void nativeGyroscope(float X, float Y, float Z);
}
