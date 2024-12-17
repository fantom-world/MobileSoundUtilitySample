// Fill out your copyright notice in the Description page of Project Settings.


#include "MobileSoundUtility.h"
#include "OptionalMobileFeaturesBPLibrary.h"

#if PLATFORM_ANDROID 
#include "Private/OS/Android/com_epicgames_unreal_MobileSoundTestNativeAccess.h"
#endif


int AMobileSoundUtility::GetMobileVolume()
{
#if PLATFORM_ANDROID || PLATFORM_IOS 
	return UOptionalMobileFeaturesBPLibrary::GetVolumeState();
#endif
	return 0;
}


void AMobileSoundUtility::PrintMobileVolume()
{
#if PLATFORM_ANDROID || PLATFORM_IOS 
	int volume = GetMobileVolume();
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("SystemSoundVolume is %i %%"), volume));
#endif
}

void AMobileSoundUtility::OnAudioStateChangedIos(bool IsMute, int Volume)
{
#if PLATFORM_IOS 
	if (IsMute)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Changed: SystemSound is Muted(%i%%)"), Volume));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Changed: SystemSound is On(%i%%)"), Volume));
	}
#endif
}

void AMobileSoundUtility::BindAudioStateChangedAndroid()
{
	//AppがForeground/Backgroundに入ったときのイベントにAudioReceiverを有効/無効にする関数をBind
	OnDisableAudioReceiverAndroidHandle = FCoreDelegates::ApplicationWillEnterBackgroundDelegate.AddLambda([this]() {
		OnDisableAudioReceiverAndroid();
		});
	OnEnableAudioReceiverAndroidHandle = FCoreDelegates::ApplicationHasEnteredForegroundDelegate.AddLambda([this]() 
		{
		OnEnableAudioReceiverAndroid();
		});
}

void AMobileSoundUtility::OnDisableAudioReceiverAndroid()
{
#if PLATFORM_ANDROID	
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	if (nullptr != Env)
	{
		jmethodID EnableVolumeReceiverMethod = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_EnableAudioVolumeReceiver", "(Z)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, EnableVolumeReceiverMethod, false);
	}
#endif
}

void AMobileSoundUtility::OnEnableAudioReceiverAndroid()
{
#if PLATFORM_ANDROID	
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	if (nullptr != Env)
	{
		jmethodID EnableVolumeReceiverMethod = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_EnableAudioVolumeReceiver", "(Z)V", false);
		FJavaWrapper::CallVoidMethod(Env, FJavaWrapper::GameActivityThis, EnableVolumeReceiverMethod, true);
	}
#endif
}



void AMobileSoundUtility::BeginPlay()
{
	Super::BeginPlay();
	//システム音量変更時のイベントをBind
#if PLATFORM_ANDROID
	BindAudioStateChangedAndroid();
#elif PLATFORM_IOS 
	OnAudioMuteIosDelegateHandle = FCoreDelegates::AudioMuteDelegate.AddStatic(&AMobileSoundUtility::OnAudioStateChangedIos);
#endif

}

void AMobileSoundUtility::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	//システム音量変更時のイベントをBind解除
#if PLATFORM_ANDROID
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
#elif PLATFORM_IOS 
	if (OnAudioMuteIosDelegateHandle.IsValid())
	{
		FCoreDelegates::AudioMuteDelegate.Remove(OnAudioMuteIosDelegateHandle);
		OnAudioMuteIosDelegateHandle.Reset();
	}

#endif
}
