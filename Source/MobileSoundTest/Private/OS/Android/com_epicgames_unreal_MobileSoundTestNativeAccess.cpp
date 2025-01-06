#include "com_epicgames_unreal_MobileSoundTestNativeAccess.h"

JNIEXPORT void JNICALL
Java_com_epicgames_unreal_AudioVolumeReceiver_volumeChanged(JNIEnv* jni, jclass clazz, jint volume)
{
	int scaledVolume = (volume * 100) / 15;;
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Changed: SystemSound is On(%i%%)"), scaledVolume));
}

JNIEXPORT void JNICALL 
Java_com_epicgames_unreal_AudioVolumeReceiver_ringerModeChanged(JNIEnv* jni, jclass clazz, jboolean isMuted)
{
	bool Mute = isMuted;
	if (Mute)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Changed: RingerMode is Muted")));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Changed: RingerMode isn't Muted")));
	}
	
}
