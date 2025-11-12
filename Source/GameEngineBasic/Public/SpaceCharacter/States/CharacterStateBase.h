
#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "CharacterStateBase.generated.h"


class ASpaceCharacter;


UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class GAMEENGINEBASIC_API UCharacterStateBase : public UObject
{
    GENERATED_BODY()

public:

    // 상태 진입 시 호출
    UFUNCTION(BlueprintNativeEvent, Category = "State")
    void Enter(ASpaceCharacter* Character);
    virtual void Enter_Implementation(ASpaceCharacter* Character);

    // 매 프레임 호출
    UFUNCTION(BlueprintNativeEvent, Category = "State")
    void Tick(ASpaceCharacter* Character, float DeltaTime);
    virtual void Tick_Implementation(ASpaceCharacter* Character, float DeltaTime);

    // 상태 종료 시 호출
    UFUNCTION(BlueprintNativeEvent, Category = "State")
    void Exit(ASpaceCharacter* Character);
    virtual void Exit_Implementation(ASpaceCharacter* Character);
};
