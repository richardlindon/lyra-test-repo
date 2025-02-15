// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroClassListData.h"


TArray<FString> UHeroClassListData::GetAllHeroClassIDs()
{
	TArray<FString> IDs;
	for (const TObjectPtr<UHeroClassData> HeroClass : HeroClasses)
	{
		if (HeroClass)
		{
			IDs.Add(HeroClass->UniqueID);  
		}
	}
	return IDs;
}
