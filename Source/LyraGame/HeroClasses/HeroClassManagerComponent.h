// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HeroClassData.h"
#include "Components/ActorComponent.h"
#include "HeroClassManagerComponent.generated.h"

class UPlayerProgressionComponent;

/** A message when the active class changes */
USTRUCT(BlueprintType)
struct FHeroClassChangeMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Hero Classes")
	TObjectPtr<AActor> Owner = nullptr;
	
	UPROPERTY(BlueprintReadOnly, Category="Hero Classes")
	FGameplayTag NewClassTag;

	UPROPERTY(BlueprintReadOnly, Category="Hero Classes")
	FText ClassDisplayName;
};

/** A message when an ability binding for slot 1 or 2 is changed for the active class */
USTRUCT(BlueprintType)
struct FHeroAbilityChangeMessage
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Hero Classes")
	TObjectPtr<AActor> Owner = nullptr;
	
	UPROPERTY(BlueprintReadOnly, Category="Hero Classes")
	int32 SlotIndexChanged = -1;
};

// Controller component
UCLASS(BlueprintType)
class LYRAGAME_API UHeroClassManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHeroClassManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	virtual void BeginPlay() override;

	// Function to swap to a new ability set
	UFUNCTION(BlueprintCallable, Category = "Hero Classes")
	void SwapHeroClass(UHeroClassData* NewHeroClass, ULyraAbilitySystemComponent* ASC);

	UFUNCTION(BlueprintCallable, Category = "Hero Classes")
	void SaveAbilityToSlot(const FHeroClassData_GameplayAbility& AbilityToSwap, ULyraAbilitySystemComponent* ASC, FGameplayTag ClassTag, int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Hero Classes")
	void GrantAbilityToSlot(const FHeroClassData_GameplayAbility& AbilityToGrant, ULyraAbilitySystemComponent* ASC, int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Hero Classes")
	void RemoveAbilitySet(ULyraAbilitySystemComponent* ASC);

	UPROPERTY(BlueprintReadOnly, Category = "Hero Classes", ReplicatedUsing=OnRep_CurrentHeroClass)
	TObjectPtr<UHeroClassData> CurrentHeroClass;

	//Referenced for UI
	UPROPERTY(BlueprintReadOnly, Category = "Hero Classes", ReplicatedUsing=OnRep_CurrentClassAbilitySlot1)
	FHeroClassData_GameplayAbility CurrentClassAbilitySlot1;

	//Referenced for UI
	UPROPERTY(BlueprintReadOnly, Category = "Hero Classes", ReplicatedUsing=OnRep_CurrentClassAbilitySlot2)
	FHeroClassData_GameplayAbility CurrentClassAbilitySlot2;

	UFUNCTION(BlueprintCallable, Category = "Hero Classes")
	FText GetCurrentHeroClassDisplayName() const
	{
		return CurrentHeroClass ? CurrentHeroClass->ClassDisplayName : FText::GetEmpty();
	}
	
protected:

	UFUNCTION()
	void OnRep_CurrentHeroClass();

	UFUNCTION()
	void OnRep_CurrentClassAbilitySlot1();

	UFUNCTION()
	void OnRep_CurrentClassAbilitySlot2();
	
	FLyraAbilitySet_GrantedHandles GrantedAbilitySetHandles;

	FGameplayAbilitySpecHandle ClassAbilitySlot1Handles;

	FGameplayAbilitySpecHandle ClassAbilitySlot2Handles;
	
private:

	FGameplayTag ClassSpecial1Tag;

	FGameplayTag ClassSpecial2Tag;

	UPlayerProgressionComponent* GetProgressionComponent() const;	
};
