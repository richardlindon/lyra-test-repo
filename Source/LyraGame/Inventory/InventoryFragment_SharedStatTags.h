// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LyraInventoryItemDefinition.h"
#include "System/GameplayTagStack.h"
#include "InventoryFragment_SharedStatTags.generated.h"

/**
 * 
 */
UCLASS()
class LYRAGAME_API UInventoryFragment_SharedStatTags : public ULyraInventoryItemFragment
{
	GENERATED_BODY()

public:
	//For now only a single tag stack can be consumed
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=SharedStats)
	FGameplayTag ConsumedSharedStatStack;

	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly, Category=SharedStats)
	FGameplayTagContainer SharedStatTags;
};
