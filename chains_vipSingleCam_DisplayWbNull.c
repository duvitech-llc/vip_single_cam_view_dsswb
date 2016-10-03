/*
 *******************************************************************************
 *
 * Copyright (C) 2016 Duvitech - http://www.duvitech.com/
 * ALL RIGHTS RESERVED
 *
 *******************************************************************************
 */


/*******************************************************************************
 *  INCLUDE FILES
 *******************************************************************************
 */

#include "chains_vipSingleCam_DisplayWbNull_priv.h"
#include <linux/examples/tda2xx/include/chains.h>
#include <linux/examples/common/chains_common.h>
#include <linux/src/system/system_gbm_allocator.h>

#define CAPTURE_SENSOR_WIDTH      (1280)
#define CAPTURE_SENSOR_HEIGHT     (720)
#define LCD_DISPLAY_WIDTH         (800)
#define LCD_DISPLAY_HEIGHT        (480)

/* Display writes two pixels extra for the interlaced modes, so
   DSS write back should allocate memory for extra two pixels.
   This macro is used for providing extra pixels to the display write back path
   And also for configuring CRC modules ROI correctly. */
#define DISPLAY_WB_EXTRA_PIXELS             (2U)

static volatile UInt32 frameCount = 0;

/**
 *******************************************************************************
 *
 *  \brief  SingleCameraViewObject
 *
 *        This structure contains all the LinksId's and create Params.
 *        The same is passed to all create, start, stop functions.
 *
 *******************************************************************************
*/
typedef struct {

    chains_vipSingleCam_DisplayWbNullObj ucObj;

    /**< Link Id's and device IDs to use for this use-case */
    UInt32  appCtrlLinkId;

    UInt32  captureOutWidth;
    UInt32  captureOutHeight;

    Chains_Ctrl *chainsCfg;

}chains_vipSingleCam_DisplayWbNullAppObj;

static struct control_srv_egl_ctx chainsEglParams = {
   .get_egl_native_display = gbm_allocator_get_native_display,
   .get_egl_native_buffer = gbm_allocator_get_native_buffer,
   .destroy_egl_native_buffer = gbm_allocator_destroy_native_buffer,
};

Int32 chains_vipSingleCam_DisplayWbNull_CbFxn(
                         Void *appObj,
                         System_Buffer *pBuffer)
{
    frameCount++;
    if(frameCount%30 == 0){
        Vps_printf("\nFrameCount '%i'\n", frameCount);
    }

    return SYSTEM_LINK_STATUS_SOK;
}

/**
 *******************************************************************************
 *
 * \brief   Set SGXDISPLAY Link Parameters
 *
 *          It is called in Create function.

 *
 * \param   pPrm    [IN]    IpcLink_CreateParams
 *
 *******************************************************************************
*/
Void chains_vipSingleCam_DisplayWbNull_SetSgxDisplayLinkPrms (
                                  SgxFrmcpyLink_CreateParams *prms,
                                  UInt32 width, UInt32 height)
{
    prms->displayWidth = width;
    prms->displayHeight = height;
    prms->renderType = SGXFRMCPY_RENDER_TYPE_1x1;
    prms->inBufType = SYSTEM_BUFFER_TYPE_VIDEO_FRAME;
    prms->bEglInfoInCreate = TRUE;
    prms->EglInfo = (void *)&chainsEglParams;
}

/**
 *******************************************************************************
 *
 * \brief   Set link Parameters of the DSS WB capture
 *
 *******************************************************************************
*/
static void chains_vipSingleCam_DisplayWbNull_SetCaptureDssWbPrms(
                                   CaptureLink_CreateParams *pPrm,
                                   UInt32 displayWidth,
                                   UInt32 displayHeight,
                                   Chains_DisplayType displayType)
{
    pPrm->callback = chains_vipSingleCam_DisplayWbNull_CbFxn;
    pPrm->numVipInst = 0;
    pPrm->numDssWbInst = 1;
    pPrm->dssWbInst[0].dssWbInstId = 0;
    System_VideoScanFormat scanFormat = SYSTEM_SF_PROGRESSIVE;

    pPrm->dssWbInst[0].dssWbInputPrms.inNode = SYSTEM_WB_IN_NODE_TV;
    /* Set this to SYSTEM_WB_IN_NODE_TV for TDA2xx platform
       and to SYSTEM_WB_IN_NODE_LCD1 for TDA3xx platform */
    pPrm->dssWbInst[0].dssWbInputPrms.wbInSourceWidth = displayWidth;
    pPrm->dssWbInst[0].dssWbInputPrms.wbInSourceHeight = displayHeight;
    pPrm->dssWbInst[0].dssWbInputPrms.wbInWidth = displayWidth;
    pPrm->dssWbInst[0].dssWbInputPrms.wbInHeight = displayHeight;
    pPrm->dssWbInst[0].dssWbInputPrms.wbPosx = 0;
    pPrm->dssWbInst[0].dssWbInputPrms.wbPosy = 0;
    pPrm->dssWbInst[0].dssWbInputPrms.wbInSourceDataFmt = SYSTEM_DF_BGR24_888;
    pPrm->dssWbInst[0].dssWbInputPrms.wbScanFormat = scanFormat;

    pPrm->dssWbInst[0].dssWbOutputPrms.wbWidth = displayWidth;
    pPrm->dssWbInst[0].dssWbOutputPrms.wbHeight = displayHeight;
    pPrm->dssWbInst[0].dssWbOutputPrms.wbDataFmt = SYSTEM_DF_BGR24_888;
    pPrm->dssWbInst[0].dssWbOutputPrms.wbScanFormat = scanFormat;

    pPrm->dssWbInst[0].numBufs = CAPTURE_LINK_NUM_BUFS_PER_CH_DEFAULT;
}



/**
 *******************************************************************************
 *
 * \brief   Set link Parameters
 *
 *          It is called in Create function of the auto generated use-case file.
 *
 * \param pUcObj    [IN] Auto-generated usecase object
 * \param appObj    [IN] Application specific object
 *
 *******************************************************************************
*/

Void chains_vipSingleCam_DisplayWbNull_SetAppPrms(chains_vipSingleCam_DisplayWbNullObj *pUcObj, Void *appObj)
{
    UInt32 displayWidth, displayHeight;

    chains_vipSingleCam_DisplayWbNullAppObj *pObj
            = (chains_vipSingleCam_DisplayWbNullAppObj*)appObj;

    pObj->captureOutWidth  = CAPTURE_SENSOR_WIDTH;
    pObj->captureOutHeight = CAPTURE_SENSOR_HEIGHT;

    ChainsCommon_SingleCam_SetCapturePrms(&(pUcObj->Capture_dsswbPrm),
            CAPTURE_SENSOR_WIDTH,
            CAPTURE_SENSOR_HEIGHT,
            pObj->captureOutWidth,
            pObj->captureOutHeight,
            pObj->chainsCfg->captureSrc
            );

    ChainsCommon_GetDisplayWidthHeight(
        pObj->chainsCfg->displayType,
        &displayWidth,
        &displayHeight
        );

    chains_vipSingleCam_DisplayWbNull_SetCaptureDssWbPrms(
                            &pUcObj->Capture_dsswbPrm,
                            displayWidth,
                            displayHeight,
                            pObj->chainsCfg->displayType);

    chains_vipSingleCam_DisplayWbNull_SetSgxDisplayLinkPrms
                    (&pUcObj->SgxFrmcpyPrm,
                     displayWidth,
                     displayHeight
                    );

    ChainsCommon_SetGrpxSrcPrms(&pUcObj->GrpxSrcPrm,
                                displayWidth,
                                displayHeight
                                );

    pUcObj->Display_GrpxPrm.rtParams.tarWidth       = displayWidth;
    pUcObj->Display_GrpxPrm.rtParams.tarHeight      = displayHeight;
    pUcObj->Display_GrpxPrm.rtParams.posX           = 0;
    pUcObj->Display_GrpxPrm.rtParams.posY           = 0;
    pUcObj->Display_GrpxPrm.displayId               = DISPLAY_LINK_INST_DSS_GFX1;

    ChainsCommon_SetDisplayPrms(&pUcObj->Display_M4Prm,
                                NULL,
                                pObj->chainsCfg->displayType,
                                displayWidth,
                                displayHeight
                               );

    ChainsCommon_StartDisplayCtrl(
        pObj->chainsCfg->displayType,
        displayWidth,
        displayHeight
        );

}

/**
 *******************************************************************************
 *
 * \brief   Start the capture display Links
 *
 *          Function sends a control command to capture and display link to
 *          to Start all the required links . Links are started in reverce
 *          order as information of next link is required to connect.
 *          System_linkStart is called with LinkId to start the links.
 *
 * \param   pObj  [IN] chains_vipSingleCam_SgxDisplayAppObj
 *
 *
 *******************************************************************************
*/
Void chains_vipSingleCam_DisplayWbNull_StartApp(chains_vipSingleCam_DisplayWbNullAppObj *pObj)
{
    ChainsCommon_statCollectorReset();
    ChainsCommon_memPrintHeapStatus();

    chains_vipSingleCam_DisplayWbNull_Start(&pObj->ucObj);

    ChainsCommon_prfLoadCalcEnable(TRUE, FALSE, FALSE);
}

/**
 *******************************************************************************
 *
 * \brief   Stop the capture display Links
 *
 *          Function sends a control command to capture and display link to
 *          to Start all the required links . Links are started in reverce
 *          order as information of next link is required to connect.
 *          System_linkStart is called with LinkId to start the links.
 *
 * \param   pObj  [IN] chains_vipSingleCam_SgxDisplayAppObj
 *
 *
 *******************************************************************************
*/
Void chains_vipSingleCam_DisplayWbNull_StopApp(chains_vipSingleCam_DisplayWbNullAppObj *pObj)
{

    chains_vipSingleCam_DisplayWbNull_Stop(&pObj->ucObj);

    chains_vipSingleCam_DisplayWbNull_Delete(&pObj->ucObj);

    ChainsCommon_StopDisplayCtrl();

    ChainsCommon_prfLoadCalcEnable(FALSE, FALSE, FALSE);

}

/**
 *******************************************************************************
 *
 * \brief   Single Channel Capture Display usecase function
 *
 *          This functions executes the create, start functions
 *
 *          Further in a while loop displays run time menu and waits
 *          for user inputs to print the statistics or to end the demo.
 *
 *          Once the user inputs end of demo stop and delete
 *          functions are executed.
 *
 * \param   chainsCfg       [IN]   Chains_Ctrl
 *
 *******************************************************************************
*/
Void chains_vipSingleCam_DisplayWbNull(Chains_Ctrl *chainsCfg)
{
    char ch;
    UInt32 done = FALSE;
    chains_vipSingleCam_DisplayWbNullAppObj chainsObj;

    chainsObj.chainsCfg = chainsCfg;

    chains_vipSingleCam_DisplayWbNull_Create(&chainsObj.ucObj, &chainsObj);

    chains_vipSingleCam_DisplayWbNull_StartApp(&chainsObj);

    while(!done)
    {
        ch = Chains_menuRunTime();

        switch(ch)
        {
            case '0':
                done = TRUE;
                break;
            case 'p':
            case 'P':
                ChainsCommon_prfCpuLoadPrint();
                ChainsCommon_statCollectorPrint();
                chains_vipSingleCam_DisplayWbNull_printStatistics(&chainsObj.ucObj);
                chains_vipSingleCam_DisplayWbNull_printBufferStatistics(&chainsObj.ucObj);
                break;
            default:
                Vps_printf("\nUnsupported option '%c'. Please try again\n", ch);
                break;
        }
    }

    chains_vipSingleCam_DisplayWbNull_StopApp(&chainsObj);

}
