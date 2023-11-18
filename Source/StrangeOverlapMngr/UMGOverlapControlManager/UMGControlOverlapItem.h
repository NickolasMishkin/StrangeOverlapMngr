#pragma once

#include "CoreMinimal.h"
#include "UMGControlOverlapItem.generated.h"

class UUMGControlOverlapGroup;
class UWidgetComponent;

UCLASS(Blueprintable)
class STRANGEOVERLAPMNGR_API UUMGControlOverlapItem : public UObject
{
	GENERATED_BODY()

public:

	FVector2D PosForSort{ 0.0f,0.0f };

	bool SetPositionInViewport(APlayerController* PlayerController, const FVector2D& Position);
	void SetWorldLocation(const FVector& NewPosition);
	FVector2D GetDesiredSize() const;
	bool GetPositionInViewport(APlayerController* PlayerController, FVector2D& ViewPortPosition) const;
	void SetStartedLoaction();
	void SetControlledWidgetComponent(UWidgetComponent* WidgetComponent);
	void Destroy();

	FORCEINLINE UWidgetComponent* GetWidgetComponent() const { return m_ControlledWidgetComponent; }


	FVector GetWorldLocation() const;

	FORCEINLINE bool IsGrouping() const { return m_bIsGrouping; }
	void SetIsGrouping(bool NewbIsGrouping);
	FORCEINLINE FVector GetStartedPosition() const { return m_StartedWorldPosition; }

	void UpdateIndex(int32 NewIndex);

private:

	int32 m_Index = -1;

	bool m_bIsGrouping = false;

	UPROPERTY()
	UWidgetComponent* m_ControlledWidgetComponent = nullptr;

	FVector m_StartedWorldPosition = FVector(0.0f, 0.0f, 0.0f);
	FVector m_StartedRelativeLocation = FVector(0.0f, 0.0f, 0.0f);

	UPROPERTY()
	AActor* m_Owner = nullptr;
};
