// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraTeamCreationComponent.h"
#include "GameModes/LyraExperienceManagerComponent.h"
#include "LyraTeamPublicInfo.h"
#include "LyraTeamPrivateInfo.h"
#include "Player/LyraPlayerState.h"
#include "Engine/World.h"
#include "GameModes/LyraGameMode.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraTeamCreationComponent)

ULyraTeamCreationComponent::ULyraTeamCreationComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PublicTeamInfoClass = ALyraTeamPublicInfo::StaticClass();
	PrivateTeamInfoClass = ALyraTeamPrivateInfo::StaticClass();
}

#if WITH_EDITOR
EDataValidationResult ULyraTeamCreationComponent::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	//@TODO: TEAMS: Validate that all display assets have the same properties set!

	return Result;
}
#endif

void ULyraTeamCreationComponent::BeginPlay()
{
	Super::BeginPlay();

	// Listen for the experience load to complete
	AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
	ULyraExperienceManagerComponent* ExperienceComponent = GameState->FindComponentByClass<ULyraExperienceManagerComponent>();
	check(ExperienceComponent);
	ExperienceComponent->CallOrRegister_OnExperienceLoaded_HighPriority(FOnLyraExperienceLoaded::FDelegate::CreateUObject(this, &ThisClass::OnExperienceLoaded));
}

void ULyraTeamCreationComponent::OnExperienceLoaded(const ULyraExperienceDefinition* Experience)
{
#if WITH_SERVER_CODE
	if (HasAuthority())
	{
		ServerCreateTeams();
		ServerAssignPlayersToTeams();
	}
#endif
}

#if WITH_SERVER_CODE

void ULyraTeamCreationComponent::ServerCreateTeams()
{
	for (const auto& KVP : TeamsToCreate)
	{
		const int32 TeamId = KVP.Key;
		ServerCreateTeam(TeamId, KVP.Value);
	}
}

void ULyraTeamCreationComponent::ServerAssignPlayersToTeams()
{
	// Assign players that already exist to teams
	AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
	for (APlayerState* PS : GameState->PlayerArray)
	{
		if (ALyraPlayerState* LyraPS = Cast<ALyraPlayerState>(PS))
		{

			// @Game-Change start skip choosing a team for NPC spawned characters with a PlayerState
			// #todo handle when the NPC has the same team as the player ID. Should we balance the number of players on both sides?
			// Skip choosing a team for the player if they already have a team
			if (const int32 TeamID = LyraPS->GetTeamId(); TeamID != INDEX_NONE) // && NPCTeamIDs.Contains(TeamID))
			{
				continue;
			}
			// @Game-Change end skip choosing a team for NPC spawned characters with a PlayerState, should be set in the NPCSpawner
			ServerChooseTeamForPlayer(LyraPS);
		}
	}

	// Listen for new players logging in
	ALyraGameMode* GameMode = Cast<ALyraGameMode>(GameState->AuthorityGameMode);
	check(GameMode);

	GameMode->OnGameModePlayerInitialized.AddUObject(this, &ThisClass::OnPlayerInitialized);
}

void ULyraTeamCreationComponent::ServerChooseTeamForPlayer(ALyraPlayerState* PS)
{
	if (PS->IsOnlyASpectator())
	{
		PS->SetGenericTeamId(FGenericTeamId::NoTeam);
	}
	else
	{
		const FGenericTeamId TeamID = IntegerToGenericTeamId(GetLeastPopulatedTeamID());
		PS->SetGenericTeamId(TeamID);
	}
}

void ULyraTeamCreationComponent::OnPlayerInitialized(AGameModeBase* GameMode, AController* NewPlayer)
{
	check(NewPlayer);
	check(NewPlayer->PlayerState);
	if (ALyraPlayerState* LyraPS = Cast<ALyraPlayerState>(NewPlayer->PlayerState))
	{
		// @Game-Change start skip choosing a team for NPC spawned characters with a PlayerState
		// #todo handle when the NPC has the same team as the player ID. Should we balance the number of players on both sides?
		// Skip choosing a team for the player if they already have a team
		if (const int32 TeamID = LyraPS->GetTeamId(); TeamID != INDEX_NONE) // && NPCTeamIDs.Contains(TeamID))
		{
			return;
		}
		// @Game-Change end skip choosing a team for NPC spawned characters with a PlayerState, should be set in the NPCSpawner

		ServerChooseTeamForPlayer(LyraPS);
	}
}

void ULyraTeamCreationComponent::ServerCreateTeam(int32 TeamId, ULyraTeamDisplayAsset* DisplayAsset)
{
	check(HasAuthority());

	//@TODO: ensure the team doesn't already exist

	UWorld* World = GetWorld();
	check(World);

	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ALyraTeamPublicInfo* NewTeamPublicInfo = World->SpawnActor<ALyraTeamPublicInfo>(PublicTeamInfoClass, SpawnInfo);
	checkf(NewTeamPublicInfo != nullptr, TEXT("Failed to create public team actor from class %s"), *GetPathNameSafe(*PublicTeamInfoClass));
	NewTeamPublicInfo->SetTeamId(TeamId);
	// @Game-Change Set Is NPC only team so we can access that info after creating the teams, during gameplay
	NewTeamPublicInfo->SetIsNPCOnlyTeam(NPCTeamIDs.Contains(TeamId));
	NewTeamPublicInfo->SetTeamDisplayAsset(DisplayAsset);

	ALyraTeamPrivateInfo* NewTeamPrivateInfo = World->SpawnActor<ALyraTeamPrivateInfo>(PrivateTeamInfoClass, SpawnInfo);
	checkf(NewTeamPrivateInfo != nullptr, TEXT("Failed to create private team actor from class %s"), *GetPathNameSafe(*PrivateTeamInfoClass));
	NewTeamPrivateInfo->SetTeamId(TeamId);
}

int32 ULyraTeamCreationComponent::GetLeastPopulatedTeamID() const
{
	// @Game-Change don't consider NPC teams when choosing the least populatedTeamID
	const int32 NumTeams = UseNPCTeams ? TeamsToCreate.Num() - NPCTeamIDs.Num() : TeamsToCreate.Num();
	if (NumTeams > 0)
	{
		TMap<int32, uint32> TeamMemberCounts;
		TeamMemberCounts.Reserve(NumTeams);
		for (const auto& KVP : TeamsToCreate)
		{
			const int32 TeamId = KVP.Key;
			// @Game-Change don't consider NPC teams when choosing the least populatedTeamID
			if (NPCTeamIDs.Contains(TeamId)) continue; // skip the npc teams
			TeamMemberCounts.Add(TeamId, 0);
		}

		AGameStateBase* GameState = GetGameStateChecked<AGameStateBase>();
		for (APlayerState* PS : GameState->PlayerArray)
		{
			if (ALyraPlayerState* LyraPS = Cast<ALyraPlayerState>(PS))
			{
				const int32 PlayerTeamID = LyraPS->GetTeamId();

				// @Game-Change don't consider NPC teams when choosing the least populatedTeamID
				if (NPCTeamIDs.Contains(PlayerTeamID)) continue; // skip the npc teams

				if ((PlayerTeamID != INDEX_NONE) && !LyraPS->IsInactive())
					// do not count unassigned or disconnected players
				{
					check(TeamMemberCounts.Contains(PlayerTeamID))
					TeamMemberCounts[PlayerTeamID] += 1;
				}
			}
		}

		// sort by lowest team population, then by team ID
		int32 BestTeamId = INDEX_NONE;
		uint32 BestPlayerCount = TNumericLimits<uint32>::Max();
		for (const auto& KVP : TeamMemberCounts)
		{
			const int32 TestTeamId = KVP.Key;
			const uint32 TestTeamPlayerCount = KVP.Value;

			if (TestTeamPlayerCount < BestPlayerCount)
			{
				BestTeamId = TestTeamId;
				BestPlayerCount = TestTeamPlayerCount;
			}
			else if (TestTeamPlayerCount == BestPlayerCount)
			{
				if ((TestTeamId < BestTeamId) || (BestTeamId == INDEX_NONE))
				{
					BestTeamId = TestTeamId;
					BestPlayerCount = TestTeamPlayerCount;
				}
			}
		}

		return BestTeamId;
	}

	return INDEX_NONE;
}
#endif	// WITH_SERVER_CODE

