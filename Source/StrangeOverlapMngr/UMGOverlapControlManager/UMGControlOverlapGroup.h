// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UMGOverlapControlManager.h"
#include "UMGControlOverlapGroup.generated.h"

class UUMGControlOverlapItem;
//enum class EControlOverlapType;

UCLASS()
class STRANGEOVERLAPMNGR_API UUMGControlOverlapGroup : public UObject
{
	GENERATED_BODY()

public:

	virtual void Update();

	bool AddWidgetComponents(TArray<UWidgetComponent*>& WidgetComponents);
	bool AddWidgetComponent(UWidgetComponent* WidgetComponent);

	FORCEINLINE int32 GetItemsCount() const { return Items.Num(); }
	void Init(EControlOverlapType NewControlOverlapType, const FUMGOverlapControlGroupSettings& InSettings);

protected:

	virtual bool NeedUpdate() const;

private:

	UPROPERTY()
	EControlOverlapType ControlOverlapType;

	UPROPERTY()
	TArray<UUMGControlOverlapItem*> Items;

	UPROPERTY()
	int32 MaxItemsCount = 5;

	UPROPERTY()
	APlayerController* PlayerController = nullptr;

	bool ItemsItersect(UUMGControlOverlapItem* ItemA, UUMGControlOverlapItem* ItemB) const;
};


UCLASS()
class STRANGEOVERLAPMNGR_API UUMGControlOverlapGroupContainer : public UObject
{
	GENERATED_BODY()

public:
	
	void Update();

	bool RemoveGroup(const FString& TagId);
	bool CreateGroup(TArray<UWidgetComponent*>& WidgetCompoennts, const FString& TagId, const FUMGOverlapControlGroupSettings& Settings);

	FORCEINLINE void SetControlOverlapType(EControlOverlapType NewControlOverlapType) { ControlOverlapType = NewControlOverlapType; }
private:

	int32 chance = 0;

	UPROPERTY()
	TMap<FString, UUMGControlOverlapGroup*> GroupByTag;

	UPROPERTY()
	EControlOverlapType ControlOverlapType;

};