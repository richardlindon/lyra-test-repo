// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MovementSetComponent.h"
#include "LyraLogChannels.h"

#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/MovementSet.h"
#include "Net/UnrealNetwork.h"

UMovementSetComponent::UMovementSetComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);

	AbilitySystemComponent = nullptr;
	MovementSet = nullptr;
}

// void UMovementSetComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
// {
// 	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
//
// 	DOREPLIFETIME(UMovementSetComponent, MovementSet);
//
// }

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
	if (MovementSet)
	{
		return MovementSet->GetMoveSpeed(); 
	}
	
	return 600.0f;
	
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