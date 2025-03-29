// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroClasses/HeroClassManagerComponent.h"

#include "GameplayAbilitySpec.h"
#include "HeroClassData.h"
#include "LyraLogChannels.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/LyraGameplayAbility.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "Progression/PlayerProgressionComponent.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Lyra_Class_Message_Changed, "Lyra.Class.Message.Changed");

// Sets default values for this component's properties
UHeroClassManagerComponent::UHeroClassManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UHeroClassManagerComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentHeroClass);
	DOREPLIFETIME(ThisClass, CurrentClassAbilitySlot1);
	DOREPLIFETIME(ThisClass, CurrentClassAbilitySlot2);
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
		
		CurrentHeroClass->GiveToAbilitySystem(ASC, &GrantedAbilitySetHandles, this);
		
		UPlayerProgressionComponent* PC = GetProgressionComponent();	
		if (!PC)
		{
			UE_LOG(LogLyraAbilitySystem, Error, TEXT("GrantAbilityToSlot: Could not get progression component to save update"));
			return;
		}
		TArray<FGameplayTag> SavedAbilities = PC->GetSavedAbilitiesByClassTag(CurrentHeroClass->ClassTag);
		//Add specific abilities
		//Determine which ability in a data set array to add to slot 1
		//Temporary for loop to assign them until we set up ability for player to select ability for slot 
		if (SavedAbilities.IsEmpty())
		{
			for (int32 AbilityIndex = 0; AbilityIndex < CurrentHeroClass->ClassAbilities.Num() && AbilityIndex < 2; ++AbilityIndex)
			{
			
				const FHeroClassData_GameplayAbility& Ability = CurrentHeroClass->ClassAbilities[AbilityIndex];
				GrantAbilityToSlot(Ability, ASC, CurrentHeroClass->ClassTag, AbilityIndex);
			}
		}
		else
		{
			for (int32 AbilityIndex = 0; AbilityIndex < CurrentHeroClass->ClassAbilities.Num(); ++AbilityIndex)
			{
				const FHeroClassData_GameplayAbility& Ability = CurrentHeroClass->ClassAbilities[AbilityIndex];
				//Check for assigning to slot 1
				if (Ability.AbilityTag == SavedAbilities[0])
				{
					GrantAbilityToSlot(Ability, ASC,CurrentHeroClass->ClassTag, 0);
				}
				if (Ability.AbilityTag == SavedAbilities[1])
				{
					GrantAbilityToSlot(Ability, ASC,CurrentHeroClass->ClassTag, 1);
				}
			}
		}
		
		FHeroClassChangeMessage Message;
		Message.Owner = GetOwner();
		Message.NewClassTag = CurrentHeroClass->ClassTag;
		Message.ClassDisplayName = CurrentHeroClass->ClassDisplayName;
		UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(this->GetWorld());
		MessageSystem.BroadcastMessage(TAG_Lyra_Class_Message_Changed, Message);
	}
}

//TODO: This method does not yet handle if the class is not active
// We potentially need a new function for saving any ability to the progression component at any time (EG for use in a loadout menu)
// and to limit this on the fly adjustment to the current class only
void UHeroClassManagerComponent::SaveAbilityToSlot(const FHeroClassData_GameplayAbility& AbilityToSave, ULyraAbilitySystemComponent* ASC, FGameplayTag ClassTag, int32 SlotIndex = 0)
{
	UE_LOG(LogLyraAbilitySystem, Error, TEXT("GrantAbilityToSlot: Invalid slot number when attempting to grant class ability"));
	if (SlotIndex < 0 || SlotIndex > 1)
	{
		UE_LOG(LogLyraAbilitySystem, Error, TEXT("GrantAbilityToSlot: Invalid slot number when attempting to grant class ability"));
		return;
	}
	if (!ASC)
	{
		return;
	}
	
	bool isActiveClass = CurrentHeroClass && ClassTag == CurrentHeroClass->ClassTag;
	
	UPlayerProgressionComponent* PC = GetProgressionComponent();	
	if (!PC)
	{
		UE_LOG(LogLyraAbilitySystem, Error, TEXT("GrantAbilityToSlot: Could not get progression component to save update"));
		return;
	}
	//Is ability in other slot?
	if (SlotIndex == 0)
	{
		if (CurrentClassAbilitySlot2.Ability == AbilityToSave.Ability)
		{
			//Swap current slot 1 ability to slot 2
			if (isActiveClass) GrantAbilityToSlot(CurrentClassAbilitySlot1, ASC,CurrentHeroClass->ClassTag, 1);
			PC->SaveAbilityToSlot(CurrentClassAbilitySlot1.AbilityTag, 1, ClassTag);
		}
		if (CurrentClassAbilitySlot1.Ability == AbilityToSave.Ability)
		{
			return;
		}
	} else if (SlotIndex == 1)
	{
		if (CurrentClassAbilitySlot1.Ability == AbilityToSave.Ability)
		{
			//Swap current slot 2 ability to slot 1
			if (isActiveClass)  GrantAbilityToSlot(CurrentClassAbilitySlot2, ASC, ClassTag, 0);
			PC->SaveAbilityToSlot(CurrentClassAbilitySlot2.AbilityTag, 0,ClassTag);
		}
		if (CurrentClassAbilitySlot2.Ability == AbilityToSave.Ability)
		{
			return;
		}
	}

	if (isActiveClass)  GrantAbilityToSlot(AbilityToSave, ASC, ClassTag, SlotIndex);
	PC->SaveAbilityToSlot(AbilityToSave.AbilityTag, SlotIndex,ClassTag);
	
}

/**
 * Called when ever ability is loaded, including when swapping class
 * @param AbilityToGrant 
 * @param ASC
 * @param ClassTag 
 * @param SlotIndex 
 */
void UHeroClassManagerComponent::GrantAbilityToSlot(const FHeroClassData_GameplayAbility& AbilityToGrant, ULyraAbilitySystemComponent* ASC, FGameplayTag ClassTag,  int32 SlotIndex = 0)
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
		return;
	}

	//A bit of pointless extra work every time its called given the current ClassTag is accessible here,
	// but it ensures that when abilities are added, dev is prompted and aware which class the abilit is for
	// A better approach would be to include the ClassTag in FHeroClassData_GameplayAbility during init somehow and check it here 
	if (!CurrentHeroClass || ClassTag != CurrentHeroClass->ClassTag)
	{
		UE_LOG(LogLyraAbilitySystem, Error, TEXT("GrantAbilityToSlot: Cannot set ability to slot for inactive class."));
		return;
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
		CurrentClassAbilitySlot1 = AbilityToGrant;
	} else if (SlotIndex == 1)
	{
		if (ClassAbilitySlot2Handles.IsValid())
		{
			ASC->ClearAbility(ClassAbilitySlot2Handles);
		}
		ClassAbilitySlot2Handles = AbilitySpecHandle;
		CurrentClassAbilitySlot2 = AbilityToGrant;
	}
	
}

void UHeroClassManagerComponent::RemoveAbilitySet(ULyraAbilitySystemComponent* ASC)
{
	if (ASC)
	{
		GrantedAbilitySetHandles.TakeFromAbilitySystem(ASC);
	}
}

void UHeroClassManagerComponent::OnRep_CurrentHeroClass()
{
	//UI Message TODO
}

void UHeroClassManagerComponent::OnRep_CurrentClassAbilitySlot1()
{
	//UI Message TODO
}

void UHeroClassManagerComponent::OnRep_CurrentClassAbilitySlot2()
{
	//UI Message TODO
}

UPlayerProgressionComponent* UHeroClassManagerComponent::GetProgressionComponent() const
{
	if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
	{
		if (APlayerState* PS =  PC->PlayerState)
		{
			if (UPlayerProgressionComponent* ProgressionComponent = PS->GetComponentByClass<UPlayerProgressionComponent>()) {
				return ProgressionComponent;
			}
		}
	}
	return nullptr;
}

