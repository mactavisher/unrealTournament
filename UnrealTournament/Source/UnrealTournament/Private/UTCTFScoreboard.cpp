// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.
#include "UnrealTournament.h"
#include "UTCTFScoreboard.h"
#include "UTTeamScoreboard.h"
#include "UTCTFGameState.h"

UUTCTFScoreboard::UUTCTFScoreboard(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	ScoringPlaysHeader = NSLOCTEXT("CTF", "ScoringPlaysHeader", "SCORING TIMELINE");
	AssistedByText = NSLOCTEXT("CTF", "AssistedBy", "Assists:");
	UnassistedText = NSLOCTEXT("CTF", "Unassisted", "Unassisted");
	NoScoringText = NSLOCTEXT("CTF", "NoScoring", "No Scoring");

	ColumnHeaderScoreX = 0.54;
	ColumnHeaderCapsX = 0.66;
	ColumnHeaderAssistsX = 0.73;
	ColumnHeaderReturnsX = 0.79;
	ReadyX = 0.7f;
	NumPages = 2;
}

void UUTCTFScoreboard::OpenScoringPlaysPage()
{
	SetPage(1);
}
void UUTCTFScoreboard::PageChanged_Implementation()
{
	GetWorld()->GetTimerManager().ClearTimer(OpenScoringPlaysHandle);
}

void UUTCTFScoreboard::Draw_Implementation(float DeltaTime)
{
	if (UTHUDOwner->ScoreboardPage == 1)
	{
		float YOffset = 0.0f;
		DrawGamePanel(DeltaTime, YOffset);
		DrawScoringPlays(DeltaTime, YOffset);
		YOffset = 936.0f;
		DrawServerPanel(DeltaTime, YOffset);
	}
	else
	{
		Super::Draw_Implementation(DeltaTime);
	}
}

void UUTCTFScoreboard::DrawGameOptions(float RenderDelta, float& YOffset)
{
	if (UTGameState)
	{
		FText StatusText = UTGameState->GetGameStatusText();
		if (!StatusText.IsEmpty())
		{
			DrawText(StatusText, 1255, 38, UTHUDOwner->MediumFont, 1.0, 1.0, FLinearColor::Yellow, ETextHorzPos::Right, ETextVertPos::Center);
		} 
		DrawText(UTHUDOwner->ConvertTime(FText::GetEmpty(), FText::GetEmpty(), UTGameState->RemainingTime, true, true, true), 1255, 88, UTHUDOwner->NumberFont, 1.0, 1.0, FLinearColor::White, ETextHorzPos::Right, ETextVertPos::Center);
	}
}

void UUTCTFScoreboard::DrawScoreHeaders(float RenderDelta, float& YOffset)
{
	float XOffset = 0.0;
	float Width = (Size.X * 0.5f) - CenterBuffer;
	float Height = 23;

	FText CH_PlayerName = NSLOCTEXT("UTScoreboard", "ColumnHeader_PlayerName", "Player");
	FText CH_Score = NSLOCTEXT("UTScoreboard", "ColumnHeader_PlayerScore", "Score");
	FText CH_Caps = NSLOCTEXT("UTCTFScoreboard", "ColumnHeader_PlayerCaps", "C");
	FText CH_Assists = NSLOCTEXT("UTCTFScoreboard", "ColumnHeader_PlayerAssists", "A");
	FText CH_Returns = NSLOCTEXT("UTCTFScoreboard", "ColumnHeader_PlayerReturns", "R");
	FText CH_Ping = (GetWorld()->GetNetMode() == NM_Standalone) ? NSLOCTEXT("UTScoreboard", "ColumnHeader_BotSkill", "Skill") : NSLOCTEXT("UTScoreboard", "ColumnHeader_PlayerPing", "Ping");
	FText CH_Ready = NSLOCTEXT("UTScoreboard", "ColumnHeader_Ready", "");

	for (int32 i = 0; i < 2; i++)
	{
		// Draw the background Border
		DrawTexture(TextureAtlas, XOffset, YOffset, Width, Height, 149, 138, 32, 32, 1.0, FLinearColor(0.72f, 0.72f, 0.72f, 0.85f));

		DrawText(CH_PlayerName, XOffset + (Width * ColumnHeaderPlayerX), YOffset + ColumnHeaderY, UTHUDOwner->TinyFont, 1.0f, 1.0f, FLinearColor::Black, ETextHorzPos::Left, ETextVertPos::Center);
		if (UTGameState && UTGameState->HasMatchStarted())
		{
			DrawText(CH_Score, XOffset + (Width * ColumnHeaderScoreX), YOffset + ColumnHeaderY, UTHUDOwner->TinyFont, 1.0f, 1.0f, FLinearColor::Black, ETextHorzPos::Center, ETextVertPos::Center);
			DrawText(CH_Caps, XOffset + (Width * ColumnHeaderCapsX), YOffset + ColumnHeaderY, UTHUDOwner->TinyFont, 1.0f, 1.0f, FLinearColor::Black, ETextHorzPos::Center, ETextVertPos::Center);
			DrawText(CH_Assists, XOffset + (Width * ColumnHeaderAssistsX), YOffset + ColumnHeaderY, UTHUDOwner->TinyFont, 1.0f, 1.0f, FLinearColor::Black, ETextHorzPos::Center, ETextVertPos::Center);
			DrawText(CH_Returns, XOffset + (Width * ColumnHeaderReturnsX), YOffset + ColumnHeaderY, UTHUDOwner->TinyFont, 1.0f, 1.0f, FLinearColor::Black, ETextHorzPos::Center, ETextVertPos::Center);
		}
		else
		{
			DrawText(CH_Ready, XOffset + (Width * ReadyX), YOffset + ColumnHeaderY, UTHUDOwner->TinyFont, 1.0f, 1.0f, FLinearColor::Black, ETextHorzPos::Center, ETextVertPos::Center);
		}

		DrawText(CH_Ping, XOffset + (Width * ColumnHeaderPingX), YOffset + ColumnHeaderY, UTHUDOwner->TinyFont, 1.0f, 1.0f, FLinearColor::Black, ETextHorzPos::Center, ETextVertPos::Center);

		XOffset = Size.X - Width;
	}

	YOffset += Height + 4;
}

void UUTCTFScoreboard::DrawPlayer(int32 Index, AUTPlayerState* PlayerState, float RenderDelta, float XOffset, float YOffset)
{
	if (PlayerState == NULL) return;	// Safeguard

	FLinearColor DrawColor = FLinearColor::White;
	float BarOpacity = 0.3f;
	float Width = (Size.X * 0.5f) - CenterBuffer;

	bool bIsUnderCursor = false;
	// If we are interactive, store off the bounds of this cell for selection
	if (bIsInteractive)
	{
		FVector4 Bounds = FVector4(RenderPosition.X + (XOffset * RenderScale), RenderPosition.Y + (YOffset * RenderScale),
										RenderPosition.X + ((XOffset + Width) * RenderScale), RenderPosition.Y + ((YOffset + CellHeight) * RenderScale));

		SelectionStack.Add(FSelectionObject(PlayerState, Bounds));
		bIsUnderCursor = (CursorPosition.X >= Bounds.X && CursorPosition.X <= Bounds.Z && CursorPosition.Y >= Bounds.Y && CursorPosition.Y <= Bounds.W);
	}

	FText PlayerName = FText::FromString(GetClampedName(PlayerState, UTHUDOwner->MediumFont, 1.f, 0.475f*Width));
	FText PlayerScore = FText::AsNumber(int32(PlayerState->Score/10.f));
	FText PlayerCaps = FText::AsNumber(PlayerState->FlagCaptures);
	FText PlayerAssists = FText::AsNumber(PlayerState->Assists);
	FText PlayerReturns = FText::AsNumber(PlayerState->FlagReturns);
	int32 Ping = PlayerState->Ping * 4;
	if (UTHUDOwner->UTPlayerOwner->UTPlayerState == PlayerState)
	{
		Ping = PlayerState->ExactPing;
		DrawColor = FLinearColor(0.0f, 0.92f, 1.0f, 1.0f);
		BarOpacity = 0.5;
	}
	else if (PlayerState->bIsFriend)
	{
		DrawColor = FLinearColor(FColor(254,255,174,255));
	}

	FText PlayerPing;
	if (GetWorld()->GetNetMode() == NM_Standalone)
	{
		AUTBot* Bot = Cast<AUTBot>(PlayerState->GetOwner());
		PlayerPing = Bot ? FText::AsNumber(Bot->Skill) : FText::FromString(TEXT("-"));
	}
	else
	{
		PlayerPing = FText::Format(NSLOCTEXT("UTScoreboard", "PingFormatText", "{0}ms"), FText::AsNumber(Ping));
	}

	// Draw the background border.
	FLinearColor BarColor = FLinearColor::Black;
	float FinalBarOpacity = BarOpacity;
	if (bIsUnderCursor) 
	{
		BarColor = FLinearColor(0.0, 0.3, 0.0, 1.0);
		FinalBarOpacity = 0.75f;
	}
	if (PlayerState == SelectedPlayer) 
	{
		BarColor = FLinearColor(0.0, 0.3, 0.3, 1.0);
		FinalBarOpacity = 0.75f;
	}

	DrawTexture(TextureAtlas, XOffset, YOffset, Width, 36, 149, 138, 32, 32, FinalBarOpacity, BarColor);

	int32 FlagU = (PlayerState->CountryFlag % 8) * 32;
	int32 FlagV = (PlayerState->CountryFlag / 8) * 24;

	DrawTexture(FlagAtlas, XOffset + (Width * FlagX), YOffset + 18, 32,24, FlagU,FlagV,32,24,1.0, FLinearColor::White, FVector2D(0.0f,0.5f));	// Add a function to support additional flags

	// Draw the Text
	FVector2D NameSize = DrawText(PlayerName, XOffset + (Width * ColumnHeaderPlayerX), YOffset + ColumnY, UTHUDOwner->MediumFont, 1.0f, 1.0f, DrawColor, ETextHorzPos::Left, ETextVertPos::Center);

	if (PlayerState->bIsFriend)
	{
		DrawTexture(TextureAtlas, XOffset + (Width * ColumnHeaderPlayerX) + NameSize.X + 5, YOffset + 18, 30, 24, 236, 136, 30, 24, 1.0, FLinearColor::White, FVector2D(0.0f, 0.5f));
	}

	if (UTGameState && UTGameState->HasMatchStarted())
	{
		DrawText(PlayerScore, XOffset + (Width * ColumnHeaderScoreX), YOffset + ColumnY, UTHUDOwner->MediumFont, 1.0f, 1.0f, DrawColor, ETextHorzPos::Center, ETextVertPos::Center);
		DrawText(PlayerCaps, XOffset + (Width * ColumnHeaderCapsX), YOffset + ColumnY, UTHUDOwner->SmallFont, 1.0f, 1.0f, DrawColor, ETextHorzPos::Center, ETextVertPos::Center);
		DrawText(PlayerAssists, XOffset + (Width * ColumnHeaderAssistsX), YOffset + ColumnY, UTHUDOwner->SmallFont, 1.0f, 1.0f, DrawColor, ETextHorzPos::Center, ETextVertPos::Center);
		DrawText(PlayerReturns, XOffset + (Width * ColumnHeaderReturnsX), YOffset + ColumnY, UTHUDOwner->SmallFont, 1.0f, 1.0f, DrawColor, ETextHorzPos::Center, ETextVertPos::Center);
	}
	else
	{
		FText PlayerReady = PlayerState->bReadyToPlay ? NSLOCTEXT("UTScoreboard", "READY", "READY") : NSLOCTEXT("UTScoreboard", "NOTREADY", "");
		if (PlayerState->bPendingTeamSwitch)
		{
			PlayerReady = NSLOCTEXT("UTScoreboard", "TEAMSWITCH", "TEAM SWAP");
		}
		DrawText(PlayerReady, XOffset + (Width * ReadyX), YOffset + ColumnY, UTHUDOwner->MediumFont, 1.0f, 1.0f, DrawColor, ETextHorzPos::Center, ETextVertPos::Center);
	}

	DrawText(PlayerPing, XOffset + (Width * ColumnHeaderPingX), YOffset + ColumnY, UTHUDOwner->SmallFont, 1.0f, 1.0f, DrawColor, ETextHorzPos::Center, ETextVertPos::Center);
}

void UUTCTFScoreboard::DrawScoringPlays(float DeltaTime, float& YPos)
{
	AUTCTFGameState* CTFState = Cast<AUTCTFGameState>(UTGameState);
	uint8 CurrentPeriod = 0;
	if (CTFState->HasMatchEnded())
	{
		// show scores for last played period
		for (const FCTFScoringPlay& Play : CTFState->GetScoringPlays())
		{
			CurrentPeriod = FMath::Max<uint8>(CurrentPeriod, Play.Period);
		}
	}
	// TODO: currently there's no intermission between second half and OT
	else if (CTFState->IsMatchAtHalftime())
	{
		CurrentPeriod = 0;
	}
	else
	{
		CurrentPeriod = CTFState->bSecondHalf ? 1 : 0;
	}
	Canvas->SetLinearDrawColor(FLinearColor::White);
	const float XOffset = Canvas->ClipX * ((Canvas->ClipX / Canvas->ClipY > 1.5f) ? 0.2f : 0.1f);
	FFontRenderInfo TextRenderInfo;
	TextRenderInfo.bEnableShadow = true;
	TextRenderInfo.bClipText = true;
	{
		float XL, YL;
		Canvas->TextSize(UTHUDOwner->LargeFont, ScoringPlaysHeader.ToString(), XL, YL, RenderScale, RenderScale);
		YPos += YL * 0.5f;
		Canvas->DrawText(UTHUDOwner->LargeFont, ScoringPlaysHeader, (Canvas->ClipX - XL) * 0.5, YPos, RenderScale, RenderScale, TextRenderInfo);
		YPos += YL * 1.2f;
	}
	TArray<int32> ScoresSoFar;
	ScoresSoFar.SetNumZeroed(CTFState->Teams.Num());
	TMap<FSafePlayerName, int32> CapCount;
	bool bDrewSomething = false;
	for (const FCTFScoringPlay& Play : CTFState->GetScoringPlays())
	{
		if (Play.Team != NULL) // should always be true...
		{
			ScoresSoFar[Play.Team->TeamIndex]++;
			int32* PlayerCaps = CapCount.Find(Play.ScoredBy);
			if (PlayerCaps != NULL)
			{
				(*PlayerCaps)++;
			}
			else
			{
				CapCount.Add(Play.ScoredBy, 1);
			}
			if (Play.Period >= CurrentPeriod)
			{
				float XL, YL;
				bDrewSomething = true;
				// draw this cap
				Canvas->SetLinearDrawColor(Play.Team->TeamColor);
				// scored by
				FString ScoredByLine = Play.ScoredBy.GetPlayerName();
				if (PlayerCaps != NULL)
				{
					ScoredByLine += FString::Printf(TEXT(" (%i)"), *PlayerCaps);
				}
				float PrevYPos = YPos;
				Canvas->TextSize(UTHUDOwner->MediumFont, ScoredByLine, XL, YL, RenderScale, RenderScale);
				Canvas->DrawText(UTHUDOwner->MediumFont, ScoredByLine, XOffset, YPos, RenderScale, RenderScale, TextRenderInfo);
				YPos += YL;
				// assists
				FString AssistLine;
				if (Play.Assists.Num() > 0)
				{
					AssistLine = AssistedByText.ToString() + TEXT(" ");
					for (const FCTFAssist& Assist : Play.Assists)
					{
						AssistLine += Assist.AssistName.GetPlayerName() + TEXT(", ");
					}
					AssistLine = AssistLine.LeftChop(2);
				}
				else
				{
					AssistLine = UnassistedText.ToString();
				}
				Canvas->TextSize(UTHUDOwner->SmallFont, AssistLine, XL, YL, RenderScale, RenderScale);
				Canvas->DrawText(UTHUDOwner->SmallFont, AssistLine, XOffset, YPos, RenderScale, RenderScale, TextRenderInfo);
				YPos += YL;
				// time of game
				FString TimeStampLine = UTHUDOwner->ConvertTime(FText::GetEmpty(), FText::GetEmpty(), Play.ElapsedTime, false, true, false).ToString();
				Canvas->TextSize(UTHUDOwner->SmallFont, TimeStampLine, XL, YL, RenderScale, RenderScale);
				Canvas->DrawText(UTHUDOwner->SmallFont, TimeStampLine, XOffset, YPos, RenderScale, RenderScale, TextRenderInfo);
				YPos += YL;
				// team score after this cap
				Canvas->SetLinearDrawColor(FLinearColor::White);
				FString CurrentScoreLine;
				for (int32 Score : ScoresSoFar)
				{
					CurrentScoreLine += FString::Printf(TEXT("%i      "), Score);
				}
				Canvas->TextSize(UTHUDOwner->MediumFont, CurrentScoreLine, XL, YL, RenderScale, RenderScale);
				{
					float XPos = Canvas->ClipX - XOffset - XL;
					for (int32 i = 0; i < ScoresSoFar.Num(); i++)
					{
						Canvas->SetLinearDrawColor(CTFState->Teams[i]->TeamColor);
						FString SingleScorePart = FString::Printf(TEXT("      %i"), ScoresSoFar[i]);
						float SingleXL, SingleYL;
						Canvas->TextSize(UTHUDOwner->MediumFont, SingleScorePart, SingleXL, SingleYL, RenderScale, RenderScale);
						Canvas->DrawText(UTHUDOwner->MediumFont, SingleScorePart, XPos, PrevYPos + (YPos - PrevYPos - YL) * 0.5, RenderScale, RenderScale, TextRenderInfo);
						XPos += SingleXL;
					}
				}
				Canvas->SetLinearDrawColor(FLinearColor::White);

				YPos += YL * 0.25f;
				if (YPos >= 775.0f)
				{
					// TODO: pagination
					break;
				}
			}
		}
	}
	if (!bDrewSomething)
	{
		float XL, YL;
		Canvas->TextSize(UTHUDOwner->MediumFont, NoScoringText.ToString(), XL, YL, RenderScale, RenderScale);
		Canvas->DrawText(UTHUDOwner->MediumFont, NoScoringText, Canvas->ClipX * 0.5f - XL * 0.5f, YPos, RenderScale, RenderScale, TextRenderInfo);
	}
}

