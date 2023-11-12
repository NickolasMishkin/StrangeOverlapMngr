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

	void Update();

	bool AddWidgetComponents(TArray<UWidgetComponent*>& WidgetComponents);
	bool AddWidgetComponent(UWidgetComponent* WidgetComponent);
	bool RemoveWidgetComponent(UWidgetComponent* WidgetComponent);
	bool Destroy();

	FORCEINLINE int32 GetItemsCount() const { return Items.Num(); }
	void Init(EControlOverlapType NewControlOverlapType, const FUMGOverlapControlGroupSettings& InSettings);

protected:

	bool NeedUpdate() const;

private:

	UUMGControlOverlapItem* GetItemByWidgetComponent(UWidgetComponent* WidgetComponent) const;

	bool GetAndPrepareItemsForAllign(TArray<UUMGControlOverlapItem*>& ItemsToAllign, TArray<FVector2D>& Positions);

	FVector2D ViewPortSize{ 0.0f,0.0f };

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
	bool AddWidgetComponentToGroup(UWidgetComponent* WidgetComponent, const FString& TagId);
	bool RemoveWidgetComponentFromGroup(UWidgetComponent* WidgetComponent, const FString& TagId);

	FORCEINLINE void SetControlOverlapType(EControlOverlapType NewControlOverlapType) { ControlOverlapType = NewControlOverlapType; }
private:

	UPROPERTY()
	TMap<FString, UUMGControlOverlapGroup*> GroupByTag;

	UPROPERTY()
	EControlOverlapType ControlOverlapType;

};