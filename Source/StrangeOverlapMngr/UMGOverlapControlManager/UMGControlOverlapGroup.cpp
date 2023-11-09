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


void UUMGControlOverlapGroup::Update()
{
    if (!NeedUpdate())
    {
        return;
    }


    TArray<FVector2D> ViewportPositions;
    TArray<UUMGControlOverlapItem*> ItemsForAllign;
    for (auto it : Items)
    {
        FVector2D Pos(0.0f,0.0f);
        if (it->GetPositionInViewport(PlayerController, Pos))
        {
            ViewportPositions.Add(Pos);
            ItemsForAllign.Add(it);
            it->PosForSort = Pos;
        }
    }
    if (ItemsForAllign.Num() <= 1)
    {
        return;
    }
    ItemsForAllign.Sort([&](const UUMGControlOverlapItem& ItemA, const UUMGControlOverlapItem& ItemB)
    {
        return ItemA.PosForSort.X < ItemB.PosForSort.X;

    });
    
    Items.Sort([&](const UUMGControlOverlapItem& ItemA, const UUMGControlOverlapItem& ItemB)
    {
        FVector2D PosA, PosB = FVector2D::Zero();
        if (ItemA.GetPositionInViewport(PlayerController, PosA))
        {
            if (ItemB.GetPositionInViewport(PlayerController, PosB))
            {
                return PosA.X < PosB.X;
            }
            return false;
        }
        else
        {
            return false;
        }
        
    });
    
    bool bShouldAlign = false;
    for (int32 i = 0; i< ItemsForAllign.Num(); i++)
    {
        UUMGControlOverlapItem* ItemA = ItemsForAllign[i];
        if (!ItemA)
        {
            continue;
        }
        FVector2D ItemASize = ItemA->GetDesiredSize();
        FVector2D ItemAScreenPos;;
        if (!ItemA->GetPositionInViewport(PlayerController, ItemAScreenPos))
        {
            //ItemA->SetWorldLocation(ItemA->GetStartedPosition());
            /*ItemA->SetIsGrouping(false);
            ItemA->SetWorldLocation(ItemA->GetStartedPosition());
            GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("False"), true, FVector2D(1.0f, 1.0f));*/
            continue;
        }
        float XOffset = ItemAScreenPos.X + ItemASize.X;
        float YPos = ItemAScreenPos.Y;
        FSlateRect ItemARect(ItemAScreenPos.X, ItemAScreenPos.Y, ItemAScreenPos.X + ItemASize.X, ItemAScreenPos.Y + ItemASize.Y);
        
        for (int32 j = i + 1; j < ItemsForAllign.Num(); j++)
        {
            UUMGControlOverlapItem* ItemB = ItemsForAllign[j];
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
                //ItemB->SetWorldLocation(ItemB->GetStartedPosition());
                continue;
            }
            FSlateRect ItemBRect(ItemBScreenPos.X, ItemBScreenPos.Y, ItemBScreenPos.X + ItemBSize.X, ItemBScreenPos.Y + ItemBSize.Y);

            FVector2D ItemBStartScreenPos;
            if (!UGameplayStatics::ProjectWorldToScreen(PlayerController, ItemB->GetStartedPosition(), ItemBStartScreenPos))
            {
               /* ItemB->SetWorldLocation(ItemB->GetStartedPosition());
                GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("False"), true, FVector2D(1.0f, 1.0f));*/
                //ItemB->SetWorldLocation(ItemB->GetStartedPosition());
                continue;
            }
            
            FSlateRect ItemBStartPosRect(ItemBStartScreenPos.X, ItemBStartScreenPos.Y, ItemBStartScreenPos.X + ItemBSize.X, ItemBStartScreenPos.Y + ItemBSize.Y);

            if (FSlateRect::DoRectanglesIntersect(ItemARect, ItemBRect))
            {
               
                if (!FSlateRect::DoRectanglesIntersect(ItemARect, ItemBStartPosRect))
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
                    //if (!FSlateRect::DoRectanglesIntersect(ItemARect, ItemBStartPosRect))
                    //{
                    //    ItemB->SetWorldLocation(ItemB->GetStartedPosition());
                    //    //ItemB->SetIsGrouping(false);
                    //}
                    //ItemB->SetWorldLocation(ItemB->GetStartedPosition());
                    ItemB->SetWorldLocation(ItemB->GetStartedPosition());
                    continue;
                }
                 XOffset += ItemBSize.X;
                GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Red, TEXT("True"), true, FVector2D(1.0f, 1.0f));
            }
            else
            {
                //ItemB->SetWorldLocation(ItemB->GetStartedPosition());
                //ItemB->SetWorldLocation(ItemB->GetStartedPosition());
                if (!FSlateRect::DoRectanglesIntersect(ItemARect, ItemBStartPosRect))
                {
                    ItemB->SetWorldLocation(ItemB->GetStartedPosition());
                    //ItemB->SetIsGrouping(false);
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
