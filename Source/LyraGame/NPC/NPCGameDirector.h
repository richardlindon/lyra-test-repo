// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Character/LyraPawnData.h"
#include "Components/GameStateComponent.h"
#include "NPCGameDirector.generated.h"

UENUM()
enum class EWavePhaseType : uint8
{
	WaveReset = 0,

	WaveBegun,

	WaveSpawningCompleted,
	
	WaveCompleted,

	AllWavesCompleted
};

/** A message when a wave begins or ends */
USTRUCT(BlueprintType)
struct FWaveChangeMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	EWavePhaseType WavePhase = EWavePhaseType::WaveBegun;
	
	UPROPERTY(BlueprintReadOnly, Category="Wave")
	int32 WaveNumber = 0;

	UPROPERTY(BlueprintReadOnly, Category="Wave")
	bool IsWaveActive = false;

	UPROPERTY(BlueprintReadOnly, Category="Wave")
	bool IsWavesCompleted = false;

	UPROPERTY(BlueprintReadOnly, Category="Wave")
	bool IsCurrentWaveSpawningCompleted = false;
	
};

/**
 * Game director handles spawning NPCs for the current wave
 * It should accept properties
 * - max number alive at a time (NPCs to spawn)
 * - spawn rate (spawns per second, to curb overwhelming players with 50 at once etc)
 * - respawn time
 * - team settings for NPCs
 * - NPC types available to spawn (struct)
 *		- their rarity for spawning
 *		- flags for particular spawn points to use (player/npc spawns can have tags?)
 *		- max alive
 *		- spawn rate
 *
 * Post prototype:
 * - evenly distributed across spawn groups
 * - spawn based on players triggering waypoints/volumes
 *
 * Should probably control this via gameplay in editor
 * Expose public editor functions to begin wave, adjust current settings, end wave,
 *
 * TBC if in class:
 * - how many have died this wave (in editor? gameplay/gamestate stack?)
 * 
 *
 * NPCs themselves will handle any checks for lost NPCs to ensure they don't get stuck on the map
 * 
 */
UCLASS()
class LYRAGAME_API UNPCGameDirector : public UGameStateComponent
{
	GENERATED_BODY()

private:
	void RetryChoosePawnStart(APawn* NewPawn);

public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Pawn)
	TSoftObjectPtr<ULyraPawnData> PawnData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	FGenericTeamId TeamID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	UBehaviorTree* BehaviorTree;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Game Director")
	void BeginWave(int32 NewWave = -1, int32 NewSpawnsPerWave = -1, int32 NewMaxSpawnsPerTick = -1);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Wave)
	void ResetWaves();
	// void OnExperienceLoaded(const ULyraExperienceDefinition* Experience);

	UPROPERTY(BlueprintReadOnly, Category=Wave)
	bool IsWaveActive = false;

	UPROPERTY(BlueprintReadOnly, Category=Wave)
	bool IsWavesCompleted = false;

	UPROPERTY(BlueprintReadOnly, Category=Wave)
	bool IsCurrentWaveSpawningCompleted = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Wave, meta=(UIMin=0))
	int32 CurrentWave = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Wave, meta=(UIMin=1))
	int32 TotalWaves = 10;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Spawn, meta=(UIMin=1))
	int32 SpawnsPerWave = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Spawn, meta=(UIMin=0))
	float SpawnCheckTimerSeconds = 0.5f;
		
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Spawn)
	int32 MaxSpawnsPerTick = 5;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=Spawn)
	int32 MaxAliveAtOnce = 10;

	int32 NumSpawnedThisWave = 0;
	
	UPROPERTY(EditAnywhere, Category=Gameplay)
	TSubclassOf<AAIController> ControllerClass;

	UPROPERTY(Transient)
	TArray<TObjectPtr<APawn>> SpawnedNPCList;

	virtual void ServerCreateNPCs();
	void CheckSpawningCompleted();
	void CheckWaveCompleted();
	void FinishWaves();

	void EndWave();
	
	void BroadcastWaveMessage() const;
	
	AActor* ChoosePawnStart(APawn* NewPawn);

	void StartSpawningProcess(APawn* NewPawn);

	void FinishSpawningProcess(APawn* NewPawn, FTransform SpawnTransform);

	APawn* SpawnAIFromClass(UObject* WorldContextObject, ULyraPawnData* LoadedPawnData, UBehaviorTree* BehaviorTreeToRun, TSubclassOf<AAIController> ControllerClassToSpawn);
	
	UFUNCTION()
	void OnSpawnedPawnDestroyed(AActor* DestroyedActor);

	virtual void SpawnOneNPC();
	void LogCurrentNPCCount() const;
};