// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "LyraAttributeSet.h"
#include "NativeGameplayTags.h"

#include "ResourceSet.generated.h"


/**
 * 
 */
UCLASS(BlueprintType)
class UManaSet : public ULyraAttributeSet
{
	GENERATED_BODY()
	
	UManaSet();

public:

	ATTRIBUTE_ACCESSORS(UManaSet, Mana);
	ATTRIBUTE_ACCESSORS(UManaSet, MaxMana);

protected:

	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldValue);

	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;

private:

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Mana, Category = "Lyra|Mana", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Mana;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxMana, Category = "Lyra|Mana", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxMana;
};
