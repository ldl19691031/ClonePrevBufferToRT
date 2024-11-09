#include "CopyPrevFrameSceneViewExtension.h"
#include "CopyPrevFrameBufferManager.h"
#include "CopyPrevFrameBufferSettings.h"
#include "PostProcess/PostProcessing.h"
#include "SceneRendering.h"
#include <Engine/TextureRenderTarget2D.h>
#include "SceneRendering.h"
#include "ScreenPass.h"
#include "RenderGraphEvent.h"
#include "PostProcess/PostProcessing.h"
#include "PostProcess/PostProcessMaterialInputs.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "TextureResource.h"
#include "Async/Async.h"
void FCopyPrevFrameSceneViewExtension::PrePostProcessPass_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessingInputs& Inputs)
{
    if (NeedCopyPrevFrame == false)
    {
        return;
    }

    const UCopyPrevFrameBufferSettings* CopyPrevFrameBufferSettings = GetDefault<UCopyPrevFrameBufferSettings>();
    const bool bCopySceneColorBeforePostProcess = CopyPrevFrameBufferSettings->CopySceneColorBeforePPToRT;
    if (bCopySceneColorBeforePostProcess
        && CopyPrevFrameBufferSettings->SceneColorBeforePPRT
        && CopyPrevFrameBufferSettings->SceneColorBeforePPRT->GetRenderTargetResource()
        && CopyPrevFrameBufferSettings->SceneColorBeforePPRT->GetRenderTargetResource()->GetRenderTargetTexture())
    {
        RDG_EVENT_SCOPE(GraphBuilder, "CopyPrevFrameScene_ColorBeforePP");
        this->PrevFrameColorRT_BeforePostProcess = CreateRenderTarget(
            CopyPrevFrameBufferSettings->SceneColorBeforePPRT->GetRenderTargetResource()->GetRenderTargetTexture(),
            TEXT("PrevFrameColorRT_BeforePostProcess")
        );
        check(View.bIsViewInfo);
        const auto& ViewInfo = static_cast<const FViewInfo&>(View);
        const FScreenPassTexture SceneColor((*Inputs.SceneTextures)->SceneColorTexture, ViewInfo.ViewRect);
        FRDGTextureRef SrcTexture = SceneColor.Texture;
        FRDGTextureRef DstTexture = GraphBuilder.RegisterExternalTexture(this->PrevFrameColorRT_BeforePostProcess);
        const FIntRect PrimaryViewRect = ((const FViewInfo&)View).ViewRect;
        const FIntPoint PrimaryViewSize = PrimaryViewRect.Size();
        const float RTSizeScale = CopyPrevFrameBufferSettings->RTSizeScale;
        const FIntPoint RTSize = FIntPoint(FMath::Max(1, FMath::RoundToInt(PrimaryViewSize.X * RTSizeScale)),
            FMath::Max(1, FMath::RoundToInt(PrimaryViewSize.Y * RTSizeScale)));
        const FIntPoint SrcSize = SceneColor.ViewRect.Size();
        const FIntPoint DstSize = DstTexture->Desc.Extent;
        const bool isRTSizeValid = SrcSize.Size() > 0 && DstSize.Size() > 0;
        if (isRTSizeValid)
        {
            AddDrawTexturePass(GraphBuilder, View, SrcTexture, DstTexture, FIntPoint::ZeroValue, SrcSize, FIntPoint::ZeroValue, DstSize);
        }
    }

}

void FCopyPrevFrameSceneViewExtension::PostRenderView_RenderThread(FRDGBuilder& GraphBuilder, FSceneView& View)
{
    FViewInfo& ViewInfo = (FViewInfo&)View;
    
}

void FCopyPrevFrameSceneViewExtension::SetupViewFamily(FSceneViewFamily& InViewFamily)
{
}

void FCopyPrevFrameSceneViewExtension::SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView)
{
    NeedCopyPrevFrame = false;
    UCopyPrevFrameBufferManager* CopyPrevFrameBufferManager = Manager.Get();
    if (CopyPrevFrameBufferManager == nullptr)
    {
        NeedCopyPrevFrame = false;
        return;
    }
    const UCopyPrevFrameBufferSettings* CopyPrevFrameBufferSettings = GetDefault<UCopyPrevFrameBufferSettings>();
    if (CopyPrevFrameBufferSettings == nullptr)
    {
        NeedCopyPrevFrame = false;
        return;
    }

    if (InView.bIsSceneCapture)
    {
        NeedCopyPrevFrame = false;
        return;
    }

    NeedCopyPrevFrame |= CopyPrevFrameBufferSettings->CopyFinalSceneColorToRT;
    NeedCopyPrevFrame |= CopyPrevFrameBufferSettings->CopySceneColorBeforePPToRT;

    const FIntRect PrimaryViewRect = InView.UnscaledViewRect;
    const FIntPoint PrimaryViewSize = PrimaryViewRect.Size();
    const float RTSizeScale = CopyPrevFrameBufferSettings->RTSizeScale;
    const FIntPoint RTSize = FIntPoint(FMath::Max(1, FMath::RoundToInt(PrimaryViewSize.X * RTSizeScale)),
        FMath::Max(1, FMath::RoundToInt(PrimaryViewSize.Y * RTSizeScale)));
    if (NeedCopyPrevFrame)
    {
        UTextureRenderTarget2D* PrevFinalSceneColorRT = CopyPrevFrameBufferSettings->FinalSceneColorRT.LoadSynchronous();
        if (PrevFinalSceneColorRT && CopyPrevFrameBufferSettings->CopyFinalSceneColorToRT)
        {
            if (PrevFinalSceneColorRT->SizeX != RTSize.X || PrevFinalSceneColorRT->SizeY != RTSize.Y)
            {
                PrevFinalSceneColorRT->ResizeTarget(RTSize.X, RTSize.Y);
                FlushRenderingCommands();
            }
        }

        UTextureRenderTarget2D* PrevSceneColorBeforePPRT = CopyPrevFrameBufferSettings->SceneColorBeforePPRT.LoadSynchronous();
        if (PrevSceneColorBeforePPRT && CopyPrevFrameBufferSettings->CopySceneColorBeforePPToRT)
        {
            if (PrevSceneColorBeforePPRT->SizeX != RTSize.X || PrevSceneColorBeforePPRT->SizeY != RTSize.Y)
            {
                PrevSceneColorBeforePPRT->ResizeTarget(RTSize.X, RTSize.Y);
                FlushRenderingCommands();
            }
        }
    }
}

void FCopyPrevFrameSceneViewExtension::BeginRenderViewFamily(FSceneViewFamily& InViewFamily)
{
}

void FCopyPrevFrameSceneViewExtension::SubscribeToPostProcessingPass(EPostProcessingPass PassId, FAfterPassCallbackDelegateArray& InOutPassCallbacks, bool bIsPassEnabled)
{
    if (PassId == EPostProcessingPass::FXAA)
    {
        InOutPassCallbacks.Add(FAfterPassCallbackDelegate::CreateRaw(this, &FCopyPrevFrameSceneViewExtension::PostProcessPassAfterTonemap_RenderThread));
    }
}

FCopyPrevFrameSceneViewExtension::FCopyPrevFrameSceneViewExtension(const FAutoRegister& AutoRegister, UWorld* InWorld)
    : FWorldSceneViewExtension(AutoRegister, InWorld)
{
}

FCopyPrevFrameSceneViewExtension::~FCopyPrevFrameSceneViewExtension()
{
}

FScreenPassTexture FCopyPrevFrameSceneViewExtension::PostProcessPassAfterTonemap_RenderThread(FRDGBuilder& GraphBuilder, const FSceneView& View, const FPostProcessMaterialInputs& InOutInputs)
{
    
    // If the override output is provided, it means that this is the last pass in post processing.
    if (NeedCopyPrevFrame == false)
    {
        return InOutInputs.ReturnUntouchedSceneColorForPostProcessing(GraphBuilder);
    }
    

    const UCopyPrevFrameBufferSettings* CopyPrevFrameBufferSettings = GetDefault<UCopyPrevFrameBufferSettings>();
    const bool bCopySceneColorAfterPostProcess = CopyPrevFrameBufferSettings->CopyFinalSceneColorToRT;
    if (bCopySceneColorAfterPostProcess
        && CopyPrevFrameBufferSettings->FinalSceneColorRT
        && CopyPrevFrameBufferSettings->FinalSceneColorRT->GetRenderTargetResource()
        && CopyPrevFrameBufferSettings->FinalSceneColorRT->GetRenderTargetResource()->GetRenderTargetTexture())
    {
        RDG_EVENT_SCOPE(GraphBuilder, "CopyPrevFrameScene_ColorAfterPP");
        this->PrevFrameColorRT_AfterPostProcess = CreateRenderTarget(
            CopyPrevFrameBufferSettings->FinalSceneColorRT->GetRenderTargetResource()->GetRenderTargetTexture(),
            TEXT("PrevFrameColorRT_AfterPostProcess")
        );
        check(View.bIsViewInfo);
        const auto& ViewInfo = static_cast<const FViewInfo&>(View);
        
        FRDGTextureRef DstTexture = GraphBuilder.RegisterExternalTexture(this->PrevFrameColorRT_AfterPostProcess);
        const FIntRect PrimaryViewRect = ((const FViewInfo&)View).ViewRect;
        const FIntPoint PrimaryViewSize = PrimaryViewRect.Size();
        const float RTSizeScale = CopyPrevFrameBufferSettings->RTSizeScale;
        const FIntPoint RTSize = FIntPoint(FMath::Max(1, FMath::RoundToInt(PrimaryViewSize.X * RTSizeScale)),
            FMath::Max(1, FMath::RoundToInt(PrimaryViewSize.Y * RTSizeScale)));
        const FScreenPassTexture& SceneColor = FScreenPassTexture::CopyFromSlice(GraphBuilder, InOutInputs.GetInput(EPostProcessMaterialInput::SceneColor));
        const FIntPoint SrcSize = SceneColor.ViewRect.Size();
        const FIntPoint DstSize = DstTexture->Desc.Extent;
        const bool isRTSizeValid = SrcSize.Size() > 0 && DstSize.Size() > 0;
        if (isRTSizeValid)
        {
            check(SceneColor.IsValid());
            FRDGTextureRef SrcTexture = SceneColor.Texture;
            AddDrawTexturePass(GraphBuilder, View, SrcTexture, DstTexture, FIntPoint::ZeroValue, SrcSize, FIntPoint::ZeroValue, DstSize);
        }
    }
    return InOutInputs.ReturnUntouchedSceneColorForPostProcessing(GraphBuilder);
}
