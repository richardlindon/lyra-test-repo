// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraQuickBarComponent.h"

#include "AbilitySystemGlobals.h"
#include "LyraLogChannels.h"
#include "Equipment/LyraEquipmentDefinition.h"
#include "Equipment/LyraEquipmentInstance.h"
#include "Equipment/LyraEquipmentManagerComponent.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "GameFramework/Pawn.h"
#include "Inventory/InventoryFragment_EquippableItem.h"
#include "NativeGameplayTags.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Character/LyraCharacter.h"
#include "Inventory/InventoryFragment_StatItem.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraQuickBarComponent)

class FLifetimeProperty;
class ULyraEquipmentDefinition;

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Lyra_QuickBar_Message_SlotsChanged, "Lyra.QuickBar.Message.SlotsChanged");
UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Lyra_QuickBar_Message_ActiveIndexChanged, "Lyra.QuickBar.Message.ActiveIndexChanged");

ULyraQuickBarComponent::ULyraQuickBarComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsReplicatedByDefault(true);
}

void ULyraQuickBarComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Slots);
	DOREPLIFETIME(ThisClass, ActiveSlotIndex);
}

void ULyraQuickBarComponent::BeginPlay()
{
	if (Slots.Num() < NumSlots)
	{
		Slots.AddDefaulted(NumSlots - Slots.Num());
	}

	Super::BeginPlay();
}

//Non-equippable items will be in slots. Cycle must be reworked to skip over (FS-89)
void ULyraQuickBarComponent::CycleActiveSlotForward()
{
	if (Slots.Num() < 2)
	{
		return;
	}

	const int32 OldIndex = (ActiveSlotIndex < 0 ? Slots.Num()-1 : ActiveSlotIndex);
	int32 NewIndex = ActiveSlotIndex;
	do
	{
		NewIndex = (NewIndex + 1) % Slots.Num();
		if (Slots[NewIndex] != nullptr)
		{
			SetActiveSlotIndex(NewIndex);
			return;
		}
	} while (NewIndex != OldIndex);
}

//Non-equippable items will be in slots. Cycle must be reworked to skip over (FS-89)
void ULyraQuickBarComponent::CycleActiveSlotBackward()
{
	if (Slots.Num() < 2)
	{
		return;
	}

	const int32 OldIndex = (ActiveSlotIndex < 0 ? Slots.Num()-1 : ActiveSlotIndex);
	int32 NewIndex = ActiveSlotIndex;
	do
	{
		NewIndex = (NewIndex - 1 + Slots.Num()) % Slots.Num();
		if (Slots[NewIndex] != nullptr)
		{
			SetActiveSlotIndex(NewIndex);
			return;
		}
	} while (NewIndex != OldIndex);
}

bool ULyraQuickBarComponent::IsEquippableItemInSlot(int32 SlotIndex)
{
	if (ULyraInventoryItemInstance* SlotItem = Slots[SlotIndex])
	{
		if (const UInventoryFragment_EquippableItem* EquipInfo = SlotItem->FindFragmentByClass<UInventoryFragment_EquippableItem>())
		{
			TSubclassOf<ULyraEquipmentDefinition> EquipDef = EquipInfo->EquipmentDefinition;
			return EquipDef != nullptr;
		}
	}
	return false;
}

bool ULyraQuickBarComponent::IsActivatableItemInSlot(int32 SlotIndex)
{
	if (ULyraInventoryItemInstance* SlotItem = Slots[SlotIndex])
	{
		TArray<FGameplayAbilitySpecHandle> AbilityHandles = SlotItem->GrantedHandles.GetAllAbilityHandles();
		return !AbilityHandles.IsEmpty();
	}
	return false;
}

void ULyraQuickBarComponent::ActivateItemAbilityInSlot(int32 SlotIndex)
{
	if (ULyraInventoryItemInstance* SlotItem = Slots[SlotIndex])
	{
		if (ULyraAbilitySystemComponent* ASC = GetAbilitySystemComponent())
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					5.0f, 
					FColor::Green,
					FString::Printf(TEXT("Found ASC, attempting to activate"))
				);
			}
			//Activate ability in slot if swapping
			//Should not swap to slot
			TArray<FGameplayAbilitySpecHandle> AbilityHandles = SlotItem->GrantedHandles.GetAllAbilityHandles();
			if (!AbilityHandles.IsEmpty())
			{
				ASC->TryActivateAbility(AbilityHandles[0]);
			}
		}
	}
}

void ULyraQuickBarComponent::EquipItemInSlot()
{
	check(Slots.IsValidIndex(ActiveSlotIndex));
	check(EquippedItem == nullptr);

	if (ULyraInventoryItemInstance* SlotItem = Slots[ActiveSlotIndex])
	{
		if (const UInventoryFragment_EquippableItem* EquipInfo = SlotItem->FindFragmentByClass<UInventoryFragment_EquippableItem>())
		{
			TSubclassOf<ULyraEquipmentDefinition> EquipDef = EquipInfo->EquipmentDefinition;
			if (EquipDef != nullptr)
			{
				if (ULyraEquipmentManagerComponent* EquipmentManager = FindEquipmentManager())
				{
					EquippedItem = EquipmentManager->EquipItem(EquipDef);
					if (EquippedItem != nullptr)
					{
						EquippedItem->SetInstigator(SlotItem);
					}
				}
			}
		}

		
	}
}

void ULyraQuickBarComponent::UnequipItemInSlot()
{
	if (ULyraEquipmentManagerComponent* EquipmentManager = FindEquipmentManager())
	{
		if (EquippedItem != nullptr)
		{
			EquipmentManager->UnequipItem(EquippedItem);
			EquippedItem = nullptr;
		}
	}
}

ULyraEquipmentManagerComponent* ULyraQuickBarComponent::FindEquipmentManager() const
{
	if (AController* OwnerController = Cast<AController>(GetOwner()))
	{
		if (APawn* Pawn = OwnerController->GetPawn())
		{
			return Pawn->FindComponentByClass<ULyraEquipmentManagerComponent>();
		}
	}
	return nullptr;
}

void ULyraQuickBarComponent::SetActiveSlotIndex_Implementation(int32 NewIndex)
{
	if (Slots.IsValidIndex(NewIndex))
	{
		if (IsEquippableItemInSlot(NewIndex) && (ActiveSlotIndex != NewIndex))
		{
			UnequipItemInSlot();

			ActiveSlotIndex = NewIndex;

			EquipItemInSlot();

			OnRep_ActiveSlotIndex();
		}
		else if (IsActivatableItemInSlot(NewIndex))
		{
			ActivateItemAbilityInSlot(NewIndex);
		}
	}
}

ULyraInventoryItemInstance* ULyraQuickBarComponent::GetActiveSlotItem() const
{
	return Slots.IsValidIndex(ActiveSlotIndex) ? Slots[ActiveSlotIndex] : nullptr;
}

int32 ULyraQuickBarComponent::GetNextFreeItemSlot() const
{
	int32 SlotIndex = 0;
	for (TObjectPtr<ULyraInventoryItemInstance> ItemPtr : Slots)
	{
		if (ItemPtr == nullptr)
		{
			return SlotIndex;
		}
		++SlotIndex;
	}

	return INDEX_NONE;
}

int32  ULyraQuickBarComponent::GetItemCurrentSlotIndex(const ULyraInventoryItemInstance* Item) const
{
	if (!Item)
	{
		return -1;
	}

	for (int32 Index = 0; Index < Slots.Num(); ++Index)
	{
		if (Slots[Index] == Item) 
		{
			return Index; 
		}
	}

	return -1;
}

TArray<FGameplayTag> ULyraQuickBarComponent::GetSlottedClasses() 
{
	TArray<FGameplayTag> ClassesSlotted;
	for (int32 Index = 0; Index < Slots.Num(); ++Index)
	{
		if (ULyraInventoryItemInstance* Item = Slots[Index])
		{
			if (const UInventoryFragment_StatItem* StatItemFragment = Item->FindFragmentByClass<UInventoryFragment_StatItem>())
			{
				if (StatItemFragment->ClassGranted.IsValid() && StatItemFragment->ClassGranted.RequestDirectParent().ToString() == "Class")
				{
					ClassesSlotted.Add(StatItemFragment->ClassGranted);
				}
			}
		}
	}
	return ClassesSlotted;	
}

void ULyraQuickBarComponent::RemoveItemFromQuickbar(const ULyraInventoryItemInstance* Item)
{
	const int32 index = GetItemCurrentSlotIndex(Item);
	if (index > -1)
	{
		RemoveItemFromSlot(index);
	}
}

bool ULyraQuickBarComponent::IsSlottableItem(ULyraInventoryItemInstance* Item)
{
	const UInventoryFragment_StatItem* StatItemFragment = Item->FindFragmentByClass<UInventoryFragment_StatItem>();
	const UInventoryFragment_EquippableItem* EquippableItemFragment = Item->FindFragmentByClass<UInventoryFragment_EquippableItem>();
	return (StatItemFragment || EquippableItemFragment);
}

void ULyraQuickBarComponent::AddItemToSlot(int32 SlotIndex, ULyraInventoryItemInstance* Item)
{
	if (!IsSlottableItem(Item))
	{
		UE_LOG(LogLyra, Warning, TEXT("Attempting to slot a non-slottable item. Item must have statItem or EquippableItem fragment."));
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				5.0f, 
				FColor::Red,
				FString::Printf(TEXT("Attempting to slot a non-slottable item. Item must have statItem or EquippableItem fragment."))
			);
		}
		return;
	}
	if (Slots.IsValidIndex(SlotIndex) && (Item != nullptr))
	{
		//This gets the index the newly adding item is potentially already slotted at
		//THis helps prevent dupe slotting of same item
		int32 CurrentlySlottedIndex = GetItemCurrentSlotIndex(Item);
		if (CurrentlySlottedIndex > -1)
		{
			//Unassign from the slot
			RemoveItemFromSlot(CurrentlySlottedIndex);
		}

		//Add ability sets tied to the new item, ready for activation. Items apply active effects when slotted, and can also be activated to trigger abilities
		if (const UInventoryFragment_StatItem* StatItemFragment = Item->FindFragmentByClass<UInventoryFragment_StatItem>())
		{
			if (ULyraAbilitySystemComponent* ASC = GetAbilitySystemComponent())
			{
				for (TObjectPtr<const ULyraAbilitySet> AbilitySet : StatItemFragment->AbilitySetsToGrant)
				{
					AbilitySet->GiveToAbilitySystem(ASC, /*inout*/ &Item->GrantedHandles, Item);
				}
			}
		}
\
		if (Slots[SlotIndex] != nullptr)
		{
			//Remove whatever is currently slotted, appropriately removing abilities/equipped item etc
			RemoveItemFromSlot(SlotIndex);
		}
		Slots[SlotIndex] = Item;
		
		OnRep_Slots();
	}
}

ULyraInventoryItemInstance* ULyraQuickBarComponent::RemoveItemFromSlot(int32 SlotIndex)
{
	ULyraInventoryItemInstance* Result = nullptr;

	if (ActiveSlotIndex == SlotIndex)
	{
		UnequipItemInSlot();
		ActiveSlotIndex = -1;
	}

	if (Slots.IsValidIndex(SlotIndex))
	{
		Result = Slots[SlotIndex];

		if (Result != nullptr)
		{
			if (ULyraAbilitySystemComponent* ASC = GetAbilitySystemComponent())
			{
				Result->GrantedHandles.TakeFromAbilitySystem(ASC);
			}
			Slots[SlotIndex] = nullptr;
			OnRep_Slots();
		}
	}

	return Result;
}

void ULyraQuickBarComponent::OnRep_Slots()
{
	FLyraQuickBarSlotsChangedMessage Message;
	Message.Owner = GetOwner();
	Message.Slots = Slots;

	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(this);
	MessageSystem.BroadcastMessage(TAG_Lyra_QuickBar_Message_SlotsChanged, Message);
}

void ULyraQuickBarComponent::OnRep_ActiveSlotIndex()
{
	FLyraQuickBarActiveIndexChangedMessage Message;
	Message.Owner = GetOwner();
	Message.ActiveIndex = ActiveSlotIndex;

	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(this);
	MessageSystem.BroadcastMessage(TAG_Lyra_QuickBar_Message_ActiveIndexChanged, Message);
}


ULyraAbilitySystemComponent* ULyraQuickBarComponent::GetAbilitySystemComponent() const
{
	if (AActor* OwningActor = GetOwner())
	{
		if (ULyraAbilitySystemComponent* ASC = Cast<ULyraAbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(OwningActor)))
		{
			return ASC;
		}
		
		// Attempt to cast the OwningActor to AController (or its derived class)
		if (AController* Controller = Cast<AController>(OwningActor))
		{
			if (APawn* Pawn = Controller->GetPawn())
			{
				if (ALyraCharacter* Character = Cast<ALyraCharacter>(Pawn))
				{
					return Character->GetLyraAbilitySystemComponent();
				}
			}
		}
	}
	
		
	return nullptr;
}
