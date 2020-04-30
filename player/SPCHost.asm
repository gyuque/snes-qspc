.setcpu     "65816"
.autoimport on
.include "GlobalVarsAndConsts.inc"
.include "SoundPorts.inc"
.include "utility-macros.inc"

; ----------------------------------------------------

.export spcSetup
.export spcQuickReload
.export spcRequestSE1
.export spcRequestSE2
.export spcRequestReset


.define kDrvSendSize #128

; A16, I16
; in mem: gBlobSrcAddrLo/Hi, gBlobSrcAddrLoNext/HiNext
.proc spcSetup
.a16
.i16
	save_paxy

	jsr spcInternalInit

	; send driver image (Lo part)
	lda kDriverLoImageSize
	sta gBlobSizeToSend
	; *** gBlobSrcAddrLo/Hi MUST BE SET ***
	ldx #0 ; start from origin
	jsr spcSendDriverBlob

	; send driver image (High part)
	lda kDriverHiImageSize
	sta gBlobSizeToSend
	; *** gBlobSrcAddrLoNext/HiNext MUST BE SET ***
	ldsta gBlobSrcAddrLo, gBlobSrcAddrLoNext
	ldsta gBlobSrcAddrHi, gBlobSrcAddrHiNext

	ldx kDriverLoImageSize ; start from the tail of lo-part
	jsr spcSendDriverBlob

	; launch driver code
	jsr spcLaunchDriver
	jsr spcWaitDriverReady

	restore_paxy
	rts
.endproc

; A16, I16
; DON'T call before spcSetup
.proc spcQuickReload
.a16
.i16
	save_paxy

	jsr spcInternalInit

	; send driver image (Lo part, quick)
	lda gQuickLoadSize
	sta gBlobSizeToSend
	; *** gBlobSrcAddrLo/Hi MUST BE SET ***
	ldx #0 ; start from origin
	jsr spcSendDriverBlob

	; launch driver code
	jsr spcLaunchDriver
	jsr spcWaitDriverReady

	restore_paxy
	rts
.endproc

; A16, I16
.proc spcInternalInit
.a16
.i16
	save_paxy

	; Initialize SE seq. count
	lda #$06 ; <- avoid initial value of the port
	sta gSE1SeqCount
	sta gSE2SeqCount
	
	; Initialize counter
	lda kSPCCounterInitiatorVal
	sta gSPCSendCounter

	jsr spcWaitReady
	
	restore_paxy
	rts
.endproc



; A16, I16
; in mem gBlobSizeToSend: size to send
; in mem gBlobSrcAddrLo: source address (lower byte)
; in mem gBlobSrcAddrHi: source address (higher byte)
; in X: Dest offset
.proc spcSendDriverBlob
.a16
.i16
	save_paxy
	begin_lvar_2
	stz gSPCBlockCounter

	; var2 <- X
	txa
	staw_v2

	; var1 <- 0 (sent size)
	lda #0
	staw_v1

	loop_start:
		inc gSPCBlockCounter
		lda gSPCBlockCounter
		bit #$000F
		bne no_progress
			jsr spcUpdateProgressHandler
		no_progress:

		; - - - - - - - - - -
		; Y <- block size
		ldy kDrvSendSize

		; X <- base(ARAM) + sent size + specified offset
		lda kSPCDriverEntryAddr
		add_v1
		addw_v2
		tax

		; A <- base(src) + sent size
		lda gBlobSrcAddrLo ; offset
		add_v1
		
		; Y = size
		; X = dest
		; A = src
		jsr spcSendBlock
		; - - - - - - - - - -
		
		; var1 += block size
		ldaw_v1
		add kDrvSendSize
		staw_v1

		cmp gBlobSizeToSend
		bcc loop_start
		
	end_lvar_2
	restore_paxy
	rts
.endproc


; in A: wait until this value is out
.macro mWaitSPC0Val
:	nop
	cmp pSPC0
	bne :-
.endmacro

; A16, I16
.proc spcWaitReady
.a16
.i16
	save_paxy

set_a8
.a8

	lda kSPCReadyCode
	mWaitSPC0Val

set_a16
.a16

	restore_paxy
	rts
.endproc

; A16, I16
; in A: Source offset(16bit)
; in X: SPC dest address
; in Y: Block size
.proc spcSendBlock
.a16
.i16
	save_paxy

	; gTxIndirectTemp(long) <- Driver image source address

	sta gTxIndirectTemp

	lda gBlobSrcAddrHi ; bank
	sta gTxIndirectTemp+2

set_a8
.a8

	; set addr
	stx pSPC2

	; X <- size to send
	; Y <- 0
	tyx
	ldy #0

	; 1=Write, 0=Launch
	lda #$01
	sta pSPC1

	; Send initiator(1st block) or previous block terminator(otherwise)
	lda gSPCSendCounter
	sta pSPC0
	mWaitSPC0Val

	; Reset counter
	stz gSPCSendCounter
	
	; send body
transfer_loop_start:

	; - - - - - - SEND - - - - - -
	; Port0 <- counter
	; Port1 <- data
	
	lda gSPCSendCounter
	sta pSPC0

	lda [gTxIndirectTemp],y
	sta pSPC1
	
	lda gSPCSendCounter
	mWaitSPC0Val

	inc gSPCSendCounter
	iny
	dex
	bne transfer_loop_start

	; Make 'terminator' value
	lda gSPCSendCounter
	add #2
	bne no_again
		add #3
	no_again:
	sta gSPCSendCounter

set_a16
.a16

	restore_paxy
	rts
.endproc


; A16, I16
.proc spcLaunchDriver
.a16
.i16
	save_paxy


	; Set launch address
	lda kSPCDriverEntryAddr
set_a8
.a8
	sta pSPC2 ; LO
	xba
	sta pSPC3 ; HI
	
	; Finish IPL
	lda gSPCSendCounter
	sta pSPC0
	stz pSPC1
set_a16
.a16

	restore_paxy
	rts
.endproc


.proc spcWaitDriverReady
.a16
.i16
	save_paxy

set_a8
.a8
	wait_loop:
		lda pSPC1
		bne wait_loop

;	stz pSPC2 ;
;	stz pSPC3 ;
set_a16
.a16
		
	restore_paxy
	rts
.endproc

.define kCounterMask #$0007

; TRIGGER COUNTER = 3bits
.macro increment_sendcounter counter
	; counter = (counter+1) & 0x0007
	lda counter
	inc
	and kCounterMask
	sta counter
.endmacro

; in A: voice index
.proc spcRequestSE1
.a16
.i16
	save_paxy
	begin_lvar_1 ; v1 <- A

	increment_sendcounter gSE1SeqCount
	left_shift_5
	ora_v1
	
set_a8
.a8
	sta pSPC2
set_a16
.a16

	end_lvar_1
	restore_paxy
	rts
.endproc

; in A: voice index
.proc spcRequestSE2
.a16
.i16
	save_paxy
	begin_lvar_1 ; v1 <- A

	increment_sendcounter gSE2SeqCount
	left_shift_5
	ora_v1
	
set_a8
.a8
	sta pSPC3
set_a16
.a16

	end_lvar_1
	restore_paxy
	rts
.endproc



.proc spcRequestReset
.a16
.i16
	save_paxy
set_a8
.a8
	lda #$FF
	sta pSPC3
set_a16
.a16
	restore_paxy
	rts
.endproc



