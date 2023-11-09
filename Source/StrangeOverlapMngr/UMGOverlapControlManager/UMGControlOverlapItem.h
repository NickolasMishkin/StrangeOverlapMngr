#pragma once

#include "CoreMinimal.h"
#include "UMGControlOverlapItem.generated.h"

class UUMGControlOverlapGroup;
class UWidgetComponent;

UCLASS()
class STRANGEOVERLAPMNGR_API UUMGControlOverlapItem : public UObject
{
	GENERATED_BODY()

public:

	FVector2D PosForSort{ 0.0f,0.0f };

	bool SetPositionInViewport(APlayerController* PlayerController, const FVector2D& Position);
	void SetWorldLocation(const FVector& NewPosition);
	FVector2D GetDesiredSize() const;
	bool GetPositionInViewport(APlayerController* PlayerController, FVector2D& ViewPortPosition) const;

	FORCEINLINE void SetIndex(int32 NewIndex) { Index = NewIndex; }
	void SetControlledWidgetComponent(UWidgetComponent* WidgetComponent);
	void Destroy();

	FORCEINLINE UWidgetComponent* GetWidgetComponent() const { return ControlledWidgetComponent; }

	FVector GetWorldLocation() const;

	FORCEINLINE bool IsGrouping() const { return bIsGrouping; }
	FORCEINLINE void SetIsGrouping(bool NewbIsGrouping) { bIsGrouping = NewbIsGrouping; };
	FORCEINLINE FVector GetStartedPosition() const { return StartedWorldPosition; }

private:

	int32 Index = -1;

	bool bIsGrouping = false;

	UPROPERTY()
	UWidgetComponent* ControlledWidgetComponent = nullptr;

	FVector StartedWorldPosition;
};
