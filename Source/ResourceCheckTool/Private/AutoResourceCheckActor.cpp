// Fill out your copyright notice in the Description page of Project Settings.


#include "AutoResourceCheckActor.h"
#include "ResourceCheckFunction.h"
#include "Tools.h"
#include "EngineUtils.h"

FAutoConsoleCommand AAutoResourceCheckActor::CCRenderTargetClear(
	TEXT("d.clearrendertarget"),
	TEXT("Clear Unuse RenderTarget"),
	FConsoleCommandDelegate::CreateStatic(&ResourceCheckFunction::ClearUnuseRenderTarget)
);

FAutoConsoleCommand AAutoResourceCheckActor::CCClearStaticMesh(
	TEXT("d.clearstaticmesh"),
	TEXT("Clear All StaticMesh Resource"),
	FConsoleCommandDelegate::CreateStatic(&ResourceCheckFunction::ClearStaticMeshActor)
);


FAutoConsoleCommand AAutoResourceCheckActor::CCDumpAllResource(
	TEXT("d.dumpallresource"),
	TEXT("Dump All Type Resource"),
	FConsoleCommandDelegate::CreateStatic(&ResourceCheckFunction::DumpAllResource)
);

FAutoConsoleCommand AAutoResourceCheckActor::CCForceGC(
	TEXT("d.GC"),
	TEXT("Force GC"),
	FConsoleCommandDelegate::CreateStatic(&ResourceCheckFunction::ForceGC)
);

// Sets default values
AAutoResourceCheckActor::AAutoResourceCheckActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CurrCheckStatus = ECheckStatus::WAIT_START_CHECK;

	/*MaxWaitTick = 60;

	WaitTickCount = 0;*/

	if (!Monitor.IsEnableLLM())
	{
		CurrCheckStatus = ECheckStatus::DISABLE;
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Please Enable LLM For Memory Track"));
		}
		UE_LOG(LogResourceCheckTool, Log, TEXT("Disable LLM = Disable AutoResourceCheck"));

	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, TEXT("Create Auto Resource Check Actor..."));
		}

		UE_LOG(LogResourceCheckTool, Log, TEXT("Create Auto Resource Check Actor"));
	}

	this->LoadConfig(AAutoResourceCheckActor::StaticClass());


}

// Called when the game starts or when spawned
void AAutoResourceCheckActor::BeginPlay() 
{
	Super::BeginPlay();
}

// Called every frame
void AAutoResourceCheckActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Monitor.SampleCurrMemoryStatus();

	CheckTick();
}

void AAutoResourceCheckActor::EndAutoResourceCheck()
{

	OutputStream->Close();

}

void AAutoResourceCheckActor::Init()
{

	FString DumpDirPath = FPaths::ProjectSavedDir() + "/ResourceCheckTool";

	if (!IFileManager::Get().DirectoryExists(*DumpDirPath))
	{
		IFileManager::Get().MakeDirectory(*DumpDirPath);
	}

	FString Label = FDateTime::Now().ToString();

	DumpFilePath = DumpDirPath + "/" + Label + ".txt";

	OutputStream = IFileManager::Get().CreateFileWriter(*DumpFilePath);

	IsInit = true;

	FString ParameterInfo = FString::Printf(
		TEXT("MaxWaitTick : %d \n MaxUnchangedTime : %d \n MaxDupDeleteTime : %d \n SafeTickNum : %d \n"),
		MaxWaitTick,
		MaxUnchangedTime,
		MaxDupDeleteTime,
		SafeTickNum
	);

	OutputStream->Serialize((void*)(*ParameterInfo),sizeof(TCHAR)*ParameterInfo.Len());

}

void AAutoResourceCheckActor::CheckTick()
{
	ClearNameCache();

	switch (CurrCheckStatus)
	{
	case ECheckStatus::WAIT_START_CHECK:
		StartTick();
		break;
	case ECheckStatus::WAIT_REMOVE:
		RemoveTick();
		break;
	case ECheckStatus::WAIT_GC:
		GCTick();
		break;
	case ECheckStatus::WAIT_COLLECT_INFO:
		CollectInfoTick();
		break;
	case ECheckStatus::END_CHECK:
		EndTick();
		break;
	default:
		break;
	}
}


void AAutoResourceCheckActor::ClearNameCache()
{
	//ensure cache not too big 
	for (auto Element = NameList.CreateIterator();Element;++Element)
	{
		Element->Value.Value = Element->Value.Value + 1;
		if (Element->Value.Value > MaxUnchangedTime)
		{
			Element.RemoveCurrent();
		}
	}
}

void AAutoResourceCheckActor::StartTick()
{
	if (!IsInit)
	{
		Init();
	}
	
	RestCurrResourceInfo();

	WaitTickCount = 0;

	CurrCheckStatus = ECheckStatus::WAIT_REMOVE;

}

void AAutoResourceCheckActor::RemoveTick()
{
	WaitTickCount++;

	if (WaitTickCount < SafeTickNum)
	{
		return;
	}

	if (IsMemSteady()|| WaitTickCount > MaxWaitTick)
	{

		bool IsSteady = true;

		if (WaitTickCount > MaxWaitTick)
		{
			IsSteady = false;
		}

		bool HasActor = false;

		for (TActorIterator<AActor> Iterator(GEngine->GetCurrentPlayWorld()); Iterator; ++Iterator)
		{

			AActor* CurrActor = *Iterator;

			FString ActorName;

			FString ClassName;

			CurrActor->GetName(ActorName);

			CurrActor->GetClass()->GetName(ClassName);

			if (FindInNameWhiteList(ActorName) || FindInClassWhiteList(ClassName))
			{
				continue;
			}
			else
			{
				if (NameList.Contains(ActorName))
				{
					NameList[ActorName].Key = NameList[ActorName].Key + 1;

					NameList[ActorName].Value = 0;

					if (NameList[ActorName].Key > MaxDupDeleteTime)
					{
						WhiteNameList.Add(ActorName);
					}
				}
				else
				{
					NameList.Add(TPair<FString,TPair<int,int>>(ActorName,TPair<int,int>(1,0)));
				}

				HasActor = true;

				UE_LOG(LogResourceCheckTool, Log, TEXT("Cpature Current Memory Status"));

				CaptureBeforeDeleteMemoryStatus(ActorName,ClassName);

				UE_LOG(LogResourceCheckTool, Log, TEXT("Remove Actor : %s Class : %s"), *ActorName, *ClassName);

				CurrActor->Destroy();

				CurrCheckStatus = ECheckStatus::WAIT_GC;

				break;
			}

		}
		if (!HasActor)
		{
			UE_LOG(LogResourceCheckTool, Log, TEXT("No Actor To Delete"));
			CurrCheckStatus =  ECheckStatus::END_CHECK;
		}
	}

}

void AAutoResourceCheckActor::GCTick()
{
	GEngine->GetCurrentPlayWorld()->ForceGarbageCollection(true);

	WaitTickCount = 0;

	CurrCheckStatus = ECheckStatus::WAIT_COLLECT_INFO;
}

void AAutoResourceCheckActor::CollectInfoTick()
{
	WaitTickCount++;

	if (WaitTickCount < SafeTickNum)
	{
		return;
	}

	if (IsMemSteady() || WaitTickCount > MaxWaitTick)
	{
		bool IsSteady = true;

		if (WaitTickCount > MaxWaitTick)
		{
			IsSteady = false;
		}

		UE_LOG(LogResourceCheckTool, Log, TEXT("Collect Mem Info"));

		CaptureAfterDeleteMemoryStatus();

		DumpCollectedMemoryInfo();

		CurrCheckStatus = ECheckStatus::WAIT_START_CHECK;
	}
}


void AAutoResourceCheckActor::DumpCollectedMemoryInfo()
{
	FString MemoryInfo = FString::Printf(TEXT("%s --- Type : %s --- Memory : %f --- Before : %f --- After : %f \n"),
		*CurrResourceInfo.ResourceName,
		*CurrResourceInfo.ResourceType,
		CurrResourceInfo.Capture[0].TotalMemory - CurrResourceInfo.Capture[1].TotalMemory,
		CurrResourceInfo.Capture[0].TotalMemory,
		CurrResourceInfo.Capture[1].TotalMemory);
	GEngine->AddOnScreenDebugMessage(-1,3.0f,FColor::Blue,MemoryInfo);

	OutputStream->Serialize((void*)(*MemoryInfo),sizeof(TCHAR)*MemoryInfo.Len());
	
}

void AAutoResourceCheckActor::EndTick()
{
	for(TActorIterator<AActor> Iterator(GEngine->GetCurrentPlayWorld());Iterator;++Iterator)
	{
		AActor* CurrActor = *Iterator;

		FString ActorName;

		FString ClassName;

		CurrActor->GetName(ActorName);

		CurrActor->GetClass()->GetName(ClassName);

		if (FindInNameWhiteList(ActorName) || FindInClassWhiteList(ClassName))
		{
			continue;
		}
		else
		{
			UE_LOG(LogResourceCheckTool, Log, TEXT("Find New Actor Spawn"));
			CurrCheckStatus = ECheckStatus::WAIT_START_CHECK;
			break;
		}
	}
	
}