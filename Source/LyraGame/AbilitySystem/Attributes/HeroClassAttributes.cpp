// Copyright Epic Games, Inc. All Rights Reserved.

#include "HeroClassAttributes.h"

#include "AbilitySystem/Attributes/LyraAttributeSet.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(HeroClassAttributes)

class FLifetimeProperty;


UHeroClassAttributes::UHeroClassAttributes()
	: Strength(0.0f)
	, Intelligence(0.0f)
	, Dexterity(0.0f)
{
}

void UHeroClassAttributes::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UHeroClassAttributes, Strength, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHeroClassAttributes, Intelligence, COND_OwnerOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UHeroClassAttributes, Dexterity, COND_OwnerOnly, REPNOTIFY_Always);
}

void UHeroClassAttributes::OnRep_Strength(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHeroClassAttributes, Strength, OldValue);
}

void UHeroClassAttributes::OnRep_Intelligence(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHeroClassAttributes, Intelligence, OldValue);
}

void UHeroClassAttributes::OnRep_Dexterity(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UHeroClassAttributes, Dexterity, OldValue);
}

