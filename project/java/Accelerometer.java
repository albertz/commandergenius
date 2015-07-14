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
import android.os.Build;


class AccelerometerReader implements SensorEventListener
{

	private SensorManager _manager = null;
	public boolean openedBySDL = false;
	public static final GyroscopeListener gyro = new GyroscopeListener();
	public static final OrientationListener orientation = new OrientationListener();

	public AccelerometerReader(Activity context)
	{
		_manager = (SensorManager) context.getSystemService(Context.SENSOR_SERVICE);
	}
	
	public synchronized void stop()
	{
		if( _manager != null )
		{
			Log.i("SDL", "libSDL: stopping accelerometer/gyroscope/orientation");
			_manager.unregisterListener(this);
			_manager.unregisterListener(gyro);
			_manager.unregisterListener(orientation);
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
		if( (Globals.AppUsesGyroscope || Globals.MoveMouseWithGyroscope) &&
			_manager != null && _manager.getDefaultSensor(Sensor.TYPE_GYROSCOPE) != null )
		{
			Log.i("SDL", "libSDL: starting gyroscope");
			_manager.registerListener(gyro, _manager.getDefaultSensor(Sensor.TYPE_GYROSCOPE), SensorManager.SENSOR_DELAY_GAME);
		}
		if( (Globals.AppUsesOrientationSensor) && _manager != null &&
			_manager.getDefaultSensor(Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2 ? Sensor.TYPE_GAME_ROTATION_VECTOR : Sensor.TYPE_ROTATION_VECTOR) != null )
		{
			Log.i("SDL", "libSDL: starting orientation sensor");
			_manager.registerListener(orientation, _manager.getDefaultSensor(
				Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2 ? Sensor.TYPE_GAME_ROTATION_VECTOR : Sensor.TYPE_ROTATION_VECTOR),
				SensorManager.SENSOR_DELAY_GAME);
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
		public float x1 = 0.0f, x2 = 0.0f, xc = 0.0f, y1 = 0.0f, y2 = 0.0f, yc = 0.0f, z1 = 0.0f, z2 = 0.0f, zc = 0.0f;
		public boolean invertedOrientation = false;
		public GyroscopeListener()
		{
		}
		public void onSensorChanged(SensorEvent event)
		{
			if( event.values[0] < x1 || event.values[0] > x2 ||
				event.values[1] < y1 || event.values[1] > y2 ||
				event.values[2] < z1 || event.values[2] > z2 )
			{
				if( Globals.HorizontalOrientation )
				{
					if( invertedOrientation )
						nativeGyroscope(-(event.values[0] - xc), -(event.values[1] - yc), event.values[2] - zc);
					else
						nativeGyroscope(event.values[0] - xc, event.values[1] - yc, event.values[2] - zc);
				}
				else
				{
					if( invertedOrientation )
						nativeGyroscope(-(event.values[1] - yc), event.values[0] - xc, event.values[2] - zc);
					else
						nativeGyroscope(event.values[1] - yc, -(event.values[0] - xc), event.values[2] - zc);
				}
			}
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
			manager.registerListener(gyro, manager.getDefaultSensor(
				Globals.AppUsesOrientationSensor ? Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR2 ?
				Sensor.TYPE_GAME_ROTATION_VECTOR : Sensor.TYPE_ROTATION_VECTOR : Sensor.TYPE_GYROSCOPE),
				SensorManager.SENSOR_DELAY_GAME);
		}
		public void unregisterListener(Activity context,SensorEventListener l)
		{
			SensorManager manager = (SensorManager) context.getSystemService(Context.SENSOR_SERVICE);
			if ( manager == null )
				return;
			manager.unregisterListener(l);
		}
	}

	static class OrientationListener implements SensorEventListener
	{
		public OrientationListener()
		{
		}
		public void onSensorChanged(SensorEvent event)
		{
			nativeOrientation(event.values[0], event.values[1], event.values[2]);
		}
		public void onAccuracyChanged(Sensor s, int a)
		{
		}
	}

	private static native void nativeAccelerometer(float accX, float accY, float accZ);
	private static native void nativeGyroscope(float X, float Y, float Z);
	private static native void nativeOrientation(float X, float Y, float Z);
}
