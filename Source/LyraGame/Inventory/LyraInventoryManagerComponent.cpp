// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraInventoryManagerComponent.h"

#include "AbilitySystemGlobals.h"
#include "InventoryFragment_SharedStatTags.h"
#include "InventoryFragment_StatItem.h"
#include "Engine/ActorChannel.h"
#include "Engine/World.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "LyraInventoryItemDefinition.h"
#include "LyraInventoryItemInstance.h"
#include "NativeGameplayTags.h"
#include "AbilitySystem/LyraAbilitySet.h"
#include "AbilitySystem/LyraAbilitySystemComponent.h"
#include "Character/LyraCharacter.h"
#include "Net/UnrealNetwork.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraInventoryManagerComponent)
DEFINE_LOG_CATEGORY_STATIC(LogInventoryManager, Log, All);

class FLifetimeProperty;
struct FReplicationFlags;

UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_Lyra_Inventory_Message_StackChanged, "Lyra.Inventory.Message.StackChanged");

//////////////////////////////////////////////////////////////////////
// FLyraInventoryEntry

FString FLyraInventoryEntry::GetDebugString() const
{
	TSubclassOf<ULyraInventoryItemDefinition> ItemDef;
	if (Instance != nullptr)
	{
		ItemDef = Instance->GetItemDef();
	}

	return FString::Printf(TEXT("%s (%d x %s)"), *GetNameSafe(Instance), StackCount, *GetNameSafe(ItemDef));
}

//////////////////////////////////////////////////////////////////////
// FLyraInventoryList

void FLyraInventoryList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (int32 Index : RemovedIndices)
	{
		FLyraInventoryEntry& Stack = Entries[Index];
		BroadcastChangeMessage(Stack, /*OldCount=*/ Stack.StackCount, /*NewCount=*/ 0);
		Stack.LastObservedCount = 0;
	}
}

void FLyraInventoryList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		FLyraInventoryEntry& Stack = Entries[Index];
		BroadcastChangeMessage(Stack, /*OldCount=*/ 0, /*NewCount=*/ Stack.StackCount);
		Stack.LastObservedCount = Stack.StackCount;
	}
}

void FLyraInventoryList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		FLyraInventoryEntry& Stack = Entries[Index];
		check(Stack.LastObservedCount != INDEX_NONE);
		BroadcastChangeMessage(Stack, /*OldCount=*/ Stack.LastObservedCount, /*NewCount=*/ Stack.StackCount);
		Stack.LastObservedCount = Stack.StackCount;
	}
}

void FLyraInventoryList::BroadcastChangeMessage(FLyraInventoryEntry& Entry, int32 OldCount, int32 NewCount)
{
	FLyraInventoryChangeMessage Message;
	Message.InventoryOwner = OwnerComponent;
	Message.Instance = Entry.Instance;
	Message.NewCount = NewCount;
	Message.Delta = NewCount - OldCount;
	UE_LOG(LogInventoryManager, Warning, TEXT("Broadcasting change message"));
	UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(OwnerComponent->GetWorld());
	MessageSystem.BroadcastMessage(TAG_Lyra_Inventory_Message_StackChanged, Message);
}

ULyraAbilitySystemComponent* FLyraInventoryList::GetAbilitySystemComponent() const
{
	check(OwnerComponent);
	AActor* OwningActor = OwnerComponent->GetOwner();
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
	
	return nullptr;
}

ULyraInventoryItemInstance* FLyraInventoryList::AddEntry(TSubclassOf<ULyraInventoryItemDefinition> ItemDef, int32 StackCount)
{
	ULyraInventoryItemInstance* Result = nullptr;

	check(ItemDef != nullptr);
 	check(OwnerComponent);

	AActor* OwningActor = OwnerComponent->GetOwner();
	check(OwningActor->HasAuthority());


	FLyraInventoryEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.Instance = NewObject<ULyraInventoryItemInstance>(OwnerComponent->GetOwner());  //@TODO: Using the actor instead of component as the outer due to UE-127172
	NewEntry.Instance->SetItemDef(ItemDef);
	
	for (ULyraInventoryItemFragment* Fragment : GetDefault<ULyraInventoryItemDefinition>(ItemDef)->Fragments)
	{
		if (Fragment != nullptr)
		{
			Fragment->OnInstanceCreated(NewEntry.Instance);

			// if (UInventoryFragment_StatItem* StatItemFragment = Cast<UInventoryFragment_StatItem>(Fragment))
			// {
			// 	UE_LOG(LogInventoryManager, Warning, TEXT("Found StatItem fragment"));
			// 	if (GEngine)
			// 	{
			// 		GEngine->AddOnScreenDebugMessage(
			// 			-1,
			// 			5.0f, 
			// 			FColor::Green,
			// 			FString::Printf(TEXT("Found StatItem fragment"))
			// 		);
			// 	}
			// 	
			// 	if (ULyraAbilitySystemComponent* ASC = GetAbilitySystemComponent())
			// 	{
			// 		UE_LOG(LogInventoryManager, Warning, TEXT("Found ASC"));
			//
			// 		
			// 		for (TObjectPtr<const ULyraAbilitySet> AbilitySet : StatItemFragment->AbilitySetsToGrant)
			// 		{
			// 			AbilitySet->GiveToAbilitySystem(ASC, /*inout*/ &NewEntry.GrantedHandles, Result);
			// 		}
			// 	}
			// }
		}
	}

	NewEntry.StackCount = StackCount;
	Result = NewEntry.Instance;

	MarkItemDirty(NewEntry);

	return Result;
}

ULyraInventoryItemInstance* FLyraInventoryList::AddEntry(ULyraInventoryItemInstance* Instance)
{
	//Push new item definition, or add the instance directly to the Entries?
	TSubclassOf<ULyraInventoryItemDefinition> ItemDef = Instance->GetItemDef();
	return AddEntry(ItemDef, 1);

}

void FLyraInventoryList::RemoveEntry(ULyraInventoryItemInstance* Instance)
{
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FLyraInventoryEntry& Entry = *EntryIt;
		if (Entry.Instance == Instance)
		{
			// if (ULyraAbilitySystemComponent* ASC = GetAbilitySystemComponent())
			// {
			// 	Entry.GrantedHandles.TakeFromAbilitySystem(ASC);
			// }
			
			EntryIt.RemoveCurrent();
			MarkArrayDirty();
		}
	}
}

//
// void FLyraInventoryList::RemoveItemEffect(ULyraInventoryItemInstance* ItemInstance)
// {
// 	//FindFragmentByClass<UInventoryFragment_EquippableItem>()
// 	
// 	if (ULyraAbilitySystemComponent* ASC = GetAbilitySystemComponent())
// 	{
// 		//ASC->RemoveActiveGameplayEffect(StatItemFragment->AppliedEffectHandle);
// 		if (ItemInstance->AppliedEffectHandle.IsValid())
// 		{
// 			UE_LOG(LogTemp, Warning, TEXT("Valid effect handle found on item: %s"), *ItemInstance->GetName());
// 			ASC->RemoveActiveGameplayEffect(ItemInstance->AppliedEffectHandle);
// 			ItemInstance->AppliedEffectHandle.Invalidate();
// 		}
// 	}
// }

TArray<ULyraInventoryItemInstance*> FLyraInventoryList::GetAllItems() const
{
	TArray<ULyraInventoryItemInstance*> Results;
	Results.Reserve(Entries.Num());
	for (const FLyraInventoryEntry& Entry : Entries)
	{
		if (Entry.Instance != nullptr) //@TODO: Would prefer to not deal with this here and hide it further?
		{
			Results.Add(Entry.Instance);
		}
	}
	return Results;
}

//////////////////////////////////////////////////////////////////////
// ULyraInventoryManagerComponent

ULyraInventoryManagerComponent::ULyraInventoryManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, InventoryList(this)
{
	SetIsReplicatedByDefault(true);
}

void ULyraInventoryManagerComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, InventoryList);
}

bool ULyraInventoryManagerComponent::CanAddItemDefinition(TSubclassOf<ULyraInventoryItemDefinition> ItemDef, int32 StackCount)
{
	//@TODO: Add support for stack limit / uniqueness checks / etc...
	return true;
}

ULyraInventoryItemInstance* ULyraInventoryManagerComponent::AddItemDefinition(TSubclassOf<ULyraInventoryItemDefinition> ItemDef, int32 StackCount)
{
	ULyraInventoryItemInstance* Result = nullptr;
	if (ItemDef != nullptr)
	{
		Result = InventoryList.AddEntry(ItemDef, StackCount);
		
		if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && Result)
		{
			AddReplicatedSubObject(Result);
		}
	}
	return Result;
}


ULyraInventoryItemInstance* ULyraInventoryManagerComponent::AddItemInstance(ULyraInventoryItemInstance* ItemInstance)
{
	ULyraInventoryItemInstance* Result = nullptr;
	Result = InventoryList.AddEntry(ItemInstance);
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && ItemInstance)
	{
		AddReplicatedSubObject(ItemInstance);
	}
	return Result;
}

void ULyraInventoryManagerComponent::RemoveItemInstance(ULyraInventoryItemInstance* ItemInstance)
{
	InventoryList.RemoveEntry(ItemInstance);

	if (ItemInstance && IsUsingRegisteredSubObjectList())
	{
		RemoveReplicatedSubObject(ItemInstance);
	}
}


TArray<ULyraInventoryItemInstance*> ULyraInventoryManagerComponent::GetAllItems() const
{
	return InventoryList.GetAllItems();
}

ULyraInventoryItemInstance* ULyraInventoryManagerComponent::FindFirstItemStackByDefinition(TSubclassOf<ULyraInventoryItemDefinition> ItemDef) const
{
	for (const FLyraInventoryEntry& Entry : InventoryList.Entries)
	{
		ULyraInventoryItemInstance* Instance = Entry.Instance;

		if (IsValid(Instance))
		{
			if (Instance->GetItemDef() == ItemDef)
			{
				return Instance;
			}
		}
	}

	return nullptr;
}

int32 ULyraInventoryManagerComponent::GetTotalItemCountByDefinition(TSubclassOf<ULyraInventoryItemDefinition> ItemDef) const
{
	int32 TotalCount = 0;
	for (const FLyraInventoryEntry& Entry : InventoryList.Entries)
	{
		ULyraInventoryItemInstance* Instance = Entry.Instance;

		if (IsValid(Instance))
		{
			if (Instance->GetItemDef() == ItemDef)
			{
				++TotalCount;
			}
		}
	}

	return TotalCount;
}

bool ULyraInventoryManagerComponent::ConsumeItemsByDefinition(TSubclassOf<ULyraInventoryItemDefinition> ItemDef, int32 NumToConsume)
{
	AActor* OwningActor = GetOwner();
	if (!OwningActor || !OwningActor->HasAuthority())
	{
		return false;
	}

	//@TODO: N squared right now as there's no acceleration structure
	int32 TotalConsumed = 0;
	while (TotalConsumed < NumToConsume)
	{
		if (ULyraInventoryItemInstance* Instance = ULyraInventoryManagerComponent::FindFirstItemStackByDefinition(ItemDef))
		{
			InventoryList.RemoveEntry(Instance);
			++TotalConsumed;
		}
		else
		{
			return false;
		}
	}

	return TotalConsumed == NumToConsume;
}

TArray<ULyraInventoryItemInstance*> ULyraInventoryManagerComponent::FindItemsProvidingSharedStat(FGameplayTag SharedTag)
{
	TArray<ULyraInventoryItemInstance*> MatchingItems;

	for (ULyraInventoryItemInstance* Item : InventoryList.GetAllItems())
	{
		if (!Item) continue;

		if (Item->HasSharedStatTag(SharedTag))
		{
			MatchingItems.Add(Item);
		}
	}

	return MatchingItems;
}

int32 ULyraInventoryManagerComponent::GetTotalSharedStatStack(FGameplayTag SharedTag)
{
	if (!SharedTag.IsValid())
	{
		return 0;
	}
	
	int32 Total = 0;

	for (ULyraInventoryItemInstance* Item : InventoryList.GetAllItems())
	{
		if (!Item) continue;
		int32 Count = Item->GetSharedStackCount(SharedTag);
		Total += Count;
	}

	return Total;
}

bool ULyraInventoryManagerComponent::ConsumeFromSmallestStack(FGameplayTag SharedTag, int32 Amount)
{
	if (Amount <= 0)
	{
		return false;
	}

	// Find all items that provide the shared stat
	TArray<ULyraInventoryItemInstance*> MatchingItems = FindItemsProvidingSharedStat(SharedTag);
	if (MatchingItems.Num() == 0)
	{
		return false;
	}

	// Sort by stack count ascending (smallest first)
	MatchingItems.Sort([&SharedTag](const ULyraInventoryItemInstance& A, const ULyraInventoryItemInstance& B)
	{
		const int32 StackA = A.GetSharedStackCount(SharedTag);
		const int32 StackB = B.GetSharedStackCount(SharedTag);
		return StackA < StackB;
	});

	int32 Remaining = Amount;
	TArray<ULyraInventoryItemInstance*> ItemsToRemove;

	for (ULyraInventoryItemInstance* Item : MatchingItems)
	{
		if (!Item) continue;

		const int32 CurrentCount = Item->GetSharedStackCount(SharedTag);

		if (CurrentCount <= 0)
		{
			continue;
		}
		
		const int32 ToConsume = FMath::Min(CurrentCount, Remaining);
		if (CurrentCount <= ToConsume)
		{
			//Consume entire item stack
			ItemsToRemove.Add(Item);
		}
		else
		{
			Item->RemoveStatTagStack(SharedTag, ToConsume);
		}
		Remaining -= ToConsume;
		
		
		if (Remaining <= 0)
		{
			break; 
		}
	}

	for (ULyraInventoryItemInstance* Item : ItemsToRemove)
	{
		RemoveItemInstance(Item);
	}
	
	return Remaining <= 0;
}

void ULyraInventoryManagerComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	// Register existing ULyraInventoryItemInstance
	if (IsUsingRegisteredSubObjectList())
	{
		for (const FLyraInventoryEntry& Entry : InventoryList.Entries)
		{
			ULyraInventoryItemInstance* Instance = Entry.Instance;

			if (IsValid(Instance))
			{
				AddReplicatedSubObject(Instance);
			}
		}

	}
}


bool ULyraInventoryManagerComponent::ReplicateSubobjects(UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (FLyraInventoryEntry& Entry : InventoryList.Entries)
	{
		ULyraInventoryItemInstance* Instance = Entry.Instance;

		if (Instance && IsValid(Instance))
		{
			WroteSomething |= Channel->ReplicateSubobject(Instance, *Bunch, *RepFlags);
		}
	}

	return WroteSomething;
}

//////////////////////////////////////////////////////////////////////
//

// UCLASS(Abstract)
// class ULyraInventoryFilter : public UObject
// {
// public:
// 	virtual bool PassesFilter(ULyraInventoryItemInstance* Instance) const { return true; }
// };

// UCLASS()
// class ULyraInventoryFilter_HasTag : public ULyraInventoryFilter
// {
// public:
// 	virtual bool PassesFilter(ULyraInventoryItemInstance* Instance) const { return true; }
// };


