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


    TArray<FVector2D> LViewportPositions;
    TArray<UUMGControlOverlapItem*> LItemsForAllign;
    GetAndPrepareItemsForAllign(LItemsForAllign, LViewportPositions);
    
    for (int32 i = 0; i < LViewportPositions.Num(); i++)
    {
        UUMGControlOverlapItem* LItemA = LItemsForAllign[i];
        if (!LItemA)
        {
            continue;
        }
        
        FVector2D LItemASize = LItemA->GetDesiredSize();
        FVector2D LItemAScreenPos = LViewportPositions[i];
        LItemA->Update(i);
        float XOffset = LItemAScreenPos.X + LItemASize.X;
        float YPos = LItemAScreenPos.Y;
        FSlateRect LItemARect(LItemAScreenPos.X, LItemAScreenPos.Y, LItemAScreenPos.X + 10.0f + LItemASize.X, LItemAScreenPos.Y + LItemASize.Y);

        for (int32 j = i + 1; j < LViewportPositions.Num(); j++)
        {
            UUMGControlOverlapItem* LItemB = LItemsForAllign[j];
            if (!LItemB)
            {
                continue;
            }
            FVector2D LItemBSize = LItemB->GetDesiredSize();
            FVector2D LItemBScreenPos = LViewportPositions[j];
            FSlateRect LItemBRect(LItemBScreenPos.X, LItemBScreenPos.Y, LItemBScreenPos.X + LItemBSize.X, LItemBScreenPos.Y + LItemBSize.Y);

            if (FSlateRect::DoRectanglesIntersect(LItemARect, LItemBRect))
            {
                if (!FVector::PointsAreNear(LItemB->GetStartedPosition(), LItemB->GetWorldLocation(), 5))
                {

                    FVector2D ItemBStartScreenPos;
                    if (UGameplayStatics::ProjectWorldToScreen(m_PlayerController, LItemB->GetStartedPosition(), ItemBStartScreenPos))
                    {
                        if (ItemBStartScreenPos.X > 0 && ItemBStartScreenPos.X < m_ViewPortSize.X && ItemBStartScreenPos.Y>0 && ItemBStartScreenPos.Y < m_ViewPortSize.Y)
                        {
                            FSlateRect ItemBStartPosRect(ItemBStartScreenPos.X, ItemBStartScreenPos.Y, ItemBStartScreenPos.X + LItemBSize.X, ItemBStartScreenPos.Y + LItemBSize.Y);
                            if (!FSlateRect::DoRectanglesIntersect(LItemARect, ItemBStartPosRect))
                            {
                                LItemB->SetStartedLoaction();
                                LViewportPositions[j] = ItemBStartScreenPos;
                                continue;
                            }

                        }
                        
                    }
                }


                FVector2D NewViewPortPosition = FVector2D(LItemAScreenPos.X + LItemASize.X /*- ItemBSize.X / 2.0f*/, YPos);
                if (LItemB->SetPositionInViewport(m_PlayerController, NewViewPortPosition))
                {
                    LViewportPositions[j] = NewViewPortPosition;
                    continue;
                }
                else
                {
                    GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("True"), true, FVector2D(1.0f, 1.0f));
                }
                XOffset += LItemBSize.X;
                
            }
            else
            {
                if (!FVector::PointsAreNear(LItemB->GetStartedPosition(),LItemB->GetWorldLocation(), 5))
                {
                    FVector2D ItemBStartScreenPos(0.0f,0.0f);
                    if (UGameplayStatics::ProjectWorldToScreen(m_PlayerController, LItemB->GetStartedPosition(), ItemBStartScreenPos))
                    {
                        const float Tolerance = 0.0001f; // Set a small tolerance value
                        bool bIsDenormalized = FMath::Abs(1.0f - ItemBStartScreenPos.Size()) > Tolerance;
                        if (ItemBStartScreenPos.ContainsNaN() || ItemBStartScreenPos.IsNearlyZero(LItemBSize.X)/*> 1800 || ItemBStartScreenPos.Y < 0 || ItemBStartScreenPos.X>2500 || ItemBStartScreenPos.X < 0*/)
                        {
                            LItemB->SetStartedLoaction();
                            LViewportPositions[j] = ItemBStartScreenPos;
                            GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("NONE"), true, FVector2D(1.0f, 1.0f));
                            continue;
                        }

                        if (bIsDenormalized)
                        {
                            LItemB->SetStartedLoaction();
                            UGameplayStatics::ProjectWorldToScreen(m_PlayerController, LItemB->GetStartedPosition(), ItemBStartScreenPos);
                            LViewportPositions[j] = ItemBStartScreenPos;
                            continue;
                        }

                        FSlateRect ItemBStartPosRect(ItemBStartScreenPos.X, ItemBStartScreenPos.Y, ItemBStartScreenPos.X + LItemBSize.X, ItemBStartScreenPos.Y + LItemBSize.Y);
                        if (!FSlateRect::DoRectanglesIntersect(LItemARect, ItemBStartPosRect))
                        {
                            LItemB->SetStartedLoaction();
                            LViewportPositions[j] = ItemBStartScreenPos;
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
    auto LNewOverlapItem = NewObject<UUMGControlOverlapItem>(this, UUMGControlOverlapItem::StaticClass());
    m_Items.Add(LNewOverlapItem);
    LNewOverlapItem->SetControlledWidgetComponent(WidgetComponent);
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
        int32 LViewPortX, LViewPortY = 0;
        m_PlayerController->GetViewportSize(LViewPortX, LViewPortY);
        m_ViewPortSize = FVector2D(static_cast<float>(LViewPortX), static_cast<float>(LViewPortY));
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
        if (auto LGroup = *m_GroupByTag.Find(TagId))
        {
            return LGroup->AddWidgetComponents(WidgetComponent);
        }
    }
    auto LNewGroup = NewObject<UUMGControlOverlapGroup>(this, UUMGControlOverlapGroup::StaticClass());
    LNewGroup->Init(m_ControlOverlapType, Settings);
    m_GroupByTag.Add(TagId, LNewGroup);
    return LNewGroup->AddWidgetComponents(WidgetComponent);;
}

bool UUMGControlOverlapGroupContainer::AddWidgetComponentToGroup(UWidgetComponent* WidgetComponent, const FString& TagId)
{
    if (WidgetComponent && !TagId.IsEmpty() && m_GroupByTag.Contains(TagId))
    {
        if (auto LGroup = *m_GroupByTag.Find(TagId))
        {
            return LGroup->AddWidgetComponent(WidgetComponent);
        }
    }
    return false;
}

bool UUMGControlOverlapGroupContainer::RemoveWidgetComponentFromGroup(UWidgetComponent* WidgetComponent, const FString& TagId)
{
    if (WidgetComponent && !TagId.IsEmpty() && m_GroupByTag.Contains(TagId))
    {
        if (auto LGroup = *m_GroupByTag.Find(TagId))
        {
            return LGroup->RemoveWidgetComponent(WidgetComponent);
        }
    }
    return false;
}

bool UUMGControlOverlapGroupContainer::RemoveGroup(const FString& TagId)
{
    if (!TagId.IsEmpty() && m_GroupByTag.Contains(TagId))
    {
        if (auto LGroup = *m_GroupByTag.Find(TagId))
        {
            if (LGroup->Destroy())
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
