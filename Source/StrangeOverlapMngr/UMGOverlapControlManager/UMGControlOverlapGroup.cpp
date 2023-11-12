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
        for (const auto Item : Items)
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
    for (auto it : Items)
    {
        FVector2D Pos(0.0f, 0.0f);
        const float Tolerance = 0.0001f;
        if (it->GetPositionInViewport(PlayerController, Pos))
        {
            if (Pos.X > 0 && Pos.X < ViewPortSize.X && Pos.Y>0 && Pos.Y < ViewPortSize.Y)
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
            if (it->GetPositionInViewport(PlayerController, Pos))
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


    TArray<FVector2D> ViewportPositions;
    TArray<UUMGControlOverlapItem*> ItemsForAllign;
    GetAndPrepareItemsForAllign(ItemsForAllign, ViewportPositions);
    
    for (int32 i = 0; i < ViewportPositions.Num(); i++)
    {
        UUMGControlOverlapItem* ItemA = ItemsForAllign[i];
        if (!ItemA)
        {
            continue;
        }
        
        FVector2D ItemASize = ItemA->GetDesiredSize();
        FVector2D ItemAScreenPos = ViewportPositions[i];
        ItemA->Update(i);
        float XOffset = ItemAScreenPos.X + ItemASize.X;
        float YPos = ItemAScreenPos.Y;
        FSlateRect ItemARect(ItemAScreenPos.X, ItemAScreenPos.Y, ItemAScreenPos.X + 10.0f + ItemASize.X, ItemAScreenPos.Y + ItemASize.Y);

        for (int32 j = i + 1; j < ViewportPositions.Num(); j++)
        {
            UUMGControlOverlapItem* ItemB = ItemsForAllign[j];
            if (!ItemB)
            {
                continue;
            }
            FVector2D ItemBSize = ItemB->GetDesiredSize();
            FVector2D ItemBScreenPos = ViewportPositions[j];
            FSlateRect ItemBRect(ItemBScreenPos.X, ItemBScreenPos.Y, ItemBScreenPos.X + ItemBSize.X, ItemBScreenPos.Y + ItemBSize.Y);

            if (FSlateRect::DoRectanglesIntersect(ItemARect, ItemBRect))
            {
                if (!FVector::PointsAreNear(ItemB->GetStartedPosition(), ItemB->GetWorldLocation(), 5))
                {

                    FVector2D ItemBStartScreenPos;
                    if (UGameplayStatics::ProjectWorldToScreen(PlayerController, ItemB->GetStartedPosition(), ItemBStartScreenPos))
                    {
                        if (ItemBStartScreenPos.X > 0 && ItemBStartScreenPos.X < ViewPortSize.X && ItemBStartScreenPos.Y>0 && ItemBStartScreenPos.Y < ViewPortSize.Y)
                        {
                            FSlateRect ItemBStartPosRect(ItemBStartScreenPos.X, ItemBStartScreenPos.Y, ItemBStartScreenPos.X + ItemBSize.X, ItemBStartScreenPos.Y + ItemBSize.Y);
                            if (!FSlateRect::DoRectanglesIntersect(ItemARect, ItemBStartPosRect))
                            {
                                ItemB->SetStartedLoaction();
                                ViewportPositions[j] = ItemBStartScreenPos;
                                continue;
                            }

                        }
                        
                    }
                }


                FVector2D NewViewPortPosition = FVector2D(ItemAScreenPos.X + ItemASize.X /*- ItemBSize.X / 2.0f*/, YPos);
                if (ItemB->SetPositionInViewport(PlayerController, NewViewPortPosition))
                {
                    ViewportPositions[j] = NewViewPortPosition;
                    continue;
                }
                else
                {
                    GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("True"), true, FVector2D(1.0f, 1.0f));
                }
                XOffset += ItemBSize.X;
                
            }
            else
            {
                if (!FVector::PointsAreNear(ItemB->GetStartedPosition(),ItemB->GetWorldLocation(), 5))
                {
                    FVector2D ItemBStartScreenPos(0.0f,0.0f);
                    if (UGameplayStatics::ProjectWorldToScreen(PlayerController, ItemB->GetStartedPosition(), ItemBStartScreenPos))
                    {
                        const float Tolerance = 0.0001f; // Set a small tolerance value
                        bool bIsDenormalized = FMath::Abs(1.0f - ItemBStartScreenPos.Size()) > Tolerance;
                        if (ItemBStartScreenPos.ContainsNaN() || ItemBStartScreenPos.IsNearlyZero(ItemBSize.X)/*> 1800 || ItemBStartScreenPos.Y < 0 || ItemBStartScreenPos.X>2500 || ItemBStartScreenPos.X < 0*/)
                        {
                            ItemB->SetStartedLoaction();
                            ViewportPositions[j] = ItemBStartScreenPos;
                            GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("NONE"), true, FVector2D(1.0f, 1.0f));
                            continue;
                        }

                        if (bIsDenormalized)
                        {
                            ItemB->SetStartedLoaction();
                            UGameplayStatics::ProjectWorldToScreen(PlayerController, ItemB->GetStartedPosition(), ItemBStartScreenPos);
                            ViewportPositions[j] = ItemBStartScreenPos;
                            continue;
                        }

                        FSlateRect ItemBStartPosRect(ItemBStartScreenPos.X, ItemBStartScreenPos.Y, ItemBStartScreenPos.X + ItemBSize.X, ItemBStartScreenPos.Y + ItemBSize.Y);
                        if (!FSlateRect::DoRectanglesIntersect(ItemARect, ItemBStartPosRect))
                        {
                            ItemB->SetStartedLoaction();
                            ViewportPositions[j] = ItemBStartScreenPos;
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

bool UUMGControlOverlapGroup::RemoveWidgetComponent(UWidgetComponent* WidgetComponent)
{
    //here add destroy of group if items count less 1
    if(WidgetComponent)
    {
        if (auto Item = GetItemByWidgetComponent(WidgetComponent))
        {
            Items.Remove(Item);
            Item->Destroy();
            return true;
        }
    }
    return false;
}

bool UUMGControlOverlapGroup::Destroy()
{
    for (auto Item : Items)
    {
        Item->Destroy();
    }
    return ConditionalBeginDestroy();
}

void UUMGControlOverlapGroup::Init(EControlOverlapType NewControlOverlapType, const FUMGOverlapControlGroupSettings& InSettings)
{
    PlayerController = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    ControlOverlapType = NewControlOverlapType;
    MaxItemsCount = InSettings.MaxItemsCount;
    if (PlayerController)
    {
        int32 ViewPortX, ViewPortY = 0;
        PlayerController->GetViewportSize(ViewPortX, ViewPortY);
        ViewPortSize = FVector2D(static_cast<float>(ViewPortX), static_cast<float>(ViewPortY));
    }
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

bool UUMGControlOverlapGroupContainer::CreateGroup(TArray<UWidgetComponent*>& WidgetComponent, const FString& TagId, const FUMGOverlapControlGroupSettings& Settings)
{
    if (WidgetComponent.Num() <= 0 || WidgetComponent.Num() > Settings.MaxItemsCount || TagId.IsEmpty())
    {
        return false;
    }
    if (GroupByTag.Contains(TagId))
    {
        if (auto Group = *GroupByTag.Find(TagId))
        {
            return Group->AddWidgetComponents(WidgetComponent);
        }
    }
    auto NewGroup = NewObject<UUMGControlOverlapGroup>(this, UUMGControlOverlapGroup::StaticClass());
    NewGroup->Init(ControlOverlapType, Settings);
    GroupByTag.Add(TagId, NewGroup);
    return NewGroup->AddWidgetComponents(WidgetComponent);;
}

bool UUMGControlOverlapGroupContainer::AddWidgetComponentToGroup(UWidgetComponent* WidgetComponent, const FString& TagId)
{
    if (WidgetComponent && !TagId.IsEmpty() && GroupByTag.Contains(TagId))
    {
        if (auto Group = *GroupByTag.Find(TagId))
        {
            return Group->AddWidgetComponent(WidgetComponent);
        }
    }
    return false;
}

bool UUMGControlOverlapGroupContainer::RemoveWidgetComponentFromGroup(UWidgetComponent* WidgetComponent, const FString& TagId)
{
    if (WidgetComponent && !TagId.IsEmpty() && GroupByTag.Contains(TagId))
    {
        if (auto Group = *GroupByTag.Find(TagId))
        {
            return Group->RemoveWidgetComponent(WidgetComponent);
        }
    }
    return false;
}

bool UUMGControlOverlapGroupContainer::RemoveGroup(const FString& TagId)
{
    if (!TagId.IsEmpty() && GroupByTag.Contains(TagId))
    {
        if (auto Group = *GroupByTag.Find(TagId))
        {
            if (Group->Destroy())
            {
                GroupByTag.Remove(TagId);
                return true;
            }
        }
    }
    return false;
}

void UUMGControlOverlapGroupContainer::Update()
{
    for (const auto& it : GroupByTag)
    {
        it.Value->Update();
    }
}
