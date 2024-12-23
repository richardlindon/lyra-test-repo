// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Attributes/MovementSet.h"

#include "GameplayEffectExtension.h"
#include "LyraGameplayTags.h"
#include "LyraHealthSet.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MovementSet)


UMovementSet::UMovementSet()
	: MoveSpeed(400.0f)
{
}

void UMovementSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(UMovementSet, MoveSpeed, COND_None, REPNOTIFY_Always);
}


void UMovementSet::OnRep_MoveSpeed(const FGameplayAttributeData& OldValue)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(UMovementSet, MoveSpeed, OldValue);
}

bool UMovementSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	if (!Super::PreGameplayEffectExecute(Data))
	{
		return false;
	}

	// Handle modifying incoming normal damage
	if (Data.EvaluatedData.Attribute == GetMoveSpeedAttribute())
	{
		
#if !UE_BUILD_SHIPPING
		// Check GodMode cheat, unlimited health is checked below
		if (Data.Target.HasMatchingGameplayTag(LyraGameplayTags::Cheat_GodMode))
		{
			// Do not take away any MoveSpeed.
			Data.EvaluatedData.Magnitude = 0.0f;
			return false;
		}
#endif // #if !UE_BUILD_SHIPPING
		
	}

	return true;
}

void UMovementSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void UMovementSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	ClampAttribute(Attribute, NewValue);
}

void UMovementSet::ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetMoveSpeedAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, 2000);
	}
	
}
