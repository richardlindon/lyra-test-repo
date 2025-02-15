// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/LyraAbilitySet.h"
#include "Engine/DataAsset.h"
#include "ActiveGameplayEffectHandle.h"
#include "Engine/DataAsset.h"
#include "AttributeSet.h"
#include "GameplayAbilitySpecHandle.h"
#include "HeroClassData.generated.h"

class UAttributeSet;
class UGameplayEffect;
class ULyraAbilitySystemComponent;
class ULyraGameplayAbility;
class UObject;


/**
 * FHeroClassData_GameplayAbility
 *
 *	Data used by the ability set to grant gameplay abilities.
 */
USTRUCT(BlueprintType)
struct FHeroClassData_GameplayAbility
{
	GENERATED_BODY()

public:

	// Gameplay ability to grant.
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ULyraGameplayAbility> Ability = nullptr;

	// Level of ability to grant.
	UPROPERTY(EditDefaultsOnly)
	int32 AbilityLevel = 1;

	// Tag used to process input for the ability.
	// UNUSED FOR NOW - hero abilities are added to a predefined input tag
	// UPROPERTY(EditDefaultsOnly, Meta = (Categories = "InputTag"))
	// FGameplayTag InputTag;

	//Unique ID to track and correlate ability between save data and active slots etc
	UPROPERTY(EditDefaultsOnly)
	FString AbilityUniqueId;

	UPROPERTY(EditDefaultsOnly)
	FText AbilityDisplayName; 

};


/**
 * Hero class data, very similar to lyra ability sets, but isolated for future customisations and lower level differences
 */
UCLASS()
class LYRAGAME_API UHeroClassData : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UHeroClassData(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Hero Classes")
	FString UniqueID = "";

	// Grants the ability set to the specified ability system component.
	// The returned handles can be used later to take away anything that was granted.
	// Does not grant abilities as those are handled in HeroClassManagerComponent
	void GiveToAbilitySystem(ULyraAbilitySystemComponent* LyraASC, FLyraAbilitySet_GrantedHandles* OutGrantedHandles, UObject* SourceObject = nullptr) const;

	// Gameplay abilities to choose from when this class is activated.
	// Only bound abilities are added, if they were added by the user to either slot 1 or slot 2
	UPROPERTY(EditDefaultsOnly, Category = "Hero Classes", meta=(TitleProperty=Ability))
	TArray<FHeroClassData_GameplayAbility> GrantedGameplayAbilities;

protected:

	// Gameplay effects to grant when this class is activated.
	UPROPERTY(EditDefaultsOnly, Category = "Hero Classes", meta=(TitleProperty=GameplayEffect))
	TArray<FLyraAbilitySet_GameplayEffect> GrantedGameplayEffects;

	// Attribute sets to grant when this class is activated.
	UPROPERTY(EditDefaultsOnly, Category = "Hero Classes", meta=(TitleProperty=AttributeSet))
	TArray<FLyraAbilitySet_AttributeSet> GrantedAttributes;

	UPROPERTY(EditDefaultsOnly, Category = "Hero Classes")
	FSlateBrush ClassIcon;

	UPROPERTY(EditDefaultsOnly, Category = "Hero Classes")
	FText ClassDisplayName;

};
