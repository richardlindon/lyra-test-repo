// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HeroClassData.h"
#include "Engine/DataAsset.h"
#include "HeroClassListData.generated.h"

/**
 * Not sure if i need this yet, but whateva
 */
UCLASS()
class LYRAGAME_API UHeroClassListData : public UDataAsset
{
	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hero Classes")
	TArray<TObjectPtr<UHeroClassData>> HeroClasses;

	// Function to get all unique IDs from the hero classes
	TArray<FString> GetAllHeroClassIDs(); 
};
