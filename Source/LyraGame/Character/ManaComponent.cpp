// Fill out your copyright notice in the Description page of Project Settings.

#include "Character/ManaComponent.h"
#include "LyraLogChannels.h"

#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "AbilitySystem/Attributes/ManaSet.h"

UManaComponent::UManaComponent(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.bCanEverTick = false;

	SetIsReplicatedByDefault(true);

	AbilitySystemComponent = nullptr;
	ManaSet = nullptr;
}

void UManaComponent::InitializeWithAbilitySystem(ULyraAbilitySystemComponent* InASC)
{
	AActor* Owner = GetOwner();
	check(Owner);

	if (AbilitySystemComponent)
	{
		UE_LOG(LogLyra, Error, TEXT("LyraManaComponent: Mana component for owner [%s] has already been initialized with an ability system."), *GetNameSafe(Owner));
		return;
	}

	AbilitySystemComponent = InASC;
	if (!AbilitySystemComponent)
	{
		UE_LOG(LogLyra, Error, TEXT("LyraManaComponent: Cannot initialize Mana component for owner [%s] with NULL ability system."), *GetNameSafe(Owner));
		return;
	}

	ManaSet = AbilitySystemComponent->GetSet<UManaSet>();
	if (!ManaSet)
	{
		UE_LOG(LogLyra, Error, TEXT("LyraManaComponent: Cannot initialize Mana component for owner [%s] with NULL Mana set on the ability system."), *GetNameSafe(Owner));
		return;
	}

	// Register to listen for attribute changes.
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UManaSet::GetManaAttribute()).AddUObject(this, &ThisClass::HandleManaChanged);
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(UManaSet::GetMaxManaAttribute()).AddUObject(this, &ThisClass::HandleMaxManaChanged);
}

void UManaComponent::UninitializeFromAbilitySystem()
{
	ManaSet = nullptr;
	AbilitySystemComponent = nullptr;
}

float UManaComponent::GetMana() const
{
	return (ManaSet ? ManaSet->GetMana() : 0.0f);
}

float UManaComponent::GetMaxMana() const
{
	return (ManaSet ? ManaSet->GetMaxMana() : 0.0f);
}

float UManaComponent::GetManaNormalized() const
{
	if (ManaSet)
	{
		const float Mana = ManaSet->GetMana();
		const float MaxMana = ManaSet->GetMaxMana();

		return ((MaxMana > 0.0f) ? (Mana / MaxMana) : 0.0f);
	}

	return 0.0f;
}

void UManaComponent::OnUnregister()
{
	UninitializeFromAbilitySystem();

	Super::OnUnregister();
}

void UManaComponent::HandleManaChanged(const FOnAttributeChangeData& ChangeData)
{
	OnManaChanged.Broadcast(this, ChangeData.OldValue, ChangeData.NewValue);
}

void UManaComponent::HandleMaxManaChanged(const FOnAttributeChangeData& ChangeData)
{
	OnMaxManaChanged.Broadcast(this, ChangeData.OldValue, ChangeData.NewValue);
}