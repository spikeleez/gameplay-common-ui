// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Subsystems/GameplayPerformanceStatSubsystem.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/NetConnection.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameStateBase.h"
#include "Performance/LatencyMarkerModule.h"
#include "ProfilingDebugging/CsvProfiler.h"
#include "Interfaces/GameplayPerformanceStatInterface.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplayPerformanceStatSubsystem)

CSV_DEFINE_CATEGORY(SettingsPerformance, /*bIsEnabledByDefault=*/false);

//===================================================
// FGameplayPerformanceStatCache
//===================================================

void FGameplayPerformanceStatCache::StartCharting()
{
	// Ignored
}

void FGameplayPerformanceStatCache::StopCharting()
{
	// Ignored
}

void FGameplayPerformanceStatCache::ProcessFrame(const FFrameData& FrameData)
{
	// Record stats about the frame data.
	{
		RecordStat(EGameplayDisplayablePerformanceStat::ClientFPS, (FrameData.TrueDeltaSeconds != 0.0) ? 1.0 / FrameData.TrueDeltaSeconds : 0.0);
		RecordStat(EGameplayDisplayablePerformanceStat::IdleTime, FrameData.IdleSeconds);
		RecordStat(EGameplayDisplayablePerformanceStat::FrameTime_Total, FrameData.TrueDeltaSeconds);
		RecordStat(EGameplayDisplayablePerformanceStat::FrameTime_GameThread, FrameData.GameThreadTimeSeconds);
		RecordStat(EGameplayDisplayablePerformanceStat::FrameTime_RenderThread, FrameData.RenderThreadTimeSeconds);
		RecordStat(EGameplayDisplayablePerformanceStat::FrameTime_RHIThread, FrameData.RHIThreadTimeSeconds);
		RecordStat(EGameplayDisplayablePerformanceStat::FrameTime_GPU, FrameData.GPUTimeSeconds);
	}

	if (const UWorld* World = MySubsystem->GetGameInstance()->GetWorld())
	{
		// Record some networking related stats.
		if (const AGameStateBase* GameState = World->GetGameState<AGameStateBase>())
		{
			const IGameplayPerformanceStatInterface* StatInterface = Cast<IGameplayPerformanceStatInterface>(GameState);
			if (StatInterface)
			{
				RecordStat(EGameplayDisplayablePerformanceStat::ServerFPS, StatInterface->GetServerFPS());
			}

			if (const APlayerController* LocalPC = GEngine->GetFirstLocalPlayerController(World))
			{
				if (const APlayerState* PS = LocalPC->GetPlayerState<APlayerState>())
				{
					RecordStat(EGameplayDisplayablePerformanceStat::Ping, PS->GetPingInMilliseconds());
				}

				if (const UNetConnection* NetConnection = LocalPC->GetNetConnection())
				{
					const UNetConnection::FNetConnectionPacketLoss& InLoss = NetConnection->GetInLossPercentage();
					const UNetConnection::FNetConnectionPacketLoss& OutLoss = NetConnection->GetOutLossPercentage();

					RecordStat(EGameplayDisplayablePerformanceStat::PacketLoss_Incoming, InLoss.GetAvgLossPercentage());
					RecordStat(EGameplayDisplayablePerformanceStat::PacketLoss_Outgoing, OutLoss.GetAvgLossPercentage());
					RecordStat(EGameplayDisplayablePerformanceStat::PacketRate_Incoming, NetConnection->InPacketsPerSecond);
					RecordStat(EGameplayDisplayablePerformanceStat::PacketRate_Outgoing, NetConnection->OutPacketsPerSecond);
					RecordStat(EGameplayDisplayablePerformanceStat::PacketSize_Incoming, (NetConnection->InPacketsPerSecond != 0));
					RecordStat(EGameplayDisplayablePerformanceStat::PacketSize_Outgoing, (NetConnection->OutPacketsPerSecond != 0));
				}

				// Finally, record some input latency related stats if they are enabled.
				TArray<ILatencyMarkerModule*> LatencyMarkerModules = IModularFeatures::Get().GetModularFeatureImplementations<ILatencyMarkerModule>(ILatencyMarkerModule::GetModularFeatureName());
				for (ILatencyMarkerModule* LatencyMarkerModule : LatencyMarkerModules)
				{
					if (LatencyMarkerModule->GetEnabled())
					{
						const float TotalLatencyMs = LatencyMarkerModule->GetTotalLatencyInMs();
						if (TotalLatencyMs > 0.0f)
						{
							// Record some stats about the latency of the game.
							RecordStat(EGameplayDisplayablePerformanceStat::Latency_Total, TotalLatencyMs);
							RecordStat(EGameplayDisplayablePerformanceStat::Latency_Game, LatencyMarkerModule->GetGameLatencyInMs());
							RecordStat(EGameplayDisplayablePerformanceStat::Latency_Render, LatencyMarkerModule->GetRenderLatencyInMs());

							// Record some CSV profile stats.
							// You can see these by using the following commands
							// Start and stop the profile:
							//	CsvProfile Start
							//	CsvProfile Stop
							//
							// Or, you can profile for a certain number of frames:
							// CsvProfile Frames=10
							//
							// And this will output a .csv file to the Saved\Profiling\CSV folder
#if CSV_PROFILER
							if (const FCsvProfiler* Profiler = FCsvProfiler::Get())
							{
								static const FName TotalLatencyStatName = TEXT("Latency_Total");
								Profiler->RecordCustomStat(TotalLatencyStatName, CSV_CATEGORY_INDEX(SettingsPerformance), TotalLatencyMs, ECsvCustomStatOp::Set);

								static const FName GameLatencyStatName = TEXT("Latency_Game");
								Profiler->RecordCustomStat(GameLatencyStatName, CSV_CATEGORY_INDEX(SettingsPerformance), LatencyMarkerModule->GetGameLatencyInMs(), ECsvCustomStatOp::Set);

								static const FName RenderLatencyStatName = TEXT("Latency_Render");
								Profiler->RecordCustomStat(RenderLatencyStatName, CSV_CATEGORY_INDEX(SettingsPerformance), LatencyMarkerModule->GetRenderLatencyInMs(), ECsvCustomStatOp::Set);
							}
#endif

							// Some more fine grain latency numbers can be found on the marker module if desired
							//LatencyMarkerModule->GetRenderLatencyInMs()));
							//LatencyMarkerModule->GetDriverLatencyInMs()));
							//LatencyMarkerModule->GetOSRenderQueueLatencyInMs()));
							//LatencyMarkerModule->GetGPURenderLatencyInMs()));
							break;
						}
					}
				}
			}
		}
	}
}

void FGameplayPerformanceStatCache::RecordStat(const EGameplayDisplayablePerformanceStat Stat, const double Value)
{
	PerfStatCache.FindOrAdd(Stat).RecordSample(Value);
}

double FGameplayPerformanceStatCache::GetCachedStat(EGameplayDisplayablePerformanceStat Stat) const
{
	static_assert((int32)EGameplayDisplayablePerformanceStat::MAX == 18, "Need to update this function to deal with new performance stats");

	if (const FGameplaySampledStatCache* Cache = GetCachedStatData(Stat))
	{
		return Cache->GetLastCachedStat();
	}

	return 0.0;
}

const FGameplaySampledStatCache* FGameplayPerformanceStatCache::GetCachedStatData(const EGameplayDisplayablePerformanceStat Stat) const
{
	static_assert((int32)EGameplayDisplayablePerformanceStat::MAX == 18, "Need to update this function to deal with new performance stats");

	return PerfStatCache.Find(Stat);
}

//===================================================
// UGameplayPerformanceStatSubsystem
//===================================================

UGameplayPerformanceStatSubsystem::UGameplayPerformanceStatSubsystem()
{

}

void UGameplayPerformanceStatSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	Tracker = MakeShared<FGameplayPerformanceStatCache>(this);
	GEngine->AddPerformanceDataConsumer(Tracker);
}

void UGameplayPerformanceStatSubsystem::Deinitialize()
{
	GEngine->RemovePerformanceDataConsumer(Tracker);
	Tracker.Reset();

	Super::Deinitialize();
}

double UGameplayPerformanceStatSubsystem::GetCachedStat(EGameplayDisplayablePerformanceStat Stat) const
{
	return Tracker->GetCachedStat(Stat);
}

const FGameplaySampledStatCache* UGameplayPerformanceStatSubsystem::GetCachedStatData(const EGameplayDisplayablePerformanceStat Stat) const
{
	return Tracker->GetCachedStatData(Stat);
}
