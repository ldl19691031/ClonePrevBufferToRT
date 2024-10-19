#include "CopyPrevFrameBufferManager.h"
#include "CopyPrevFrameSceneViewExtension.h"
void UCopyPrevFrameBufferManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    CopyPrevFrameSceneViewExtension = FSceneViewExtensions::NewExtension<FCopyPrevFrameSceneViewExtension>();
}

void UCopyPrevFrameBufferManager::Deinitialize()
{
    Super::Deinitialize();
}
