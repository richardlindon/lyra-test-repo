// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "AbilitySystem/LyraAbilitySet.h"

#include "LyraInventoryManagerComponent.generated.h"

class ULyraInventoryItemDefinition;
class ULyraAbilitySystemComponent;
class ULyraInventoryItemInstance;
class ULyraInventoryManagerComponent;
class UObject;
struct FFrame;
struct FLyraInventoryList;
struct FNetDeltaSerializeInfo;
struct FReplicationFlags;

/** A message when an item is added to the inventory */
USTRUCT(BlueprintType)
struct FLyraInventoryChangeMessage
{
	GENERATED_BODY()

	//@TODO: Tag based names+owning actors for inventories instead of directly exposing the component?
	UPROPERTY(BlueprintReadOnly, Category=Inventory)
	TObjectPtr<UActorComponent> InventoryOwner = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = Inventory)
	TObjectPtr<ULyraInventoryItemInstance> Instance = nullptr;

	UPROPERTY(BlueprintReadOnly, Category=Inventory)
	int32 NewCount = 0;

	UPROPERTY(BlueprintReadOnly, Category=Inventory)
	int32 Delta = 0;
};

/** A single entry in an inventory */
USTRUCT(BlueprintType)
struct FLyraInventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FLyraInventoryEntry()
	{}

	FString GetDebugString() const;

private:
	friend FLyraInventoryList;
	friend ULyraInventoryManagerComponent;

	UPROPERTY()
	TObjectPtr<ULyraInventoryItemInstance> Instance = nullptr;

	UPROPERTY()
	int32 StackCount = 0;

	UPROPERTY(NotReplicated)
	int32 LastObservedCount = INDEX_NONE;
	
	// Authority-only list of granted handles
	// TODO: Remove as we moving into the item instance instead
	UPROPERTY(NotReplicated)
	FLyraAbilitySet_GrantedHandles GrantedHandles;
};

/** List of inventory items */
USTRUCT(BlueprintType)
struct FLyraInventoryList : public FFastArraySerializer
{
	GENERATED_BODY()

	FLyraInventoryList()
		: OwnerComponent(nullptr)
	{
	}

	FLyraInventoryList(UActorComponent* InOwnerComponent)
		: OwnerComponent(InOwnerComponent)
	{
	}

	TArray<ULyraInventoryItemInstance*> GetAllItems() const;

public:
	//~FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FLyraInventoryEntry, FLyraInventoryList>(Entries, DeltaParms, *this);
	}

	ULyraInventoryItemInstance* AddEntry(TSubclassOf<ULyraInventoryItemDefinition> ItemClass, int32 StackCount);
	ULyraInventoryItemInstance* AddEntry(ULyraInventoryItemInstance* Instance);

	void RemoveEntry(ULyraInventoryItemInstance* Instance);
	// void RemoveItemEffect(ULyraInventoryItemInstance* ItemInstance);

private:
	void BroadcastChangeMessage(FLyraInventoryEntry& Entry, int32 OldCount, int32 NewCount);
	ULyraAbilitySystemComponent* GetAbilitySystemComponent() const;

private:
	friend ULyraInventoryManagerComponent;

private:
	// Replicated list of items
	UPROPERTY()
	TArray<FLyraInventoryEntry> Entries;

	UPROPERTY(NotReplicated)
	TObjectPtr<UActorComponent> OwnerComponent;
};

template<>
struct TStructOpsTypeTraits<FLyraInventoryList> : public TStructOpsTypeTraitsBase2<FLyraInventoryList>
{
	enum { WithNetDeltaSerializer = true };
};










/**
 * Manages an inventory
 */
UCLASS(BlueprintType)
class LYRAGAME_API ULyraInventoryManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULyraInventoryManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	bool CanAddItemDefinition(TSubclassOf<ULyraInventoryItemDefinition> ItemDef, int32 StackCount = 1);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	ULyraInventoryItemInstance* AddItemDefinition(TSubclassOf<ULyraInventoryItemDefinition> ItemDef, int32 StackCount = 1);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	ULyraInventoryItemInstance* AddItemInstance(ULyraInventoryItemInstance* ItemInstance);

	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	void RemoveItemInstance(ULyraInventoryItemInstance* ItemInstance);

	UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintPure=false)
	TArray<ULyraInventoryItemInstance*> GetAllItems() const;

	UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintPure)
	ULyraInventoryItemInstance* FindFirstItemStackByDefinition(TSubclassOf<ULyraInventoryItemDefinition> ItemDef) const;

	int32 GetTotalItemCountByDefinition(TSubclassOf<ULyraInventoryItemDefinition> ItemDef) const;
	bool ConsumeItemsByDefinition(TSubclassOf<ULyraInventoryItemDefinition> ItemDef, int32 NumToConsume);

	// Get all items that provide the shared tag
	UFUNCTION(BlueprintCallable, Category=Inventory)
	TArray<ULyraInventoryItemInstance*> FindItemsProvidingSharedStat(FGameplayTag SharedTag);

	// Get current total for a shared tag
	UFUNCTION(BlueprintCallable, Category=Inventory)
	int32 GetTotalSharedStatStack(FGameplayTag SharedTag);
	
	// Remove N ammo from the smallest matching stack
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	bool ConsumeFromSmallestStack(FGameplayTag SharedTag, int32 Amount);

	
	//~UObject interface
	virtual bool ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags) override;
	virtual void ReadyForReplication() override;
	//~End of UObject interface

private:
	UPROPERTY(Replicated)
	FLyraInventoryList InventoryList;
		
};


