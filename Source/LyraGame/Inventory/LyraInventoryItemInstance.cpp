// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraInventoryItemInstance.h"

#include "Inventory/LyraInventoryItemDefinition.h"
#include "Net/UnrealNetwork.h"

#if UE_WITH_IRIS
#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
#endif // UE_WITH_IRIS

#include "InventoryFragment_SharedStatTags.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraInventoryItemInstance)

class FLifetimeProperty;

ULyraInventoryItemInstance::ULyraInventoryItemInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void ULyraInventoryItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, StatTags);
	DOREPLIFETIME(ThisClass, ItemDef);
}

#if UE_WITH_IRIS
void ULyraInventoryItemInstance::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	using namespace UE::Net;

	// Build descriptors and allocate PropertyReplicationFragments for this object
	FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif // UE_WITH_IRIS

void ULyraInventoryItemInstance::AddStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.AddStack(Tag, StackCount);
}

void ULyraInventoryItemInstance::RemoveStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.RemoveStack(Tag, StackCount);
}

int32 ULyraInventoryItemInstance::GetStatTagStackCount(FGameplayTag Tag) const
{
	return StatTags.GetStackCount(Tag);
}

int32 ULyraInventoryItemInstance::GetSharedStackCount(FGameplayTag SharedTag) const
{
	if (const UInventoryFragment_SharedStatTags* SharedStatFragment = FindFragmentByClass<UInventoryFragment_SharedStatTags>())
	{
		if (SharedStatFragment->SharedStatTags.HasTag(SharedTag))
		{
			return GetStatTagStackCount(SharedTag);
		}
	}
	return 0;
}


bool ULyraInventoryItemInstance::HasStatTag(FGameplayTag Tag) const
{
	return StatTags.ContainsTag(Tag);
}

bool ULyraInventoryItemInstance::HasSharedStatTag(FGameplayTag SharedTag) const
{
	if (const UInventoryFragment_SharedStatTags* SharedStatFragment = FindFragmentByClass<UInventoryFragment_SharedStatTags>())
	{
		return SharedStatFragment->SharedStatTags.HasTag(SharedTag);
	}
	return false;
}

FGameplayTag ULyraInventoryItemInstance::GetConsumedSharedStatTag() const
{
	if (const UInventoryFragment_SharedStatTags* SharedStatFragment = FindFragmentByClass<UInventoryFragment_SharedStatTags>())
	{
		return SharedStatFragment->ConsumedSharedStatStack;
	}
	return FGameplayTag::EmptyTag;
}

void ULyraInventoryItemInstance::SetItemDef(TSubclassOf<ULyraInventoryItemDefinition> InDef)
{
	ItemDef = InDef;
}

const ULyraInventoryItemFragment* ULyraInventoryItemInstance::FindFragmentByClass(TSubclassOf<ULyraInventoryItemFragment> FragmentClass) const
{
	if ((ItemDef != nullptr) && (FragmentClass != nullptr))
	{
		return GetDefault<ULyraInventoryItemDefinition>(ItemDef)->FindFragmentByClass(FragmentClass);
	}

	return nullptr;
}


