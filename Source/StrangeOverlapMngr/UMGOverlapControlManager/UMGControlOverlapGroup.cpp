// Fill out your copyright notice in the Description page of Project Settings.

#include "UMGControlOverlapGroup.h"
#include "UMGControlOverlapItem.h"
#include "UMGOverlapControlManager.h"
#include "Components/WidgetComponent.h"
#include <Kismet/GameplayStatics.h>
#include <Blueprint/WidgetLayoutLibrary.h>

bool GetWidgetGeometry(const UWidgetComponent* WidgetComp, const APlayerController* PlayerController, FGeometry& OutGeometry)
{
    FVector2D Size = WidgetComp->GetDrawSize(); // You need to implement GetDesiredSize in your WidgetComponent
    FVector2D Position;
    if (PlayerController->ProjectWorldLocationToScreen(WidgetComp->GetComponentLocation(), Position))
    {
        OutGeometry = FGeometry::MakeRoot(Size, FSlateLayoutTransform(Position - (Size * 0.5f)));
        return true;
    }
    return false;
}

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

bool UUMGControlOverlapGroup::ItemsPositionsItersect(const FVector2D& ItemAPos, const UUMGControlOverlapItem* ItemA, const UUMGControlOverlapItem* ItemB) const
{
    if (!m_PlayerController)
    {
        return false;
    }

    FVector2D ItemBPos;
    if (!ItemB->GetPositionInViewport(m_PlayerController, ItemBPos))
    {
        return false;
    }
    FVector2D ItemASize = ItemA->GetDesiredSize();
    FSlateRect RectA(ItemAPos.X, ItemAPos.Y, ItemAPos.X + ItemASize.X, ItemAPos.Y + ItemASize.Y);

    FVector2D ItemBSize = ItemB->GetDesiredSize();
    FSlateRect RectB(ItemBPos.X, ItemBPos.Y, ItemBPos.X + ItemBSize.X, ItemBPos.Y + ItemBSize.Y);

    return FSlateRect::DoRectanglesIntersect(RectA, RectB);
}

void AlignWidgetsInLine(const TArray<UWidgetComponent*>& Widgets, APlayerController* PlayerController)
{
    FVector2D AlignmentStart; // This would be determined based on your UI design
    // Vertical alignment along the Y-axis
    for (UWidgetComponent* WidgetComp : Widgets)
    {
        FVector NewLocation = WidgetComp->GetComponentLocation();
        FVector2D NewScreenLocation;
        PlayerController->ProjectWorldLocationToScreen(NewLocation, NewScreenLocation);
        NewScreenLocation.Y = AlignmentStart.Y; // Adjust to the desired Y position

        // Convert back to world space (this is a simplistic approach, see notes below)
        FVector NewWorldLocation;
        UGameplayStatics::DeprojectScreenToWorld(PlayerController, NewScreenLocation, NewWorldLocation, NewLocation);// : : DeprojectScreenToWorld(PlayerController, NewScreenLocation, NewWorldLocation, NewLocation);
        WidgetComp->SetWorldLocation(NewWorldLocation);//->SetWorldLocation(NewWorldLocation);
    }
}

bool ProjectWorldToScreenNormalized(APlayerController* PlayerController, const FVector& WorldLocation, FVector2D& OutScreenPosition)
{
    // Project world location to screen
    if (PlayerController && PlayerController->ProjectWorldLocationToScreen(WorldLocation, OutScreenPosition))
    {
        // Get viewport size
        int32 ViewportSizeX, ViewportSizeY;
        PlayerController->GetViewportSize(ViewportSizeX, ViewportSizeY);

        // Check if the screen position is within the viewport bounds
        return OutScreenPosition.X >= 0.0f && OutScreenPosition.X <= ViewportSizeX &&
            OutScreenPosition.Y >= 0.0f && OutScreenPosition.Y <= ViewportSizeY;
    }

    return false;
}

bool UUMGControlOverlapGroup::IsItemOverlapedWithOther(const UUMGControlOverlapItem* CheckingItem, const TArray<UUMGControlOverlapItem*>& Items) const
{
    FVector2D lItemSize = CheckingItem->GetDesiredSize();
    FVector2D lItemScreenPos{0.0f,0.0f};
    //if (CheckingItem->GetPositionInViewport(m_PlayerController, lItemScreenPos))
    //{
    //    if (lItemScreenPos.X >= 0.0f && lItemScreenPos.X <= m_ViewPortSize.X && lItemScreenPos.Y >= 0 && lItemScreenPos.Y <= m_ViewPortSize.Y)
    //    {
    //        for (auto Item : Items)
    //        {
    //            if (Item != CheckingItem)
    //            {
    //                if (WidgetsOverlap(CheckingItem->GetWidgetComponent(), Item->GetWidgetComponent(), m_PlayerController))
    //                {
    //                    return true;
    //                }
    //            }
    //        }
    //       

    //    }
    //}

if (ProjectWorldToScreenNormalized(m_PlayerController, CheckingItem->GetStartedPosition(), lItemScreenPos))
{
    for (auto Item : Items)
    {
        if (Item != CheckingItem)
        {
            return ItemsPositionsItersect(lItemScreenPos, CheckingItem, Item);
        }
    }
}

return false;
}

UUMGControlOverlapItem* UUMGControlOverlapGroup::GetItemByWidgetComponent(UWidgetComponent* WidgetComponent) const
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

bool UUMGControlOverlapGroup::GetAndPrepareItemsForAllign(TArray<UUMGControlOverlapItem*>& ItemsToAllign, TArray<FVector2D>& Positions, TArray<UUMGControlOverlapItem*>& ItemsForCorrectPos)
{
    ItemsToAllign.Empty();
    Positions.Empty();
    for (auto it : m_Items)
    {
        FVector2D Pos(0.0f, 0.0f);
        if (it->GetPositionInViewport(m_PlayerController, Pos))
        {
            if (Pos.X > 0.0f && Pos.X <= m_ViewPortSize.X && Pos.Y > 0 && Pos.Y < m_ViewPortSize.Y)
            {
                Positions.Add(Pos);
                ItemsToAllign.Add(it);
                it->PosForSort = Pos;
                continue;
            }
        }
        if (!FVector::PointsAreNear(it->GetStartedPosition(), it->GetWorldLocation(), 5))
        {
            if (it->IsGrouping())
            {
                Positions.Add(FVector2D(m_ViewPortSize));
                ItemsToAllign.Add(it);
                it->PosForSort = m_ViewPortSize;
                continue;
            }
        }

    }

    ItemsToAllign.Sort([&](const UUMGControlOverlapItem& ItemA, const UUMGControlOverlapItem& ItemB)
        {
            return ItemA.PosForSort.X < ItemB.PosForSort.X;

        });

    Positions.Sort([&](const FVector2D& ItemA, const FVector2D& ItemB)
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
    TArray<UUMGControlOverlapItem*> lItemsForCorectPos;

    //For TESTS
    //for (int32 i = 0; i < m_Items.Num(); i++)
    //{
    //    if (i == 0)
    //    {
    //        continue;
    //    }
    //    FVector2D Pos(0.0f, 0.0f);
    //    if (m_Items[0]->GetPositionInViewport(m_PlayerController, Pos))
    //    {
    //        m_Items[i]->SetPositionInViewport(m_PlayerController, FVector2D(Pos.X + m_Items[0]->GetDesiredSize().X, Pos.Y));
    //    }
    //}
    //GetAndPrepareItemsForAllign(lItemsForAllign, lViewportPositions, lItemsForCorectPos);
    
    
    GetAndPrepareItemsForAllign(lItemsForAllign, lViewportPositions, lItemsForCorectPos);

    if (lItemsForAllign.IsValidIndex(0))
    {
        if (!FVector::PointsAreNear(lItemsForAllign[0]->GetStartedPosition(), lItemsForAllign[0]->GetWorldLocation(), 5))
        {
            lItemsForAllign[0]->SetStartedLoaction();
            GetAndPrepareItemsForAllign(lItemsForAllign, lViewportPositions, lItemsForCorectPos);
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
        float YPos = lItemAScreenPos.Y;
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
                FVector2D lNewViewPortPosition = FVector2D(lXOffset, YPos);
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
                if (!IsItemOverlapedWithOther(lItemB, m_Items))
                {
                    lItemB->SetStartedLoaction();
                }
                else
                {
                    FVector2D lNewViewPortPosition = FVector2D(lItemAScreenPos.X + lItemASize.X, YPos);
                    if (lItemB->SetPositionInViewport(m_PlayerController, lNewViewPortPosition))
                    {
                        lItemA->SetIsGrouping(true);
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
