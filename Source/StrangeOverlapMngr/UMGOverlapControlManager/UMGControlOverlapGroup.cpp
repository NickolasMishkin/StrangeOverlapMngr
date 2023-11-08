// Fill out your copyright notice in the Description page of Project Settings.

#include "UMGControlOverlapGroup.h"
#include "UMGControlOverlapItem.h"
#include "UMGOverlapControlManager.h"
#include "Components/WidgetComponent.h"
#include <Kismet/GameplayStatics.h>
#include <Blueprint/WidgetLayoutLibrary.h>

void UUMGControlOverlapGroup::Update()
{
    if (!NeedUpdate())
    {
        return;
    }

    struct Params
    {
        FVector2D ViewportPosition;
        bool bInViewPort = false;
    };

    TArray<FVector2D> ViewportPositions;

    for (int32 i = 0; i < Items.Num(); i++)
    {
    }
    Items.Sort([&](const UUMGControlOverlapItem& ItemA, const UUMGControlOverlapItem& ItemB)
    {
        FVector2D PosA, PosB;
        ItemA.GetPositionInViewport(PlayerController, PosA);
        ItemB.GetPositionInViewport(PlayerController, PosB);
        return PosA.X < PosB.X;
    });
    
    for (int32 i = 0; i< Items.Num(); i++)
    {
        UUMGControlOverlapItem* ItemA = Items[i];
        if (!ItemA)
        {
            continue;
        }
        FVector2D ItemASize = ItemA->GetDesiredSize();
        FVector2D ItemAScreenPos;;
        if (!ItemA->GetPositionInViewport(PlayerController, ItemAScreenPos))
        {
            /*ItemA->SetIsGrouping(false);
            ItemA->SetWorldLocation(ItemA->GetStartedPosition());
            GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("False"), true, FVector2D(1.0f, 1.0f));*/
            continue;
        }
        float XOffset = ItemAScreenPos.X + ItemASize.X;
        float YPos = ItemAScreenPos.Y;
        FSlateRect ItemARect(ItemAScreenPos.X, ItemAScreenPos.Y, ItemAScreenPos.X + ItemASize.X, ItemAScreenPos.Y + ItemASize.Y);
        
        for (int32 j = i + 1; j < Items.Num(); j++)
        {
            UUMGControlOverlapItem* ItemB = Items[j];
            if (!ItemB)
            {
                continue;
            }
            FVector2D ItemBSize = ItemB->GetDesiredSize();
            FVector2D ItemBScreenPos;
            if (!ItemB->GetPositionInViewport(PlayerController, ItemBScreenPos))
            {
                /*ItemB->SetWorldLocation(ItemB->GetStartedPosition());
                ItemB->SetIsGrouping(false);
                GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("False"), true, FVector2D(1.0f, 1.0f));*/
                continue;
            }
            FSlateRect ItemBRect(ItemBScreenPos.X, ItemBScreenPos.Y, ItemBScreenPos.X + ItemBSize.X, ItemBScreenPos.Y + ItemBSize.Y);

            FVector2D ItemBStartScreenPos;
            if (!UGameplayStatics::ProjectWorldToScreen(PlayerController, ItemB->GetStartedPosition(), ItemBStartScreenPos))
            {
               /* ItemB->SetWorldLocation(ItemB->GetStartedPosition());
                GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("False"), true, FVector2D(1.0f, 1.0f));*/
                continue;
            }
            
            FSlateRect ItemBStartPosRect(ItemBStartScreenPos.X, ItemBStartScreenPos.Y, ItemBStartScreenPos.X + ItemBSize.X, ItemBStartScreenPos.Y + ItemBSize.Y);

            if (FSlateRect::DoRectanglesIntersect(ItemARect, ItemBRect))
            {
               
                if (!FSlateRect::DoRectanglesIntersect(ItemARect, ItemBStartPosRect) && ItemB->IsGrouping())
                {
                    ItemB->SetWorldLocation(ItemB->GetStartedPosition());
                    ItemB->SetIsGrouping(false);
                }
                
                FVector2D NewViewPortPosition = FVector2D(ItemAScreenPos.X + ItemASize.X, YPos);
                if (!ItemB->SetPositionInViewport(PlayerController, NewViewPortPosition))
                {
                    /*ItemB->SetWorldLocation(ItemB->GetStartedPosition());
                    ItemB->SetIsGrouping(false);
                    GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("False"), true, FVector2D(1.0f, 1.0f));*/
                    continue;
                }
                ItemB->SetIsGrouping(true);
                 XOffset += ItemBSize.X;
                GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("True"), true, FVector2D(1.0f, 1.0f));
            }
            else
            {
                if (ItemB->IsGrouping())
                {
                    if (!FSlateRect::DoRectanglesIntersect(ItemARect, ItemBStartPosRect))
                    {
                        ItemB->SetWorldLocation(ItemB->GetStartedPosition());
                        ItemB->SetIsGrouping(false);
                    }
                }
                GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("False"), true, FVector2D(1.0f, 1.0f));
            }
        }
    }
}

bool UUMGControlOverlapGroup::AddWidgetComponents(TArray<UWidgetComponent*>& WidgetComponents)
{
    if (Items.Num() + WidgetComponents.Num() > MaxItemsCount)
    {
        return false;
    }
    for (auto WidgetComponent : WidgetComponents)
    {
        AddWidgetComponent(WidgetComponent);
    }
    return true;
}

bool UUMGControlOverlapGroup::AddWidgetComponent(UWidgetComponent* WidgetComponent)
{
    if (!WidgetComponent || !WidgetComponent->GetWidget() || Items.Num() + 1 > MaxItemsCount)
    {
        return false;
    }
    auto NewOverlapItem = NewObject<UUMGControlOverlapItem>(this, UUMGControlOverlapItem::StaticClass());
    Items.Add(NewOverlapItem);
    NewOverlapItem->SetControlledWidgetComponent(WidgetComponent);   
    return true;
}

void UUMGControlOverlapGroup::Init(EControlOverlapType NewControlOverlapType, const FUMGOverlapControlGroupSettings& InSettings)
{
    PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    ControlOverlapType = NewControlOverlapType;
    MaxItemsCount = InSettings.MaxItemsCount;
}

bool UUMGControlOverlapGroup::NeedUpdate() const
{
    if (PlayerController && Items.Num()>0)
    {
        return true;
    }
    return false;
}

bool UUMGControlOverlapGroup::ItemsItersect(UUMGControlOverlapItem* ItemA, UUMGControlOverlapItem* ItemB) const
{
    if (!PlayerController)
    {
        return false;
    }

    FVector2D ItemAPos;
    if (!ItemA->GetPositionInViewport(PlayerController,ItemAPos))
    {
        return false;
    }
    FVector2D ItemASize = ItemA->GetDesiredSize();
    FSlateRect RectA(ItemAPos.X, ItemAPos.Y, ItemAPos.X + ItemASize.X, ItemAPos.Y + ItemASize.Y);

    FVector2D ItemBPos;
    if (!ItemB->GetPositionInViewport(PlayerController, ItemBPos))
    {
        return false;
    }
    FVector2D ItemBSize = ItemB->GetDesiredSize();
    FSlateRect RectB(ItemBPos.X, ItemBPos.Y, ItemBPos.X + ItemBSize.X, ItemBPos.Y + ItemBSize.Y);
    
    return FSlateRect::DoRectanglesIntersect(RectA, RectB);
}

bool UUMGControlOverlapGroupContainer::CreateGroup(TArray<UWidgetComponent*>& WidgetCompoennts, const FString& TagId, const FUMGOverlapControlGroupSettings& Settings)
{
    if (WidgetCompoennts.Num() <= 0 || WidgetCompoennts.Num() > Settings.MaxItemsCount || TagId.IsEmpty())
    {
        return false;
    }
    if (GroupByTag.Contains(TagId))
    {
        if (auto Group = *GroupByTag.Find(TagId))
        {
            return Group->AddWidgetComponents(WidgetCompoennts);
        }
    }
    auto NewGroup = NewObject<UUMGControlOverlapGroup>(this, UUMGControlOverlapGroup::StaticClass());
    NewGroup->Init(ControlOverlapType, Settings);
    GroupByTag.Add(TagId, NewGroup);
    return NewGroup->AddWidgetComponents(WidgetCompoennts);;
}

bool UUMGControlOverlapGroupContainer::RemoveGroup(const FString& TagId)
{
    return false;
}

void UUMGControlOverlapGroupContainer::Update()
{
    for (const auto& it : GroupByTag)
    {
        it.Value->Update();
    }
}
