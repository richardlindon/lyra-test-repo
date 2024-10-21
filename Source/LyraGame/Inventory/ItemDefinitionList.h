// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LyraInventoryItemDefinition.h"
#include "Engine/DataAsset.h"
#include "ItemDefinitionList.generated.h"

USTRUCT(BlueprintType)
struct FItemListStruct
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<ULyraInventoryItemDefinition>> Items;
};


/**
 * 
 */
UCLASS()
class LYRAGAME_API UItemDefinitionList : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item list")
	FItemListStruct ItemList;

	UFUNCTION(BlueprintCallable, Category = "Item list")
	TArray<TSubclassOf<ULyraInventoryItemDefinition>> GetItems();
};

