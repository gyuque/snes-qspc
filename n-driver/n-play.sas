
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
scope doMainLoop: {

	main_loop:
		jsr waitTimer
		
		bra main_loop

	rts
}

scope waitTimer: {
	// FDh -> T0�p��ʃJ�E���^�iBGM�p�j
	// FEh -> T1�p��ʃJ�E���^�iSE�p�j

	// BGM�p�J�E���^���A�b�v����܂ő҂i�A�b�v�����烋�[�v�𔲂���j
	// ���̊Ԃ�SE�p�J�E���^���A�b�v�����SE���������荞�܂���
	timer_wait_loop:
		// + �o�̓��x���v�Z�i�s�v�Ȃ炱�����R�����g�A�E�g�j
		jsr calcResultLevel
		// + �o�̓��x���v�Z�����܂�

		// SE�p��ʃJ�E���^���`�F�b�N
		// 0�ȊO�i�J�E���g�A�b�v�j�Ȃ�SE�����������Ɋ��荞�܂���
		lda $FE // SE�p�J�E���^�l���[�h
		beq no_sei
//			>> SE�i�s
			jsr advanceSETracks
			// ���W�X�^�X�V
			 jsr applyTemporaryKeyOnOffFlags
			 jsr transferDSPRegisterShadow
			 jsr resetOneShotRegistersShadow
			 
			jsr checkSE1Invoke
			jsr checkSE2Invoke
		no_sei:
	
		lda $FD // BGM�p�J�E���^�l���[�h
		beq timer_wait_loop
	// - - - - - - - - - - - - - - - - - - - - - - - -
	// >> BGM�i�s
	lda HeaderPauseAtBoot
	bne end_bgmplay
	
	jsr advanceBGMTracks
	inc gGlobalTickCount
	// ���W�X�^�X�V
	 jsr applyTemporaryKeyOnOffFlags
	 jsr transferDSPRegisterShadow
	 jsr resetOneShotRegistersShadow
  end_bgmplay:
	
	// jsr processSystemCommand
	rts
}

scope advanceSETracks: {
	lda HeaderSEEnabled
	beq fend // SKIP if SE is desabled
	
	lda #0
	sta gNextKeyOnFlags
	sta gNextKeyOffFlags
	
	sta gVelocityMinus // V���̂܂ܓK�p

	lda #6
	jsr advanceATrack
	lda #7
	jsr advanceATrack

fend:
	rts
}

scope advanceBGMTracks: {
	lda #32
	sta gVelocityMinus // V��⏬���߂�

	lda #0
	sta gNextKeyOnFlags
	sta gNextKeyOffFlags

	lda #0
	lp:
		jsr advanceATrack
		inc
		cmp HeaderNumTracks
		bcc lp

	rts
}

// in A: Track index
scope advanceATrack: {
	save_axy()

	// ���[�L���O�ϐ��X�V =============================================
	sta gProcessingTrackIndex
	//  �g���b�N�X�e�[�g�̃x�[�X�I�t�Z�b�g�v�Z
	xcn // Track=x -> +$x0
	sta gProcessingTrackStateBaseOfs

	//  �V�[�P���X�f�[�^�̃x�[�X�I�t�Z�b�g�擾
	lda gProcessingTrackIndex
	asl
	tax
	lda SeqOffsetTable+1,x // __ Y <- tbl[i*2+1]
	tay                    // _/ 
	lda SeqOffsetTable,x   // A <- tbl[i*2  ]
	stw gSequenceBaseOfs   // Hi=Y, Lo=A

	cpy #$FF // FFxx�Ȃ�g���b�N����
	beq no_next_cmd

	jsr decrementBGMGateTime
	jsr decrementBGMRemainDuration
	// �� �T�u���[�`�����Ŏ��̃R�}���h���������邩���f
	// C�t���O�������Ă���Ώ���
	bcc no_next_cmd
		process_cmd_start:
		
		// �g���b�N�X�e�[�g���牉�t�ʒu���擾(Y,A 16bit)
		ldx gProcessingTrackStateBaseOfs
		lda gTrackStatusesBase+tofs_PlayHeadPos+1 ,x
		tay
		lda gTrackStatusesBase+tofs_PlayHeadPos   ,x

		// �V�[�P���X�f�[�^�ʒu���v�Z
		//  ���v�I�t�Z�b�g�Z�o(�g���b�N�P�ʂ̃I�t�Z�b�g+���t�ʒu�I�t�Z�b�g)
		adw gSequenceBaseOfs
		stw gLongTemp1 // <- �v�Z�p�ɕێ�
		//  ��Έʒu�Z�o
		lda #0
		ldy #((MusicSeqOrigin+ProgramOrigin) >> 8)
		adw gLongTemp1 // �I�t�Z�b�g���Z
		stw gCurSeqHeadAddr // <- �ǂݍ��݈ʒu�ێ�(���/16bit)

		// ���̃R�}���h�ւ̃X�g���C�h�i�����l��1�A2�o�C�g�ȏ�̃R�}���h�����������瑝�₷�j
		lda #1
		sta gCurCommandAdvance

		// ���t�f�[�^�t�F�b�`=====================================
		// 3�悩�珇�Ԃɋt���Ŏ擾
		ldy #2
		lda (gCurSeqHeadAddr),y
		sta gCurCommand3

		dey
		lda (gCurSeqHeadAddr),y
		sta gCurCommand2

		dey
		lda (gCurSeqHeadAddr),y
		sta gCurCommand1

		beq null_cmd // �R�}���h=00�Ȃ牽�����Ȃ��i�V�[�P���X�I���j
			// A: First command
			jsr processSeqCommand

			// ���������R�}���h�̃o�C�g��(gCurCommandAdvance�Ɋi�[)�����擪�ʒu��i�߂�
			jsr updatePlayHeadPosition
			
			// �f�����[�V������0�̂܂܂Ȃ�A�����Ɏ��̃R�}���h������
			ldx gProcessingTrackStateBaseOfs
			lda gTrackStatusesBase+tofs_RemainDur,x
			beq process_cmd_start
		null_cmd:

	no_next_cmd:

	restore_axy()
	rts
}

scope decrementBGMRemainDuration: {
	phx
	ldx gProcessingTrackStateBaseOfs

	// ���O�̃m�[�g�̎c�莞��
	lda gTrackStatusesBase+tofs_RemainDur,x
	beq move_next // ���Ƀ[���Ȃ玟�̃R�}���h������ --------(1)
		// �[���łȂ����1���炷
		dec
		sta gTrackStatusesBase+tofs_RemainDur,x
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
	ldx gProcessingTrackStateBaseOfs

	// ���݂̎c��Q�[�g�^�C�����擾
	lda gTrackStatusesBase+tofs_RemainGT,x
	beq no_gt // 0�̏ꍇ�������Ȃ�
	
	cmp #kTieTick // �^�C�E�X���[�̏ꍇ�������Ȃ�
	beq no_gt
		dec
		sta gTrackStatusesBase+tofs_RemainGT,x
		bne no_gt
		
			// �[���ɂȂ����̂ŃL�[�I�t
			jsr setNextKeyOff
		
	no_gt:

	plx
	rts
}

// �� �L�[�I�t��\��
// in global: gProcessingTrackIndex
scope setNextKeyOff: {
	save_axy()

	// Y <- Bit mask
	ldx gProcessingTrackIndex
	lda BitTable,x // �e�[�u������g���b�N�ԍ��ƑΉ�����r�b�g�}�X�N���擾
	
	ora gNextKeyOffFlags
	sta gNextKeyOffFlags

	restore_axy() 
	rts
}

scope applyTemporaryKeyOnOffFlags: {
	lda gNextKeyOnFlags
	sta gRegisterShadowBase + 0x4C

	lda gNextKeyOffFlags
	sta gRegisterShadowBase + 0x5C

	rts
}


scope updatePlayHeadPosition: {
	// YA <- ���t�w�b�h�ʒu(16bit)
	ldx gProcessingTrackStateBaseOfs
	lda gTrackStatusesBase+tofs_PlayHeadPos+1 ,x
	tay
	lda gTrackStatusesBase+tofs_PlayHeadPos   ,x

	// �X�g���C�h�����Z
	// YA <- old_head + adv
	adw gCurCommandAdvance
	
	// A(Lo), Y(Hi)�̏��ŃX�g�A
	sta gTrackStatusesBase+tofs_PlayHeadPos   ,x
	tya
	sta gTrackStatusesBase+tofs_PlayHeadPos+1 ,x
	
	rts
}

scope resetDriver: {
	
	// Key off all
	lda #$5C
	sta pPokeAddr
	lda #$FF
	sta pPokeVal
	
	// jump to IPL
	jmp $FFC0
	
	rts
}

// �� �o�͉��ʌv�Z
scope calcResultLevel: {
	save_axy()

	// port addr <- (ch << 4) | 9
	lda gLvMeterCurTrack
	xcn
	tax // X��DSP�V���h�E�p�I�t�Z�b�g��ۑ�(ch << 4)
	ora #$09
	sta pPokeAddr
	// [��]
	// �`�����l����OUTX���擾
	lda pPokeVal
	and #$7F // �������̂ĂĐ�Βl��
	tay // Y�ɔg���Z�b�g
	lda gRegisterShadowBase,x // A��L_vol�Z�b�g
	lsr // Vol�̒l��1/2�Ōv�Z
	
	// �p���K�p(��)
	mul // YA <- outx*3
	adw gLvMeterLeftSum // �ώZ�l�ƍ��킹��
	stw gLvMeterLeftSum // �ώZ�l���X�V

	// [�E]
	// �`�����l����OUTX���擾
	lda pPokeVal
	and #$7F // �������̂ĂĐ�Βl��

	tay // Y�ɔg���Z�b�g
	lda gRegisterShadowBase+1,x // A��R_vol�Z�b�g
	lsr // Vol�̒l��1/2�Ōv�Z

	mul
	adw gLvMeterRightSum // �ώZ�l�ƍ��킹��
	stw gLvMeterRightSum// �ώZ�l���X�V

	// �Ώۃ`�����l���ړ�
	lda gLvMeterCurTrack
	inc
	cmp HeaderNumTracks
	bcc no_reset
		lda gLvMeterLeftSum+1 // ��ʃo�C�g�̂ݎ擾(=���ʐ؂�̂�)
		sta pIOPort2
		lda gLvMeterRightSum+1 // ��ʃo�C�g�̂ݎ擾(=���ʐ؂�̂�)
		sta pIOPort3
	
		lda #0
		sta gLvMeterLeftSum   // ���ł�sum�����Z�b�g
		sta gLvMeterLeftSum+1 // (16bit)
		sta gLvMeterRightSum
		sta gLvMeterRightSum+1
	no_reset:
	sta gLvMeterCurTrack

	restore_axy() 
	rts
}


// ------------------ test
scope playTestSound: {
	pha
	
		// P(Lo)
		lda #$00
		sta gRegisterShadowBase,x
		// P(Hi)
		lda #$10
		sta gRegisterShadowBase+1,x

		// Key On
		lda #$01
		sta gRegisterShadowBase + 0x4C
	
		jsr transferDSPRegisterShadow
	pla
	rts
}
