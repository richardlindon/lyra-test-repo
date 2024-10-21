// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemDefinitionList.h"

TArray<TSubclassOf<ULyraInventoryItemDefinition>> UItemDefinitionList::GetItems()
{
	return ItemList.Items;
}
