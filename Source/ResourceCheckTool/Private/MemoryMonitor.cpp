// Fill out your copyright notice in the Description page of Project Settings.


#include "MemoryMonitor.h"
#include "HAL/LowLevelMemTracker.h"


MemoryMonitor::MemoryMonitor()
{

}

MemoryMonitor::~MemoryMonitor()
{
}

bool MemoryMonitor::IsEnableLLM()
{
	return FLowLevelMemTracker::IsEnabled();
}

void MemoryMonitor::SampleCurrMemoryStatus()
{

}

bool MemoryMonitor::IsMemSteady()
{
	return true;
}

MemoryCapture MemoryMonitor::GetCurrMemoryCapture()
{
	MemoryCapture Capture;

	int TotalMemory = FLowLevelMemTracker::Get().GetTagAmountForTracker(ELLMTracker::Platform,ELLMTag::PlatformTotal);

	float Scale = 1.0f / (1024 * 1024);

	Capture.TotalMemory = TotalMemory * Scale;

	return Capture;
}
