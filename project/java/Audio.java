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
import android.media.AudioTrack;
import android.media.AudioManager;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder.AudioSource;
import java.io.*;
import android.util.Log;
import java.lang.Thread;


class AudioThread
{

	private MainActivity mParent;
	private AudioTrack mAudio;
	private byte[] mAudioBuffer;
	private int mVirtualBufSize;

	public AudioThread(MainActivity parent)
	{
		mParent = parent;
		mAudio = null;
		mAudioBuffer = null;
		nativeAudioInitJavaCallbacks();
	}
	
	public int fillBuffer()
	{
		if( mParent.isPaused() )
		{
			try{
				Thread.sleep(500);
			} catch (InterruptedException e) {}
		}
		else
		{
			//if( Globals.AudioBufferConfig == 0 ) // Gives too much spam to logcat, makes things worse
			//	mAudio.flush();

			mAudio.write( mAudioBuffer, 0, mVirtualBufSize );
		}
		
		return 1;
	}
	
	public int initAudio(int rate, int channels, int encoding, int bufSize)
	{
			if( mAudio == null )
			{
					channels = ( channels == 1 ) ? AudioFormat.CHANNEL_CONFIGURATION_MONO : 
													AudioFormat.CHANNEL_CONFIGURATION_STEREO;
					encoding = ( encoding == 1 ) ? AudioFormat.ENCODING_PCM_16BIT :
													AudioFormat.ENCODING_PCM_8BIT;

					mVirtualBufSize = bufSize;

					if( AudioTrack.getMinBufferSize( rate, channels, encoding ) > bufSize )
						bufSize = AudioTrack.getMinBufferSize( rate, channels, encoding );

					if(Globals.AudioBufferConfig != 0) {    // application's choice - use minimal buffer
						bufSize = (int)((float)bufSize * (((float)(Globals.AudioBufferConfig - 1) * 2.5f) + 1.0f));
						mVirtualBufSize = bufSize;
					}
					mAudioBuffer = new byte[bufSize];

					mAudio = new AudioTrack(AudioManager.STREAM_MUSIC,
												rate,
												channels,
												encoding,
												bufSize,
												AudioTrack.MODE_STREAM );
					mAudio.play();
			}
			return mVirtualBufSize;
	}
	
	public byte[] getBuffer()
	{
		return mAudioBuffer;
	}
	
	public int deinitAudio()
	{
		if( mAudio != null )
		{
			mAudio.stop();
			mAudio.release();
			mAudio = null;
		}
		mAudioBuffer = null;
		return 1;
	}
	
	public int initAudioThread()
	{
		// Make audio thread priority higher so audio thread won't get underrun
		Thread.currentThread().setPriority(Thread.MAX_PRIORITY);
		return 1;
	}
	
	public int pauseAudioPlayback()
	{
		if( mAudio != null )
		{
			mAudio.pause();
		}
		if( mRecorder != null )
		{
			mRecorder.stop();
		}
		return 1;
	}

	public int resumeAudioPlayback()
	{
		if( mAudio != null )
		{
			mAudio.play();
		}
		if( mRecorder != null )
		{
			mRecorder.startRecording();
		}
		return 1;
	}

	private native int nativeAudioInitJavaCallbacks();

	// ----- Audio recording -----

	private AudioRecord mRecorder = null;
	private RecordingThread mRecordThread = null;
	//private int mRecordSize;
	//private int mRecordPos;

	private int startRecording(int rate, int channels, int encoding, int bufsize)
	{
		//mRecordSize = bufsize;
		//mRecordPos = 0;
		if( mRecordThread != null )
		{
			System.out.println("SDL: error: application already opened audio recording device");
			return 0;
		}
		channels = ( channels == 1 ) ? AudioFormat.CHANNEL_IN_MONO :
										AudioFormat.CHANNEL_IN_STEREO;
		encoding = ( encoding == 1 ) ? AudioFormat.ENCODING_PCM_16BIT :
										AudioFormat.ENCODING_PCM_8BIT;

		int minBufDevice = AudioRecord.getMinBufferSize(rate, channels, encoding);
		int minBufferSize = Math.max(bufsize * 8, minBufDevice + (bufsize - (minBufDevice % bufsize)));
		System.out.println("SDL: app opened recording device, rate " + rate + " channels " + channels + " sample size " + (encoding+1) + " bufsize " + bufsize + " internal bufsize " + minBufferSize);
		try {
			mRecorder = new AudioRecord(AudioSource.DEFAULT, rate, channels, encoding, minBufferSize);
		} catch (IllegalArgumentException e) {
			System.out.println("SDL: error: failed to open recording device!");
			return 0;
		}
		mRecordThread = new RecordingThread(bufsize);
		mRecorder.startRecording();
		mRecordThread.start();
		return minBufferSize;
	}

	private void stopRecording()
	{
		if( mRecordThread == null )
		{
			System.out.println("SDL: error: application already closed audio recording device");
			return;
		}
		mRecordThread.terminate = true;
		while( !mRecordThread.stopped )
		{
			try{
				Thread.sleep(100);
			} catch (InterruptedException e) {}
		}
		mRecordThread = null;
		mRecorder.stop();
		mRecorder.release();
		mRecorder = null;
		System.out.println("SDL: app closed recording device");
	}

	private class RecordingThread extends Thread
	{
		public boolean terminate = false;
		public boolean stopped = false;
		private byte[] mRecordBuffer;

		RecordingThread(int bufsize)
		{
			super();
			mRecordBuffer = new byte[bufsize];
		}

		public void run()
		{
			while( !terminate )
			{
				int got = mRecorder.read(mRecordBuffer, 0, mRecordBuffer.length);
				if( got != mRecordBuffer.length )
				{
					System.out.println("SDL: warning: RecordingThread: mRecorder.read returned short byte count " + got + " for bufsize " + mRecordBuffer.length);
					// TODO: record in a loop?
				}
				System.out.println("SDL: nativeAudioRecordCallback with len " + mRecordBuffer.length);
				nativeAudioRecordCallback(mRecordBuffer);
				System.out.println("SDL: nativeAudioRecordCallback returned");
			}
			stopped = true;
		}
	}

	private native void nativeAudioRecordCallback(byte[] buffer);

	/*
	private int recordRead()
	{
		mRecordPos += mRecordSize;
		if( mRecordPos >= mRecordBuffer.length )
			mRecordPos = 0;
		mRecorder.read(mRecordBuffer, mRecordPos, mRecordSize);
		return mRecordPos;
	}

	public byte[] getRecordBuffer()
	{
		return mRecordBuffer;
	}
	*/
}
