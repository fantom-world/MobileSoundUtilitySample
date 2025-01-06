// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MobileSoundUtility.generated.h"

//System音量を取得するUtilityクラス
UCLASS(Blueprintable, BlueprintType)
class MOBILESOUNDTEST_API AMobileSoundUtility : public AActor
{
	GENERATED_BODY()

protected:
	// 開始時のイベント
	virtual void BeginPlay() override;
	// 終了時のイベント
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:
	//Mobileデバイスの音量を取得する 0-100 (%)
	//Muteのとき、0にならない
	UFUNCTION(BlueprintCallable)
	int GetMobileVolume();

	//Mobileデバイスの音量を表示する
	UFUNCTION(BlueprintCallable)
	void PrintMobileVolume();

	//Mobileデバイスの音量を取得する 0-100 (%)
	//Muteのときは0
	UFUNCTION(BlueprintCallable)
	int GetFinalMobileVolume();

	//MobileデバイスのMute状態を取得
	UFUNCTION(BlueprintCallable)
	bool GetIsMuted();

	//Mobileデバイスの外部出力デバイス接続状態を取得
	UFUNCTION(BlueprintCallable)
	bool GetIsExternalAudioDevicesConnected();

#if PLATFORM_IOS
public:
	//iOSデバイスのMute状態を取得
	bool GetIsIOSMuted();

	//IOSデバイスにてミュートスイッチが変更されたことを検出したとき、または音量が変更されたときに呼び出される
	static void OnAudioStateChangedIos(bool IsMute, int Volume);

	//IOSデバイスがMuteかどうか（音量変更時に更新される）
	//MuteSwitchが付いていない機種は必ずfalseになる
	static bool IsIosLastMuted;

	//iOSデバイスの外部出力デバイス接続状態を取得
	bool GetIsIOSExternalAudioDevicesConnected();
private:
	//DelegateHandles
	FDelegateHandle OnAudioMuteIosDelegateHandle;
#elif PLATFORM_ANDROID
public:
	//AndroidデバイスのMute状態を取得
	bool GetIsAndroidMuted();

	//Androidデバイスの外部出力デバイス接続状態を取得
	bool GetIsAndroidExternalAudioDevicesConnected();

	//音量変更を受け取るReceiverを登録/解除（Android）
	void OnEnableAudioReceiverAndroid();
	void OnDisableAudioReceiverAndroid();
private:
	//DelegateHandles
	FDelegateHandle OnEnableAudioReceiverAndroidHandle;
	FDelegateHandle OnDisableAudioReceiverAndroidHandle;
#endif

};
