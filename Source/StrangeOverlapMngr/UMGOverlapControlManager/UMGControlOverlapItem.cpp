// Fill out your copyright notice in the Description page of Project Settings.
#include "UMGControlOverlapItem.h"
#include "UMGControlOverlapGroup.h"
#include "Blueprint/SlateBlueprintLibrary.h"
#include "Blueprint/UserWidget.h"
#include "Components/WidgetComponent.h"
#include <Kismet/GameplayStatics.h>

bool UUMGControlOverlapItem::SetPositionInViewport(APlayerController* PlayerController, const FVector2D& Position)
{
    if (PlayerController)
    {
        FVector NewWorldPosition, WorldDirection;
        if (PlayerController->DeprojectScreenPositionToWorld(Position.X, Position.Y, NewWorldPosition, WorldDirection))
        {
            SetWorldLocation(NewWorldPosition);
            return true;
        }
        else
        {
            return false;
        }
            
    }
    return false;
}

void UUMGControlOverlapItem::SetWorldLocation(const FVector& NewPosition)
{
    if (ControlledWidgetComponent)
    {
        ControlledWidgetComponent->SetWorldLocation(NewPosition);
    }
}

FVector2D UUMGControlOverlapItem::GetDesiredSize() const
{
    if (ControlledWidgetComponent)
    {
        return ControlledWidgetComponent->GetDrawSize();
    }
    return FVector2D();
}

bool UUMGControlOverlapItem::GetPositionInViewport(APlayerController* PlayerController, FVector2D& ViewPortPosition) const
{
    return UGameplayStatics::ProjectWorldToScreen(PlayerController, GetWorldLocation(), ViewPortPosition);
}

void UUMGControlOverlapItem::SetStartedLoaction()
{
    if (ControlledWidgetComponent)
    {
        ControlledWidgetComponent->SetWorldLocation(StartedWorldPosition);
        GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("SetWorldLocation"), true, FVector2D(1.0f, 1.0f));
    }
}

void UUMGControlOverlapItem::SetControlledWidgetComponent(UWidgetComponent* WidgetComponent)
{
    ControlledWidgetComponent = WidgetComponent;
    StartedWorldPosition = GetWorldLocation();
}

void UUMGControlOverlapItem::Destroy()
{
    ConditionalBeginDestroy();// may be need destroy upper level
}

FVector UUMGControlOverlapItem::GetWorldLocation() const
{
    if (ControlledWidgetComponent)
    {
        return ControlledWidgetComponent->GetComponentLocation();
    }
    return FVector();
}
