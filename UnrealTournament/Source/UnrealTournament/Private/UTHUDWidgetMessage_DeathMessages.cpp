// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "UnrealTournament.h"
#include "UTLocalMessage.h"
#include "UTHUDWidgetMessage.h"
#include "UTHUDWidgetMessage_DeathMessages.h"
#include "UTVictimMessage.h"

UUTHUDWidgetMessage_DeathMessages::UUTHUDWidgetMessage_DeathMessages(const class FPostConstructInitializeProperties& PCIP) : Super(PCIP)
{
	ManagedMessageArea = FName(TEXT("DeathMessage"));
	Position = FVector2D(0.0f, 0.0f);			
	ScreenPosition = FVector2D(0.5f, 0.35f);
	Size = FVector2D(0.0f, 0.0f);			
	Origin = FVector2D(0.5f, 0.0f);				

	FadeTime = 1.0;

	MessageColor = FLinearColor::Red;

	static ConstructorHelpers::FObjectFinder<UFont> Font(TEXT("Font'/Game/RestrictedAssets/Fonts/fntAmbex36.fntAmbex36'"));
	MessageFont = Font.Object;
}

void UUTHUDWidgetMessage_DeathMessages::DrawMessages(float DeltaTime)
{
	Canvas->Reset();
	float Y = 0;
	int DrawCnt=0;
	for (int MessageIndex = MessageQueue.Num() - 1; MessageIndex >= 0 && DrawCnt < 2; MessageIndex--)
	{
		if (MessageQueue[MessageIndex].MessageClass != NULL)	
		{
			DrawMessage(MessageIndex, 0, Y);
			Y += MessageQueue[MessageIndex].TextHeight;
			DrawCnt++;
		}
	}
}

void UUTHUDWidgetMessage_DeathMessages::LayoutMessage(int32 QueueIndex, TSubclassOf<class UUTLocalMessage> MessageClass, uint32 MessageIndex, FText LocalMessageText, int32 MessageCount, APlayerState* RelatedPlayerState_1, APlayerState* RelatedPlayerState_2, UObject* OptionalObject)
{
	MessageQueue[QueueIndex].DisplayFont = MessageFont == NULL ? GEngine->GetSmallFont() : MessageFont;
	MessageQueue[QueueIndex].OptionalObject = OptionalObject;

	FName MessageStyle = MessageClass->GetDefaultObject<UUTLocalMessage>()->StyleTag;

	if ( MessageStyle == FName(TEXT("Spree")))
	{
		MessageQueue[QueueIndex].DrawColor = FLinearColor::Yellow;
	}
	else if (MessageStyle == FName(TEXT("Victim")))
	{
		MessageQueue[QueueIndex].DrawColor = FLinearColor::Red;
	}
	else
	{
		MessageQueue[QueueIndex].DrawColor = FLinearColor::White;
	}
}
