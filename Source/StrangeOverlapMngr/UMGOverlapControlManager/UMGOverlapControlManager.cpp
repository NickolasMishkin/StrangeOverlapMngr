// Fill out your copyright notice in the Description page of Project Settings.

#include "UMGOverlapControlManager.h"
#include "UMGControlOverlapItem.h"
#include "UMGControlOverlapGroup.h"
#include "Components/WidgetComponent.h"

bool UUMGOverlapControlManager::RemoveControlOverlapGroup(EControlOverlapType ControlOverlapType, const FString& GroupTagId)
{
    if (ControlOverlapType != EControlOverlapType::None && !GroupTagId.IsEmpty())
    {
        if (GroupsByControlOverlapType.Contains(ControlOverlapType))
        {
            if (auto GroupsContainer = *GroupsByControlOverlapType.Find(ControlOverlapType))
            {
                return GroupsContainer->RemoveGroup(GroupTagId);
            }
        }
    }
    return false;
}

void UUMGOverlapControlManager::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    for (const auto& It : GroupsByControlOverlapType)
    {
        It.Value->Update();
    }
}

bool UUMGOverlapControlManager::CreateControlOverlapGroup(TArray<UWidgetComponent*> WidgetComponents, EControlOverlapType ControlOverlapType, const FString& GroupTagId, const FUMGOverlapControlGroupSettings& GroupSettings)
{
    if (WidgetComponents.Num() <= 0 || WidgetComponents.Num() > GroupSettings.MaxItemsCount || ControlOverlapType == EControlOverlapType::None || GroupTagId.IsEmpty())
    {
        return false;
    }

    if (GroupsByControlOverlapType.Contains(ControlOverlapType))
    {
        if (auto GroupsContainer = *GroupsByControlOverlapType.Find(ControlOverlapType))
        {
            return GroupsContainer->CreateGroup(WidgetComponents, GroupTagId, GroupSettings);
        }
    }

    auto NewGroupsContainer = NewObject<UUMGControlOverlapGroupContainer>(this, UUMGControlOverlapGroupContainer::StaticClass());
    NewGroupsContainer->SetControlOverlapType(ControlOverlapType);
    GroupsByControlOverlapType.Add(ControlOverlapType, NewGroupsContainer);
    return NewGroupsContainer->CreateGroup(WidgetComponents, GroupTagId, GroupSettings);
}