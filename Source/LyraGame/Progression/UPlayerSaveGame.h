// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/SaveGame.h"
#include "UPlayerSaveGame.generated.h"


struct FClassProgressionEntry;

USTRUCT(BlueprintType)
struct FClassProgressionSaveEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category="Progression")
	FGameplayTag ClassTag;

	UPROPERTY(BlueprintReadWrite, Category="Progression")
	int32 Level = 0;

	UPROPERTY(BlueprintReadWrite, Category="Progression")
	int32 Experience = 0;

};

/**
 * 
 */
UCLASS()
class LYRAGAME_API UPlayerSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// The unique save slot name
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category="SaveGame")
	FString SaveSlotName = TEXT("PlayerClassesSaveSlot");
	
	UPROPERTY(BlueprintReadWrite, Category="Progression")
	TArray<FClassProgressionSaveEntry> SavedProgression;

};
