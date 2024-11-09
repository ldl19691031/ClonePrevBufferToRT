#include "CopyPrevFrameBufferManager.h"
#include "CopyPrevFrameSceneViewExtension.h"
void UCopyPrevFrameBufferManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UWorld* World = GetWorld();
    if (World)
    {
        CopyPrevFrameSceneViewExtension = FSceneViewExtensions::NewExtension<FCopyPrevFrameSceneViewExtension>(World);
        CopyPrevFrameSceneViewExtension->Manager = this;
    }
}

void UCopyPrevFrameBufferManager::Deinitialize()
{
    Super::Deinitialize();
}
