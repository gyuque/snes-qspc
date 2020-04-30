.setcpu     "65816"
.autoimport on
.include "GlobalVarsAndConsts.inc"
.include "utility-macros.inc"
.import AssetBGGraphic:far
.import AssetSpGraphic:far
.import AssetFont:far
.import BGMapAsset:far
.import SongMetadataList:far
.import StrTitleDisp:far
.import StrSingleMode:far
.import StrSongListCaption:far
.import StrBy:far
.import StrLoading:far
.import StrEmpty:far
.import WStrInstruction:far

.export setupMainScreen
.export startTextLayerHDMA
.export processMainInput
.export processSETriggers
.export moveCursorSprite
.export msUpdateCursorSpriteTile
.export msAdvanceSpinnerAnimation

.export spcUpdateProgressHandler

.define kCursorBaseX #24
.define kCursorBaseY #110
.define kIndicatorBaseX #44
.define kIndicatorBaseY #112
.define kMenuTextBaseY  #14

.define kMetaTextStride #32
.define kMaxTracks      #4

; A16, I16
.proc setupMainScreen
.a16
.i16
	save_paxy
	jsr tbClear
	
	stz gQuickLoadOK
	stz gLoaderBusy
	stz gSPCResetReq
	stz gSPCLoadReq
	
	stz gSpinnerAnimWait
	stz gSpinnerFrameIdx
	
	stz gCursorPosIndex
	stz gCursorAnimCount
	stz gCursorAnimDir
	stz gCursorXOffset
	stz gCursorYOffset
	ldsta gScrollCount,#-160

	stz gPadState
	stz gPrevLPush
	stz gPrevRPush
	stz gCurPadUD
	stz gPrevPadUD
	stz gCurPadTrg
	stz gPrevPadTrg
	stz gSelectAnimCount

	
	loadWithAssetAddress AssetBGGraphic, #$0000, #6144
	loadWithAssetAddress AssetSpGraphic, #$4000, #3072
	loadWithAssetAddress AssetFont, #$6000, #2048

	; for BG
	ldx #0
	ldy #0
	jsr transferPaletteData

	ldx #1
	ldy #16
	jsr transferPaletteData

	; for Palette
	ldx #2
	ldy #128
	jsr transferPaletteData
	
	; build BG
	jsr textClearBG
	textDirectWriteSZResource StrTitleDisp, #5, kTickerY
	textDirectWriteWordSZResource WStrInstruction, #2, kTickerY+32
	; textDirectWriteSZResource StrSongListCaption, #12, #15
	jsr writeSongMenu
	jsr writeEmptyTrackTitle
	
	jsr transferBGMap

	ldx #2
	jsr configureSpriteForGame
	jsr spInitialize
	
	; show cursor - - - -
	ldx kCursorBaseX
	ldy kCursorBaseY
	jsr spMoveFirstSpriteDirect
	
	ldx kIndicatorBaseX
	ldy #-16
	jsr spMoveSecondSpriteDirect

	ldx #15
	jsr enableScreensWithBrightness
	
	restore_paxy
	rts
.endproc

; A16, I16
.proc writeSongMenu
.a16
.i16
	save_paxy
	stz gMenuItemsCount

	ldstx_long SongMetadataList
	ldy kMenuTextBaseY ; y pos

	begin_loop:
		phy
			ldy #0
			lda [gAssetOffsetTemp],y
		ply
		cmp #0
		beq exit_loop

		phx
			ldx #6  ; x pos
			jsr textDirectWriteSZ
		plx

		iny
		inx
		txa
		and #1
		bne no_skipline
			iny
			inc gMenuItemsCount
		no_skipline:

		; move base address
		lda gAssetOffsetTemp
		add kMetaTextStride
		sta gAssetOffsetTemp

		bra begin_loop
	exit_loop:

	restore_paxy
	rts
.endproc

; A16, I16
.proc writeEmptyTrackTitle
.a16
.i16
	save_paxy

	; Y <- gMenuItemsCount*3 + by
	lda gMenuItemsCount
	asl
	add gMenuItemsCount
	add kMenuTextBaseY
	tay
	
	lda kMaxTracks
	sub gMenuItemsCount
	bmi fend
	beq fend
	
	tax
	begin_loop:
		phx
		
			; row 1
			ldstx_long StrEmpty
			ldx #6
			jsr textDirectWriteSZ

			; row 2
			iny
			ldstx_long StrEmpty
			ldx #6
			jsr textDirectWriteSZ
			
			iny
			iny
			
		plx
		dex
		bne begin_loop
	
	
	
	fend:
	restore_paxy
	rts
.endproc


.proc transferBGMap
.a16
.i16
	save_paxy

	lda kMap1StartWSh
	sta pVWriteAddrW
	
	ldx #0
	ldy #1024
	begin_loop:
	
		lda f:BGMapAsset,x
		sta pVWriteValW
		inxinx
		
		dey
		bne begin_loop
	
	restore_paxy
	rts
.endproc


; A16, I16
.proc spcUpdateProgressHandler
.a16
.i16
	save_paxy

	; WAIT VBLANK
set_a8
.a8
:	lda $4212
	bit #$80
	beq :-
set_a16
.a16

	jsr msAdvanceSpinnerAnimation
	jsr msUpdateCursorSpriteTile

	restore_paxy
	rts
.endproc

; A16, I16
.proc msAdvanceSpinnerAnimation
.a16
.i16
	save_paxy

	inc gSpinnerFrameIdx

	lda gSpinnerFrameIdx
	cmp #12
	bcc no_frame_reset
		stz gSpinnerFrameIdx
	no_frame_reset:

	restore_paxy
	rts
.endproc

; A16, I16
.proc msUpdateCursorSpriteTile
.a16
.i16
	save_paxy
	ldx #0
	lda gLoaderBusy
	beq not_busy
		lda gSpinnerFrameIdx
		cmp #6
		bcs second_row
			asl ; 0-5 -> 0-10
		bra endif_row
		second_row:
			asl
			add #20 ; 6-11 -> 32-42
		endif_row:
		
		add #32
		tax
	not_busy:
	jsr spSetFirstSpriteTile
	
	restore_paxy
	rts
.endproc

; A16, I16
.proc processMainInput
.a16
.i16
	save_paxy
	; busy check
	lda gLoaderBusy
	bne endf
	
	lda gSelectAnimCount
	bne endf
	; -----------------
	
	lda gPadState
	and kPad16_UpDown
	sta gCurPadUD
	
	cmp gPrevPadUD
	beq no_chg

		cmp kPad16_Down
		bne not_down
			jsr moveCursorDown
		bra endif_updn
		not_down:
			cmp kPad16_Up
			bne endif_updn
				jsr moveCursorUp
		endif_updn:

	no_chg:
	
	lda gCurPadUD
	sta gPrevPadUD
	
	
	; TRIGGERS =================
	lda gPadState
	and kPad16_AB
	sta gCurPadTrg

	cmp gPrevPadTrg
	beq trg_no_chg
		jsr processInputTrigger
	trg_no_chg:

	ldsta gPrevPadTrg, gCurPadTrg

	
	endf:
	restore_paxy
	rts
.endproc

; A16, I16
.proc processSETriggers
.a16
.i16
	save_paxy

	; SE triggers
	lda gPadState
	and kPad16_TrgL
	cmp gPrevLPush
	beq no_chgL
		sta gPrevLPush
		bit kPad16_TrgL
		beq no_chgL
			lda #0
			jsr spcRequestSE1
	no_chgL:
	
	lda gPadState
	and kPad16_TrgR
	cmp gPrevRPush
	beq no_chgR
		sta gPrevRPush
		bit kPad16_TrgR
		beq no_chgR
			lda #1
			jsr spcRequestSE2
	no_chgR:

	restore_paxy
	rts
.endproc


; A16, I16
.proc processInputTrigger
.a16
.i16
	save_paxy
	
	cmp kPad16_TrgA
	bne not_Atrg
		jsr startSelectedSong
	bra endif_trg
	not_Atrg:
		cmp kPad16_TrgB
		bne endif_trg
			jsr stopSong
	endif_trg:
	
	restore_paxy
	rts
.endproc

; A16, I16
.proc stopSong
.a16
.i16
	save_paxy

	jsr msResetSPCDriver

	ldx kIndicatorBaseX
	ldy #-16
	jsr spMoveSecondSpriteDirect


	; Update ticker
	jsr tbClear
	ldsta gTextBufWPos,#10
	
	ldstx_long StrTitleDisp
	jsr tbAppendFromAssetSZ

	lda gTextBufWPos
	add #4
	sta gTextBufWPos

	ldstx_long WStrInstruction
	jsr tbAppendFromAssetWordSZ

	; Reset scroll and request tx
	inc gTextBufTxRequest
	ldsta gScrollCount,#-160
	
	restore_paxy
	rts
.endproc

; A16, I16
.proc startSelectedSong
.a16
.i16
	save_paxy
	jsr tbClear
	ldstx_long StrLoading
	jsr tbAppendFromAssetSZ
	inc gTextBufTxRequest
	stz gScrollCount

	ldsta gSelectedIndex, gCursorPosIndex

	jsr moveIndicatorSprite
	ldsta gSelectAnimCount,#7 ; start animation

	jsr msResetSPCDriver

	restore_paxy
	rts
.endproc


; A16, I16
.proc moveCursorDown
.a16
.i16
	save_paxy
	
	; if ((gCursorPosIndex+1) >= gMenuItemsCount)
	lda gCursorPosIndex
	inc
	cmp gMenuItemsCount
	bcs cant_move
	
		inc gCursorPosIndex
		ldsta gCursorAnimCount, #5
		ldsta gCursorAnimDir, #1
	
	cant_move:
	
	restore_paxy
	rts
.endproc

; A16, I16
.proc moveCursorUp
.a16
.i16
	save_paxy

	lda gCursorPosIndex
	beq cant_move
	
		dec gCursorPosIndex
		ldsta gCursorAnimCount, #5
		ldsta gCursorAnimDir, #-1

	cant_move:
	
	restore_paxy
	rts
.endproc

; A16, I16
.proc applyCursorOffsetX
.a16
.i16
	save_paxy
	
	lda gSelectAnimCount
	beq no_animation
	
	asl
	tax

	lda HAnimationTable,x
	sta gCursorXOffset

	dec gSelectAnimCount
	bne anim_continue
		; Last frame
		jsr msLoadSong
		jsr msUpdateTickerTextBuffer
	anim_continue:
	
	no_animation:
	
	restore_paxy
	rts
.endproc

; A16, I16
.proc applyCursorOffsetY
.a16
.i16
	save_paxy
	begin_lvar_1

	lda gCursorAnimCount
	asl
	tax
	lda VAnimationTable,x
	staw_v1

	
	lda gCursorAnimDir
	beq no_animation
	
	bmi dir_up
		lda #0
		sub_v1
	bra endif_dir
	dir_up:
		ldaw_v1
	endif_dir:
	
	sta gCursorYOffset
	
	; decrement counter
	lda gCursorAnimCount
	bne still_nz
		stz gCursorAnimDir
	bra endif_nz
	still_nz:
		dec gCursorAnimCount
	endif_nz:
	
	no_animation:
	
	end_lvar_1
	restore_paxy
	rts
.endproc

; A16, I16
.proc moveCursorSprite
.a16
.i16
	save_paxy
	jsr applyCursorOffsetX
	jsr applyCursorOffsetY
	
	; Y
	lda gCursorPosIndex
	jsr ldy_times24

	; X
	lda gCursorXOffset
	add kCursorBaseX
	tax
	jsr spMoveFirstSpriteDirect

	restore_paxy
	rts
.endproc

.macro m_times24
.endmacro

; A16, I16
.proc moveIndicatorSprite
.a16
.i16
	save_paxy
	begin_lvar_1

	lda gCursorPosIndex

	; *24
	left_shift_3
	staw_v1
	add_v1
	add_v1
	
	add kIndicatorBaseY
	tay
	
	ldx kIndicatorBaseX
	jsr spMoveSecondSpriteDirect

	end_lvar_1
	restore_paxy
	rts
.endproc


; A16, I16
; in A: in val
; out Y: result
.proc ldy_times24
.a16
.i16
	begin_lvar_1

	left_shift_3
	staw_v1
	
	add_v1
	add_v1

	add kCursorBaseY
	add gCursorYOffset

	tay
	end_lvar_1
	rts
.endproc


; A16, I16
.proc msResetSPCDriver
.a16
.i16
	save_paxy
	
	inc gSPCResetReq
	
	restore_paxy
	rts
.endproc

; A16, I16
.proc msLoadSong
.a16
.i16
	save_paxy
	
	inc gLoaderBusy
	inc gSPCLoadReq
	
	restore_paxy
	rts
.endproc

; A16, I16
.proc msUpdateTickerTextBuffer
.a16
.i16
	save_paxy
	begin_lvar_1
		
	ldstx_long SongMetadataList
	
	; Apply offset
	lda gAssetOffsetTemp
	staw_v1
	
	lda gSelectedIndex
	left_shift_6 ; *32*2
	add_v1
	sta gAssetOffsetTemp
	staw_v1
	
	; Append title
	jsr tbClear
	lda #$0C62
	jsr tbAppendWord
	jsr tbAppendFromAssetSZ

	; ' BY '
	ldstx_long StrBy
	jsr tbAppendFromAssetSZ
	ldstx_long SongMetadataList

	; Append author
	ldaw_v1
	add kMetaTextStride
	sta gAssetOffsetTemp
	jsr tbAppendFromAssetSZ

	inc gTextBufTxRequest
	ldsta gScrollCount,#-48
	
	
	; Append fixed message
	lda gTextBufWPos
	cmp #(46*2)
	bcs skip_footstr ; Skip if very long title/author name is set
		add #4
		sta gTextBufWPos
		
		ldstx_long WStrInstruction
		jsr tbAppendFromAssetWordSZ
	skip_footstr:
	
	end_lvar_1
	restore_paxy
	rts
.endproc

; A16, I16
.proc renewHDMATable
.a16
.i16
	save_paxy
	lda gScrollCount
	beq force_do
		lda gSelectAnimCount
		bne stop_scr
	force_do:
	
	; 0 , 1,2
	lda #80
	sta gHDMATable
	lda gScrollCount
	sta gHDMATable+1

	; 3 , 4,5
	lda #1
	sta gHDMATable+3
	stz gHDMATable+4

	; 6
	stz gHDMATable+6
	
	lda gScrollCount
	inc
	and #511
	sta gScrollCount ; gScrollCount = (gScrollCount+1) % 512
	
	stop_scr:
	restore_paxy
	rts
.endproc




; A16, I16
.proc startTextLayerHDMA
.a16
.i16
	save_paxy
	
	jsr renewHDMATable

	set_a8
	.a8
		; Set DMA target
		; write to $2111(BG3 HScroll)
		lda #$11
		sta $4311

		; Table bank
		lda #$00
		sta $4314
		
		; Table address
		ldy #gHDMATable
		sty $4312
		
		; byte-twice mode, normal HDMA
		lda #$02
		sta $4310

		; Enable HDMA Ch1 - - - - - - - - - - - - -
		lda #$02
		sta $420c

	set_a16
	.a16

	restore_paxy
	rts
.endproc

HAnimationTable:
	.word 0
	.word 0
	.word 1
	.word 1
	.word 2
	.word 3
	.word 4
	.word 2

VAnimationTable:
	.word 0
	.word -1
	.word -2
	.word 1
	.word 4
	.word 8
