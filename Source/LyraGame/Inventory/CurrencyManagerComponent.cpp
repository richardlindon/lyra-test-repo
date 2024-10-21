// Fill out your copyright notice in the Description page of Project Settings.


#include "CurrencyManagerComponent.h"

#include "GameModes/LyraExperienceManagerComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "NPC/NPCGameDirector.h"
#include "NativeGameplayTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(CurrencyManagerComponent)
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Currency_Message_CurrencyChanged, "Lyra.Inventory.Message.CurrencyChanged");


// Sets default values for this component's properties
UCurrencyManagerComponent::UCurrencyManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
	Currency = 0;
}

bool UCurrencyManagerComponent::CanAffordCost(int32 Cost)
{
	return Currency >= Cost;
}

void UCurrencyManagerComponent::RemoveCurrency(int32 Amount)
{
	if (Amount > 0)
	{
		Currency = Currency - Amount;
		OnRep_Currency();
	}
}

void UCurrencyManagerComponent::AddCurrency(int32 Amount)
{
	if (Amount > 0)
	{
		Currency = Currency + Amount;
		OnRep_Currency();
	}
}

void UCurrencyManagerComponent::StartRegularCurrencyIncome()
{
	GetWorld()->GetTimerManager().SetTimer(TimerHandle_AddCurrency, this, &UCurrencyManagerComponent::AddRegularCurrency, IncomeTickRateSeconds, true);
}

void UCurrencyManagerComponent::StopRegularCurrencyIncome()
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_AddCurrency);
}

void UCurrencyManagerComponent::AddRegularCurrency()
{
	AddCurrency(IncomePerTick);
}

void UCurrencyManagerComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Currency);
}

void UCurrencyManagerComponent::OnRep_Currency() const
{
	FCurrencyChangedMessage Message;
	Message.NewCurrency = Currency;
	Message.Owner = GetOwner();
	
	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(this);
	MessageSystem.BroadcastMessage(TAG_Currency_Message_CurrencyChanged, Message);
}

