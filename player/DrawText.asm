.setcpu "65816"
.autoimport on
.include "GlobalVarsAndConsts.inc"
.include "utility-macros.inc"
.import StrScoreDisp:far

.export textClearBG
.export textDirectWriteAChar
.export textDirectWriteDec2
.export textDirectWriteSZ
.export textDirectWriteWordSZ
.export textSplitDecimal2C

; .define kPriorityMask #$2000
.define kPriorityMask #$0000
.define kNumToChar    #$2010

; A16, I16
.proc textClearBG
.a16
.i16
	save_paxy

	lda kMap3StartWSh
	sta pVWriteAddrW

	lda #0 ; initial value

	ldy #(64*32)
	lp:
		sta pVWriteValW
	
		dey
		bne lp

	restore_paxy
	rts
.endproc

.macro ldv1_xypos
	; v1 <- Y * 32
	tya
	left_shift_5
	staw_v1

	; v1 <- x + v1
	txa
	add_v1
	staw_v1
.endmacro

; A16, I16
; in A: Letter Index(raw)
; in X: X start posirion
; in Y: Y start position
.proc textDirectWriteAChar
.a16
.i16
	save_paxy
	begin_lvar_2
	ldv1_xypos

	lda kMap3StartWSh
	add_v1
	sta pVWriteAddrW
	
	ldaw_v2
	ora kPriorityMask
	sta pVWriteValW

	end_lvar_2
	restore_paxy
	rts
.endproc

; A16, I16
; in A: Numeric(0-99)
; in X: X start posirion
; in Y: Y start position
.proc textDirectWriteDec2
.a16
.i16
	save_paxy
	begin_lvar_2

	jsr textSplitDecimal2C
	staw_v2
	
	ldv1_xypos
	lda kMap3StartWSh
	add_v1
	sta pVWriteAddrW

	; first letter
	ldaw_v2
	and #$00FF
	add #$10
	ora kPriorityMask
	sta pVWriteValW

	; second letter
	ldaw_v2
	xba
	and #$00FF
	add #$10
	ora kPriorityMask
	sta pVWriteValW

	
	end_lvar_2
	restore_paxy
	rts
.endproc

; A16, I16
; in X: X start posirion
; in Y: Y start position
; in mem: gAssetOffsetTemp
.proc textDirectWriteSZ
.a16
.i16
	save_paxy
	begin_lvar_1
	ldv1_xypos

	lda kMap3StartWSh
	add_v1
	sta pVWriteAddrW

	ldy #0
	begin_loop:
		lda [gAssetOffsetTemp],y
		and #$00FF
		beq break_lp
		sub #$20
		bpl nomin
			lda #0
		nomin:
		ora kPriorityMask

		sta pVWriteValW

		iny
		jmp begin_loop
	break_lp:

	end_lvar_1
	restore_paxy
	rts
.endproc

; A16, I16
; in X: X start posirion
; in Y: Y start position
; in mem: gAssetOffsetTemp
.proc textDirectWriteWordSZ
.a16
.i16
	save_paxy
	begin_lvar_1
	ldv1_xypos

	lda kMap3StartWSh
	add_v1
	sta pVWriteAddrW

	ldy #0
	begin_loop:
		lda [gAssetOffsetTemp],y
		beq break_lp
		sub #$20
		bpl nomin
			lda #0
		nomin:

		sta pVWriteValW

		iny
		iny
		jmp begin_loop
	break_lp:

	end_lvar_1
	restore_paxy
	rts
.endproc

; A16, I16
; in A: input value
; out A: Decimal digits(joined bytes)
.proc textSplitDecimal2C
.a16
.i16
	save_pxy
	cmp #100
	bcc limit_ok
		lda #$0909
		jmp fend
	limit_ok:

	sta pDivc_dividend
	ldsta pDivc_divisor,#10

	; 3 cycles * 5 + 2
	xba
	xba
	xba
	xba
	xba
	inc
	
	lda pDivc_outM ; 1
	xba
	ora pDivc_outQ ; 10

	fend:
	restore_pxy
	rts
.endproc

; A16, I16
; in A: input value
; out A: Decimal digits(joined bytes)
.proc textSplitDecimal2CQuick
.a16
.i16
	save_pxy
	cmp #20
	bcc limit_ok
		lda #$0109
		jmp fend
	limit_ok:

	cmp #10
	bcc und_10
		; A >= 10
		sub #10
		ora #$0100
	und_10:

	xba

	fend:
	restore_pxy
	rts
.endproc

