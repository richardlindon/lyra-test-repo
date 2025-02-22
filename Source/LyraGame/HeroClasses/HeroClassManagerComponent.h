// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HeroClassData.h"
#include "Components/ActorComponent.h"
#include "HeroClassManagerComponent.generated.h"

/**
 * @TODO: No idea if i need replication 
 */
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
	void GrantAbilityToSlot(const FHeroClassData_GameplayAbility& AbilityToGrant, ULyraAbilitySystemComponent* ASC, int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Hero Classes")
	void RemoveAbilitySet(ULyraAbilitySystemComponent* ASC);

	UPROPERTY(BlueprintReadOnly, Category = "Hero Classes")
	TObjectPtr<UHeroClassData> CurrentHeroClass;

	UFUNCTION(BlueprintCallable, Category = "Hero Classes")
	FText GetCurrentHeroClassDisplayName() const
	{
		return CurrentHeroClass ? CurrentHeroClass->ClassDisplayName : FText::GetEmpty();
	}
	
protected:

	FLyraAbilitySet_GrantedHandles GrantedAbilitySetHandles;

	FGameplayAbilitySpecHandle ClassAbilitySlot1Handles;

	FGameplayAbilitySpecHandle ClassAbilitySlot2Handles;
	
private:

	FGameplayTag ClassSpecial1Tag;

	FGameplayTag ClassSpecial2Tag;

	
};
