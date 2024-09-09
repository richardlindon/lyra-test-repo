// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CurrencyManagerComponent.generated.h"

/** A message when a wave begins or ends */
USTRUCT(BlueprintType)
struct FCurrencyChangedMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Currency")
	TObjectPtr<AActor> Owner = nullptr;
	UPROPERTY(BlueprintReadOnly, Category="Currency")
	int32 NewCurrency = 0;
};

UCLASS(BlueprintType)
class LYRAGAME_API UCurrencyManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UCurrencyManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());


public:

	UPROPERTY(EditAnywhere, Category="Currency")
	float IncomeTickRateSeconds = 1.0f;

	UPROPERTY(EditAnywhere, Category="Currency")
	int32 IncomePerTick = 1;
	
	UFUNCTION(BlueprintCallable,BlueprintAuthorityOnly, Category="Currency")
	bool CanAffordCost(int32 Cost);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category="Currency")
	void RemoveCurrency(int32 Amount);

	UFUNCTION(BlueprintCallable,BlueprintAuthorityOnly,  Category="Currency")
	void AddCurrency(int32 Amount);

	UFUNCTION(BlueprintCallable,BlueprintAuthorityOnly, Category="Currency")
	void StartRegularCurrencyIncome();

	UFUNCTION(BlueprintCallable,BlueprintAuthorityOnly, Category="Currency")
	void StopRegularCurrencyIncome();
	
	UPROPERTY(ReplicatedUsing=OnRep_Currency, BlueprintReadOnly)
	int32 Currency;
private:
	
	void AddRegularCurrency();

	FTimerHandle TimerHandle_AddCurrency;

	UFUNCTION()
	void OnRep_Currency() const;
};
