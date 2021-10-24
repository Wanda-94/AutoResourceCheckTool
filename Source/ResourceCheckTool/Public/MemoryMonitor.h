// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */

struct MemoryCapture {

	float TotalMemory;

};


class RESOURCECHECKTOOL_API MemoryMonitor
{
public:

	MemoryMonitor();

	~MemoryMonitor();

	bool IsEnableLLM();

	void SampleCurrMemoryStatus();

	bool IsMemSteady();

	MemoryCapture GetCurrMemoryCapture();

private:

	FLowLevelMemTracker* LLMInstance = nullptr;

};
