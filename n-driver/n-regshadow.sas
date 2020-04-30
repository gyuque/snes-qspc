scope transferDSPRegisterShadow: {
	ldy #$08
	ldx #$00

	// 各チャンネルのステータス転送
	// Channel status registers
	ch_loop:
		// Poke DSP register
		phx
				// in X: DSP port address
			     dsp_transfer_x()  // 0 Vol.L
			inx; dsp_transfer_x()  // 1 Vol.R
			inx; dsp_transfer_x()  // 2 Pch.L
			inx; dsp_transfer_x()  // 3 Pch.R
			inx; dsp_transfer_x()  // 4 SRCN
			inx; dsp_transfer_x()  // 5 ADSR1
			inx; dsp_transfer_x()  // 6 ADSR2
			inx; dsp_transfer_x()  // 7 GAIN
		plx

		// Next
		txa
		clc
		adc #$10
		tax

		dey
		bne ch_loop


	// 共通ステータス転送
	// Global status registers
	ldy #$07
	ldx #$0C
	gloop:
		// Poke DSP register
		stx pPokeAddr

		lda gRegisterShadowBase,x
		sta pPokeVal

		// Next
		txa
		clc
		adc #$10
		tax

		dey
		bne gloop

	rts
}


// 特殊ステータス($0D-$7D)の転送 主にエコー関係
// ただしDIRは基本的な機能にも必須
scope transferDSPSpecialRegisterShadow: {
	ldy #$08
	ldx #$0D
	loop:
		dsp_transfer_x()

		// Next
		txa
		clc
		adc #$10
		tax

		dey
		bne loop

	rts
}


// 1tickごとにリセット
scope resetOneShotRegistersShadow: {
	lda #0
	sta gRegisterShadowBase + 0x4C // Reset KEYON(All Ch.)
	sta gRegisterShadowBase + 0x5C // Reset KEYOFF(All Ch.)

	rts
}

