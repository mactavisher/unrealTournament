// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#include "UnrealTournament.h"
#include "UTProgressionStorage.h"
#include "UTProfileSettings.h"

UUTProgressionStorage::UUTProgressionStorage(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	bNeedsUpdate = false;
}

void UUTProgressionStorage::VersionFixup()
{
	TokensCommit(); // just in case any achievements failed to unlock previously due to bug
}

bool UUTProgressionStorage::HasTokenBeenPickedUpBefore(FName TokenUniqueID)
{
	return FoundTokenUniqueIDs.Contains(TokenUniqueID);
}

void UUTProgressionStorage::TokenPickedUp(FName TokenUniqueID)
{
	bNeedsUpdate = true;
	TempFoundTokenUniqueIDs.AddUnique(TokenUniqueID);
}

void UUTProgressionStorage::TokenRevoke(FName TokenUniqueID)
{
	bNeedsUpdate = true;
	TempFoundTokenUniqueIDs.Remove(TokenUniqueID);
}

void UUTProgressionStorage::TokensCommit()
{
	for (FName ID : TempFoundTokenUniqueIDs)
	{
		FoundTokenUniqueIDs.AddUnique(ID);
	}

	// see if all achievement tokens have been picked up
	if (!Achievements.Contains(AchievementIDs::TutorialComplete))
	{
		bool bCompletedTutorial = true;
		static TArray<FName, TInlineAllocator<60>> TutorialTokens = []()
		{
			TArray<FName, TInlineAllocator<60>> List;
			FNumberFormattingOptions Options;
			Options.MinimumIntegralDigits = 3;
			for (int32 i = 0; i < 15; i++)
			{
				List.Add(FName(*FString::Printf(TEXT("movementtraining_token_%s"), *FText::AsNumber(i, &Options).ToString())));
			}
			for (int32 i = 0; i < 15; i++)
			{
				List.Add(FName(*FString::Printf(TEXT("weapontraining_token_%s"), *FText::AsNumber(i, &Options).ToString())));
			}
			for (int32 i = 0; i < 10; i++)
			{
				List.Add(FName(*FString::Printf(TEXT("pickuptraining_token_%s"), *FText::AsNumber(i, &Options).ToString())));
			}
			for (int32 i = 0; i < 5; i++)
			{
				List.Add(FName(*FString::Printf(TEXT("tuba_token_%s"), *FText::AsNumber(i, &Options).ToString())));
			}
			for (int32 i = 0; i < 5; i++)
			{
				List.Add(FName(*FString::Printf(TEXT("outpost23_token_%s"), *FText::AsNumber(i, &Options).ToString())));
			}
			for (int32 i = 0; i < 5; i++)
			{
				List.Add(FName(*FString::Printf(TEXT("face_token_%s"), *FText::AsNumber(i, &Options).ToString())));
			}
			for (int32 i = 0; i < 5; i++)
			{
				List.Add(FName(*FString::Printf(TEXT("asdf_token_%s"), *FText::AsNumber(i, &Options).ToString())));
			}
			return List;
		}();
		for (FName TestToken : TutorialTokens)
		{
			if (!FoundTokenUniqueIDs.Contains(TestToken))
			{
				bCompletedTutorial = false;
				break;
			}
		}
		if (bCompletedTutorial)
		{
			Achievements.Add(AchievementIDs::TutorialComplete);
			UUTLocalPlayer* LP = Cast<UUTLocalPlayer>(GEngine->GetFirstGamePlayer(GWorld));	
			if (LP)
			{
				LP->ShowToast(NSLOCTEXT("UT", "ItemRewardVise", "You earned Visse - The Armor of Sacrifce!"));
			}
		}
	}

	bNeedsUpdate = true;
	TempFoundTokenUniqueIDs.Empty();
}

void UUTProgressionStorage::TokensReset()
{
	bNeedsUpdate = true;
	TempFoundTokenUniqueIDs.Empty();
}

void UUTProgressionStorage::TokensClear()
{
	bNeedsUpdate = true;
	TempFoundTokenUniqueIDs.Empty();
	FoundTokenUniqueIDs.Empty();
}

bool UUTProgressionStorage::GetBestTime(FName TimingName, float& OutBestTime)
{
	OutBestTime = 0;

	float* BestTime = BestTimes.Find(TimingName);
	if (BestTime)
	{
		OutBestTime = *BestTime;
		return true;
	}

	return false;
}

void UUTProgressionStorage::SetBestTime(FName TimingName, float InBestTime)
{
	BestTimes.Add(TimingName, InBestTime);
	bNeedsUpdate = true;

	// hacky halloween reward implementation
	if (TimingName == AchievementIDs::FacePumpkins)
	{
		if (InBestTime >= 6666.0f)
		{
			for (TObjectIterator<UUTLocalPlayer> It; It; ++It)
			{
				if (It->GetProgressionStorage() == this)
				{
					It->AwardAchievement(AchievementIDs::FacePumpkins);
				}
			}
		}

		if (InBestTime >= 5000.0f)
		{
			for (TObjectIterator<UUTLocalPlayer> It; It; ++It)
			{
				if (It->GetProgressionStorage() == this)
				{
					It->AwardAchievement(AchievementIDs::PumpkinHead2015Level3);
				}
			}
		}

		if (InBestTime >= 1000.0f)
		{
			for (TObjectIterator<UUTLocalPlayer> It; It; ++It)
			{
				if (It->GetProgressionStorage() == this)
				{
					It->AwardAchievement(AchievementIDs::PumpkinHead2015Level2);
				}
			}
		}

		if (InBestTime >= 200.0f)
		{
			for (TObjectIterator<UUTLocalPlayer> It; It; ++It)
			{
				if (It->GetProgressionStorage() == this)
				{
					It->AwardAchievement(AchievementIDs::PumpkinHead2015Level1);
				}
			}
		}
	}
}
