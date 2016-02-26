// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#include "SynthBenchmarkPrivatePCH.h"
#include "RenderCore.h"
#include "RHI.h"
#include "RendererInterface.h"

float RayIntersectBenchmark();
float FractalBenchmark();


DEFINE_LOG_CATEGORY_STATIC(LogSynthBenchmark, Log, All);

// to prevent compiler optimizations
static float GGlobalStateObject = 0.0f;

class FSynthBenchmark : public ISynthBenchmark
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** ISynthBenchmark implementation */
	virtual void Run(FSynthBenchmarkResults& InOut, bool bGPUBenchmark, float WorkScale) const override;
};

IMPLEMENT_MODULE( FSynthBenchmark, SynthBenchmark )

void FSynthBenchmark::StartupModule()
{
	// This code will execute after your module is loaded into memory (but after global variables are initialized, of course.)
}

void FSynthBenchmark::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

// @param RunCount should be around 10 but can be adjusted for precision 
// @param Function should run for about 3 ms
static FTimeSample RunBenchmark(float WorkScale, float (*Function)())
{
	float Sum = 0;

	// this test doesn't support fractional WorkScale
	uint32 RunCount = FMath::Max((int32)1, (int32)WorkScale);

	for(uint32 i = 0; i < RunCount; ++i)
	{
		FPlatformMisc::MemoryBarrier();
		// todo: compiler reorder might be an issue, use _ReadWriteBarrier or something like http://svn.openimageio.org/oiio/tags/Release-0.6.3/src/include/tbb/machine/windows_em64t.h
		const double StartTime = FPlatformTime::Seconds();

		FPlatformMisc::MemoryBarrier();

		GGlobalStateObject += Function();

		FPlatformMisc::MemoryBarrier();
		Sum += (float)(FPlatformTime::Seconds() - StartTime);
		FPlatformMisc::MemoryBarrier();
	}
	
	return FTimeSample(Sum, Sum / RunCount);
}

void FSynthBenchmark::Run(FSynthBenchmarkResults& InOut, bool bGPUBenchmark, float WorkScale) const
{
	check(WorkScale > 0);

	if(!bGPUBenchmark)
	{
		// run a very quick GPU benchmark (less confidence but at least we get some numbers)
		// it costs little time and we get some stats
		WorkScale = 1.0f;
	}

	const double StartTime = FPlatformTime::Seconds();

	UE_LOG(LogSynthBenchmark, Display, TEXT("FSynthBenchmark (V0.95):  requested WorkScale=%.2f"), WorkScale);
	UE_LOG(LogSynthBenchmark, Display, TEXT("==============="));
	
#if UE_BUILD_DEBUG
	UE_LOG(LogSynthBenchmark, Display, TEXT("         Note: Values are not trustable because this is a DEBUG build!"));
#endif
	
	UE_LOG(LogSynthBenchmark, Display, TEXT("Main Processor:"));

	// developer machine: Intel Xeon E5-2660 2.2GHz
	// divided by the actual value on a developer machine to normalize the results
	// Index should be around 100 +-4 on developer machine in a development build (should be the same in shipping)

	InOut.CPUStats[0] = FSynthBenchmarkStat(TEXT("RayIntersect"), 0.02561f, TEXT("s/Run"), 1.f);
	InOut.CPUStats[0].SetMeasuredTime(RunBenchmark(WorkScale, RayIntersectBenchmark));

	InOut.CPUStats[1] = FSynthBenchmarkStat(TEXT("Fractal"), 0.0286f, TEXT("s/Run"), 1.5f);
	InOut.CPUStats[1].SetMeasuredTime(RunBenchmark(WorkScale, FractalBenchmark));

	for(uint32 i = 0; i < ARRAY_COUNT(InOut.CPUStats); ++i)
	{
		UE_LOG(LogSynthBenchmark, Display, TEXT("         ... %f %s '%s'"), InOut.CPUStats[i].GetNormalizedTime(), InOut.CPUStats[i].GetValueType(), InOut.CPUStats[i].GetDesc());
	}

	UE_LOG(LogSynthBenchmark, Display, TEXT(""));

	bool bAppIs64Bit = (sizeof(void*) == 8);

	UE_LOG(LogSynthBenchmark, Display, TEXT("  CompiledTarget_x_Bits: %s"), bAppIs64Bit ? TEXT("64") : TEXT("32"));
	UE_LOG(LogSynthBenchmark, Display, TEXT("  UE_BUILD_SHIPPING: %d"), UE_BUILD_SHIPPING);
	UE_LOG(LogSynthBenchmark, Display, TEXT("  UE_BUILD_TEST: %d"), UE_BUILD_TEST);
	UE_LOG(LogSynthBenchmark, Display, TEXT("  UE_BUILD_DEBUG: %d"), UE_BUILD_DEBUG);

	UE_LOG(LogSynthBenchmark, Display, TEXT("  TotalPhysicalGBRam: %d"), FPlatformMemory::GetPhysicalGBRam());
	UE_LOG(LogSynthBenchmark, Display, TEXT("  NumberOfCores (physical): %d"), FPlatformMisc::NumberOfCores());
	UE_LOG(LogSynthBenchmark, Display, TEXT("  NumberOfCores (logical): %d"), FPlatformMisc::NumberOfCoresIncludingHyperthreads());

	UE_LOG(LogSynthBenchmark, Display, TEXT("  CPU Perf Index 0: %.1f (weight %.2f)"), InOut.CPUStats[0].ComputePerfIndex(), InOut.CPUStats[0].GetWeight());
	UE_LOG(LogSynthBenchmark, Display, TEXT("  CPU Perf Index 1: %.1f (weight %.2f)"), InOut.CPUStats[1].ComputePerfIndex(), InOut.CPUStats[1].GetWeight());
	
	// separator line
	UE_LOG(LogSynthBenchmark, Display, TEXT(" "));

	UE_LOG(LogSynthBenchmark, Display, TEXT("Graphics:"));

	UE_LOG(LogSynthBenchmark, Display, TEXT("  Adapter Name: '%s'"), *GRHIAdapterName);
	UE_LOG(LogSynthBenchmark, Display, TEXT("  (On Optimus the name might be wrong, memory should be ok)"));
	UE_LOG(LogSynthBenchmark, Display, TEXT("  Vendor Id: 0x%x"), GRHIVendorId);

	{
		FTextureMemoryStats Stats;

		RHIGetTextureMemoryStats(Stats);

		if(Stats.AreHardwareStatsValid())
		{
			UE_LOG(LogSynthBenchmark, Display, TEXT("  GPU Memory: %d/%d/%d MB"), 
				FMath::DivideAndRoundUp(Stats.DedicatedVideoMemory, (int64)(1024 * 1024) ),
				FMath::DivideAndRoundUp(Stats.DedicatedSystemMemory, (int64)(1024 * 1024) ),
				FMath::DivideAndRoundUp(Stats.SharedSystemMemory, (int64)(1024 * 1024) ));
		}
	}

	// not always done - cost some time.
	if(bGPUBenchmark)
	{
		IRendererModule& RendererModule = FModuleManager::LoadModuleChecked<IRendererModule>(TEXT("Renderer"));

		// First we run a quick test. If that shows very bad performance we don't need another test
		// The hardware is slow, we don't need a long test and risk driver TDR (driver recovery).
		// We have seen this problem on very low end GPUs.
		{
			const float fFirstWorkScale = 0.2f;
			const float fSecondWorkScale = 0.6f;

			float GPUTime = 0.0f;

			RendererModule.GPUBenchmark(InOut, fFirstWorkScale);
			GPUTime = InOut.ComputeTotalGPUTime();
			UE_LOG(LogSynthBenchmark, Display, TEXT("  GPU first test: %.2fs"), GPUTime);

			for(uint32 MethodId = 0; MethodId < sizeof(InOut.GPUStats) / sizeof(InOut.GPUStats[0]); ++MethodId)
			{
				UE_LOG(LogSynthBenchmark, Display, TEXT("         ... %.3f GigaPix/s, Confidence=%.0f%% '%s' (likely to be very inaccurate)"),
					1.0f / InOut.GPUStats[MethodId].GetNormalizedTime(), InOut.GPUStats[MethodId].GetConfidence(), InOut.GPUStats[MethodId].GetDesc());
			}

			if (GPUTime < fSecondWorkScale)
			{
				RendererModule.GPUBenchmark(InOut, fSecondWorkScale);
				GPUTime = InOut.ComputeTotalGPUTime();
				UE_LOG(LogSynthBenchmark, Display, TEXT("  GPU second test: %.2fs"), GPUTime);

				// for testing
				for(uint32 MethodId = 0; MethodId < sizeof(InOut.GPUStats) / sizeof(InOut.GPUStats[0]); ++MethodId)
				{
					UE_LOG(LogSynthBenchmark, Display, TEXT("         ... %.3f GigaPix/s, Confidence=%.0f%% '%s' (likely to be inaccurate)"),
						1.0f / InOut.GPUStats[MethodId].GetNormalizedTime(), InOut.GPUStats[MethodId].GetConfidence(), InOut.GPUStats[MethodId].GetDesc());
				}

				if (GPUTime < fSecondWorkScale)
				{
					RendererModule.GPUBenchmark(InOut, WorkScale);
					GPUTime = InOut.ComputeTotalGPUTime();
					UE_LOG(LogSynthBenchmark, Display, TEXT("  GPU third test: %.2fs"), GPUTime);
				}
			}
		}

		for(uint32 MethodId = 0; MethodId < sizeof(InOut.GPUStats) / sizeof(InOut.GPUStats[0]); ++MethodId)
		{
			UE_LOG(LogSynthBenchmark, Display, TEXT("         ... %.3f GigaPix/s, Confidence=%.0f%% '%s'"),
				1.0f / InOut.GPUStats[MethodId].GetNormalizedTime(), InOut.GPUStats[MethodId].GetConfidence(), InOut.GPUStats[MethodId].GetDesc());
		}
		UE_LOG(LogSynthBenchmark, Display, TEXT(""));

		UE_LOG(LogSynthBenchmark, Display, TEXT("  GPU Perf Index 0: %.1f (weight %.2f)"), InOut.GPUStats[0].ComputePerfIndex(), InOut.GPUStats[0].GetWeight());
		UE_LOG(LogSynthBenchmark, Display, TEXT("  GPU Perf Index 1: %.1f (weight %.2f)"), InOut.GPUStats[1].ComputePerfIndex(), InOut.GPUStats[1].GetWeight());
		UE_LOG(LogSynthBenchmark, Display, TEXT("  GPU Perf Index 2: %.1f (weight %.2f)"), InOut.GPUStats[2].ComputePerfIndex(), InOut.GPUStats[2].GetWeight());
		UE_LOG(LogSynthBenchmark, Display, TEXT("  GPU Perf Index 3: %.1f (weight %.2f)"), InOut.GPUStats[3].ComputePerfIndex(), InOut.GPUStats[3].GetWeight());
		UE_LOG(LogSynthBenchmark, Display, TEXT("  GPU Perf Index 4: %.1f (weight %.2f)"), InOut.GPUStats[4].ComputePerfIndex(), InOut.GPUStats[4].GetWeight());
	}
	
	UE_LOG(LogSynthBenchmark, Display, TEXT("  CPUIndex: %.1f"), InOut.ComputeCPUPerfIndex());

	if(bGPUBenchmark)
	{
		UE_LOG(LogSynthBenchmark, Display, TEXT("  GPUIndex: %.1f"), InOut.ComputeGPUPerfIndex());
	}

	UE_LOG(LogSynthBenchmark, Display, TEXT(""));
	UE_LOG(LogSynthBenchmark, Display, TEXT("         ... Total Time: %f sec"),  (float)(FPlatformTime::Seconds() - StartTime));
}