#include "com_epicgames_unreal_MobileSoundTestNativeAccess.h"

JNIEXPORT void JNICALL
Java_com_epicgames_unreal_AudioVolumeReceiver_volumeChanged(JNIEnv* jni, jclass clazz, jint volume)
{
	int scaledVolume = (volume * 100) / 15;;
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Changed: SystemSound is On(%i%%)"), scaledVolume));
}
