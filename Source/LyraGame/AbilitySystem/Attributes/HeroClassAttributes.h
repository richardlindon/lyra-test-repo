// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "AbilitySystemComponent.h"
#include "LyraAttributeSet.h"

#include "HeroClassAttributes.generated.h"

class UObject;
struct FFrame;


/**
 * UHeroClassAttributes
 * 
 * Class that defines traditional class based attributes relating to class roles, such as strength, intelligence, dexterity
 */
UCLASS(BlueprintType)
class UHeroClassAttributes : public ULyraAttributeSet
{
	GENERATED_BODY()

public:

	UHeroClassAttributes();

	ATTRIBUTE_ACCESSORS(UHeroClassAttributes, Strength);
	ATTRIBUTE_ACCESSORS(UHeroClassAttributes, Intelligence);
	ATTRIBUTE_ACCESSORS(UHeroClassAttributes, Dexterity);


	
protected:

	UFUNCTION()
	void OnRep_Strength(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_Intelligence(const FGameplayAttributeData& OldValue);
	
	UFUNCTION()
	void OnRep_Dexterity(const FGameplayAttributeData& OldValue);

private:

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Strength, Category = "Lyra|Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Strength;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Intelligence, Category = "Lyra|Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Intelligence;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Dexterity, Category = "Lyra|Attributes", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Dexterity;
	
};
