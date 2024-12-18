// Fill out your copyright notice in the Description page of Project Settings.


#include "MobileSoundUtility.h"
#include "OptionalMobileFeaturesBPLibrary.h"

#if PLATFORM_ANDROID 
#include "Private/OS/Android/com_epicgames_unreal_MobileSoundTestNativeAccess.h"
#endif

#if PLATFORM_IOS
#include "IOS/IOSAppDelegate.h"
#include "IOS/IOSAsyncTask.h"
#import <AVFoundation/AVFoundation.h>
#include "IOS/IOSAppDelegate.h"

#endif


int AMobileSoundUtility::GetMobileVolume()
{
#if PLATFORM_ANDROID || PLATFORM_IOS 
	return UOptionalMobileFeaturesBPLibrary::GetVolumeState();
#endif
	return 0;
}

int AMobileSoundUtility::GetFinalMobileVolume()
{
	//Mute��Ԃ��O���o�̓f�o�C�X���ڑ�����Ă��Ȃ��Ƃ��͉��͂łȂ��̂�0��Ԃ�
	if (GetIsMuted() && !GetAreHeadphonesPluggedIn())
	{
		return 0;		
	}
	return GetMobileVolume();
}


void AMobileSoundUtility::PrintMobileVolume()
{
	//System���ʂ�\��
#if PLATFORM_ANDROID || PLATFORM_IOS 
	int volume = GetMobileVolume();
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("SystemSoundVolume is %i %%"), volume));
	
#endif
	//Mute��Ԃ���������System���ʂ�\��
#if PLATFORM_ANDROID || PLATFORM_IOS 
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("FinalSystemSoundVolume is %i %%"), GetFinalMobileVolume()));
#endif
#if PLATFORM_ANDROID || PLATFORM_IOS 
	GetIsMuted() ? GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Mute"))) : GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Not Mute")));
	GetAreHeadphonesPluggedIn() ? GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("HeadPhone"))) : GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("InDevice")));
#endif
}

bool AMobileSoundUtility::GetIsMuted()
{
#if PLATFORM_ANDROID	
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	if (nullptr != Env)
	{
		jmethodID GetIsMutedMethod = FJavaWrapper::FindMethod(Env, FJavaWrapper::GameActivityClassID, "AndroidThunkJava_GetIsSilentMode", "()Z", false);
		return FJavaWrapper::CallBooleanMethod(Env, FJavaWrapper::GameActivityThis, GetIsMutedMethod);
	}
#elif PLATFORM_IOS && USE_MUTE_SWITCH_DETECTION
	return [IOSAppDelegate GetDelegate].bLastMutedState;
#endif
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("Failed GetIsMuted")));
	return false;
	
}

bool AMobileSoundUtility::GetAreHeadphonesPluggedIn()
{
#if PLATFORM_ANDROID || PLATFORM_IOS 
	return UOptionalMobileFeaturesBPLibrary::AreHeadphonesPluggedIn();
#endif
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("Failed GetIsExternalAudioDevicesConnected")));
	return false;
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
	//App��Foreground/Background�ɓ������Ƃ��̃C�x���g��AudioReceiver��L��/�����ɂ���֐���Bind
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
	//�V�X�e�����ʕύX���̃C�x���g��Bind
#if PLATFORM_ANDROID
	BindAudioStateChangedAndroid();
#elif PLATFORM_IOS 
	OnAudioMuteIosDelegateHandle = FCoreDelegates::AudioMuteDelegate.AddStatic(&AMobileSoundUtility::OnAudioStateChangedIos);
#endif

}

void AMobileSoundUtility::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	//�V�X�e�����ʕύX���̃C�x���g��Bind����
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
