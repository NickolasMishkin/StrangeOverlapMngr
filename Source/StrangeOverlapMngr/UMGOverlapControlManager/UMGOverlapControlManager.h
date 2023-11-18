// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UMGOverlapControlManager.generated.h"

class UUMGControlOverlapItem;
class UUMGControlOverlapGroup;
class UUMGControlOverlapGroupContainer;
class UWidgetComponent;

UENUM(Blueprintable)
enum class EControlOverlapType : uint8
{
	None = 0,
	LineAllign
};

USTRUCT(Blueprintable)
struct FUMGOverlapControlGroupSettings
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int32 MaxItemsCount = 5; 
};

UINTERFACE(Blueprintable)
class STRANGEOVERLAPMNGR_API UUMGOverlapWidgetInterface : public UInterface
{
	GENERATED_BODY()
};

class STRANGEOVERLAPMNGR_API IUMGOverlapWidgetInterface
{
	GENERATED_BODY()

public:

	//Call when overlap item index in group has been changed
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Strange Components")
	void UpdateIndexInGroup(int32 NewIndex);

	//Call when overlap item has been removed from or added to a group
	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Strange Components")
	void UpdateInGroup(bool bInGroup);
};

UCLASS(BlueprintType)
class STRANGEOVERLAPMNGR_API UUMGOverlapControlManager : public UUserWidget
{
	GENERATED_BODY()
	
public:

	//Remove control overlap group by tag id and and control overlap type
	UFUNCTION(BlueprintCallable, Category = "Strange Components")
	bool RemoveControlOverlapGroup(EControlOverlapType ControlOverlapType, const FString& GroupTagId);

	//Create control overlap group with tag id and control overlap type of widget components
	UFUNCTION(BlueprintCallable, Category = "Strange Components")
	bool CreateControlOverlapGroup(TArray<UWidgetComponent*> WidgetComponents, EControlOverlapType ControlOverlapType, const FString& GroupTagId, const FUMGOverlapControlGroupSettings& GroupSettings);

	//Add widget component to control overlap group with tag id and control overlap type
	UFUNCTION(BlueprintCallable, Category = "Strange Components")
	bool AddWidgetComponentToControlOverlapGroup(UWidgetComponent* WidgetComponent, EControlOverlapType ControlOverlapType, const FString& GroupTagId);

	//Remove widget component from control overlap group with tag id and control overlap type
	UFUNCTION(BlueprintCallable, Category = "Strange Components")
	bool RemoveWidgetComponentFromControlOverlapGroup(UWidgetComponent* WidgetComponent, EControlOverlapType ControlOverlapType, const FString& GroupTagId);

protected:

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:

	UPROPERTY()
	TMap<EControlOverlapType, UUMGControlOverlapGroupContainer*> m_GroupsByControlOverlapType;
};
