// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/LyraAbilitySet.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Components/ActorComponent.h"
#include "HeroClassManagerComponent.generated.h"

/**
 * @TODO: No idea if i need replication 
 */
UCLASS(BlueprintType)
class LYRAGAME_API UHeroClassManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHeroClassManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Function to swap to a new ability set
	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void SwapAbilitySet(ULyraAbilitySet* NewAbilitySet, ULyraAbilitySystemComponent* ASC);

	UFUNCTION(BlueprintCallable, Category = "Abilities")
	void RemoveAbilitySet(ULyraAbilitySystemComponent* ASC);
	
protected:

	FLyraAbilitySet_GrantedHandles GrantedAbilitySetHandles;

};
