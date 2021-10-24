// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


class AAutoResourceCheckActor;
/**
 * 
 */
class RESOURCECHECKTOOL_API ResourceCheckFunction
{
public:
	ResourceCheckFunction();

	~ResourceCheckFunction();


	static AAutoResourceCheckActor* GetSingleton();

	static void ResetAutoResourceCheckActorStatus();

	static void StartResourceCheck();

	static void EndResourceCheck();

	static void ClearUnuseRenderTarget();

	static void ClearStaticMeshActor();

	static void DumpAllResource();

	static void ForceGC();

private:

	static void AddAutoResourceCheckActorToWorld();

	static void RemoveAutoResourceCheckActorFromWorld();

	static AAutoResourceCheckActor* Singleton;

	static FAutoConsoleCommand CCStartResourceCheck;

	static FAutoConsoleCommand CCEndResourceCheck;

};
