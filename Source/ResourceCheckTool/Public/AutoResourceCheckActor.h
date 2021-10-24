// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MemoryMonitor.h"
#include "AutoResourceCheckActor.generated.h"


struct FResourceInfo {
	//both before start collection and after collection total memory not steady will set IsSteady false
	bool IsSteady;

	FString ResourceName;

	FString ResourcePath;

	FString ResourceType;

	MemoryCapture Capture[2];

};

UCLASS(Config=Game)
class RESOURCECHECKTOOL_API AAutoResourceCheckActor : public AActor
{
	GENERATED_BODY()
	
public:

	enum class ECheckStatus : uint8 {
		WAIT_START_CHECK = 0x00,
		WAIT_REMOVE,
		WAIT_GC,
		WAIT_COLLECT_INFO,
		END_CHECK,
		DISABLE,
	};

	// Sets default values for this actor's properties
	AAutoResourceCheckActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void EndAutoResourceCheck();

private:

	void Init();

	void CheckTick();

	void RestCurrResourceInfo() { 
	

		CurrResourceInfo.IsSteady = false;

		CurrResourceInfo.ResourceName = FString("Unknow");

		CurrResourceInfo.ResourcePath = FString("Unknow");

		CurrResourceInfo.ResourceType = FString("Unknow");


	}

	void ClearNameCache();
	//TODO:
	//void ResetMemMonitor() {};

	bool IsMemSteady() { return Monitor.IsMemSteady(); };

	void StartTick();

	void CaptureBeforeDeleteMemoryStatus(FString& ActorName,FString& ClassName) { 
		CurrResourceInfo.ResourceName = ActorName;
		CurrResourceInfo.ResourceType = ClassName;
		CurrResourceInfo.Capture[0] = Monitor.GetCurrMemoryCapture(); }

	void RemoveTick();

	void GCTick();

	void CaptureAfterDeleteMemoryStatus() { CurrResourceInfo.Capture[1] = Monitor.GetCurrMemoryCapture(); }

	void CollectInfoTick();

	void DumpCollectedMemoryInfo();

	void EndTick();

	bool FindInNameWhiteList(FString Name) { return INDEX_NONE == WhiteNameList.Find(Name)?false:true; }

	bool FindInClassWhiteList(FString Name) { return INDEX_NONE ==WhiteClassList.Find(Name)?false:true; }

	bool IsInit = false;

	MemoryMonitor Monitor;

	ECheckStatus CurrCheckStatus;

	FResourceInfo CurrResourceInfo;

	int32 WaitTickCount = 0;

	UPROPERTY(Config)
	int32 MaxWaitTick = 600;

	UPROPERTY(Config)
	int32 MaxUnchangedTime = 100;

	UPROPERTY(Config)
	int32 MaxDupDeleteTime = 10;

	UPROPERTY(Config)
	int32 SafeTickNum = 200;

	UPROPERTY(Config)
	TArray<FString> WhiteNameList;

	UPROPERTY(Config)
	TArray<FString> WhiteClassList;
	//key count value lru
	TMap<FString, TPair<int,int>> NameList;

	FString DumpFilePath;

	FArchive* OutputStream = nullptr;

	static FAutoConsoleCommand CCRenderTargetClear;

	static FAutoConsoleCommand CCClearStaticMesh;

	static FAutoConsoleCommand CCDumpAllResource;

	static FAutoConsoleCommand CCForceGC;

};
