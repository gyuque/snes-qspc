// - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
scope doMainLoop: {
	main_loop:
		jsr advanceBGM

		jsr applyTemporaryKeyOnOffFlags
		jsr transferDSPRegisterShadow
		jsr resetOneShotRegistersShadow
		
		jsr waitTimer

		bra main_loop

	rts
}

scope waitTimer: {
	// FDh -> T0�p��ʃJ�E���^�iBGM�p�j
	// FEh -> T1�p��ʃJ�E���^�iSE�p�j

	// BGM�p�J�E���^���A�b�v����܂ő҂i�A�b�v��v���V�[�W���𔲂���j
	// ���̊Ԃ�SE�p�J�E���^���A�b�v�����SE���������荞�܂���
	timer_wait_loop:
		// SE�p��ʃJ�E���^���`�F�b�N
		// 0�ȊO�i�J�E���g�A�b�v�j�Ȃ�SE�����������Ɋ��荞�܂���
		lda $FE
		beq no_sei
			jsr processSystemCommand
			// jsr seProc
		no_sei:
	
		lda $FD // BGM�p�J�E���^�`�F�b�N
		beq timer_wait_loop
		
	// SE�p�J�E���^���C���N�������g
	lda gSETickCount
	inc
	cmp #kTickMax
	bne no_tmax
		lda #0
	no_tmax:
	sta gSETickCount
	
	rts
}

scope advanceBGM: {
	save_axy()

	lda #0 // �g���b�N�C���f�b�N�X�i�X�^�[�g�j
	ldy HeaderTracksCountByte // �����g���b�N���i8ch - SE���蓖�Đ� �܂łɂ��邱�Ɓj
	
	sta gNextKeyOnFlags  // �L�[�I���t���O(�ꎞ)��0�ŏ�����
	sta gNextKeyOffFlags // �L�[�I�t�t���O(�ꎞ)��0�ŏ�����
	sta gSequenceStrideCounter // �V�[�P���X�g���b�N�̃X�g���C�h��ώZ����J�E���^�i0�ŏ������j

	track_loop:
		jsr advanceABGMTrack

		jsr advanceStrideCounter
		inc // �g���b�N�C���f�b�N�X +1
		dey // �J�E���^ -1
		bne track_loop

	restore_axy()
	rts
}

scope advanceStrideCounter: {
	pha
	
	lda HeaderTrackStride
	add(gSequenceStrideCounter)
	sta gSequenceStrideCounter
	
	pla
	rts
}

// in A: Track index
scope advanceABGMTrack: {
	save_axy()

	// ���[�L���O�ϐ��X�V =============================================
	sta gCurBGMTrackIndex
	//  �g���b�N�X�e�[�g�A�V�[�P���X�f�[�^�̃x�[�X�I�t�Z�b�g�����ꂼ��v�Z
	xcn
	sta gCurBGMTrackStateBaseOfs

	// �g���b�N�̃I�t�Z�b�g�Ƃ���counter << 8���Z�b�g
	ldy gSequenceStrideCounter
	lda #0
	stw gCurBGMTrackSequenceBaseOfs // Hi=Y, Lo=A

	jsr decrementBGMGateTime
	jsr decrementBGMRemainDuration
	// �� �T�u���[�`�����Ŏ��̃R�}���h���������邩���f
	// C�t���O�������Ă���Ώ���
	bcc no_next_cmd
		process_cmd_start:

		// �g���b�N�X�e�[�g���牉�t�ʒu���擾(Y,A 16bit)
		ldx gCurBGMTrackStateBaseOfs
		lda gBGMTrackStatusesBase+tofs_PlayHeadPos+1 ,x
		tay
		lda gBGMTrackStatusesBase+tofs_PlayHeadPos   ,x

		adw gCurBGMTrackSequenceBaseOfs
		stw gLongTemp1 // <- �g���b�N�P�ʂ̃I�t�Z�b�g+���t�ʒu
		lda #0
		ldy #((MusicSeqOrigin+ProgramOrigin) >> 8)
		adw gLongTemp1 // + �V�[�P���X�f�[�^�̈�̃x�[�X
		stw gCurBGMTrackHeadAddr // <- ���t�f�[�^�̓ǂݍ��݈ʒu(���/16bit)

		// ���̃R�}���h�ւ̃X�g���C�h�i�����l��1�A2�o�C�g�ȏ�̃R�}���h�����������瑝�₷�j
		lda #1
		sta gCurCommandAdvance

		// ���t�f�[�^�t�F�b�`=====================================
		ldy #2
		lda (gCurBGMTrackHeadAddr),y
		sta gCurCommand3
		
		dey
		lda (gCurBGMTrackHeadAddr),y
		sta gCurCommand2

		dey
		lda (gCurBGMTrackHeadAddr),y
		sta gCurCommand1

		beq null_cmd // �R�}���h=00�Ȃ牽�����Ȃ��i�V�[�P���X�I���j
			// A: First command
			jsr processBGMCommand

			// ���������R�}���h�̃o�C�g��(gCurCommandAdvance�Ɋi�[)�����擪�ʒu��i�߂�
			jsr updateBGMTrackHeadPosition
			
			// �f�����[�V������0�̂܂܂Ȃ�A�����Ɏ��̃R�}���h������
			ldx gCurBGMTrackStateBaseOfs
			lda gBGMTrackStatusesBase+tofs_RemainDur,x
			beq process_cmd_start
		null_cmd:
	no_next_cmd:

	restore_axy()
	rts
}

scope decrementBGMRemainDuration: {
	phx
	ldx gCurBGMTrackStateBaseOfs

	// ���O�̃m�[�g�̎c�莞��
	lda gBGMTrackStatusesBase+tofs_RemainDur,x
	beq move_next // ���Ƀ[���Ȃ玟�̃R�}���h������ --------(1)
		// �[���łȂ����1���炷
		dec
		sta gBGMTrackStatusesBase+tofs_RemainDur,x
		beq move_next // �V�����l��0�Ȃ玟������ -----------(2)

		plx
		clc // <- C�t���O�N���A(���R�}���h���������Ȃ�)
		rts
	// -------------------------------------------------

	move_next:
		plx
		sec // <- C�t���O�Z�b�g(���R�}���h�������w��)
		rts
}

scope decrementBGMGateTime: {
	phx
	ldx gCurBGMTrackStateBaseOfs

	// ���݂̎c��Q�[�g�^�C�����擾
	lda gBGMTrackStatusesBase+tofs_RemainGT,x
	beq no_gt // 0�̏ꍇ�������Ȃ�
	
	cmp #kTieTick // �^�C�E�X���[�̏ꍇ�������Ȃ�
	beq no_gt
		dec
		sta gBGMTrackStatusesBase+tofs_RemainGT,x
		bne no_gt
		
			// �[���ɂȂ����̂ŃL�[�I�t
			jsr setNextKeyOff
		
	no_gt:

	plx
	rts
}

// in global: gCurBGMTrackIndex
scope setNextKeyOff: {
	save_axy()

	// Y <- Bit mask
	ldx gCurBGMTrackIndex
	lda BitTable,x // �e�[�u������g���b�N�ԍ��ƑΉ�����r�b�g�}�X�N���擾
	
	ora gNextKeyOffFlags
	sta gNextKeyOffFlags

	restore_axy() 
	rts
}

scope updateBGMTrackHeadPosition: {
	// YA <- ���t�w�b�h�ʒu(16bit)
	ldx gCurBGMTrackStateBaseOfs
	lda gBGMTrackStatusesBase+tofs_PlayHeadPos+1 ,x
	tay
	lda gBGMTrackStatusesBase+tofs_PlayHeadPos   ,x
	
	adw gCurCommandAdvance
	// YA <- old_head + adv
	
	// A(Lo), Y(Hi)�̏��ŃX�g�A
	sta gBGMTrackStatusesBase+tofs_PlayHeadPos   ,x
	tya
	sta gBGMTrackStatusesBase+tofs_PlayHeadPos+1 ,x
	
	rts
}


scope applyTemporaryKeyOnOffFlags: {
	lda gNextKeyOnFlags
	sta gRegisterShadowBase + 0x4C

	lda gNextKeyOffFlags
	sta gRegisterShadowBase + 0x5C

	rts
}


scope processSystemCommand: {
	pha
	
	lda pIOPort3
	eor #$FF
	bne not_ff
		// (P3) xor FFh == 0
		
		// Key off all
		lda #$5C
		sta pPokeAddr
		lda #$FF
		sta pPokeVal
		
		// jump to IPL
		jmp $FFC0
	not_ff:
	
	pla
	rts
}

