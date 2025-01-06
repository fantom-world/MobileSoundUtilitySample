// Fill out your copyright notice in the Description page of Project Settings.

#include "MobileSoundUtility.h"
#include "OptionalMobileFeaturesBPLibrary.h"

#if PLATFORM_IOS
#include "IOS/IOSAppDelegate.h"
#include <AVFoundation/AVAudioSession.h>
#endif

#if PLATFORM_ANDROID
#include "Android/AndroidApplication.h"
#include "Android/AndroidJNI.h"
#endif

void AMobileSoundUtility::BeginPlay()
{
	Super::BeginPlay();
	//システム音量変更時のイベントをBind
#if PLATFORM_IOS 
	OnAudioMuteIosDelegateHandle = FCoreDelegates::AudioMuteDelegate.AddStatic(&AMobileSoundUtility::OnAudioStateChangedIos);
#elif PLATFORM_ANDROID
	//AppがForeground/Backgroundに入ったときのイベントにAudioReceiverを有効/無効にする関数をBind
	OnDisableAudioReceiverAndroidHandle = FCoreDelegates::ApplicationWillEnterBackgroundDelegate.AddLambda([this]() {
		OnDisableAudioReceiverAndroid();
		});
	OnEnableAudioReceiverAndroidHandle = FCoreDelegates::ApplicationHasEnteredForegroundDelegate.AddLambda([this]()
		{
			OnEnableAudioReceiverAndroid();
		});
#endif
}

void AMobileSoundUtility::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
		Super::EndPlay(EndPlayReason);
	//システム音量変更時のイベントをBind解除
#if PLATFORM_IOS 
	if (OnAudioMuteIosDelegateHandle.IsValid())
	{
		FCoreDelegates::AudioMuteDelegate.Remove(OnAudioMuteIosDelegateHandle);
		OnAudioMuteIosDelegateHandle.Reset();
	}
#elif PLATFORM_ANDROID
	if (OnDisableAudioReceiverAndroidHandle.IsValid())
	{
		FCoreDelegates::ApplicationWillEnterBackgroundDelegate.Remove(OnDisableAudioReceiverAndroidHandle);
		OnDisableAudioReceiverAndroidHandle.Reset();
	}
	if (OnEnableAudioReceiverAndroidHandle.IsValid())
	{
		FCoreDelegates::ApplicationHasEnteredForegroundDelegate.Remove(OnEnableAudioReceiverAndroidHandle);
		OnEnableAudioReceiverAndroidHandle.Reset();
	}
#endif
}

int AMobileSoundUtility::GetMobileVolume()
{
    #if PLATFORM_ANDROID || PLATFORM_IOS 
	return UOptionalMobileFeaturesBPLibrary::GetVolumeState();
	#endif
	return 0;
}

void AMobileSoundUtility::PrintMobileVolume()
{
	//System音量を表示
#if PLATFORM_ANDROID || PLATFORM_IOS 
	int volume = GetMobileVolume();
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("SystemSoundVolume is %i %%"), volume));

	//Mute状態を加味した最終的な音量を表示
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Orange, FString::Printf(TEXT("FinalSystemSoundVolume is %i %%: isMuted=%i, isExternalAudioDeviceConnected=%i"), GetFinalMobileVolume(),GetIsMuted(),GetIsExternalAudioDevicesConnected()));
#endif
}

int AMobileSoundUtility::GetFinalMobileVolume()
{
	//Mute状態かつ外部出力デバイスが接続されていないときは音はでないので0を返す
	if (GetIsMuted() && !GetIsExternalAudioDevicesConnected())
	{
		return 0;		
	}
	return GetMobileVolume();
}

bool AMobileSoundUtility::GetIsMuted()
{
	#if PLATFORM_IOS
	return GetIsIOSMuted();
	#elif PLATFORM_ANDROID
	return GetIsAndroidMuted();
	#endif

    return false;
}

bool AMobileSoundUtility::GetIsExternalAudioDevicesConnected()
{
	#if PLATFORM_IOS
	return GetIsIOSExternalAudioDevicesConnected();
	#elif PLATFORM_ANDROID
	return GetIsAndroidExternalAudioDevicesConnected();
	#endif
    return false;
}

#if PLATFORM_IOS
bool AMobileSoundUtility::IsIosLastMuted = false;

bool AMobileSoundUtility::GetIsIOSMuted()
{
    return IsIosLastMuted;
}

void AMobileSoundUtility::OnAudioStateChangedIos(bool IsMute, int Volume)
{
	if (IsMute)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Changed: SystemSound is Muted(%i%%)"), Volume));
	}
	else
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Changed: SystemSound is On(%i%%)"), Volume));
    }
    IsIosLastMuted = IsMute;
}

bool AMobileSoundUtility::GetIsIOSExternalAudioDevicesConnected()
{
	bool res = true;
	if (AVAudioSessionRouteDescription* CurrentRoute = [[AVAudioSession sharedInstance]currentRoute] )
	{
		for (AVAudioSessionPortDescription* Port in[CurrentRoute outputs])
		{
			if ([[Port portType]isEqualToString:AVAudioSessionPortBuiltInReceiver]
				|| [[Port portType]isEqualToString:AVAudioSessionPortBuiltInSpeaker] )
			{
				res = false;
			}
		}
	}
	return res;
}

#elif PLATFORM_ANDROID

bool AMobileSoundUtility::GetIsAndroidMuted()
{
    JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	if (nullptr != Env)
	{
		jmethodID GetIsMutedMethod = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_GetIsSilentMode", "()Z", false);
		return FJavaWrapper::CallBooleanMethod(Env, FJavaWrapper::GameActivityThis, GetIsMutedMethod);
	}
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("Failed GetIsMuted")));
	return false;
}

bool AMobileSoundUtility::GetIsAndroidExternalAudioDevicesConnected()
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	if (nullptr != Env)
	{
		jmethodID GetIsExternalAudioDevicesConnectedMethod = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_GetIsExternalAudioDevicesConnected", "()Z", false);
		return FJavaWrapper::CallBooleanMethod(Env, FJavaWrapper::GameActivityThis, GetIsExternalAudioDevicesConnectedMethod);
	}
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("Failed GetIsExternalAudioDevicesConnected")));
    return false;
}

void AMobileSoundUtility::OnEnableAudioReceiverAndroid()
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	if (nullptr != Env)
	{
		jmethodID EnableVolumeReceiverMethod = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_EnableAudioVolumeReceiver", "(Z)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, EnableVolumeReceiverMethod, true);
	}
}

void AMobileSoundUtility::OnDisableAudioReceiverAndroid()
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	if (nullptr != Env)
	{
		jmethodID EnableVolumeReceiverMethod = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_EnableAudioVolumeReceiver", "(Z)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, EnableVolumeReceiverMethod, false);
	}
}

#endif