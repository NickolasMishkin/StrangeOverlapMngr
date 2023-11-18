// Fill out your copyright notice in the Description page of Project Settings.

#include "UMGControlOverlapGroup.h"
#include "UMGControlOverlapItem.h"
#include "UMGOverlapControlManager.h"
#include "Components/WidgetComponent.h"
#include <Kismet/GameplayStatics.h>
#include <Blueprint/WidgetLayoutLibrary.h>

bool UUMGControlOverlapGroup::ItemsItersect(const UUMGControlOverlapItem* ItemA, const UUMGControlOverlapItem* ItemB) const
{
    if (!m_PlayerController)
    {
        return false;
    }

    FVector2D ItemAPos, ItemBPos;
    if (!ItemA->GetPositionInViewport(m_PlayerController, ItemAPos) || !ItemB->GetPositionInViewport(m_PlayerController, ItemBPos))
    {
        return false;
    }
    FVector2D ItemASize = ItemA->GetDesiredSize();
    FSlateRect RectA(ItemAPos.X, ItemAPos.Y, ItemAPos.X + ItemASize.X, ItemAPos.Y + ItemASize.Y);

    FVector2D ItemBSize = ItemB->GetDesiredSize();
    FSlateRect RectB(ItemBPos.X, ItemBPos.Y, ItemBPos.X + ItemBSize.X, ItemBPos.Y + ItemBSize.Y);

    return FSlateRect::DoRectanglesIntersect(RectA, RectB);
}

bool UUMGControlOverlapGroup::IsItemStartedLocationIntersectOtherItem(const FVector2D& ItemAStartedViewPortPosition, const UUMGControlOverlapItem* ItemA, const UUMGControlOverlapItem* ItemB) const
{
    if (!m_PlayerController || !ItemA || !ItemB)
    {
        return false;
    }

    FVector2D lItemBPos = FVector2D(0.0f, 0.0f);
    if (!ItemB->GetPositionInViewport(m_PlayerController, lItemBPos))
    {
        return false;
    }
    FVector2D lItemASize = ItemA->GetDesiredSize();
    FSlateRect lRectA(ItemAStartedViewPortPosition.X, ItemAStartedViewPortPosition.Y, ItemAStartedViewPortPosition.X + lItemASize.X, ItemAStartedViewPortPosition.Y + lItemASize.Y);

    FVector2D lItemBSize = ItemB->GetDesiredSize();
    FSlateRect lRectB(lItemBPos.X, lItemBPos.Y, lItemBPos.X + lItemBSize.X, lItemBPos.Y + lItemBSize.Y);

    return FSlateRect::DoRectanglesIntersect(lRectA, lRectB);
}

bool UUMGControlOverlapGroup::GetItemScreenNormalizedPositionByWorlLocation(const UUMGControlOverlapItem* Item, FVector2D& OutScreenPosition) const
{
    if (m_PlayerController && Item->GetPositionInViewport(m_PlayerController, OutScreenPosition))
    {
        return OutScreenPosition.X >= 0.0f && OutScreenPosition.X <= m_ViewPortSize.X && OutScreenPosition.Y >= 0.0f && OutScreenPosition.Y <= m_ViewPortSize.Y;
    }
    return false;
}

bool UUMGControlOverlapGroup::IsItemItersectOtherItems(const UUMGControlOverlapItem* CheckingItem, const TArray<UUMGControlOverlapItem*>& Items) const
{
    FVector2D lItemScreenPos = FVector2D{0.0f, 0.0f};
    if (GetItemScreenNormalizedPositionByWorlLocation(CheckingItem, lItemScreenPos))
    {
        for (auto Item : Items)
        {
            if (Item != CheckingItem)
            {
                if (IsItemStartedLocationIntersectOtherItem(lItemScreenPos, CheckingItem, Item))
                {
                    return true;
                }
            }
        }
    }
    return false;
}

UUMGControlOverlapItem* UUMGControlOverlapGroup::GetItemByWidgetComponent(const UWidgetComponent* WidgetComponent) const
{
    if (WidgetComponent)
    {
        for (const auto Item : m_Items)
        {
            if (Item->GetWidgetComponent() == WidgetComponent)
            {
                return Item;
            }
        }
    }
    return nullptr;
}

bool UUMGControlOverlapGroup::GetAndSortItemsForAllign(TArray<UUMGControlOverlapItem*>& OutItemsForAllign, TArray<FVector2D>& OutItemsPositionsInViewport)
{
    OutItemsForAllign.Empty();
    OutItemsPositionsInViewport.Empty();
    for (auto it : m_Items)
    {
        FVector2D Pos = FVector2D(0.0f, 0.0f);
        if (it->GetPositionInViewport(m_PlayerController, Pos))
        {
            if (Pos.X > 0.0f && Pos.X <= m_ViewPortSize.X && Pos.Y > 0 && Pos.Y < m_ViewPortSize.Y)
            {
                OutItemsPositionsInViewport.Add(Pos);
                OutItemsForAllign.Add(it);
                it->PosForSort = Pos;
                continue;
            }
        }
        if (!FVector::PointsAreNear(it->GetStartedPosition(), it->GetWorldLocation(), 5))
        {
            if (it->IsGrouping())
            {
                OutItemsPositionsInViewport.Add(m_ViewPortSize);
                OutItemsForAllign.Add(it);
                it->PosForSort = m_ViewPortSize;
                continue;
            }
        }

    }

    OutItemsForAllign.Sort([&](const UUMGControlOverlapItem& ItemA, const UUMGControlOverlapItem& ItemB)
    {
        return ItemA.PosForSort.X < ItemB.PosForSort.X;

    });

    OutItemsPositionsInViewport.Sort([&](const FVector2D& ItemA, const FVector2D& ItemB)
    {
        return ItemA.X < ItemB.X;

    });

    return true;
}

void UUMGControlOverlapGroup::Update()
{
    if (!NeedUpdate())
    {
        return;
    }

    TArray<FVector2D> lViewportPositions;
    TArray<UUMGControlOverlapItem*> lItemsForAllign;
    
    GetAndSortItemsForAllign(lItemsForAllign, lViewportPositions);

    if (lItemsForAllign.IsValidIndex(0))
    {
        if (!FVector::PointsAreNear(lItemsForAllign[0]->GetStartedPosition(), lItemsForAllign[0]->GetWorldLocation(), 5))
        {
            lItemsForAllign[0]->SetStartedLoaction();
            GetAndSortItemsForAllign(lItemsForAllign, lViewportPositions);
        }
    }

    if (lItemsForAllign.Num() <= 1)
    {
        for (auto It : m_Items)
        {
            It->SetIsGrouping(false);
        }
        return;
    }

    for (int32 i = 0; i < lViewportPositions.Num(); i++)
    {
        UUMGControlOverlapItem* lItemA = lItemsForAllign[i];
        if (!lItemA)
        {
            continue;
        }
        
        if (i == 0)
        {
            lItemA->SetIsGrouping(true);
        }

        FVector2D lItemASize = lItemA->GetDesiredSize();
        FVector2D lItemAScreenPos = lViewportPositions[i];
        lItemA->UpdateIndex(i);
        float lXOffset = lItemAScreenPos.X + lItemASize.X;
        float lYOffset = lItemAScreenPos.Y;
        FSlateRect lItemARect(lItemAScreenPos.X, lItemAScreenPos.Y, lItemAScreenPos.X + lItemASize.X, lItemAScreenPos.Y + lItemASize.Y);

        for (int32 j = i + 1; j < lViewportPositions.Num(); j++)
        {
            UUMGControlOverlapItem* lItemB = lItemsForAllign[j];
            if (!lItemB)
            {
                continue;
            }
            FVector2D lItemBSize = lItemB->GetDesiredSize();
            FVector2D lItemBScreenPos = lViewportPositions[j];
            FSlateRect lItemBRect(lItemBScreenPos.X, lItemBScreenPos.Y, lItemBScreenPos.X + lItemBSize.X, lItemBScreenPos.Y + lItemBSize.Y);

            if (FSlateRect::DoRectanglesIntersect(lItemARect, lItemBRect))
            {
                FVector2D lNewViewPortPosition = FVector2D(lXOffset, lYOffset);
                if (lItemB->SetPositionInViewport(m_PlayerController, lNewViewPortPosition))
                {
                    lItemB->SetIsGrouping(true);
                    lViewportPositions[j] = lNewViewPortPosition;
                    lXOffset += lItemBSize.X;
                    continue;
                }
                
                
            }
            else if (lItemB->IsGrouping())
            {
                if (!IsItemItersectOtherItems(lItemB, m_Items))
                {
                    lItemB->SetStartedLoaction();
                }
                else
                {
                    FVector2D lNewViewPortPosition = FVector2D(lItemAScreenPos.X + lItemASize.X, lYOffset);
                    if (lItemB->SetPositionInViewport(m_PlayerController, lNewViewPortPosition))
                    {
                        lItemB->SetIsGrouping(true);
                        lViewportPositions[j] = lNewViewPortPosition;
                        continue;
                    }
                }               
            }
        }
    }
}

bool UUMGControlOverlapGroup::AddWidgetComponents(TArray<UWidgetComponent*>& WidgetComponents)
{
    if (m_Items.Num() + WidgetComponents.Num() > m_MaxItemsCount)
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
    if (!WidgetComponent || !WidgetComponent->GetWidget() || m_Items.Num() + 1 > m_MaxItemsCount || GetItemByWidgetComponent(WidgetComponent))
    {
        return false;
    }
    UUMGControlOverlapItem* lNewOverlapItem = NewObject<UUMGControlOverlapItem>(this, UUMGControlOverlapItem::StaticClass());
    m_Items.Add(lNewOverlapItem);
    lNewOverlapItem->SetControlledWidgetComponent(WidgetComponent);
    return true;
}

bool UUMGControlOverlapGroup::RemoveWidgetComponent(UWidgetComponent* WidgetComponent)
{
    //may be here add destroy of group if items count less 1
    if(WidgetComponent)
    {
        if (auto Item = GetItemByWidgetComponent(WidgetComponent))
        {
            m_Items.Remove(Item);
            Item->Destroy();
            return true;
        }
    }
    return false;
}

bool UUMGControlOverlapGroup::Destroy()
{
    for (auto Item : m_Items)
    {
        Item->Destroy();
    }
    return ConditionalBeginDestroy();
}

void UUMGControlOverlapGroup::Init(EControlOverlapType NewControlOverlapType, const FUMGOverlapControlGroupSettings& InSettings)
{
    m_PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    m_ControlOverlapType = NewControlOverlapType;
    m_MaxItemsCount = InSettings.MaxItemsCount;
    if (m_PlayerController)
    {
        int32 lViewPortX, lViewPortY = 0;
        m_PlayerController->GetViewportSize(lViewPortX, lViewPortY);
        m_ViewPortSize = FVector2D(static_cast<float>(lViewPortX), static_cast<float>(lViewPortY));
    }
}

bool UUMGControlOverlapGroup::NeedUpdate() const
{
    if (m_PlayerController && m_Items.Num()>0)
    {
        return true;
    }
    return false;
}

bool UUMGControlOverlapGroupContainer::CreateGroup(TArray<UWidgetComponent*>& WidgetComponent, const FString& TagId, const FUMGOverlapControlGroupSettings& Settings)
{
    if (WidgetComponent.Num() <= 0 || WidgetComponent.Num() > Settings.MaxItemsCount || TagId.IsEmpty())
    {
        return false;
    }
    if (m_GroupByTag.Contains(TagId))
    {
        if (auto lGroup = *m_GroupByTag.Find(TagId))
        {
            return lGroup->AddWidgetComponents(WidgetComponent);
        }
    }
    UUMGControlOverlapGroup* lNewGroup = NewObject<UUMGControlOverlapGroup>(this, UUMGControlOverlapGroup::StaticClass());
    lNewGroup->Init(m_ControlOverlapType, Settings);
    m_GroupByTag.Add(TagId, lNewGroup);
    return lNewGroup->AddWidgetComponents(WidgetComponent);;
}

bool UUMGControlOverlapGroupContainer::AddWidgetComponentToGroup(UWidgetComponent* WidgetComponent, const FString& TagId)
{
    if (WidgetComponent && !TagId.IsEmpty() && m_GroupByTag.Contains(TagId))
    {
        if (auto lGroup = *m_GroupByTag.Find(TagId))
        {
            return lGroup->AddWidgetComponent(WidgetComponent);
        }
    }
    return false;
}

bool UUMGControlOverlapGroupContainer::RemoveWidgetComponentFromGroup(UWidgetComponent* WidgetComponent, const FString& TagId)
{
    if (WidgetComponent && !TagId.IsEmpty() && m_GroupByTag.Contains(TagId))
    {
        if (auto lGroup = *m_GroupByTag.Find(TagId))
        {
            return lGroup->RemoveWidgetComponent(WidgetComponent);
        }
    }
    return false;
}

bool UUMGControlOverlapGroupContainer::RemoveGroup(const FString& TagId)
{
    if (!TagId.IsEmpty() && m_GroupByTag.Contains(TagId))
    {
        if (auto lGroup = *m_GroupByTag.Find(TagId))
        {
            if (lGroup->Destroy())
            {
                m_GroupByTag.Remove(TagId);
                return true;
            }
        }
    }
    return false;
}

void UUMGControlOverlapGroupContainer::Update()
{
    for (const auto& it : m_GroupByTag)
    {
        it.Value->Update();
    }
}
