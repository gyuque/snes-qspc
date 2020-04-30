// ++ 初期化関連モジュール ++

// 全初期化ルーチン呼び出し
scope initializeAll: {
	save_axy()

	lda #0
	sta gGlobalTickCount
	sta gLvMeterCurTrack
	sta gLvMeterLeftSum
	sta gLvMeterLeftSum+1 // 16bit
	sta gLvMeterRightSum
	sta gLvMeterRightSum+1 // 16bit

	jsr initChannelStatusesShadow
	jsr initGlobalStatusesShadow
	jsr initDIRLocationShadow
	jsr initTrackStatusArea
	jsr initBGMSeqReferTable
	jsr initSEPort

	jsr init_setupCommandJumpTable
	jsr initTimer

	jsr transferDSPRegisterShadow
	jsr transferDSPSpecialRegisterShadow
	
	restore_axy()
	rts
}


// 各チャンネル用シャドウ初期化
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

// 全ch共通ステートのシャドウ初期化
scope initGlobalStatusesShadow: {
	lda #00
	sta gVelocityMinus

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

// トラック再生ステータス初期化
scope initTrackStatusArea: {
	ldx #0
	ldy #(16*8)
	lda #0
	begin_loop:
		sta gTrackStatusesBase,x

		inx
		dey // counter
		bne begin_loop

	// Init pan factor
	ldx #0
	ldy #8 // 8 tracks
	begin_loop2:
		lda #kCenterVolumeFactor
		sta gTrackStatusesBase+tofs_LPanFactor,x
		sta gTrackStatusesBase+tofs_RPanFactor,x

		txa
		add(#16)
		tax
		dey // counter
		bne begin_loop2

	rts
}

// DIRアドレス設定
scope initDIRLocationShadow: {
	lda #((ProgramOrigin + BRRDirOrigin) >> 8)
	sta gRegisterShadowBase + 0x5D

	rts
}

scope initializeNotifyReady: {
	pha
		lda #0
		sta pIOPort1
	pla
	rts
}


// in A: BGM timer interval
scope initTimer: {

	// T0 interval: 0.125ms * 83 = 10.375ms
	// 48 ticks = 498ms (Tempo=120.48)

	// Configure Timer0
	lda HeaderBGMTimerInterval
//	lda #83 // <- テスト用T= 120相当
//	lda #90 // <- テスト用
	sta $FA // <- T0 下位タイマのインターバル設定

	// SE interval: 0.125ms * 52 = 6.5ms
	// 48 ticks = 312ms (Tempo=192.3)
	lda #52 // <- SEテスト用
	sta $FB // <- T1 下位タイマのインターバル設定


	// read upper counters(to reset)
	lda $FD
	lda $FE

	// Start Timer 0,1
	lda #$03
	ora #$80 // <- Enable IPL read
	sta pSPCControl


	rts
}

scope initBGMSeqReferTable: {
	ldx #0
	ldy HeaderNumTracks
	
	lp:
		lda SeqDirTable,x
		sta SeqOffsetTable,x
		inx
		lda SeqDirTable,x
		sta SeqOffsetTable,x
		inx
		// -----------------
		dey
		bne lp
	
	rts
}

scope initSEPort: {
	lda pIOPort2
	sta gSE1PrevVal

	lda pIOPort3
	sta gSE2PrevVal

	rts
}

