// Fill out your copyright notice in the Description page of Project Settings.


#include "ResourceCheckFunction.h"

#include "RenderTargetPool.h"

#include "Engine/StaticMeshActor.h"

#include "EngineUtils.h"

#include "AutoResourceCheckActor.h"

AAutoResourceCheckActor* ResourceCheckFunction::Singleton = nullptr;

FAutoConsoleCommand ResourceCheckFunction::CCStartResourceCheck(
	TEXT("rc.StartResourceCheck"),
	TEXT("Start Check Resource"),
	FConsoleCommandDelegate::CreateStatic(&ResourceCheckFunction::StartResourceCheck)
);

FAutoConsoleCommand ResourceCheckFunction::CCEndResourceCheck(
	TEXT("rc.EndResourceCheck"),
	TEXT("End Check Resource"),
	FConsoleCommandDelegate::CreateStatic(&ResourceCheckFunction::EndResourceCheck)
);

ResourceCheckFunction::ResourceCheckFunction()
{

}

ResourceCheckFunction::~ResourceCheckFunction()
{

}


AAutoResourceCheckActor* ResourceCheckFunction::GetSingleton()
{
	AddAutoResourceCheckActorToWorld();

	return Singleton;
}

void ResourceCheckFunction::ResetAutoResourceCheckActorStatus()
{

	GetSingleton();

}


void ResourceCheckFunction::StartResourceCheck()
{
	ResetAutoResourceCheckActorStatus();
}

void ResourceCheckFunction::EndResourceCheck()
{
	RemoveAutoResourceCheckActorFromWorld();
}


void ResourceCheckFunction::ClearUnuseRenderTarget()
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Blue, "Clear Unuse RenderTarget");

		FOutputDevice* OutLog = GLog;

		UE_LOG(LogTemp, Log, TEXT("Before Clear"));

		GRenderTargetPool.DumpMemoryUsage(*OutLog);

		ENQUEUE_RENDER_COMMAND(ClearRenderTarget)(
			[](FRHICommandListImmediate& RHICmdList) {

				GRenderTargetPool.FreeUnusedResources();

			}
		);

	}
}


void ResourceCheckFunction::ClearStaticMeshActor()
{
	UE_LOG(LogTemp, Log, TEXT("Clear AStaticMeshActor"));
	for (TActorIterator<AStaticMeshActor> Iterator(GEngine->GetCurrentPlayWorld()); Iterator; ++Iterator)
	{
		AStaticMeshActor* CurrActor = *Iterator;
		UE_LOG(LogTemp, Log, TEXT("\t Clear Static Mesh Name : %s"), *CurrActor->GetName());
		CurrActor->Destroy();
	}
	GEngine->GetCurrentPlayWorld()->ForceGarbageCollection(true);

}

void ResourceCheckFunction::DumpAllResource()
{

	//UE_LOG(LogTemp, Log, TEXT("Show AStaticMeshActor"));
	//for (TActorIterator<AStaticMeshActor> Iterator(GEngine->GetCurrentPlayWorld()); Iterator; ++Iterator)
	//{
	//	AStaticMeshActor* CurrActor = *Iterator;
	//	UE_LOG(LogTemp, Log, TEXT("\t Static Mesh Actor Name : %s"), *CurrActor->GetName());
	//}
	//UE_LOG(LogTemp, Log, TEXT("Show UTexture2D"));
	//for (TObjectIterator<UTexture2D> Iterator; Iterator; ++Iterator)
	//{
	//	UTexture2D* CurrTexture = *Iterator;
	//	UE_LOG(LogTemp, Log, TEXT("\t Texture2D Name : %s"), *CurrTexture->GetName());
	//}
	//UE_LOG(LogTemp, Log, TEXT("Show UMaterial"));
	//for (TObjectIterator<UMaterial> Iterator; Iterator; ++Iterator)
	//{
	//	UMaterial* CurrMaterial = *Iterator;
	//	UE_LOG(LogTemp, Log, TEXT("\t Material Name : %s"), *CurrMaterial->GetName());
	//}
	//UE_LOG(LogTemp, Log, TEXT("Show StaticMesh"));
	//for (TObjectIterator<UStaticMesh> Iterator; Iterator; ++Iterator)
	//{
	//	UStaticMesh* CurrStaticMesh = *Iterator;
	//	UE_LOG(LogTemp, Log, TEXT("\t Static Mesh Name : %s"), *CurrStaticMesh->GetName());
	//}

	for (TActorIterator<AActor> Iterator(GEngine->GetCurrentPlayWorld()); Iterator; ++Iterator)
	{
		AActor* CurrActor = *Iterator;
		UE_LOG(LogTemp, Log, TEXT("\t Static Mesh Actor Name : %s Class : %s"), *CurrActor->GetName(),*CurrActor->GetClass()->GetName());
	}

}


void ResourceCheckFunction::ForceGC()
{
	UE_LOG(LogTemp, Log, TEXT("Force GC..."));
	GEngine->GetCurrentPlayWorld()->ForceGarbageCollection(true);
}





void ResourceCheckFunction::AddAutoResourceCheckActorToWorld()
{
	if (Singleton == nullptr)
	{
		Singleton = GEngine->GetCurrentPlayWorld()->SpawnActor<AAutoResourceCheckActor>();

		//Singleton->AddToRoot();
	}
}

void ResourceCheckFunction::RemoveAutoResourceCheckActorFromWorld()
{
	if (Singleton)
	{

		Singleton->EndAutoResourceCheck();

		Singleton->SaveConfig();

		Singleton->Destroy();

	}
}
