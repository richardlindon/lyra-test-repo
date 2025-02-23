// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroClasses/HeroClassManagerComponent.h"

#include "GameplayAbilitySpec.h"
#include "HeroClassData.h"
#include "LyraLogChannels.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "GameFramework/GameplayMessageSubsystem.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Lyra_Class_Message_Changed, "Lyra.Class.Message.Changed");

// Sets default values for this component's properties
UHeroClassManagerComponent::UHeroClassManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UHeroClassManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	ClassSpecial1Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("InputTag.Ability.ClassSpecial1")));
	ClassSpecial2Tag = FGameplayTag::RequestGameplayTag(FName(TEXT("InputTag.Ability.ClassSpecial2")));
}

/**
 * NEW APPROACH TODO:
 * This will now add the abilities to slots and bind them
 * It will remove previously binded abilities, which should clear bindings
 *
 * It will use progression component to get saved slots
 * It will pick from the list of heroclass abilities
 * It will match via unique id string, because the save data will be a simple string id
 *
 * All ability data is loaded from heroclassdata
 *
 * input tags are hard coded as property in this manager component to bind them
 *
 * this heroclassmanagercomponent will borrow heavily from lyraabilityset
 *   
 */
void UHeroClassManagerComponent::SwapHeroClass(UHeroClassData* NewHeroClass, ULyraAbilitySystemComponent* ASC)
{
	// Remove the previously granted abilities

	if (ASC && NewHeroClass)
	{
		GrantedAbilitySetHandles.TakeFromAbilitySystem(ASC);
		CurrentHeroClass = NewHeroClass;
		
		NewHeroClass->GiveToAbilitySystem(ASC, &GrantedAbilitySetHandles, this);

		//Add specific abilities
		//Determine which ability in a data set array to add to slot 1
		//Temporary for loop to assign them until we set up ability for player to select ability for slot 
		for (int32 AbilityIndex = 0; AbilityIndex < NewHeroClass->GrantedGameplayAbilities.Num() && AbilityIndex < 2; ++AbilityIndex)
		{
			const FHeroClassData_GameplayAbility& AbilityToGrant = NewHeroClass->GrantedGameplayAbilities[AbilityIndex];

			//Add the desired ability to slot 
			GrantAbilityToSlot(AbilityToGrant, ASC, AbilityIndex);
		}
		
		//Determine which ability in a data set array to add to slot 2

		FHeroClassChangeMessage Message;
		Message.Owner = GetOwner();
		Message.NewClassTag = NewHeroClass->ClassTag;
		Message.ClassDisplayName = NewHeroClass->ClassDisplayName;
		UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(this->GetWorld());
		MessageSystem.BroadcastMessage(TAG_Lyra_Class_Message_Changed, Message);
	}
	
}

void UHeroClassManagerComponent::GrantAbilityToSlot(const FHeroClassData_GameplayAbility& AbilityToGrant, ULyraAbilitySystemComponent* ASC, int32 SlotIndex = 0)
{
	UE_LOG(LogLyra, Log, TEXT("Granting ability to slot %d yo"), SlotIndex)
	if (SlotIndex < 0 || SlotIndex > 1)
	{
		UE_LOG(LogLyraAbilitySystem, Error, TEXT("GrantAbilityToSlot: Invalid slot number when attempting to grant class ability"));
		return;
	}
	if (!IsValid(AbilityToGrant.Ability))
	{
		UE_LOG(LogLyraAbilitySystem, Error, TEXT("GrantedGameplayAbilities on ability set [%s] is not valid."), *GetNameSafe(this));
	}

	ULyraGameplayAbility* AbilityCDO = AbilityToGrant.Ability->GetDefaultObject<ULyraGameplayAbility>();

	FGameplayAbilitySpec AbilitySpec(AbilityCDO, AbilityToGrant.AbilityLevel);
	FGameplayTag TargetInputTag  = SlotIndex == 0 ? ClassSpecial1Tag : ClassSpecial2Tag;
	AbilitySpec.DynamicAbilityTags.AddTag(TargetInputTag);

	const FGameplayAbilitySpecHandle AbilitySpecHandle = ASC->GiveAbility(AbilitySpec);

	if (SlotIndex == 0)
	{
		if (ClassAbilitySlot1Handles.IsValid())
		{
			ASC->ClearAbility(ClassAbilitySlot1Handles);
		}
		ClassAbilitySlot1Handles = AbilitySpecHandle;
	} else if (SlotIndex == 1)
	{
		if (ClassAbilitySlot2Handles.IsValid())
		{
			ASC->ClearAbility(ClassAbilitySlot2Handles);
		}
		ClassAbilitySlot2Handles = AbilitySpecHandle;
	}
	
}

void UHeroClassManagerComponent::RemoveAbilitySet(ULyraAbilitySystemComponent* ASC)
{
	if (ASC)
	{
		GrantedAbilitySetHandles.TakeFromAbilitySystem(ASC);
	}
}

