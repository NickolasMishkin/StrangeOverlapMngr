// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UMGOverlapControlManager.h"
#include "UMGControlOverlapGroup.generated.h"

class UUMGControlOverlapItem;

UCLASS()
class STRANGEOVERLAPMNGR_API UUMGControlOverlapGroup : public UObject
{
	GENERATED_BODY()

public:

	//Changes the state of widgets according to the type of overlay
	void Update();

	//Add widget components to a group
	bool AddWidgetComponents(TArray<UWidgetComponent*>& WidgetComponents);

	//Add widget component to a group
	bool AddWidgetComponent(UWidgetComponent* WidgetComponent);

	//Remove widget component from group
	bool RemoveWidgetComponent(UWidgetComponent* WidgetComponent);

	//Destroy group
	bool Destroy();

	//Initialize properties needed for work
	void Init(EControlOverlapType NewControlOverlapType, const FUMGOverlapControlGroupSettings& InSettings);

protected:

	//Check condition for update items
	bool NeedUpdate() const;

private:

	bool IsItemOverlapedWithOther(const UUMGControlOverlapItem* CheckingItem, const TArray<UUMGControlOverlapItem*>& Items) const;

	UUMGControlOverlapItem* GetItemByWidgetComponent(UWidgetComponent* WidgetComponent) const;

	bool GetAndPrepareItemsForAllign(TArray<UUMGControlOverlapItem*>& ItemsToAllign, TArray<FVector2D>& Positions, TArray<UUMGControlOverlapItem*>& ItemsForCorrectPos);

	bool ItemsItersect(const UUMGControlOverlapItem* ItemA, const UUMGControlOverlapItem* ItemB) const;

	bool ItemsPositionsItersect(const FVector2D& ItemAPos, const UUMGControlOverlapItem* ItemA, const UUMGControlOverlapItem* ItemB) const;

private:

	UPROPERTY()
	EControlOverlapType m_ControlOverlapType = EControlOverlapType::None;

	UPROPERTY()
	TArray<UUMGControlOverlapItem*> m_Items;

	UPROPERTY()
	APlayerController* m_PlayerController = nullptr;

	FVector2D m_ViewPortSize = FVector2D(0.0f,0.0f);

	int32 m_MaxItemsCount = 5;
};


UCLASS()
class STRANGEOVERLAPMNGR_API UUMGControlOverlapGroupContainer : public UObject
{
	GENERATED_BODY()

public:
	
	//Update control overlap groups
	void Update();

	//Remove control overlap group by tag id
	bool RemoveGroup(const FString& TagId);

	//Add control overlap group with tag id of widget components
	bool CreateGroup(TArray<UWidgetComponent*>& WidgetCompoennts, const FString& TagId, const FUMGOverlapControlGroupSettings& Settings);

	//Add widget component to control overlap group with tag id
	bool AddWidgetComponentToGroup(UWidgetComponent* WidgetComponent, const FString& TagId);

	//Remove widget component from control overlap group by tag id
	bool RemoveWidgetComponentFromGroup(UWidgetComponent* WidgetComponent, const FString& TagId);

	//Set control overlap type for this groups container
	FORCEINLINE void SetControlOverlapType(EControlOverlapType NewControlOverlapType) { m_ControlOverlapType = NewControlOverlapType; }

private:

	UPROPERTY()
	TMap<FString, UUMGControlOverlapGroup*> m_GroupByTag;

	UPROPERTY()
	EControlOverlapType m_ControlOverlapType = EControlOverlapType::None;

};