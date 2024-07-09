// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MovementSetComponent.h"
#include "LyraLogChannels.h"

#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/MovementSet.h"

UMovementSetComponent::UMovementSetComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);

	AbilitySystemComponent = nullptr;
	MovementSet = nullptr;
}

void UMovementSetComponent::InitializeWithAbilitySystem(ULyraAbilitySystemComponent* InASC)
{
	AActor* Owner = GetOwner();
	check(Owner);

	if (AbilitySystemComponent)
	{
		UE_LOG(LogLyra, Error, TEXT("MovementSetComponent: Movement component for owner [%s] has already been initialized with an ability system."), *GetNameSafe(Owner));
		return;
	}

	AbilitySystemComponent = InASC;
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogLyra, Error, TEXT("MovementSetComponent: Cannot initialize Movement component for owner [%s] with NULL ability system."), *GetNameSafe(Owner));
		return;
	}

	MovementSet = AbilitySystemComponent->GetSet<UMovementSet>();
	if (!MovementSet)
	{
		UE_LOG(LogLyra, Error, TEXT("MovementSetComponent: Cannot initialize Movement component for owner [%s] with NULL Movement set on the ability system."), *GetNameSafe(Owner));
		return;
	}

	// Register to listen for attribute changes.
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UMovementSet::GetMoveSpeedAttribute()).AddUObject(this, &ThisClass::HandleMoveSpeedChanged);
}

void UMovementSetComponent::UninitializeFromAbilitySystem()
{
	MovementSet = nullptr;
	AbilitySystemComponent = nullptr;
}

float UMovementSetComponent::GetMoveSpeed() const
{
	return (MovementSet ? MovementSet->GetMoveSpeed() : 0.0f);
}

void UMovementSetComponent::OnUnregister()
{
	UninitializeFromAbilitySystem();

	Super::OnUnregister();
}

void UMovementSetComponent::HandleMoveSpeedChanged(const FOnAttributeChangeData& ChangeData)
{
	OnMoveSpeedChanged.Broadcast(this, ChangeData.OldValue, ChangeData.NewValue);
}