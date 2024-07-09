// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/LyraAttributeSet.h"
#include "MovementSet.generated.h"


/**
 * 
 */
UCLASS(BlueprintType)
class UMovementSet : public ULyraAttributeSet
{
	GENERATED_BODY()
	
	UMovementSet();

public:

	ATTRIBUTE_ACCESSORS(UMovementSet, MoveSpeed);

protected:

	UFUNCTION()
	void OnRep_MoveSpeed(const FGameplayAttributeData& OldValue);

	virtual bool PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data) override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	void ClampAttribute(const FGameplayAttribute& Attribute, float& NewValue) const;

private:

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MoveSpeed, Category = "Lyra|MoveSpeed", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MoveSpeed;

};
