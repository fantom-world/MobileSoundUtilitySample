// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#if PLATFORM_ANDROID
#include "Android/AndroidApplication.h"
#include "Android/AndroidJNI.h"
#endif

#include "MobileSoundUtility.generated.h"

//System���ʂ��擾����Utility�N���X
UCLASS(Blueprintable, BlueprintType)
class MOBILESOUNDTEST_API AMobileSoundUtility : public AActor
{
	GENERATED_BODY()

protected:
	// �J�n���̃C�x���g
	virtual void BeginPlay() override;

	// �I�����̃C�x���g
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:
	//Mobile�f�o�C�X�̉��ʂ��擾���� 0-100 (%)
	//Mute�̂Ƃ��A0�ɂȂ�Ȃ��iAndroid�j
	UFUNCTION(BlueprintCallable)
	int GetMobileVolume();

	//Mobile�f�o�C�X�̉��ʂ�\������
	UFUNCTION(BlueprintCallable)
	void PrintMobileVolume();

	//IOS�f�o�C�X�ɂă~���[�g�X�C�b�`���ύX���ꂽ���Ƃ����o�����Ƃ��A�܂��͉��ʂ��ύX���ꂽ�Ƃ��ɌĂяo�����
	static void OnAudioStateChangedIos(bool IsMute, int Volume);

	//���ʕύX���󂯎��Receiver��o�^/�����iAndroid�j
	void OnEnableAudioReceiverAndroid();
	void OnDisableAudioReceiverAndroid();

	//Android�p�̉��ʕύXReceiver�o�^�C�x���g��Foreground/Background�ɓ������Ƃ���Delegate��Bind
	void BindAudioStateChangedAndroid();

private:
	FDelegateHandle OnAudioMuteIosDelegateHandle;
	FDelegateHandle OnEnableAudioReceiverAndroidHandle;
	FDelegateHandle OnDisableAudioReceiverAndroidHandle;
};