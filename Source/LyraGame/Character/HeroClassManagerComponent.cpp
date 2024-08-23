// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/HeroClassManagerComponent.h"

// Sets default values for this component's properties
UHeroClassManagerComponent::UHeroClassManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UHeroClassManagerComponent::SwapAbilitySet(ULyraAbilitySet* NewAbilitySet, ULyraAbilitySystemComponent* ASC)
{
	// Remove the previously granted abilities

	if (ASC && NewAbilitySet)
	{
		GrantedAbilitySetHandles.TakeFromAbilitySystem(ASC);
		
		NewAbilitySet->GiveToAbilitySystem(ASC, &GrantedAbilitySetHandles, this);
	}
}

void UHeroClassManagerComponent::RemoveAbilitySet(ULyraAbilitySystemComponent* ASC)
{
	if (ASC)
	{
		GrantedAbilitySetHandles.TakeFromAbilitySystem(ASC);
	}
}

