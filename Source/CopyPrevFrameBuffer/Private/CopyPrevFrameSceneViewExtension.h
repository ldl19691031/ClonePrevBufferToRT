#pragma once
#include "CoreMinimal.h"
#include "SceneViewExtension.h"

class FCopyPrevFrameSceneViewExtension : public FWorldSceneViewExtension
{
public:
    // Begin FSceneViewExtensionBase
    virtual void PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessingInputs& Inputs) override;
    virtual void PostRenderView_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& View) override;
    virtual void SetupViewFamily(FSceneViewFamily& InViewFamily) override;;
    virtual void SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView) override;
    virtual void BeginRenderViewFamily(FSceneViewFamily& InViewFamily) override;
    virtual void SubscribeToPostProcessingPass(EPostProcessingPass PassId, FAfterPassCallbackDelegateArray& InOutPassCallbacks, bool bIsPassEnabled) override;
    // End FSceneViewExtensionBase

    FCopyPrevFrameSceneViewExtension(const FAutoRegister& AutoRegister, UWorld* InWorld);

    ~FCopyPrevFrameSceneViewExtension();

    TWeakObjectPtr<class UCopyPrevFrameBufferManager > Manager;
private:
    bool NeedCopyPrevFrame = false;

    TRefCountPtr<IPooledRenderTarget> PrevFrameColorRT_BeforePostProcess;
    TRefCountPtr<IPooledRenderTarget> PrevFrameColorRT_AfterPostProcess;
private:
    FScreenPassTexture PostProcessPassAfterTonemap_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessMaterialInputs& Inputs);
};