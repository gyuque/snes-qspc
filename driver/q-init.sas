
scope initializeAll: {

	// Zero clear
	lda #0
	sta gCurCommandAdvHigh

	jsr initChannelStatusesShadow
	jsr initGlobalStatusesShadow
	jsr initDIRLocationShadow
	jsr initTrackStatusArea

	jsr transferDSPRegisterShadow
	jsr transferDSPSpecialRegisterShadow
	
	jsr initializeSetupBGMJumpTable
	jsr initializeTimer

	rts
}

scope initializeNotifyReady: {
	pha
		lda #0
		sta pIOPort1
	pla
	rts
}


scope initChannelStatusesShadow: {
	ldx #$00
	ldy #$08
	loop_start:

		// Channel vol.(L)
		lda #$3F
		sta gRegisterShadowBase,x

		// Channel vol.(R)
		inx
		sta gRegisterShadowBase,x

		// Pitch (L,H)
		inx
		lda #$00
		sta gRegisterShadowBase,x

		inx
		lda #$10
		sta gRegisterShadowBase,x

		// SRCN
		inx
		lda InstPresetTable
		sta gRegisterShadowBase,x

		// ADSR
		inx
		lda InstPresetTable+1
		sta gRegisterShadowBase,x

		inx
//		lda #%10101101
		lda InstPresetTable+2
		sta gRegisterShadowBase,x

		// GAIN
		inx
		lda #$00
		sta gRegisterShadowBase,x

		// ENVX/OUTX(dmy)
		inx
		sta gRegisterShadowBase,x
		inx
		sta gRegisterShadowBase,x

		txa
		clc
		adc #$04
		tax

		// Reset effects and filter
		lda #$00
		sta gRegisterShadowBase,x
		inx
		sta gRegisterShadowBase,x
		inx
		sta gRegisterShadowBase,x
		inx

		dey
		bne loop_start

	rts
}

scope initGlobalStatusesShadow: {

	lda #$3F
	// Master vol.(L/R)
	sta gRegisterShadowBase + 0x0C
	sta gRegisterShadowBase + 0x1C

	lda #$00
	// Echo vol.(L/R)
	sta gRegisterShadowBase + 0x2C
	sta gRegisterShadowBase + 0x3C

	lda #$00
	// Key on/off
	sta gRegisterShadowBase + 0x4C
	sta gRegisterShadowBase + 0x5C

	lda #$20
	// Flags (disable reset, disable mute, disable echo)
	sta gRegisterShadowBase + 0x6C

	lda #$00
	// ENDX(dmy)
	sta gRegisterShadowBase + 0x7C

	rts
}

scope initDIRLocationShadow: {
	lda #((ProgramOrigin + BRRDirOrigin) >> 8)
	sta gRegisterShadowBase + 0x5D

	rts
}

scope initTrackStatusArea: {
	ldx #0
	ldy #(16*8)
	lda #0
	begin_loop:
		sta gBGMTrackStatusesBase,x

		inx
		dey // counter
		bne begin_loop

	// Init pan factor
	ldx #0
	ldy #8 // 8 tracks
	begin_loop2:
		lda #kCenterVolumeFactor
		sta gBGMTrackStatusesBase+tofs_LPanFactor,x
		sta gBGMTrackStatusesBase+tofs_RPanFactor,x

		txa
		add(#16)
		tax
		dey // counter
		bne begin_loop2

	rts
}

// in A: BGM timer interval
scope initializeTimer: {

	// T0 interval: 0.125ms * 83 = 10.375ms
	// 48 ticks = 498ms (Tempo=120.48)

	// Configure Timer0
	lda HeaderTimerIntervalByte
	// lda #83 // <- テスト用T= 120相当
	sta $FA // <- T0 下位タイマのインターバル設定
	
	// SE用タイマー（速度固定）
	// T1 interval: 0.125ms * 80 = 10ms
	lda #80
	sta $FB // <- T1 下位タイマのインターバル設定


	// read upper counters(to reset)
	lda $FD
	lda $FE

	// Start Timer 0 and 1
	lda #$03
	ora #$80 // <- Enable IPL read
	sta pSPCControl


	rts
}

