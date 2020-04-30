constant kSE1Ch(6)
constant kSE2Ch(7)

// ����R�}���h:
//  31/0x1F(SE1) -> BGM�|�[�Y����
//  31/0x1F(SE2) -> SPC���Z�b�g
//  14/0x0E ------> BGM��~�A������SE�����ƕ��p��

macro impl_se_proc(prefix, portIndex) {

	save_ax()

	lda pIOPort{portIndex}
	cmp g{prefix}PrevVal
	beq fend
	// ---------------------------------
	// �|�[�g�l�ۑ�
	sta g{prefix}PrevVal

	// X <- Table offset
	and #$1F
	// ��ʃr�b�g�̓g���K�[�p�J�E���^�Ȃ̂Ő؂�̂�
	// ����5�r�b�g��SE�ԍ��Ƃ��Ďc��
	
	cmp #$1F // SPECIAL(31)
	bne not_special
		jsr on{prefix}SpecialCommand
		bra fend
	not_special:
	
	cmp #$0E // SPECIAL(14)
	bne not_pausesp
		jsr stopBGM
	not_pausesp:
	
	asl
	tax

	// �V�[�P���X�ʒu�Z�b�g
	lda SeqDirSEPart,x
	sta SeqOffsetTable+(k{prefix}Ch*2   )

	lda SeqDirSEPart+1,x
	sta SeqOffsetTable+(k{prefix}Ch*2 +1)
	
	// �X�e�[�g���Z�b�g
	lda #0
	sta gTrackStatusesBase + (k{prefix}Ch*16 + tofs_PlayHeadPos)
	sta gTrackStatusesBase + (k{prefix}Ch*16 + tofs_PlayHeadPos+1)
	sta gTrackStatusesBase + (k{prefix}Ch*16 + tofs_RemainDur)
	sta gTrackStatusesBase + (k{prefix}Ch*16 + tofs_RemainGT)
	sta gTrackStatusesBase + (k{prefix}Ch*16 + tofs_LoopCount)
	
	lda #kCenterVolumeFactor
	sta gTrackStatusesBase + (k{prefix}Ch*16 + tofs_LPanFactor)
	sta gTrackStatusesBase + (k{prefix}Ch*16 + tofs_RPanFactor)

fend:
	restore_ax()
	rts

}


scope checkSE1Invoke: {
	impl_se_proc(SE1, 2)
}

scope checkSE2Invoke: {
	impl_se_proc(SE2, 3)
}

scope onSE1SpecialCommand: {
	lda #0
	sta HeaderPauseAtBoot
	rts
}

scope onSE2SpecialCommand: {
	jsr resetDriver
	rts
}

scope stopBGM: {
	pha
		// Key off all
		lda #$5C
		sta pPokeAddr
		lda #$FF
		sta pPokeVal
		
		// set pause flag
		lda #1
		sta HeaderPauseAtBoot
	pla	
	rts
}
