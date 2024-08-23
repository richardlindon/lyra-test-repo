// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "Inventory/LyraInventoryItemDefinition.h"
#include "InventoryFragment_StatItem.generated.h"

class ULyraAbilitySet;

/**
 * 
 */
UCLASS()
class LYRAGAME_API UInventoryFragment_StatItem : public ULyraInventoryItemFragment
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category=Lyra)
	bool IsEquipped = false;

	UPROPERTY(EditAnywhere, Category=Lyra)
	int EquippedSlot = 0;
	
	UPROPERTY(EditDefaultsOnly, Category=Lyra);
	TSubclassOf<UGameplayEffect> GameplayEffect = nullptr;

	// Gameplay ability sets to grant when this is picked up
    UPROPERTY(EditDefaultsOnly, Category=Lyra)
    TArray<TObjectPtr<const ULyraAbilitySet>> AbilitySetsToGrant;

	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	FSlateBrush Brush;
};
