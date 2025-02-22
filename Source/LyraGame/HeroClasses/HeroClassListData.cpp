// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroClassListData.h"


TArray<FGameplayTag> UHeroClassListData::GetAllHeroClassTags()
{
	TArray<FGameplayTag> IDs;
	for (const TObjectPtr<UHeroClassData> HeroClass : HeroClasses)
	{
		if (HeroClass)
		{
			IDs.Add(HeroClass->ClassTag);  
		}
	}
	return IDs;
}
