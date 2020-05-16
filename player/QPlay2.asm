.setcpu     "65816"
.autoimport on
.include "utility-macros.inc"
.include "GlobalVarsAndConsts.inc"

.segment "STARTUP"
; Reset int. (Entry Point) -------------------
.proc Reset ; int handler
	sei
	clc
	xce ; Native Mode On

	phk
	plb ; PB -stack-> DB (=0)

	rep #$30 ; A,I 16bit
	.a16
	.i16

	; Initialize stack
	ldx #$1fff
	txs
	
	jsr initRegs
	jsr initSystemGenericStatuses
	jsr initVideoForGame
	jsr disableNMI

	; Read QL header
	lda f:SongMetadataHeader+2
	sta gQuickLoadSize
	
	; Load initial sound
	stz gSelectedIndex
	; jsr selectSoundAssetAddress
	; jsr spcSetup
	
	jsr setupMainScreen
	jsr enableNMI
	
	ldsta $40,#$3412
	
	begin_loop:
		lda gSPCResetReq
		beq no_spc_reset
			jsr spcRequestReset
			stz gSPCResetReq
		no_spc_reset:
	
		lda gSPCLoadReq
		beq skip_spc_load
			jsr disableNMI
			jsr selectSoundAssetAddress
			
			jsr shouldDoQuickLoad
			bcs qmode
				jsr spcSetup
			jmp endif_q
			qmode:
				jsr spcQuickReload
			endif_q:
			
			stz gSPCLoadReq
			stz gLoaderBusy
			jsr spcUpdateProgressHandler
			jsr enableNMI
			
			ldsta gQuickLoadOK,#$FF
		skip_spc_load:
	
		jsr processSETriggers
	
		nop
		jmp begin_loop

	rti
.endproc

.proc VBlank ; int handler
	save_pax

	set_a16
	.a16
	.i16
		
	ldx kTickerY*32
	ldy #0
	jsr tbTransferDMA

	ldx kTickerY*32
	ldy #1
	jsr tbTransferDMA
	
	jsr startTextLayerHDMA
	jsr moveCursorSprite
	jsr processMainInput
	jsr updatePadState

	; out_scanpos gBenchmarkOut2

	restore_pax
	rti
.endproc

.proc updatePadState

	set_a8
	.a8 

	; Wait until auto-read is finished
:	lda $4212
	and #$01
	bne :-

	set_a16
	.a16

	; Store Pad Status
	lda $4218
	sta gPadState

	rts
.endproc

.macro set_sndimage_pair part1, part2
	ldsta gBlobSrcAddrLo, #.LOWORD(part1)
	ldsta gBlobSrcAddrHi, #.HIWORD(part1)
		
	ldsta gBlobSrcAddrLoNext, #.LOWORD(part2)
	ldsta gBlobSrcAddrHiNext, #.HIWORD(part2)
.endmacro

; A16, I16
.proc shouldDoQuickLoad
.a16
.i16
	pha

	lda f:SongMetadataHeader
	and gQuickLoadOK
	bne return_yes
	
return_no:
	pla
	clc
	rts
return_yes:
	pla
	sec
	rts
.endproc

; A16, I16
.proc selectSoundAssetAddress
.a16
.i16
	pha
	
	lda gSelectedIndex
	cmp #3
	bne not_4th
		set_sndimage_pair SndDrv4LoImageBase, SndDrv4HiImageBase
		bra end_sw
	not_4th:
	
	cmp #1
	bcc case_0
	beq case_1
		set_sndimage_pair SndDrv3LoImageBase, SndDrv3HiImageBase
	bra end_sw
	case_1:
		set_sndimage_pair SndDrv2LoImageBase, SndDrv2HiImageBase
	bra end_sw
	case_0:
		set_sndimage_pair SndDrvImageBase, SndDrvImageBaseHigh
	end_sw:
	
	pla
	rts
.endproc

; ===================================================
; Metadata and Assets
; ===================================================

; Cartridge metadata - - -
.segment "CARTINFO"
	;        ||||++++||||++++||||+ 21 bytes
	.byte	"QSPC2PLAYER          "	; Game Title
	.byte	$00				; 0x01:HiRom, 0x30:FastRom(3.57MHz)
	.byte	$00				; ROM only
	.byte	$09				; ROM Size
	.byte	$00				; RAM Size (8KByte * N)
	.byte	$00				; NTSC
	.byte	$01				; Licensee
	.byte	$00				; Version

	; Embed information (must be replaced with checksum)
	.word   $1C1C
	.word   $E3E3

	.byte	$ff, $ff, $ff, $ff		; unknown

	.word	$0000       ; Native:COP
	.word	$0000       ; Native:BRK
	.word	$0000       ; Native:ABORT
	.word	VBlank		; Native:NMI
	.word	$0000		; 
	.word	$0000       ; Native:IRQ

	.word	$0000	; 
	.word	$0000	; 

	.word	$0000       ; Emulation:COP
	.word	$0000       ; 
	.word	$0000       ; Emulation:ABORT
	.word	$0000       ; Emulation:NMI
	.word	Reset		; Emulation:RESET
	.word	$0000       ; Emulation:IRQ/BRK





; assets -----------------------------------------------------
.export SndDrvImageBase:far
.export SndDrvImageBaseHigh:far
.export AssetBGGraphic:far
.export AssetSpGraphic:far
.export AssetFont:far
.export PaletteBase:far
.export SongMetadataList:far
.export BGMapAsset:far
.export StrTitleDisp:far
.export StrSingleMode:far
.export StrSongListCaption:far
.export StrBy:far
.export StrLoading:far
.export StrEmpty:far
.export WStrInstruction:far

; - - -
.segment "VISASSET1":far
AssetBGGraphic:
	.incbin "assets/bg-ptn.bin"
AssetSpGraphic:
	.incbin "assets/sp-ptn.bin"
	
AssetFont:
	.incbin "assets/font-ptn.bin"

.byte $FF,$FF,$4D ; adjust checksum

.segment "VISASSET2": far
PaletteBase:
	.incbin "assets/main.pal"
	.incbin "assets/sub.pal"
	.incbin "assets/sprite.pal"

.segment "MISCASSET": far
SongMetadataHeader:
	; header
	.byte $00,$00,$00,$00, $00,$00,$00,$00
	.byte $00,$00,$00,$00, $00,$00,$00,$00

SongMetadataList:
	;      0123456789ABCDEF
	.byte "Dummy 01"
	.byte $00,$00,$00,$00, $00,$00,$00,$00
	.byte $00,$00,$00,$00, $00,$00,$00,$00
	.byte $00,$00,$00,$00, $00,$00,$00,$00

	;      0123456789ABCDEF
	.byte "Yuji Ninomiya"
	.byte                      $00,$00,$00
	.byte $00,$00,$00,$00, $00,$00,$00,$00
	.byte $00,$00,$00,$00, $00,$00,$00,$00

	;      0123456789ABCDEF
	.byte "Dummy 02"
	.byte $00,$00,$00,$00, $00,$00,$00,$00
	.byte $00,$00,$00,$00, $00,$00,$00,$00
	.byte $00,$00,$00,$00, $00,$00,$00,$00

	;      0123456789ABCDEF
	.byte "Yuji Ninomiya"
	.byte                      $00,$00,$00
	.byte $00,$00,$00,$00, $00,$00,$00,$00
	.byte $00,$00,$00,$00, $00,$00,$00,$00

	;      0123456789ABCDEF
	.byte "Dummy 03"
	.byte $00,$00,$00,$00, $00,$00,$00,$00
	.byte $00,$00,$00,$00, $00,$00,$00,$00
	.byte $00,$00,$00,$00, $00,$00,$00,$00

	;      0123456789ABCDEF
	.byte "Yuji Ninomiya"
	.byte                      $00,$00,$00
	.byte $00,$00,$00,$00, $00,$00,$00,$00
	.byte $00,$00,$00,$00, $00,$00,$00,$00

	;      0123456789ABCDEF
	.byte $00,$00 ; DISABLED
	.byte "rrying You"
	.byte                  $00,$00,$00,$00
	.byte $00,$00,$00,$00, $00,$00,$00,$00
	.byte $00,$00,$00,$00, $00,$00,$00,$00

	;      0123456789ABCDEF
	.byte $00,$00 ; DISABLED
	.byte "xxxxxxxxxx"
	.byte                  $00,$00,$00,$00
	.byte $00,$00,$00,$00, $00,$00,$00,$00
	.byte $00,$00,$00,$00, $00,$00,$00,$00

	; terminate
	.byte $00,$00,$00,$00, $00,$00,$00,$00
	.byte $00,$00,$00,$00, $00,$00,$00,$00
	.byte $00,$00,$00,$00, $00,$00,$00,$00
	.byte $00,$00,$00,$00, $00,$00,$00,$00

	.byte $00,$00,$00,$00, $00,$00,$00,$00
	.byte $00,$00,$00,$00, $00,$00,$00,$00
	.byte $00,$00,$00,$00, $00,$00,$00,$00
	.byte $00,$00,$00,$00, $00,$00,$00,$00

BGMapAsset:
	.incbin "assets/q-bgmap.bin"

; STRING ASSETS
StrTitleDisp:
	.byte "    qSPC Player Version 2.1"
	.byte $00
StrSingleMode:
	.byte "SINGLE  MODE"
	.byte $00
StrSongListCaption:
	.byte "SONG LIST"
	.byte $00
StrBy:
	.byte " by "
	.byte $00
StrLoading:
	.byte "            Loading... "
	.byte $00
StrEmpty:
	.byte "- - - - - - - -"
	.byte $00
WStrInstruction:
	.word $0080,$007F,$0050,$006C,$0061,$0079
	.word $0020
	.word $0020
	.word $1081,$007F,$0053,$0074,$006F,$0070
	.word $0000

	;      ||||----||||----||||----
	.byte "_EMBED_DRIVER_AFTER_HERE"
	
.segment "SDRVROM1": far
SndDrvImageBase:
	.incbin "assets/demo-lo.bin"

.segment "SDRVROM2": far
SndDrvImageBaseHigh:
	.incbin "assets/demo-hi.bin"

.segment "SDRVROM3": far
SndDrv2LoImageBase:
	.incbin "assets/demo-lo.bin"

.segment "SDRVROM4": far
SndDrv2HiImageBase:
	.incbin "assets/demo-hi.bin"


.segment "SDRVROM5": far
SndDrv3LoImageBase:
;	.incbin "assets/demo-lo.bin"
	.incbin "../n-driver/split_drv_00" ;TEST

.segment "SDRVROM6": far
SndDrv3HiImageBase:
;	.incbin "assets/demo-hi.bin"
	.incbin "../n-driver/split_drv_01" ;TEST

; - - - -
.segment "SDRVROM7": far
SndDrv4LoImageBase:
	.incbin "assets/demo-lo.bin"

.segment "SDRVROM8": far
SndDrv4HiImageBase:
	.incbin "assets/demo-hi.bin"
; - - - -

.segment "DONTUSE"
	.byte "NO-DATA"
