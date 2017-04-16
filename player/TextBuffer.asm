.setcpu "65816"
.autoimport on
.include "GlobalVarsAndConsts.inc"
.include "utility-macros.inc"

.define kTextBufferAllWordCount #64
.define kTextBufferWordCount #32

.export tbResetWPos
.export tbClear
.export tbAppendWord
.export tbAppendFromAssetSZ
.export tbAppendFromAssetWordSZ
.export tbTransferDMA

; A16, I16
.proc tbResetWPos
.a16
.i16
	stz gTextBufWPos
	rts
.endproc


; A16, I16
.proc tbClear
.a16
.i16
	save_paxy
	stz gTextBufWPos
	stz gTextBufTxRequest
	
	lda #0
	ldx #0
	ldy kTextBufferAllWordCount
	begin_loop:
		sta gTextTempBuf,x
		
		inxinx
		dey
		bne begin_loop
	
	restore_paxy
	rts
.endproc

; A16, I16
; in A: value
.proc tbAppendWord
.a16
.i16
	save_paxy
	ldx gTextBufWPos
	sta gTextTempBuf,x
	
	inxinx
	stx gTextBufWPos

	restore_paxy
	rts
.endproc

; A16, I16
; in mem: gAssetOffsetTemp/gAssetBankTemp
.proc tbAppendFromAssetSZ
.a16
.i16
	save_paxy
	ldx gTextBufWPos

	ldy #0
	begin_loop:
		lda [gAssetOffsetTemp],y
		and #$00FF
		beq break_lp
		sub #$20
		bpl nomin
			lda #0
		nomin:

		sta gTextTempBuf,x

		inxinx
		iny
		bra begin_loop
	break_lp:

	stx gTextBufWPos

	restore_paxy
	rts
.endproc

; A16, I16
; in mem: gAssetOffsetTemp/gAssetBankTemp
.proc tbAppendFromAssetWordSZ
.a16
.i16
	save_paxy
	ldx gTextBufWPos

	ldy #0
	begin_loop:
		lda [gAssetOffsetTemp],y
		beq break_lp
		sub #$20
		bpl nomin
			lda #0
		nomin:

		sta gTextTempBuf,x

		inxinx
		iny
		iny
		bra begin_loop
	break_lp:

	stx gTextBufWPos

	restore_paxy
	rts
.endproc

; A16, I16
; in X: dest offset(words)
; in Y: not 0 -> second page
.proc tbTransferDMA
.a16
.i16
	save_paxy
	lda gTextBufTxRequest
	beq no_req

	; <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
	; BG DMA

	; Set length(bytes)
	lda kTextBufferWordCount*2
	sta pDMA0ByteCountW

	; Set VRAM write address
	txa
	add kMap3StartWSh

	cpy #0
	bne second_page
		sta $2116

		; Set src origin address
		ldy #gTextTempBuf
	bra endif_page
	second_page:
		add #(32*32) ; move to next page
		sta $2116

		; Set src origin address
		ldy #gTextTempBuf+64
	endif_page:

set_a8
.a8
	; Set DMA source address
	stz pDMA0SourceBank    ; bank (8bit)
	sty pDMA0SourceOffsetW ; offset(16bit)


	; Set DMA target
	; write to $2118(VRAM)
	lda #$18 ; $2100 + 18
	sta pDMA0Dest

	; ** WORD mode **
	lda #$01
	sta pDMA0Config

	; Start - - - - - - - - - - - - -
	lda #$01 ; Ch 0
	sta pDMATrigger
set_a16
.a16

	; <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

	no_req:
	restore_paxy
	rts
.endproc


