// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NPCPlayerStart.h"
#include "Components/GameStateComponent.h"
#include "NPCSpawningManagerComponent.generated.h"

/**
 * This class will control spawning bots on existing spawn points in the level
 * To be decided if we will use individual starts similar to PlayerStart,
 * Or control NPC Spawners, which will spawn bots based on their own settings
 *
 * TBC if this only controls the spawn positions... this might better suit the nature of this component structure
 * then, spawning and game design can be handled in editor
 *
 * To be event driven, this should react to events such as all NPCs/bots dying, which then checks the current total bots and determines if more waves should spawn
 *
 * This is not tied choosing player starts or the standard player start flow
 * This is simply gathering the npc spawn locations
 * And then exposing them for use elsewhere
 */
UCLASS()
class LYRAGAME_API UNPCSpawningManagerComponent : public UGameStateComponent
{
	GENERATED_BODY()
public:
	UNPCSpawningManagerComponent(const FObjectInitializer& ObjectInitializer);
	AActor* ChooseNPCStart(AController* Player);
	virtual void InitializeComponent() override;

private:

	void OnLevelAdded(ULevel* InLevel, UWorld* InWorld);
	AActor* GetFirstRandomUnoccupiedNPCStart(AController* Player, TArray<ANPCPlayerStart*> Array);
	UPROPERTY(Transient)
	TArray<TWeakObjectPtr<ANPCPlayerStart>> CachedNPCStarts;
};
