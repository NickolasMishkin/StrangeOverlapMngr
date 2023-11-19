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

	FVector2D CachedVieportPosition = FVector2D(0.0f, 0.0f);

	//Get widget viewport size
	FVector2D GetDesiredSize() const;

	//Get widget position in viewport
	bool GetPositionInViewport(APlayerController* PlayerController, FVector2D& ViewPortPosition) const;

	//Get widget component
	FORCEINLINE UWidgetComponent* GetWidgetComponent() const { return m_ControlledWidgetComponent; }

	//Get widget component world location
	FVector GetWorldLocation() const;

	//Return if is item in the group
	FORCEINLINE bool IsGrouping() const { return m_bIsGrouping; }
	
	//Get widget component started world location
	FORCEINLINE FVector GetStartedPosition() const { return m_StartedWorldPosition; }

	//Try set widget component viewport position
	bool SetPositionInViewport(APlayerController* PlayerController, const FVector2D& Position);

	//Set widget compoentn world location
	void SetWorldLocation(const FVector& NewPosition);
	
	//Set widget compoennt world location
	void SetStartedLoaction();
	
	//Set widget compoent for control
	void SetControlledWidgetComponent(UWidgetComponent* WidgetComponent);
	
	//Set item in the group
	void SetIsGrouping(bool NewbIsGrouping);
	
	//Set item index in overlap group
	void UpdateIndex(int32 NewIndex);
	
	//Destroy logic
	void Destroy();

private:

	UPROPERTY()
	UWidgetComponent* m_ControlledWidgetComponent = nullptr;

	int32 m_Index = -1;
	bool m_bIsGrouping = false;
	FVector m_StartedWorldPosition = FVector(0.0f, 0.0f, 0.0f);

	//FVector m_StartedRelativeLocation = FVector(0.0f, 0.0f, 0.0f);

	/*UPROPERTY()
	AActor* m_Owner = nullptr;*/
};
