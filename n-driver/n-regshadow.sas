scope transferDSPRegisterShadow: {
	ldy #$08
	ldx #$00

	// �e�`�����l���̃X�e�[�^�X�]��
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


	// ���ʃX�e�[�^�X�]��
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


// ����X�e�[�^�X($0D-$7D)�̓]�� ��ɃG�R�[�֌W
// ������DIR�͊�{�I�ȋ@�\�ɂ��K�{
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


// 1tick���ƂɃ��Z�b�g
scope resetOneShotRegistersShadow: {
	lda #0
	sta gRegisterShadowBase + 0x4C // Reset KEYON(All Ch.)
	sta gRegisterShadowBase + 0x5C // Reset KEYOFF(All Ch.)

	rts
}

