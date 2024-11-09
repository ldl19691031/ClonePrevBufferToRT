#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include <Engine/TextureRenderTarget2D.h>
#include "CopyPrevFrameBufferManager.generated.h"

UCLASS()
class COPYPREVFRAMEBUFFER_API UCopyPrevFrameBufferManager : public UWorldSubsystem
{
    GENERATED_BODY()
public:
    //Begin UEngineSubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    //End UEngineSubsystem interface
private:
    TSharedPtr<class FCopyPrevFrameSceneViewExtension, ESPMode::ThreadSafe> CopyPrevFrameSceneViewExtension;
};