// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "UTCTFScoreboard.h"
#include "UTFlagRunScoreboard.generated.h"

UCLASS()
class UNREALTOURNAMENT_API UUTFlagRunScoreboard : public UUTCTFScoreboard
{
	GENERATED_UCLASS_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoreboard")
		float ColumnHeaderPowerupX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoreboard")
		float ColumnHeaderPowerupEndX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoreboard")
		float ColumnHeaderPowerupXDuringReadyUp;

	UPROPERTY()
		FText PowerupText;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoreboard")
		FText CH_Powerup;

protected:
	virtual void DrawScoreHeaders(float RenderDelta, float& YOffset);
	virtual void DrawPlayerScore(AUTPlayerState* PlayerState, float XOffset, float YOffset, float Width, FLinearColor DrawColor) override;
	virtual void DrawReadyText(AUTPlayerState* PlayerState, float XOffset, float YOffset, float Width);
	virtual void DrawTeamStats(float DeltaTime, float& YPos, float XOffset, float ScoreWidth, float MaxHeight, const FStatsFontInfo& StatsFontInfo) override;

	virtual void DrawBonusTeamStatsLine(FText StatsName, float DeltaTime, float XOffset, float& YPos, const FStatsFontInfo& StatsFontInfo, float ScoreWidth, bool bSkipEmpty);

	virtual bool ShouldShowPowerupForPlayer(AUTPlayerState* PlayerState);
};