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
        FVector lNewWorldPosition, lWorldDirection;
        if (PlayerController->DeprojectScreenPositionToWorld(Position.X, Position.Y, lNewWorldPosition, lWorldDirection))
        {
            SetWorldLocation(lNewWorldPosition);
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
    if (m_ControlledWidgetComponent)
    {
        m_ControlledWidgetComponent->SetWorldLocation(NewPosition);
    }
}

FVector2D UUMGControlOverlapItem::GetDesiredSize() const
{
    if (m_ControlledWidgetComponent)
    {
        return m_ControlledWidgetComponent->GetDrawSize();
    }
    return FVector2D();
}

bool UUMGControlOverlapItem::GetPositionInViewport(APlayerController* PlayerController, FVector2D& ViewPortPosition) const
{
    return UGameplayStatics::ProjectWorldToScreen(PlayerController, GetWorldLocation(), ViewPortPosition);
}

void UUMGControlOverlapItem::SetStartedLoaction()
{
    if (m_ControlledWidgetComponent)
    {
        m_ControlledWidgetComponent->SetWorldLocation(m_StartedWorldPosition);
        GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("SetWorldLocation"), true, FVector2D(1.0f, 1.0f));
    }
}

void UUMGControlOverlapItem::SetControlledWidgetComponent(UWidgetComponent* WidgetComponent)
{
    m_ControlledWidgetComponent = WidgetComponent;
    m_StartedWorldPosition = GetWorldLocation();
}

void UUMGControlOverlapItem::Destroy()
{
    ConditionalBeginDestroy();// may be need destroy upper level
}

FVector UUMGControlOverlapItem::GetWorldLocation() const
{
    if (m_ControlledWidgetComponent)
    {
        return m_ControlledWidgetComponent->GetComponentLocation();
    }
    return FVector();
}

void UUMGControlOverlapItem::Update(int32 NewIndex)
{
    if (m_Index != NewIndex)
    {
        m_Index = NewIndex;
        if (m_ControlledWidgetComponent->GetWidget()->GetClass()->ImplementsInterface(UUMGOverlapWidgetInterface::StaticClass()))
        {
            IUMGOverlapWidgetInterface::Execute_UpdateIndexInGroup(m_ControlledWidgetComponent->GetWidget(), m_Index);
        }
    }
}
