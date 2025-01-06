package com.epicgames.unreal;

import android.util.Log;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.BroadcastReceiver;
import android.media.AudioManager;


class AudioVolumeReceiver extends BroadcastReceiver 
{
	private static native void volumeChanged( int state );
	private static native void ringerModeChanged( boolean isMuted );

	private static IntentFilter filter;
	private static AudioVolumeReceiver receiver;

	private static String VOLUME_CHANGED_ACTION = "android.media.VOLUME_CHANGED_ACTION";
	private static String RINGER_MODE_CHANGED_ACTION = "android.media.RINGER_MODE_CHANGED";
	private static String STREAM_TYPE = "android.media.EXTRA_VOLUME_STREAM_TYPE";
	private static String STREAM_VALUE = "android.media.EXTRA_VOLUME_STREAM_VALUE";

	public static void startReceiver( Activity activity )
	{
		GameActivity.Log.debug( "Registering AudioVolumeReceiver" );
		if ( filter == null ) 
		{
			filter = new IntentFilter();
			filter.addAction( VOLUME_CHANGED_ACTION );
			filter.addAction( RINGER_MODE_CHANGED_ACTION );
		}
		if ( receiver == null ) 
		{
			receiver = new AudioVolumeReceiver();
		}

		activity.registerReceiver( receiver, filter );

		AudioManager audio = (AudioManager)activity.getSystemService( Context.AUDIO_SERVICE );
		int volume = audio.getStreamVolume( AudioManager.STREAM_MUSIC );
		GameActivity.Log.debug( "startAudioVolumeReceiver: " + volume );

		// initialize with the current volume state
		volumeChanged( volume );
	}

	public static void stopReceiver( Activity activity )
	{
		GameActivity.Log.debug( "Unregistering volume receiver" );
		activity.unregisterReceiver( receiver );
	}

	@Override
	public void onReceive( final Context context, final Intent intent ) 
	{
		if(intent.getAction() == VOLUME_CHANGED_ACTION)
		{
			GameActivity.Log.debug( "OnReceive VOLUME_CHANGED_ACTION" );
			int stream = ( Integer )intent.getExtras().get( STREAM_TYPE );
			int volume = ( Integer )intent.getExtras().get( STREAM_VALUE );
			if ( stream == AudioManager.STREAM_MUSIC )
			{
				volumeChanged( volume );
			}
			else
			{
				GameActivity.Log.debug( "skipping volume change from stream " + stream );
			}
		}else if(intent.getAction() ==RINGER_MODE_CHANGED_ACTION)
		{
			GameActivity.Log.debug("OnReceive RINGER_MODE_CHANGED_ACTION");
			boolean isMuted = true;
			if (( Integer )intent.getExtras().get(AudioManager.EXTRA_RINGER_MODE) == AudioManager.RINGER_MODE_NORMAL)
			{
				isMuted = false;
			}
			ringerModeChanged(isMuted);
		}
	}
}