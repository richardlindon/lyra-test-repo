// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LyraInventoryItemDefinition.h"
#include "InventoryFragment_Currency.generated.h"

/**
 * 
 */
UCLASS()
class LYRAGAME_API UInventoryFragment_Currency : public ULyraInventoryItemFragment
{
	GENERATED_BODY()

public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item")
	int32 ItemCost = 1;
};
