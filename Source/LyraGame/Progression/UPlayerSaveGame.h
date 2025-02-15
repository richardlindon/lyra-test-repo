// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerProgressionComponent.h"
#include "GameFramework/SaveGame.h"
#include "UPlayerSaveGame.generated.h"



/**
 * 
 */
UCLASS()
class LYRAGAME_API UPlayerSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// Map of class names to progression data
	UPROPERTY()
	TMap<FName, FClassProgressionData> ClassProgress;

};
