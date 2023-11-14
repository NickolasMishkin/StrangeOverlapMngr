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

bool WidgetsOverlap(const UWidgetComponent* WidgetA, const UWidgetComponent* WidgetB, const APlayerController* PlayerController)
{
    FGeometry GeometryA;
    FGeometry GeometryB;
    /*if ()
    {*/
    if (GetWidgetGeometry(WidgetA, PlayerController, GeometryA) && GetWidgetGeometry(WidgetB, PlayerController, GeometryB))
    {
        // Check if the two geometries overlap
        FBox2D BoxA(GeometryA.AbsoluteToLocal(GeometryA.GetLocalSize()), GeometryA.GetLocalSize());
        FBox2D BoxB(GeometryB.AbsoluteToLocal(GeometryB.GetLocalSize()), GeometryB.GetLocalSize());
        return BoxA.Intersect(BoxB);
    }
    return false;
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

void CheckAndAlignWidgets(TArray<UWidgetComponent*> WidgetComponents, APlayerController* PlayerController)
{
    if (!PlayerController)
    {
        return;
    }

    // Store the original positions
    TMap<UWidgetComponent*, FVector> OriginalPositions;
    for (UWidgetComponent* WidgetComp : WidgetComponents)
    {
        OriginalPositions.Add(WidgetComp, WidgetComp->GetComponentLocation());
    }

    // Check for any overlaps
    bool bShouldAlign = false;
    for (int32 i = 0; i < WidgetComponents.Num() - 1; ++i)
    {
        for (int32 j = i + 1; j < WidgetComponents.Num(); ++j)
        {
            if (WidgetsOverlap(WidgetComponents[i], WidgetComponents[j], PlayerController))
            {
                bShouldAlign = true;
                break;
            }
        }
        if (bShouldAlign)
        {
            break;
        }
    }

    // Align widgets if there are overlaps, otherwise reset to original positions
    if (bShouldAlign)
    {
        AlignWidgetsInLine(WidgetComponents, PlayerController);
    }
    else
    {
        for (UWidgetComponent* WidgetComp : WidgetComponents)
        {
            //WidgetComp->SetWorldLocation(OriginalPositions[WidgetComp]);
        }
    }
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

bool UUMGControlOverlapGroup::GetAndPrepareItemsForAllign(TArray<UUMGControlOverlapItem*>& ItemsToAllign, TArray<FVector2D>& Positions)
{
    ItemsToAllign.Empty();
    Positions.Empty();
    for (auto it : m_Items)
    {
        FVector2D Pos(0.0f, 0.0f);
        const float Tolerance = 0.0001f;
        if (it->GetPositionInViewport(m_PlayerController, Pos))
        {
            if (Pos.X > 0 && Pos.X < m_ViewPortSize.X && Pos.Y>0 && Pos.Y < m_ViewPortSize.Y)
            {
                Positions.Add(Pos);
                ItemsToAllign.Add(it);
                it->PosForSort = Pos;
                continue;
            }
        }
        if (!FVector::PointsAreNear(it->GetStartedPosition(), it->GetWorldLocation(), 5))
        {
            it->SetStartedLoaction();
            if (it->GetPositionInViewport(m_PlayerController, Pos))
            {
                Positions.Add(Pos);
                ItemsToAllign.Add(it);
                it->PosForSort = Pos;
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

    if (ItemsToAllign.IsValidIndex(0))
    {
        if (!FVector::PointsAreNear(ItemsToAllign[0]->GetStartedPosition(), ItemsToAllign[0]->GetWorldLocation(), 5))
        {
            ItemsToAllign[0]->SetStartedLoaction();
                //GetAndPrepareItemsForAllign(ItemsToAllign, Positions);
        }
    }
    

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
    GetAndPrepareItemsForAllign(lItemsForAllign, lViewportPositions);
    
    for (int32 i = 0; i < lViewportPositions.Num(); i++)
    {
        UUMGControlOverlapItem* lItemA = lItemsForAllign[i];
        if (!lItemA)
        {
            continue;
        }
        
        FVector2D lItemASize = lItemA->GetDesiredSize();
        FVector2D lItemAScreenPos = lViewportPositions[i];
        lItemA->Update(i);
        float XOffset = lItemAScreenPos.X + lItemASize.X;
        float YPos = lItemAScreenPos.Y;
        FSlateRect lItemARect(lItemAScreenPos.X, lItemAScreenPos.Y, lItemAScreenPos.X + 10.0f + lItemASize.X, lItemAScreenPos.Y + lItemASize.Y);

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
                if (!FVector::PointsAreNear(lItemB->GetStartedPosition(), lItemB->GetWorldLocation(), 5))
                {

                    FVector2D lItemBStartScreenPos;
                    if (UGameplayStatics::ProjectWorldToScreen(m_PlayerController, lItemB->GetStartedPosition(), lItemBStartScreenPos))
                    {
                        if (lItemBStartScreenPos.X > 0 && lItemBStartScreenPos.X < m_ViewPortSize.X && lItemBStartScreenPos.Y>0 && lItemBStartScreenPos.Y < m_ViewPortSize.Y)
                        {
                            FSlateRect lItemBStartPosRect(lItemBStartScreenPos.X, lItemBStartScreenPos.Y, lItemBStartScreenPos.X + lItemBSize.X, lItemBStartScreenPos.Y + lItemBSize.Y);
                            if (!FSlateRect::DoRectanglesIntersect(lItemARect, lItemBStartPosRect))
                            {
                                lItemB->SetStartedLoaction();
                                lViewportPositions[j] = lItemBStartScreenPos;
                                continue;
                            }

                        }
                        
                    }
                }


                FVector2D lNewViewPortPosition = FVector2D(lItemAScreenPos.X + lItemASize.X /*- ItemBSize.X / 2.0f*/, YPos);
                if (lItemB->SetPositionInViewport(m_PlayerController, lNewViewPortPosition))
                {
                    lViewportPositions[j] = lNewViewPortPosition;
                    continue;
                }
                else
                {
                    GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("True"), true, FVector2D(1.0f, 1.0f));
                }
                XOffset += lItemBSize.X;
                
            }
            else
            {
                if (!FVector::PointsAreNear(lItemB->GetStartedPosition(),lItemB->GetWorldLocation(), 5))
                {
                    FVector2D lItemBStartScreenPos(0.0f,0.0f);
                    if (UGameplayStatics::ProjectWorldToScreen(m_PlayerController, lItemB->GetStartedPosition(), lItemBStartScreenPos))
                    {
                        const float lTolerance = 0.0001f; // Set a small tolerance value
                        bool lbIsDenormalized = FMath::Abs(1.0f - lItemBStartScreenPos.Size()) > lTolerance;
                        if (lItemBStartScreenPos.ContainsNaN() || lItemBStartScreenPos.IsNearlyZero(lItemBSize.X)/*> 1800 || ItemBStartScreenPos.Y < 0 || ItemBStartScreenPos.X>2500 || ItemBStartScreenPos.X < 0*/)
                        {
                            lItemB->SetStartedLoaction();
                            lViewportPositions[j] = lItemBStartScreenPos;
                            GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("NONE"), true, FVector2D(1.0f, 1.0f));
                            continue;
                        }

                        if (lbIsDenormalized)
                        {
                            lItemB->SetStartedLoaction();
                            UGameplayStatics::ProjectWorldToScreen(m_PlayerController, lItemB->GetStartedPosition(), lItemBStartScreenPos);
                            lViewportPositions[j] = lItemBStartScreenPos;
                            continue;
                        }

                        FSlateRect lItemBStartPosRect(lItemBStartScreenPos.X, lItemBStartScreenPos.Y, lItemBStartScreenPos.X + lItemBSize.X, lItemBStartScreenPos.Y + lItemBSize.Y);
                        if (!FSlateRect::DoRectanglesIntersect(lItemARect, lItemBStartPosRect))
                        {
                            lItemB->SetStartedLoaction();
                            lViewportPositions[j] = lItemBStartScreenPos;
                            continue;
                        }

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
    if (!WidgetComponent || !WidgetComponent->GetWidget() || m_Items.Num() + 1 > m_MaxItemsCount)
    {
        return false;
    }
    auto lNewOverlapItem = NewObject<UUMGControlOverlapItem>(this, UUMGControlOverlapItem::StaticClass());
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

//bool UUMGControlOverlapGroup::ItemsItersect(UUMGControlOverlapItem* ItemA, UUMGControlOverlapItem* ItemB) const
//{
//    if (!m_PlayerController)
//    {
//        return false;
//    }
//
//    FVector2D ItemAPos;
//    if (!ItemA->GetPositionInViewport(m_PlayerController,ItemAPos))
//    {
//        return false;
//    }
//    FVector2D ItemASize = ItemA->GetDesiredSize();
//    FSlateRect RectA(ItemAPos.X, ItemAPos.Y, ItemAPos.X + ItemASize.X, ItemAPos.Y + ItemASize.Y);
//
//    FVector2D ItemBPos;
//    if (!ItemB->GetPositionInViewport(m_PlayerController, ItemBPos))
//    {
//        return false;
//    }
//    FVector2D ItemBSize = ItemB->GetDesiredSize();
//    FSlateRect RectB(ItemBPos.X, ItemBPos.Y, ItemBPos.X + ItemBSize.X, ItemBPos.Y + ItemBSize.Y);
//    
//    return FSlateRect::DoRectanglesIntersect(RectA, RectB);
//}

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
    auto lNewGroup = NewObject<UUMGControlOverlapGroup>(this, UUMGControlOverlapGroup::StaticClass());
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
