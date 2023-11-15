// Fill out your copyright notice in the Description page of Project Settings.

#include "UMGOverlapControlManager.h"
#include "UMGControlOverlapItem.h"
#include "UMGControlOverlapGroup.h"
#include "Components/WidgetComponent.h"
#include <Kismet/GameplayStatics.h>

bool UUMGOverlapControlManager::RemoveControlOverlapGroup(EControlOverlapType ControlOverlapType, const FString& GroupTagId)
{
    if (ControlOverlapType != EControlOverlapType::None && !GroupTagId.IsEmpty())
    {
        if (m_GroupsByControlOverlapType.Contains(ControlOverlapType))
        {
            if (auto lGroupsContainer = *m_GroupsByControlOverlapType.Find(ControlOverlapType))
            {
                return lGroupsContainer->RemoveGroup(GroupTagId);
            }
        }
    }
    return false;
}

void UUMGOverlapControlManager::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
    if (PlayerController && PlayerController->GetPawn())
    {
        FVector CurrentCameraPosition = PlayerController->GetPawn()->GetActorLocation();

        // Check if the position has changed
        if (!LastCameraPosition.Equals(CurrentCameraPosition))
        {
            for (const auto& It : m_GroupsByControlOverlapType)
            {
                It.Value->Update();
            }
            LastCameraPosition = CurrentCameraPosition;
        }
    }
    
    
}

bool UUMGOverlapControlManager::CreateControlOverlapGroup(TArray<UWidgetComponent*> WidgetComponents, EControlOverlapType ControlOverlapType, const FString& GroupTagId, const FUMGOverlapControlGroupSettings& GroupSettings)
{
    /*FTimerHandle Handle;
    GetWorld()->GetTimerManager().SetTimer(Handle, FTimerDelegate::CreateLambda([&] { for (const auto& It : m_GroupsByControlOverlapType)
    {
        It.Value->Update();
    } }), 1.0f, true);*/
    
    if (WidgetComponents.Num() <= 0 || WidgetComponents.Num() > GroupSettings.MaxItemsCount || ControlOverlapType == EControlOverlapType::None || GroupTagId.IsEmpty())
    {
        return false;
    }

    if (m_GroupsByControlOverlapType.Contains(ControlOverlapType))
    {
        if (auto lGroupsContainer = *m_GroupsByControlOverlapType.Find(ControlOverlapType))
        {
            return lGroupsContainer->CreateGroup(WidgetComponents, GroupTagId, GroupSettings);
        }
    }

    auto LNewGroupsContainer = NewObject<UUMGControlOverlapGroupContainer>(this, UUMGControlOverlapGroupContainer::StaticClass());
    LNewGroupsContainer->SetControlOverlapType(ControlOverlapType);
    m_GroupsByControlOverlapType.Add(ControlOverlapType, LNewGroupsContainer);
    return LNewGroupsContainer->CreateGroup(WidgetComponents, GroupTagId, GroupSettings);
}

bool UUMGOverlapControlManager::AddWidgetComponentToControlOverlapGroup(UWidgetComponent* WidgetComponent, EControlOverlapType ControlOverlapType, const FString& GroupTagId)
{
    if (WidgetComponent && ControlOverlapType != EControlOverlapType::None && !GroupTagId.IsEmpty() && m_GroupsByControlOverlapType.Contains(ControlOverlapType))
    {
        if (auto lGroupsContainer = *m_GroupsByControlOverlapType.Find(ControlOverlapType))
        {
            return lGroupsContainer->AddWidgetComponentToGroup(WidgetComponent, GroupTagId);
        }
    }
    return false;
}

bool UUMGOverlapControlManager::RemoveWidgetComponentFromControlOverlapGroup(UWidgetComponent* WidgetComponent, EControlOverlapType ControlOverlapType, const FString& GroupTagId)
{
    if (WidgetComponent && ControlOverlapType != EControlOverlapType::None && !GroupTagId.IsEmpty())
    {
        if (m_GroupsByControlOverlapType.Contains(ControlOverlapType))
        {
            if (auto lGroupsContainer = *m_GroupsByControlOverlapType.Find(ControlOverlapType))
            {
                return lGroupsContainer->RemoveWidgetComponentFromGroup(WidgetComponent, GroupTagId);
            }
        }
    }
    return false;
}
