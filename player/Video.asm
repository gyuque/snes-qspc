.setcpu "65816"
.autoimport on
.include "utility-macros.inc"
.include "GlobalVarsAndConsts.inc"

.import PaletteBase:far

.export initSystemGenericStatuses
.export initVideoForGame
.export configureSpriteForGame
.export disableScreen
.export enableNMI
.export disableNMI
.export enableScreensWithBrightness
.export transferBitmapDataDMA
.export transferPaletteData
.export spInitialize
.export spMoveFirstSpriteDirect
.export spSetFirstSpriteTile
.export spMoveSecondSpriteDirect

.export testFillBG1

; A16, I16
.proc initSystemGenericStatuses
.a16
.i16
	save_paxy

	set_a8
	.a8
		; Disable old-style pad input
		stz $4016
	set_a16
	.a16

	;jsr resetSpriteFirstTable
	;jsr resetSpriteSecondTable

	restore_paxy
	rts
.endproc

; A16, I16
.proc initVideoForGame
.a16
.i16
	save_paxy

	; BG configuration
set_a8
.a8
	; Set BG tile size(Upper 4bits [BG4321])
	; Specify BGMode  MODE1={16/16/4/0} colors
	lda #$39 ; order change bit enabled
	sta $2105 ; <- SSSSPMMM (chip Size, Priority change, screen Mode)


	; BG1 Size = 32x32 tiles
	;     Map start = +6800W
	; * Tile area = 2048(0x800h) bytes
	lda kMap1StartWHi
	sta $2107 ; <- AAAAAASS

	; BG2 Size = 32x32 tiles
	;     Map start = +7800W
	; * Tile area = 2048(0x800h) bytes
	lda kMap2StartWHi
	sta $2108

	; BG3 Size = 64x32 tiles
	;     Map start = +7000W
	; * Tile area = 2048(0x800h) bytes
	lda kMap3StartWHi
	ora #$01 ; cols=64
	sta $2109


	; sta $2108 ; BG2 map/size

	; BG 1/2 Pattern Source Start(0)
	stz $210b

	; BG 3 Pattern Source Start(5*0x2000 B)
	lda #$06
	sta $210c

set_a16
.a16

	restore_paxy
	rts
.endproc


; A16, I16
.proc disableScreen
.a16
.i16
	php
	pha

set_a8
.a8
	lda #$80
	sta $2100
set_a16
.a16

	pla
	plp
	rts
.endproc


; A16, I16
; in X: brightness(1-15)
.proc enableScreensWithBrightness
.a16
.i16
	save_paxy

set_a8
.a8
	; flags = [X][X][X][SP] [BG4][BG3][BG2][BG1]
	lda #%00010101 ; SP+BG3+BG1
;	lda #%00010000 ; SP
	sta $212c ; Main screen
	stz $212d ; Sub screen (0=all off)

	; brightness
	txa
	and #$0f
	sta $2100

set_a16
.a16
	
	restore_paxy
	rts
.endproc


; A16, I16
.proc enableNMI
.a16
.i16
	pha

	set_a8
	.a8
		lda #$81 ; bit 0 is always on (Joy pad auto read)
		sta $4200
	set_a16
	.a16

	pla
	rts
.endproc

; A16, I16
.proc disableNMI
.a16
.i16
	pha

	set_a8
	.a8
		lda #$01
		sta $4200
	set_a16
	.a16

	pla
	rts
.endproc


; A16, I16
; in X: Palette select
; in Y: Dest offset (in words)
; Note: each palette entry is 16bit(1W/2B)
.proc transferPaletteData
.a16
.i16
	save_paxy

	; X <- X*32
	txa
	left_shift_5
	tax

	set_a8
	.a8

	; Set write position
	tya
	sta $2121

	ldy #32 ; Loop 32 times (2*16)

:	lda f:PaletteBase, x
	sta $2122

	inx
	dey
	bne :-

	set_a16
	.a16

	restore_paxy
	rts
.endproc



; A16, I16
; in X: Dest address
; in Y: Size (in BYTES)
.proc transferBitmapDataDMA
.a16
.i16
	save_paxy

	; Set dest VRAM address
	stx $2116

	; Set length(bytes)
	sty $4305

	; load address(long)
	lda gAssetBankTemp
	ldy gAssetOffsetTemp

	set_a8
	.a8
		; Set DMA source address
		sta $4304 ; bank (8bit)
		sty $4302 ; offset(16bit)

		; Set DMA target
		; write to $2118(VRAM channel)
		lda #$18
		sta $4301

		; Write a word
		lda #$01
		sta $4300

		; Start - - - - - - - - - - - - -
		lda #$01
		sta $420b

	set_a16
	.a16

	restore_paxy
	rts
.endproc

; A16, I16
; in A: Fill Value
.proc testFillBG1
.a16
.i16
	save_paxy
	tax

	lda kMap1StartWSh
	sta pVWriteAddrW

	ldy #(32*32)
	loop_start:
		
		stx pVWriteValW
		
		dey
		bne loop_start
	
	restore_paxy
	rts
.endproc

; Sprites ---------------------------------------


; A16, I16
.proc spInitialize
.a16
.i16
	save_paxy
	
	jsr spClearDirect
	jsr spSetSpriteTiles

	restore_paxy
	rts
.endproc

; A16, I16
; in X: Name selection
.proc configureSpriteForGame
.a16
.i16
	save_paxy
	txa
	begin_lvar_1 ; var1 <- A <- X

	; name | offset
	; -----+-------
	;    0 |  0000W
	;    1 |  2000W
	;    2 |  4000W

	; size %011 = small:16x16  large:32x32

	;     sssbbnnn (sss=size  bb=base  nnn=name)
	lda #%01100000
	ora 1,s

set_a8
.a8
	sta pSpriteConf
set_a16
.a16

	end_lvar_1
	restore_paxy
	rts
.endproc

; A16, I16
.proc spClearDirect
.a16
.i16
	save_paxy

	stz pOAMWAddress

set_a8
.a8

	ldy #128
	loop1:
		stz pOAMWrite ; set x

		lda #$E0  ; y=above the top
		sta pOAMWrite ; set y
		stz pOAMWrite ; tile
		stz pOAMWrite ; others

		dey
		bne loop1

set_a16
.a16

	; Second Table
	lda #$0100
	sta pOAMWAddress

set_a8
.a8

	ldy #32
	loop2:
		stz pOAMWrite

		dey
		bne loop2
	
set_a16
.a16


	restore_paxy
	rts
.endproc

; A16, I16
; in X: index
.proc spSetFirstSpriteTile
.a16
.i16
	save_paxy

	; set tile
	ldsta pOAMWAddress,#1

	txa
set_a8
.a8
	sta pOAMWrite
	ldsta pOAMWrite,#0
set_a16
.a16

	restore_paxy
	rts
.endproc


; A16, I16
.proc spSetSpriteTiles
.a16
.i16
	save_paxy

	; set tile
	ldsta pOAMWAddress,#3

set_a8
.a8
	ldsta pOAMWrite,#2
	ldsta pOAMWrite,#0
set_a16
.a16

	restore_paxy
	rts
.endproc


; A16, I16
; in X: x position
; in Y: y position
.proc spMoveFirstSpriteDirect
.a16
.i16
	save_paxy

	stz pOAMWAddress

set_a8
.a8

	txa
	sta pOAMWrite ; set x
	tya
	sta pOAMWrite ; set y

set_a16
.a16

	restore_paxy
	rts
.endproc


; A16, I16
; in X: x position
; in Y: y position
.proc spMoveSecondSpriteDirect
.a16
.i16
	save_paxy

	ldsta pOAMWAddress, #2

set_a8
.a8

	txa
	sta pOAMWrite ; set x
	tya
	sta pOAMWrite ; set y

set_a16
.a16

	restore_paxy
	rts
.endproc
