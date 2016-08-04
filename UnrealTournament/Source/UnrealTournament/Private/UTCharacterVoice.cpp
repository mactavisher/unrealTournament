// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#include "UnrealTournament.h"
#include "UTCharacterVoice.h"
#include "UTAnnouncer.h"

UUTCharacterVoice::UUTCharacterVoice(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	MessageArea = FName(TEXT("ConsoleMessage"));
	bIsStatusAnnouncement = false;
	bOptionalSpoken = true;
	FontSizeIndex = 1;
	Lifetime = 6.0f;

	// < 1000 is reserved for taunts
	SameTeamBaseIndex = 1000;
	FriendlyReactionBaseIndex = 2000;
	EnemyReactionBaseIndex = 2500;

	// 10000+ is reserved for status messages
	StatusBaseIndex = 10000;
	StatusOffsets.Add(StatusMessage::NeedBackup, 0);
	StatusOffsets.Add(StatusMessage::EnemyFCHere, 100);
	StatusOffsets.Add(StatusMessage::AreaSecure, 200);
	StatusOffsets.Add(StatusMessage::IGotFlag, 300);
	StatusOffsets.Add(StatusMessage::DefendFlag, 400);
	StatusOffsets.Add(StatusMessage::DefendFC, 500);
	StatusOffsets.Add(StatusMessage::GetFlagBack, 600);
	StatusOffsets.Add(StatusMessage::ImOnDefense, 700);
	StatusOffsets.Add(StatusMessage::ImOnOffense, 900);
	StatusOffsets.Add(StatusMessage::SpreadOut, 1000);

	StatusOffsets.Add(StatusMessage::ImGoingIn, KEY_CALLOUTS + 100);
	StatusOffsets.Add(StatusMessage::BaseUnderAttack, KEY_CALLOUTS + 200);
	StatusOffsets.Add(GameVolumeSpeechType::GV_Bridge, KEY_CALLOUTS + 300);
	StatusOffsets.Add(GameVolumeSpeechType::GV_River, KEY_CALLOUTS + 400);
	StatusOffsets.Add(GameVolumeSpeechType::GV_Antechamber, KEY_CALLOUTS + 500);
	StatusOffsets.Add(GameVolumeSpeechType::GV_ThroneRoom, KEY_CALLOUTS + 600);
	StatusOffsets.Add(GameVolumeSpeechType::GV_Courtyard, KEY_CALLOUTS + 700);
	StatusOffsets.Add(GameVolumeSpeechType::GV_Stables, KEY_CALLOUTS + 800);
	StatusOffsets.Add(GameVolumeSpeechType::GV_AntechamberHigh, KEY_CALLOUTS + 1000);
	StatusOffsets.Add(GameVolumeSpeechType::GV_Tower, KEY_CALLOUTS + 1100);
	StatusOffsets.Add(GameVolumeSpeechType::GV_Creek, KEY_CALLOUTS + 1200);
	StatusOffsets.Add(GameVolumeSpeechType::GV_Temple, KEY_CALLOUTS + 1300);
	StatusOffsets.Add(GameVolumeSpeechType::GV_Cave, KEY_CALLOUTS + 1400);
	StatusOffsets.Add(GameVolumeSpeechType::GV_BaseCamp, KEY_CALLOUTS + 1500);

	StatusOffsets.Add(StatusMessage::EnemyRally, KEY_CALLOUTS + 5000);
	StatusOffsets.Add(StatusMessage::FindFC, KEY_CALLOUTS + 5001);
	StatusOffsets.Add(StatusMessage::LastLife, KEY_CALLOUTS + 5002);
	StatusOffsets.Add(StatusMessage::EnemyLowLives, KEY_CALLOUTS + 5003);
	StatusOffsets.Add(StatusMessage::EnemyThreePlayers, KEY_CALLOUTS + 5004);
	StatusOffsets.Add(StatusMessage::NeedRally, KEY_CALLOUTS + 5006);
	StatusOffsets.Add(StatusMessage::NeedHealth, KEY_CALLOUTS + 5007);

	StatusOffsets.Add(PickupSpeechType::RedeemerPickup, KEY_CALLOUTS + 5100);
	StatusOffsets.Add(PickupSpeechType::UDamagePickup, KEY_CALLOUTS + 5200);
	StatusOffsets.Add(PickupSpeechType::ShieldbeltPickup, KEY_CALLOUTS + 5300);

	TauntText = NSLOCTEXT("UTCharacterVoice", "Taunt", ": {TauntMessage}");
	StatusTextFormat = NSLOCTEXT("UTCharacterVoice", "StatusFormat", " at {LastKnownLocation}: {TauntMessage}");

	FallbackLines.Add(FName(TEXT("Bridge")), GameVolumeSpeechType::GV_Bridge);
	FallbackLines.Add(FName(TEXT("River")), GameVolumeSpeechType::GV_River);
	FallbackLines.Add(FName(TEXT("Tower")), GameVolumeSpeechType::GV_Tower);
	FallbackLines.Add(FName(TEXT("Creek")), GameVolumeSpeechType::GV_Creek);
	FallbackLines.Add(FName(TEXT("Temple")), GameVolumeSpeechType::GV_Temple);
	FallbackLines.Add(FName(TEXT("Cave")), GameVolumeSpeechType::GV_Cave);
	FallbackLines.Add(FName(TEXT("Antechamber")), GameVolumeSpeechType::GV_Antechamber);
	FallbackLines.Add(FName(TEXT("Courtyard")), GameVolumeSpeechType::GV_Courtyard);
	FallbackLines.Add(FName(TEXT("Stables")), GameVolumeSpeechType::GV_Stables);
}

FName UUTCharacterVoice::GetFallbackLines(FName InName) const
{
	return FallbackLines.FindRef(InName);
}

bool UUTCharacterVoice::IsOptionalSpoken(int32 MessageIndex) const
{
	return bOptionalSpoken && (MessageIndex < KEY_CALLOUTS);
}

FText UUTCharacterVoice::GetText(int32 Switch, bool bTargetsPlayerState1, class APlayerState* RelatedPlayerState_1, class APlayerState* RelatedPlayerState_2, class UObject* OptionalObject) const
{
	FFormatNamedArguments Args;
	if (!RelatedPlayerState_1)
	{
		UE_LOG(UT, Warning, TEXT("Character voice w/ no playerstate index %d"), Switch);
		return FText::GetEmpty();
	}

	UUTGameUserSettings* GS = Cast<UUTGameUserSettings>(GEngine->GetGameUserSettings());
	if (GS != NULL && GS->GetBotSpeech() < ((Switch >= StatusBaseIndex) ? BSO_StatusTextOnly : BSO_All))
	{ 
		return FText::GetEmpty();
	}
	FCharacterSpeech PickedSpeech = GetCharacterSpeech(Switch);
	Args.Add("PlayerName", FText::AsCultureInvariant(RelatedPlayerState_1->PlayerName));
	Args.Add("TauntMessage", PickedSpeech.SpeechText);

	bool bStatusMessage = (Switch >= StatusBaseIndex) && OptionalObject && Cast<AUTGameVolume>(OptionalObject);
	if (bStatusMessage)
	{
		Args.Add("LastKnownLocation", Cast<AUTGameVolume>(OptionalObject)->VolumeName);
	}
	return bStatusMessage ? FText::Format(StatusTextFormat, Args) : FText::Format(TauntText, Args);
}

FName UUTCharacterVoice::GetAnnouncementName_Implementation(int32 Switch, const UObject* OptionalObject, const class APlayerState* RelatedPlayerState_1, const class APlayerState* RelatedPlayerState_2) const
{
	return NAME_Custom;
}

USoundBase* UUTCharacterVoice::GetAnnouncementSound_Implementation(int32 Switch, const UObject* OptionalObject) const
{
	FCharacterSpeech PickedSpeech = GetCharacterSpeech(Switch);
	return PickedSpeech.SpeechSound;
}

FCharacterSpeech UUTCharacterVoice::GetCharacterSpeech(int32 Switch) const
{
	FCharacterSpeech EmptySpeech;
	EmptySpeech.SpeechSound = nullptr;
	EmptySpeech.SpeechText = FText::GetEmpty();

	if (TauntMessages.Num() > Switch)
	{
		return TauntMessages[Switch];
	}
	else if (SameTeamMessages.Num() > Switch - SameTeamBaseIndex)
	{
		return SameTeamMessages[Switch - SameTeamBaseIndex];
	}
	else if (FriendlyReactions.Num() > Switch - FriendlyReactionBaseIndex)
	{
		return FriendlyReactions[Switch - FriendlyReactionBaseIndex];
	}
	else if (EnemyReactions.Num() > Switch - EnemyReactionBaseIndex)
	{
		return EnemyReactions[Switch - EnemyReactionBaseIndex];
	}
	else if (Switch == ACKNOWLEDGE_SWITCH_INDEX )
	{
		return AcknowledgeMessages[FMath::RandRange(0, AcknowledgeMessages.Num() - 1)];
	}
	else if (Switch == NEGATIVE_SWITCH_INDEX )
	{
		return NegativeMessages[FMath::RandRange(0, NegativeMessages.Num() - 1)];
	}
	else if (Switch == GOT_YOUR_BACK_SWITCH_INDEX)
	{
		return GotYourBackMessages[FMath::RandRange(0, GotYourBackMessages.Num() - 1)];
	}
	else if (Switch == UNDER_HEAVY_ATTACK_SWITCH_INDEX)
	{
		return UnderHeavyAttackMessages[FMath::RandRange(0, UnderHeavyAttackMessages.Num() - 1)];
	}
	else if (Switch == ATTACK_THEIR_BASE_SWITCH_INDEX)
	{
		return AttackTheirBaseMessages[FMath::RandRange(0, AttackTheirBaseMessages.Num() - 1)];
	}
	else if (Switch >= StatusBaseIndex)
	{
		if (Switch < KEY_CALLOUTS)
		{
			if (Switch == GetStatusIndex(StatusMessage::NeedBackup))
			{
				if (NeedBackupMessages.Num() == 0)
				{
					return EmptySpeech;
				}
				return NeedBackupMessages[FMath::RandRange(0, NeedBackupMessages.Num() - 1)];
			}
			else if (Switch == GetStatusIndex(StatusMessage::EnemyFCHere))
			{
				if (EnemyFCHereMessages.Num() == 0)
				{
					return EmptySpeech;
				}
				return EnemyFCHereMessages[FMath::RandRange(0, EnemyFCHereMessages.Num() - 1)];
			}
			else if (Switch == GetStatusIndex(StatusMessage::AreaSecure))
			{
				if (AreaSecureMessages.Num() == 0)
				{
					return EmptySpeech;
				}
				return AreaSecureMessages[FMath::RandRange(0, AreaSecureMessages.Num() - 1)];
			}
			else if (Switch == GetStatusIndex(StatusMessage::IGotFlag))
			{
				if (IGotFlagMessages.Num() == 0)
				{
					return EmptySpeech;
				}
				return IGotFlagMessages[FMath::RandRange(0, IGotFlagMessages.Num() - 1)];
			}
			else if (Switch == GetStatusIndex(StatusMessage::DefendFlag))
			{
				if (DefendFlagMessages.Num() == 0)
				{
					return EmptySpeech;
				}
				return DefendFlagMessages[FMath::RandRange(0, DefendFlagMessages.Num() - 1)];
			}
			else if (Switch == GetStatusIndex(StatusMessage::DefendFC))
			{
				if (DefendFCMessages.Num() == 0)
				{
					return EmptySpeech;
				}
				return DefendFCMessages[FMath::RandRange(0, DefendFCMessages.Num() - 1)];
			}
			else if (Switch == GetStatusIndex(StatusMessage::GetFlagBack))
			{
				if (GetFlagBackMessages.Num() == 0)
				{
					return EmptySpeech;
				}
				return GetFlagBackMessages[FMath::RandRange(0, GetFlagBackMessages.Num() - 1)];
			}
			else if (Switch == GetStatusIndex(StatusMessage::ImOnDefense))
			{
				if (ImOnDefenseMessages.Num() == 0)
				{
					return EmptySpeech;
				}
				return ImOnDefenseMessages[FMath::RandRange(0, ImOnDefenseMessages.Num() - 1)];
			}
			else if (Switch == GetStatusIndex(StatusMessage::ImOnOffense))
			{
				if (ImOnOffenseMessages.Num() == 0)
				{
					return EmptySpeech;
				}
				return ImOnOffenseMessages[FMath::RandRange(0, ImOnOffenseMessages.Num() - 1)];
			}
			else if (Switch == GetStatusIndex(StatusMessage::SpreadOut))
			{
				if (SpreadOutMessages.Num() == 0)
				{
					return EmptySpeech;
				}
				return SpreadOutMessages[FMath::RandRange(0, SpreadOutMessages.Num() - 1)];
			}
		}
		else
		{
			if (Switch == GetStatusIndex(StatusMessage::ImGoingIn))
			{
				if (ImGoingInMessages.Num() == 0)
				{
					return EmptySpeech;
				}
				return ImGoingInMessages[FMath::RandRange(0, ImGoingInMessages.Num() - 1)];
			}
			else if (Switch == GetStatusIndex(StatusMessage::BaseUnderAttack))
			{
				if (BaseUnderAttackMessages.Num() == 0)
				{
					return EmptySpeech;
				}
				return BaseUnderAttackMessages[FMath::RandRange(0, BaseUnderAttackMessages.Num() - 1)];
			}
			else if (Switch / 100 == GetStatusIndex(GameVolumeSpeechType::GV_Bridge) / 100)
			{
				return GetGVLine(BridgeLines, Switch - GetStatusIndex(GameVolumeSpeechType::GV_Bridge));
			}
			else if (Switch / 100 == GetStatusIndex(GameVolumeSpeechType::GV_River) / 100)
			{
				return GetGVLine(RiverLines, Switch - GetStatusIndex(GameVolumeSpeechType::GV_River));
			}
			else if (Switch / 100 == GetStatusIndex(GameVolumeSpeechType::GV_Antechamber) / 100)
			{
				return GetGVLine(AntechamberLines, Switch - GetStatusIndex(GameVolumeSpeechType::GV_Antechamber));
			}
			else if (Switch / 100 == GetStatusIndex(GameVolumeSpeechType::GV_ThroneRoom) / 100)
			{
				return GetGVLine(ThroneRoomLines, Switch - GetStatusIndex(GameVolumeSpeechType::GV_ThroneRoom));
			}
			else if (Switch / 100 == GetStatusIndex(GameVolumeSpeechType::GV_Courtyard) / 100)
			{
				return GetGVLine(CourtyardLines, Switch - GetStatusIndex(GameVolumeSpeechType::GV_Courtyard));
			}
			else if (Switch / 100 == GetStatusIndex(GameVolumeSpeechType::GV_Stables) / 100)
			{
				return GetGVLine(StablesLines, Switch - GetStatusIndex(GameVolumeSpeechType::GV_Stables));
			}
			else if (Switch / 100 == GetStatusIndex(GameVolumeSpeechType::GV_AntechamberHigh) / 100)
			{
				return GetGVLine(AntechamberHighLines, Switch - GetStatusIndex(GameVolumeSpeechType::GV_AntechamberHigh));
			}
			else if (Switch / 100 == GetStatusIndex(GameVolumeSpeechType::GV_Tower) / 100)
			{
				return GetGVLine(TowerLines, Switch - GetStatusIndex(GameVolumeSpeechType::GV_Tower));
			}
			else if (Switch / 100 == GetStatusIndex(GameVolumeSpeechType::GV_Creek) / 100)
			{
				return GetGVLine(CreekLines, Switch - GetStatusIndex(GameVolumeSpeechType::GV_Creek));
			}
			else if (Switch / 100 == GetStatusIndex(GameVolumeSpeechType::GV_Temple) / 100)
			{
				return GetGVLine(TempleLines, Switch - GetStatusIndex(GameVolumeSpeechType::GV_Temple));
			}
			else if (Switch / 100 == GetStatusIndex(GameVolumeSpeechType::GV_Cave) / 100)
			{
				return GetGVLine(CaveLines, Switch - GetStatusIndex(GameVolumeSpeechType::GV_Cave));
			}
			else if (Switch / 100 == GetStatusIndex(GameVolumeSpeechType::GV_BaseCamp) / 100)
			{
				return GetGVLine(BaseCampLines, Switch - GetStatusIndex(GameVolumeSpeechType::GV_BaseCamp));
			}
			else if (Switch / 100 == GetStatusIndex(PickupSpeechType::UDamagePickup) / 100)
			{
				return (Switch - GetStatusIndex(PickupSpeechType::UDamagePickup) == 0) ? UDamageAvailableLine : UDamagePickupLine;
			}
			else if (Switch / 100 == GetStatusIndex(PickupSpeechType::ShieldbeltPickup) / 100)
			{
				return (Switch - GetStatusIndex(PickupSpeechType::ShieldbeltPickup) == 0) ? ShieldbeltAvailableLine : ShieldbeltPickupLine;
			}
			else if (Switch / 100 == GetStatusIndex(PickupSpeechType::RedeemerPickup) / 100)
			{
				int32 Index = Switch - GetStatusIndex(PickupSpeechType::RedeemerPickup);
				if (Index == 2)
				{
					if (DroppedRedeemerMessages.Num() == 0)
					{
						return EmptySpeech;
					}
					return DroppedRedeemerMessages[FMath::RandRange(0, DroppedRedeemerMessages.Num() - 1)];
				}
				return (Index == 0) ? RedeemerAvailableLine : RedeemerPickupLine;
			}
			else if (Switch == GetStatusIndex(StatusMessage::EnemyRally))
			{
				return (EnemyRallyMessages.Num() == 0) ? EmptySpeech : EnemyRallyMessages[FMath::RandRange(0, EnemyRallyMessages.Num() - 1)];
			}
			else if (Switch == GetStatusIndex(StatusMessage::FindFC))
			{
				return (FindFCMessages.Num() == 0) ? EmptySpeech : FindFCMessages[FMath::RandRange(0, FindFCMessages.Num() - 1)];
			}
			else if (Switch == GetStatusIndex(StatusMessage::LastLife))
			{
				return (LastLifeMessages.Num() == 0) ? EmptySpeech : LastLifeMessages[FMath::RandRange(0, LastLifeMessages.Num() - 1)];
			}
			else if (Switch == GetStatusIndex(StatusMessage::EnemyLowLives))
			{
				return (EnemyLowLivesMessages.Num() == 0) ? EmptySpeech : EnemyLowLivesMessages[FMath::RandRange(0, EnemyLowLivesMessages.Num() - 1)];
			}
			else if (Switch == GetStatusIndex(StatusMessage::EnemyThreePlayers))
			{
				return (EnemyThreePlayersMessages.Num() == 0) ? EmptySpeech : EnemyThreePlayersMessages[FMath::RandRange(0, EnemyThreePlayersMessages.Num() - 1)];
			}
			else if (Switch == GetStatusIndex(StatusMessage::NeedRally))
			{
				return (NeedRallyMessages.Num() == 0) ? EmptySpeech : NeedRallyMessages[FMath::RandRange(0, NeedRallyMessages.Num() - 1)];
			}
			else if (Switch == GetStatusIndex(StatusMessage::NeedHealth))
			{
				return (NeedHealthMessages.Num() == 0) ? EmptySpeech : NeedHealthMessages[FMath::RandRange(0, NeedHealthMessages.Num() - 1)];
			}
		}
	}
	return EmptySpeech;
}

FCharacterSpeech UUTCharacterVoice::GetGVLine(const FGameVolumeSpeech& GVLines, int32 SwitchIndex) const
{
	FCharacterSpeech EmptySpeech;
	EmptySpeech.SpeechSound = nullptr;
	EmptySpeech.SpeechText = FText::GetEmpty();
	switch(SwitchIndex)
	{
	case 0: return (GVLines.EnemyFCSpeech.Num() > 0) ? GVLines.EnemyFCSpeech[FMath::RandRange(0, GVLines.EnemyFCSpeech.Num() - 1)] : EmptySpeech;
	case 1: return (GVLines.FriendlyFCSpeech.Num() > 0) ? GVLines.FriendlyFCSpeech[FMath::RandRange(0, GVLines.FriendlyFCSpeech.Num() - 1)] : EmptySpeech;
	case 2: return (GVLines.SecureSpeech.Num() > 0) ? GVLines.SecureSpeech[FMath::RandRange(0, GVLines.SecureSpeech.Num() - 1)] : EmptySpeech;
	case 3: return (GVLines.UndefendedSpeech.Num() > 0) ? GVLines.UndefendedSpeech[FMath::RandRange(0, GVLines.UndefendedSpeech.Num() - 1)] : EmptySpeech;
	}
	return EmptySpeech;
}

void UUTCharacterVoice::PrecacheAnnouncements_Implementation(UUTAnnouncer* Announcer) const
{
}

bool UUTCharacterVoice::ShouldPlayAnnouncement(const FClientReceiveData& ClientData) const
{
	if (ClientData.RelatedPlayerState_1 && ClientData.RelatedPlayerState_1->bIsABot && (TauntMessages.Num() > ClientData.MessageIndex))
	{
		UUTGameUserSettings* GS = Cast<UUTGameUserSettings>(GEngine->GetGameUserSettings());
		if (GS != NULL && GS->GetBotSpeech() < BSO_All)
		{
			return false;
		}
		return !Cast<AUTPlayerController>(ClientData.LocalPC) || ((AUTPlayerController*)(ClientData.LocalPC))->bHearsTaunts;
	}
	else
	{
		return true;
	}
}

bool UUTCharacterVoice::InterruptAnnouncement_Implementation(int32 Switch, const UObject* OptionalObject, TSubclassOf<UUTLocalMessage> OtherMessageClass, int32 OtherSwitch, const UObject* OtherOptionalObject) const
{
	return (GetClass() == OtherMessageClass) && (Switch >= KEY_CALLOUTS) && (OtherSwitch < KEY_CALLOUTS);
}

bool UUTCharacterVoice::CancelByAnnouncement_Implementation(int32 Switch, const UObject* OptionalObject, TSubclassOf<UUTLocalMessage> OtherMessageClass, int32 OtherSwitch, const UObject* OtherOptionalObject) const
{
	if (GetClass() == OtherMessageClass)
	{
		return (Switch < KEY_CALLOUTS) && (OtherSwitch >= KEY_CALLOUTS);
	}
	else
	{
		return (Switch < KEY_CALLOUTS);
	}
}

float UUTCharacterVoice::GetAnnouncementPriority(int32 Switch) const
{
	return (Switch >= KEY_CALLOUTS) ? 0.5f : 0.1f;
}

int32 UUTCharacterVoice::GetStatusIndex(FName NewStatus) const
{
	return StatusBaseIndex + StatusOffsets.FindRef(NewStatus);
}

