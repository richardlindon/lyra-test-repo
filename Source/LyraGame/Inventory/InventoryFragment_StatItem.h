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
	UPROPERTY(EditAnywhere, Category=Inventory)
	bool IsEquipped = false;

	UPROPERTY(EditAnywhere, Category=Inventory)
	int EquippedSlot = 0;
	
	UPROPERTY(EditDefaultsOnly, Category=Inventory)
	TSubclassOf<UGameplayEffect> GameplayEffect = nullptr;

	// Gameplay ability sets to grant when this is picked up
    UPROPERTY(EditDefaultsOnly, Category=Inventory)
    TArray<TObjectPtr<const ULyraAbilitySet>> AbilitySetsToGrant;

	//ClassTag this item relates to if granting a class. Used to display relevant class ability selection 
	UPROPERTY(EditAnywhere, Category=Inventory)
	FGameplayTag ClassGranted;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Appearance)
	FSlateBrush Brush;
};
