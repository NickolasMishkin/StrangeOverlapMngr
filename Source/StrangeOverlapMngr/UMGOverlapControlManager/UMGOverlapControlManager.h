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

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	bool TestFunc();
};

UCLASS(BlueprintType)
class STRANGEOVERLAPMNGR_API UUMGOverlapControlManager : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	bool RemoveControlOverlapGroup(EControlOverlapType ControlOverlapType, const FString& GroupTagId);

	UFUNCTION(BlueprintCallable)
	bool CreateControlOverlapGroup(TArray<UWidgetComponent*> WidgetComponents, EControlOverlapType ControlOverlapType, const FString& GroupTagId, const FUMGOverlapControlGroupSettings& GroupSettings);

protected:

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:

	UPROPERTY()
	TMap<EControlOverlapType, UUMGControlOverlapGroupContainer*> GroupsByControlOverlapType;
};
