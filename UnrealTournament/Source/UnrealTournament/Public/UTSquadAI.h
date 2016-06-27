// Squad AI contains gametype and role specific AI for bots
// for example, an attacker in CTF gets a different Squad than a defender in CTF
// which is different from a defender in Warfare
// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.
#pragma once

#include "UTBot.h"
#include "UTTeamInfo.h"

#include "UTSquadAI.generated.h"

struct UNREALTOURNAMENT_API FSuperPickupEval : public FBestInventoryEval
{
	/** threshold to consider a pickup sufficiently "super" */
	float MinDesireability;
	/** list of pickups to ignore because a teammate has a claim on them */
	TArray<AActor*> ClaimedPickups;

	virtual bool AllowPickup(APawn* Asker, AActor* Pickup, float Desireability, float PickupDist);

	FSuperPickupEval(float InPredictionTime, float InMoveSpeed, int32 InMaxDist = 0, float InMinDesireability = 1.0f, const TArray<AActor*>& InClaimedPickups = TArray<AActor*>(), AActor* InPrevGoal = NULL)
		: FBestInventoryEval(InPredictionTime, InMoveSpeed, InMaxDist), MinDesireability(InMinDesireability), ClaimedPickups(InClaimedPickups)
	{
		PrevGoal = InPrevGoal;
	}
};

USTRUCT()
struct FAlternateRoute
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	TArray<FRouteCacheItem> RouteCache;

	FAlternateRoute() = default;
	FAlternateRoute(const TArray<FRouteCacheItem>& InRoute)
		: RouteCache(InRoute)
	{}
};

struct FPredictedGoal
{
	FVector Location;
	/** indicates this is an active, immediately relevant game goal (assume target is pursuing it even if can't see him/her around it)
	 * for example, in CTF this is true for the flag stand only when the flag is home
	 */
	bool bCritical;

	FPredictedGoal()
	{}
	FPredictedGoal(const FVector& InLoc, bool bInCrit)
		: Location(InLoc), bCritical(bInCrit)
	{}

	bool operator== (const FPredictedGoal& Other) const
	{
		return bCritical == Other.bCritical && (Location - Other.Location).IsNearlyZero();
	}
};

extern FName NAME_Attack;
extern FName NAME_Defend;

UCLASS(NotPlaceable)
class UNREALTOURNAMENT_API AUTSquadAI : public AInfo, public IUTTeamInterface
{
	GENERATED_UCLASS_BODY()

	friend class AUTTeamInfo;

	/** team this squad is on (may be NULL) */
	UPROPERTY(BlueprintReadOnly, Category = Squad)
	AUTTeamInfo* Team;
	/** squad role/orders */
	UPROPERTY(BlueprintReadOnly, Category = Squad)
	FName Orders;
protected:
	/** cached pointer to navigation data */
	AUTRecastNavMesh* NavData;

	/** list of squad members */
	UPROPERTY(BlueprintReadOnly, Category = Squad)
	TArray<AController*> Members;
	/** leader (prefer to stay near this player when objectives permit)*/
	UPROPERTY(BlueprintReadOnly, Category = Squad)
	AController* Leader;
	/** squad objective (target to attack, defend, etc) - may be NULL */
	UPROPERTY(BlueprintReadOnly, Category = Squad)
	AActor* Objective;
	/** precast to AUTGameObjective for convenience */
	UPROPERTY(BlueprintReadOnly, Category = Squad)
	AUTGameObjective* GameObjective;
	/** possible paths to our current objective; each increasing element in the array is generated by weighting against the nodes in the previous elements */
	UPROPERTY()
	TArray<FAlternateRoute> SquadRoutes;
	/** maximum number of SquadRoutes (iterations) to store */
	int32 MaxSquadRoutes;
	/** current index into SquadRoutes array that bots should use (for coordination)
	 * note that bots following a different route when this value changes will generally continue to do so until they die or their objective changes, so they don't end up getting confused
	 */
	int32 CurrentSquadRouteIndex;
public:

	inline AController* GetLeader() const
	{
		return Leader;
	}
	inline int32 GetSize() const
	{
		return Members.Num();
	}
	inline AActor* GetObjective() const
	{
		return Objective;
	}
	inline AUTGameObjective* GetGameObjective() const
	{
		return GameObjective;
	}

	virtual uint8 GetTeamNum() const override
	{
		return (Team != NULL) ? Team->TeamIndex : 255;
	}
	// not applicable
	virtual void SetTeamForSideSwap_Implementation(uint8 NewTeamNum) override
	{}

	virtual void BeginPlay() override
	{
		NavData = GetUTNavData(GetWorld());
		Super::BeginPlay();
	}

	virtual void EndPlay(const EEndPlayReason::Type Reason) override
	{
		Super::EndPlay(Reason);

		for (int32 i = Members.Num() - 1; i >= 0; i--)
		{
			RemoveMember(Members[i]);
		}
		if (Team != NULL)
		{
			Team->RemoveSquad(this);
		}
	}

	virtual void Initialize(AUTTeamInfo* InTeam, FName InOrders)
	{
		Team = InTeam;
		Orders = InOrders;
	}

	virtual void SetObjective(AActor* InObjective)
	{
		if (InObjective != Objective)
		{
			Objective = InObjective;
			GameObjective = Cast<AUTGameObjective>(InObjective);
			SquadRoutes.Empty();
			CurrentSquadRouteIndex = INDEX_NONE;
			for (AController* C : Members)
			{
				AUTBot* B = Cast<AUTBot>(C);
				if (B != NULL)
				{
					B->UsingSquadRouteIndex = INDEX_NONE;
					B->bDisableSquadRoutes = false;
					B->SquadRouteGoal.Clear();
					B->WhatToDoNext();
				}
			}
		}
	}

	virtual void AddMember(AController* C);
	virtual void RemoveMember(AController* C);
	virtual void SetLeader(AController* NewLeader);

	/** attempt to follow one of the passed in alternate routes to Goal (may generate new if this bot is allowed to do so) */
	virtual bool FollowAlternateRoute(AUTBot* B, AActor* Goal, TArray<FAlternateRoute>& Routes, bool bAllowDetours, const FString& SuccessGoalString);
	/** try to find and start traversing path towards given objective, possibly taking into account squad alternate routes */
	virtual bool TryPathTowardObjective(AUTBot* B, AActor* Goal, bool bAllowDetours, const FString& SuccessGoalString);

	/** @return if enemy is important to track for as long as possible (e.g. threatening game objective) */
	virtual bool MustKeepEnemy(APawn* TheEnemy)
	{
		return false;
	}

	/** @return if allowed to use translocator */
	virtual bool AllowTranslocator(AUTBot* B)
	{
		return true;
	}

	/** allows squad to choose spawn point for bot when selection is possible. Return NULL to use default AI logic. */
	virtual APlayerStart* PickSpawnPointFor(AUTBot* B, const TArray<APlayerStart*>& Choices)
	{
		return NULL;
	}

	/** called when bot lost track of enemy and wants a new one. Assigning one is optional.
	 * @return whether a new enemy was assigned
	 */
	virtual bool LostEnemy(AUTBot* B);

	/** @return modified rating for enemy, taking into account objectives */
	virtual float ModifyEnemyRating(float CurrentRating, const FBotEnemyInfo& EnemyInfo, AUTBot* B)
	{
		return CurrentRating;
	}

	/** modify the bot's attack aggressiveness, generally in response to its target's relevance to game objectives */
	virtual void ModifyAggression(AUTBot* B, float& Aggressiveness)
	{
		if (MustKeepEnemy(B->GetEnemy()))
		{
			Aggressiveness += 0.5f;
			const FBotEnemyInfo* Info = B->GetEnemyInfo(B->GetEnemy(), true);
			if (Info != NULL)
			{
				// if enemy is closer to objective than the one bot is supposed to be preventing the enemy from reaching, up aggressiveness to increase liklihood of chasing
				TArray<FPredictedGoal> Goals;
				GetPossibleEnemyGoals(B, Info, Goals);
				for (const FPredictedGoal& TestGoal : Goals)
				{
					if (TestGoal.bCritical && (TestGoal.Location - B->GetPawn()->GetActorLocation()).SizeSquared() > (TestGoal.Location - Info->LastKnownLoc).SizeSquared())
					{
						Aggressiveness += 0.5f;
						break;
					}
				}
			}
		}
	}

	/** return current orders for this bot
	 * generally just returns Orders, but for certain squads (e.g. freelance) this may change based on the bot's state (for example, when stacked switch to attacking)
	 */
	virtual FName GetCurrentOrders(AUTBot* B)
	{
		return Orders;
	}

	virtual bool IsDefending(AUTBot* B)
	{
		return GetCurrentOrders(B) == NAME_Defend || B->GetDefensePoint() != NULL;
	}

	/** set an appropriate defense point for the bot to defend the current objective, if one is available */
	virtual void SetDefensePointFor(AUTBot* B);

	/** return true if the bot B has a better claim on the given pickup than the current claiming pawn (so this bot may consider taking it instead) */
	virtual bool HasBetterPickupClaim(AUTBot* B, const FPickupClaim& Claim);

	/** checks for any super pickups (powerups, strong armor, etc) that the AI should focus on above any game objectives right now (or AS the game objectives, such as in DM/TDM)
	 * also handles the possibility of teammate(s) already headed to a particular pickup so this bot shouldn't consider it
	 */
	virtual bool CheckSuperPickups(AUTBot* B, int32 MaxDist);

	/** called by bot during its decision logic to see if there's an action relating to the game's objectives it should take
	 * @return if an action was assigned
	 */
	virtual bool CheckSquadObjectives(AUTBot* B);

	/** called in bot fighting logic to ask for destination when bot wants to retreat
	 * should set MoveTarget to desired destination but don't set action
	 */
	virtual bool PickRetreatDestination(AUTBot* B);

	/** @return whether bot should try to use translocator to follow path faster (if available, skilled enough, and legal for game type) */
	virtual bool ShouldUseTranslocator(AUTBot* B);

	/** set timer to retask bot, partially just to stagger updates and partially to account for their reaction time */
	virtual void SetRetaskTimer(AUTBot* B)
	{
		SetTimerUFunc(B, FName(TEXT("WhatToDoNext")), 0.1f + 0.15f * FMath::FRand() + FMath::Max<float>(0.0f, (0.5f - 0.5f * B->Personality.ReactionTime)) * FMath::FRand(), false);
	}

	/** notifies AI of some game objective related event (e.g. flag taken)
	 * generally bots will get retasked if they were performing some action relating to the object that is no longer relevant
	 */
	virtual void NotifyObjectiveEvent(AActor* InObjective, AController* InstigatedBy, FName EventName);

	/** return whether the given bot should consider the squad objective as higher than normal priority and minimize unnecessary detours
	 * (e.g. in CTF when flag is out)
	 */
	virtual bool HasHighPriorityObjective(AUTBot* B)
	{
		return false;
	}

	/** pick potential goal locations enemy might want to go to, used by bot hunting/tracking logic */
	virtual void GetPossibleEnemyGoals(AUTBot* B, const FBotEnemyInfo* EnemyInfo, TArray<FPredictedGoal>& Goals);

	/** debug drawing of squad route for B, if applicable */
	virtual void DrawDebugSquadRoute(AUTBot* B) const
	{
		if (SquadRoutes.IsValidIndex(B->UsingSquadRouteIndex))
		{
			DrawDebugRoute(GetWorld(), B->GetPawn(), SquadRoutes[B->UsingSquadRouteIndex].RouteCache);
		}
	}
};