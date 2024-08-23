// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/GameFrameworkComponent.h"
#include "MovementSetComponent.generated.h"


struct FOnAttributeChangeData;
class UMovementSet;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FMovement_AttributeChanged, UMovementSetComponent*, MovementComponent, float,
	OldValue, float, NewValue);

class ULyraAbilitySystemComponent;
/**
 *
 */
UCLASS()
class LYRAGAME_API UMovementSetComponent : public UGameFrameworkComponent
{
	GENERATED_BODY()

public:
	UMovementSetComponent(const FObjectInitializer& ObjectInitializer);

	// Returns the Movement component if one exists on the specified actor.
	UFUNCTION(BlueprintPure, Category = "Lyra|Movement")
	static UMovementSetComponent* FindMovementComponent(const AActor* Actor) { return (Actor ? Actor->FindComponentByClass<UMovementSetComponent>() : nullptr); }

	// Initialize the component using an ability system component.
	UFUNCTION(BlueprintCallable, Category = "Lyra|Movement")
	void InitializeWithAbilitySystem(ULyraAbilitySystemComponent* InASC);

	// Uninitialize the component, clearing any references to the ability system.
	UFUNCTION(BlueprintCallable, Category = "Lyra|Movement")
	void UninitializeFromAbilitySystem();

	// Returns the current Movement value.
	UFUNCTION(BlueprintCallable, Category = "Lyra|Movement")
	float GetMoveSpeed() const;

public:

	// Delegate fired when the Movement value has changed.
	UPROPERTY(BlueprintAssignable)
	FMovement_AttributeChanged OnMoveSpeedChanged;

protected:
	virtual void OnUnregister() override;

	virtual void HandleMoveSpeedChanged(const FOnAttributeChangeData& ChangeData);


	// Ability system used by this component.
	UPROPERTY()
	TObjectPtr<ULyraAbilitySystemComponent> AbilitySystemComponent;

	// Movement set used by this component.
	UPROPERTY()
	TObjectPtr<const UMovementSet> MovementSet;
};