.define gBenchmarkOut1     $0030 ; |  G
.define gBenchmarkOut2     $0032 ; |  G


; App global statuses - - - - - - - - - - - - -
.define gFrameReady        $0050 ; |  G
.define gPadState          $0052 ; |  G

.define gCurPadUD          $0054 ; |  G
.define gPrevPadUD         $0056 ; |  G

.define gCurPadTrg         $0058 ; |  G
.define gPrevPadTrg        $005A ; |  G

; Temporary storage

.define gAssetOffsetTemp   $0060 ; |  G
.define gAssetBankTemp     $0062 ; |  G
.define gNewObjIndexTemp   $0068 ; |  G
.define gLocalTemp1        $006A ; |  G
.define gLocalTemp2        $006C ; |  G

.define gScrollCount       $0080 ; |  G
.define gSpAnimationCount  $0082 ; |  G

.define gPrevLPush         $008A ; |  G
.define gPrevRPush         $008C ; |  G

; SPC work area
.define gTxIndirectTemp    $0090 ; |  G 24bit
.define gSPCSendCounter    $0096 ; |  G
.define gSPCBlockCounter   $0098 ; |  G
.define gBlobSizeToSend    $009A ; |  G

.define gSE1SeqCount       $009C ; |  G
.define gSE2SeqCount       $009E ; |  G

.define gBlobSrcAddrLo     $00A0 ; |  G
.define gBlobSrcAddrHi     $00A2 ; |  G
.define gBlobSrcAddrLoNext $00A4 ; |  G
.define gBlobSrcAddrHiNext $00A6 ; |  G

.define gSENumShift        $00A8 ; |  G

; Load animation
.define gSpinnerAnimWait   $00C0 ; |  G
.define gSpinnerFrameIdx   $00C2 ; |  G
.define gLoaderBusy        $00C4 ; |  G

.define gQuickLoadOK       $00CA ; |  G
.define gQuickLoadSize     $00CE ; |  G

; Selection
.define gSelectedIndex     $0110 ; |  G
.define gSPCResetReq       $0118 ; |  G
.define gSPCLoadReq        $011A ; |  G

; Cursor control
.define gCursorPosIndex    $0120 ; |  G
.define gCursorAnimCount   $0122 ; |  G
.define gCursorAnimDir     $0124 ; |  G
.define gCursorYOffset     $0126 ; |  G
.define gMenuItemsCount    $0128 ; |  G
.define gSelectAnimCount   $012C ; |  G
.define gCursorXOffset     $012E ; |  G

; Text temp buffer
.define gTextBufWPos       $013A ; |  G
.define gTextBufTxRequest  $013C ; |  G
.define gTextTempBuf       $0140 ; |  G
; -> 1BFh (128bytes/64W)

; Dynamic sprite params
.define gDynSpFirstTile    $01C0 ; |  G
.define gDynSpRowBytes     $01C2 ; |  G
.define gDynSpDestAddr     $01C6 ; |  G

.define gDynSpSrcBank      $01CA ; |  G
.define gDynSpSrcOffset    $01CC ; |  G


.define gHDMATable         $0200 ; |  G


; Pad constants
.define kPad16_Up    #$0800
.define kPad16_Down  #$0400
.define kPad16_Left  #$0200
.define kPad16_Right #$0100
.define kPad16_TrgA  #$0080
.define kPad16_TrgB  #$8000
.define kPad16_TrgX  #$0040
.define kPad16_TrgY  #$4000
.define kPad16_TrgL  #$0020
.define kPad16_TrgR  #$0010
.define kPad16_Start #$1000

.define kPad16_UpDown  #$0C00
.define kPad16_AB      #$8080


; VRAM Mapping constants
.define kMap1StartWHi #$68
.define kMap1StartWSh #$6800

.define kMap3StartWHi #$70
.define kMap3StartWSh #$7000

; (not used)
.define kMap2StartWHi #$78
.define kMap2StartWSh #$7800


; SPC driver configuration
; ---------------------------
; ******* Max size = 64KB - IPL - ProgramOrigin
.define numDriverLoImageSize 32768 ; MUST BE Nx128
.define numDriverHiImageSize 30720 ; MUST BE Nx128
.define kDriverLoImageSize   #numDriverLoImageSize
.define kDriverHiImageSize   #numDriverHiImageSize

.define numDriverQuickLoadSize  8192 ; MUST BE Nx128
.define kDriverQuickLoadSize    #numDriverQuickLoadSize
; ---------------------------

.define kTickerY  #5

; I/O Registers

.define pSpriteConf  $2101
.define pOAMWAddress $2102
.define pOAMWrite    $2104

.define pScrollX0    $210D
.define pScrollY0    $210E
.define pScrollX1    $210F
.define pScrollY1    $2110

.define pVWriteAddrW $2116
.define pVWriteValW  $2118

.define pDMATrigger        $420b

.define pDMA0Config        $4300
.define pDMA0Dest          $4301
.define pDMA0SourceOffsetW $4302
.define pDMA0SourceBank    $4304
.define pDMA0ByteCountW    $4305

.define pDMA1Config        $4310
.define pDMA1Dest          $4311
.define pDMA1SourceOffsetW $4312
.define pDMA1SourceBank    $4314
.define pDMA1ByteCountW    $4315

.define pDivc_dividend     $4204
.define pDivc_divisor      $4206
.define pDivc_outQ         $4214
.define pDivc_outM         $4216
