#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "CopyPrevFrameBufferSettings.generated.h"


UCLASS(Config = Game, defaultconfig, meta = (DisplayName = "Copy Prev Frame Buffer"))
class COPYPREVFRAMEBUFFER_API UCopyPrevFrameBufferSettings : public UDeveloperSettings
{
    GENERATED_BODY()
public:

    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General", DisplayName = "Copy Final Scene Color to RT")
    bool CopyFinalSceneColorToRT = false;

    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General",  meta = (EditCondition = "CopyFinalSceneColorToRT"))
    TSoftObjectPtr<class UTextureRenderTarget2D> FinalSceneColorRT;

    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General", DisplayName = "Copy Scene Color Before PP and Depth to RT")
    bool CopySceneColorBeforePPToRT = false;

    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General", meta = (EditCondition = "CopySceneColorBeforePPAndDepthToRT"))
    TSoftObjectPtr<class UTextureRenderTarget2D> SceneColorBeforePPRT;

    UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "General", DisplayName = "RT Size Scale", meta = (ClampMin = "0.1"))
    float RTSizeScale = 1.0f;
};
