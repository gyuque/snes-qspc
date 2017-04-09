
// in A: Next command
// in globals: gCurBGMTrackIndex
//
// 
scope processBGMCommand: {
	// ** CAUTION **
	// �W�����v���rts����̂ł����ŃX�^�b�N��G��Ȃ�����


	// �m�[�g�R�}���h�͔͈͂��L���̂Ŕ�r���߂ŏ���
	// �m�[�g��C7h�܂łȂ̂ŁAA��C8h�ȏ�̂Ƃ��X�L�b�v
	cmp #$C8
	bcs no_notecmd
		jsr processBGMNoteCommand
		jmp skip_pcall
	no_notecmd:

	// �m�[�g�ȊO�̃R�}���h�̓e�[�u���Q�ƂŃW�����v
	sub(#$C8)
	asl
	tax // X <- (cmd - 0xc8) * 2

	jmp (BGMCmdJumpTable,x)
	// CALL�ł͂Ȃ�JMP�Ȃ̂Œ��Ӂi�W�����v��ł��̂܂܃��^�[������j

	skip_pcall:
	rts
}

// C8h (TBL:0)
scope processBGMTieCommand: {

	rts
}

// C9h (TBL:1)
scope processBGMRestCommand: {
	// 1�o�C�g�ڂ͌Œ�Ȃ̂ŏ���������e�͂Ȃ�
	
	// 2�o�C�g�ڂ̉������������ݏI��
	lda gCurCommand2
	jsr writeBGMNoteDuration
	inc gCurCommandAdvance // adv=2

	rts
}

// XX
scope processBGMUndefinedCommand: {
	rts
}

// ���t�ʒu���������i�W�����v�j
// x: �g���b�N�X�e�[�g�̃I�t�Z�b�g
// �R�}���h��2�o�C�g��,3�o�C�g�ڂɃW�����v�悪�i�[����Ă��邱��
macro scope rewrite_playhead() {
	lda gCurCommand2 // 2�o�C�g��=�W�����v��ʒu(Lo)
	sta gBGMTrackStatusesBase+tofs_PlayHeadPos,x
	lda gCurCommand3 // 2�o�C�g��=�W�����v��ʒu(Hi)
	sta gBGMTrackStatusesBase+tofs_PlayHeadPos+1,x

	// ��ŃZ�b�g�����ʒu����i�Ȃ��悤�ɂ���
	lda #0
	sta gCurCommandAdvance
}

// DDh
// in global: gCurBGMTrackStateBaseOfs
scope processForceJump: {
	ldx gCurBGMTrackStateBaseOfs
	rewrite_playhead()

	rts
}

// DEh
// in global: gCurBGMTrackStateBaseOfs
scope processRepeatOut: {
	ldx gCurBGMTrackStateBaseOfs
	lda gBGMTrackStatusesBase+tofs_LoopCount,x
	bne no_out
		// �Ō�̎���Ȃ̂Ŕ�����
		rewrite_playhead()
	jmp endif
	no_out:
		inc gCurCommandAdvance // adv=2
		inc gCurCommandAdvance // adv=3
	endif:

	rts
}

// DFh
// in global: gCurBGMTrackStateBaseOfs
scope processBGMBeginRepeat: {
	lda gCurCommand2 // 2�o�C�g�ڂ���܂�Ԃ��񐔂��擾
	ldx gCurBGMTrackStateBaseOfs                // ��
	sta gBGMTrackStatusesBase+tofs_LoopCount,x // �g���b�N�X�e�[�g�ɋL�^

	inc gCurCommandAdvance // adv=2

	rts
}

// EFh
// in global: gCurBGMTrackStateBaseOfs
scope processBGMEndRepeat: {
	inc gCurCommandAdvance // adv=2
	inc gCurCommandAdvance // adv=3
	// �� ���s�[�g�𔲂���ꍇ�݂̂��ɒl���g���B�܂�Ԃ��ꍇ�͖���

	ldx gCurBGMTrackStateBaseOfs
	lda gBGMTrackStatusesBase+tofs_LoopCount,x // �g���b�N�X�e�[�g���烋�[�v�J�E���g���擾
	beq exit_repeat // �J�E���g��0�Ȃ烊�s�[�g�I��
		dec
		sta gBGMTrackStatusesBase+tofs_LoopCount,x // 1���炵���J�E���^��ۑ�
		
		rewrite_playhead()
	exit_repeat:
	
	rts
}

// E0h
// in global: gCurBGMTrackIndex
scope processBGMSetInst: {
	inc gCurCommandAdvance // adv=2

	// Y <- register (shadow) offset
	lda gCurBGMTrackIndex
	xcn
	tay

	// 2�o�C�g�ڂ���INST�ԍ��擾
	lda gCurCommand2
	asl
	asl
	tax // �X�g���C�h=4�Ȃ̂�4�{����X�ɃZ�b�g
	
	lda InstPresetTable,x
	sta gRegisterShadowBase+4,y // Set SRCN

	lda InstPresetTable+1,x
	sta gRegisterShadowBase+5,y // Set ADSR(1)

	lda InstPresetTable+2,x
	sta gRegisterShadowBase+6,y // Set ADSR(1)

	rts
}

// E1h Pan
scope processBGMPan: {
	inc gCurCommandAdvance // adv=2

	lda gCurCommand2
	jsr setPannedLevel

	rts
}

// in A: raw position(0-8-16)
// in global: gCurBGMTrackStateBaseOfs
scope setPannedLevel: {

	// Default values
	ldx #kCenterVolumeFactor
	ldy #kCenterVolumeFactor
	
	cmp #8
	beq psel_end
		bcc leftpan
			// Pos = 9-16  ********
			
			// Y <- Table[pos-9]
			sub(#9)
			tax
			lda PanFactorTable,x
			tay
			
			// X <- reversed factor
			eor #$FF
			tax
			
		jmp psel_end
		leftpan:
			// Pos = 0-7  ********
		
			tax
			lda PanFactorRevTable,x
			tax

			eor #$FF
			tay
	psel_end:
	
	txa
	ldx gCurBGMTrackStateBaseOfs
	sta gBGMTrackStatusesBase+tofs_LPanFactor,x
	
	tya
	sta gBGMTrackStatusesBase+tofs_RPanFactor,x
	
	rts
}

// in A: Next command
// in global: gCurBGMTrackIndex, gCurBGMTrackStateBaseOfs
scope processBGMNoteCommand: {
	jsr savePreviousNoteGateTime

	pha
		// X <- register (shadow) offset
		// �`�����l���ɑΉ�����DSP���W�X�^�̃I�t�Z�b�g(x0h)��gRegOffsetTemp�ɃZ�b�g���Ă���
		lda gCurBGMTrackIndex
		xcn
		sta gRegOffsetTemp
	pla

	sub(#$80) // �ŏ��l80h -> 0 �Ƃ���

	// ���K�ƑΉ�������g�����Z�b�g
	jsr setChannelFqWithNote

	// 2�o�C�g��(duration)����
	lda gCurCommand2
	jsr writeBGMNoteDuration
	inc gCurCommandAdvance // adv=2

	// 3�o�C�g��(Q+V)����
	lda gCurCommand3
	cmp #$80
	bcs not_3rd // �R�}���h�o�C�g��0x80�ȏ�ł���Ώ������Ȃ��i���݂̎d�l�ł͂��蓾�Ȃ����ꉞ�`�F�b�N�j
		jsr processBGM_QV
		inc gCurCommandAdvance // adv=3
	not_3rd:
	
	lda gPrevGateTime
	cmp #kTieTick // ���O�̃m�[�g�Ƀ^�C�E�X���[���w������Ă���ꍇ�̓L�[�I�����Ȃ�
	beq no_keyon
		
		// �L�[�I���t���O�Z�b�g
		ldx gCurBGMTrackIndex
		lda BitTable,x // �e�[�u������g���b�N�ԍ��ƑΉ�����r�b�g�}�X�N���擾
		ora gNextKeyOnFlags
		sta gNextKeyOnFlags // OR���ĕۑ�

	no_keyon:
	
	rts
}

// in global: gCurBGMTrackStateBaseOfs
scope savePreviousNoteGateTime: {
	save_axy()

	ldx gCurBGMTrackStateBaseOfs
	lda gBGMTrackStatusesBase+tofs_RemainGT,x
	sta gPrevGateTime
	
	restore_axy()
	rts
}

// in A: Note index(0-)
// in globals: gRegOffsetTemp
scope setChannelFqWithNote: {
	save_axy()

	asl
	tay // Y <- �e�[�u���I�t�Z�b�g
	
	// Pitch���W�X�^�V���h�E�̃I�t�Z�b�g�v�Z�i�擪+2�j
	ldx gRegOffsetTemp
	inx
	inx
	
	// �e�[�u���̒l���s�b�N�A���W�X�^�V���h�E�ɏ�������
	// (Lo)
	lda FqRegTable,y
	sta gRegisterShadowBase,x
	// (Hi)
	lda FqRegTable+1,y
	sta gRegisterShadowBase+1,x

	restore_axy()
	rts
}


// in A: duration
// in global: gCurBGMTrackStateBaseOfs
scope writeBGMNoteDuration: {
	jsr modifyBGMNoteDuration

	ldx gCurBGMTrackStateBaseOfs
	sta gBGMTrackStatusesBase+tofs_RemainDur,x
	rts
}

// in/out A: raw duration
scope modifyBGMNoteDuration: {
	cmp #97
	bcc no_modify
		// 97�ȏ�̏ꍇ��7bit�Ɏ��܂�Ȃ��l���ό`����ē����Ă���̂Ō��ɖ߂�
		// org = (d-25)*2
		sub(#25)
		asl
	no_modify:
	
	rts
}

// Q(�Q�[�g�^�C��)��V(�x���V�e�B)�̃y�A������
// in A: cmd byte
// in globals: gRegOffsetTemp, gCurBGMTrackStateBaseOfs
scope processBGM_QV: {
	tax

	// Y <- Q << 1
	// A <- Note Duration
	and #$F0
	bne not_tie
		ldy #kTieTick
	jmp endif_tie
	not_tie:
		asl // �ő�l0x70 -> 0xE0   8bit�E�V�t�g�����Ƃ��ق�1�{�ƂȂ�悤�ɁB 
		
		tay
		lda gCurCommand2
		jsr modifyBGMNoteDuration

		mul // YA = q * duration
	endif_tie:
	// A�ɓ����Ă��鉺�ʃr�b�g�͐؂�̂āAY�̂݌�Ńg���b�N�X�e�[�g�ɃZ�b�g

	// �x���V�e�B���`�����l���̉��ʂɃZ�b�g
	txa
	jsr setPannedVelocityToShadow

	// �Q�[�g�^�C�����Z�b�g
	tya
	ldx gCurBGMTrackStateBaseOfs
	sta gBGMTrackStatusesBase+tofs_RemainGT,x

	rts
}

// in A: QV byte
scope setPannedVelocityToShadow: {
	save_axy()

	and #$0F
	xcn
	lsr // �ŏ�ʃr�b�g�͕����Ȃ̂Ŕ�����
	tay
	
	// Y=Velocity
	phy
		// �x���V�e�B�ƃp���W�����|���ă{�����[��������(L)
		ldx gCurBGMTrackStateBaseOfs
		lda gBGMTrackStatusesBase+tofs_LPanFactor,x
		mul
		tya
		ldx gRegOffsetTemp
		sta gRegisterShadowBase,x // reg <- (Y * A) / 256
	ply
	// Y=Velocity(again)

	// �x���V�e�B�ƃp���W�����|���ă{�����[��������(R)
	ldx gCurBGMTrackStateBaseOfs
	lda gBGMTrackStatusesBase+tofs_RPanFactor,x
	mul
	tya
	ldx gRegOffsetTemp
	sta gRegisterShadowBase+1,x
	
	restore_axy()
	rts
}

// Setup - - - - - - - - - - - - - - - -
macro scope put_proc_addr(proc) {
	lda #({proc} & 0x00ff)
	sta BGMCmdJumpTable,x
	inx

	lda #(({proc} >> 8) & 0x00ff)
	sta BGMCmdJumpTable,x
	inx
}

scope initializeSetupBGMJumpTable: {
	// Clear
	ldx #0
	ldy #48
	loop_begin:
		put_proc_addr(processBGMUndefinedCommand)
		dey
		bne loop_begin

	// Put procs
	
	ldx #0
	put_proc_addr(processBGMTieCommand)  // C8
	put_proc_addr(processBGMRestCommand) // C9

	ldx #((0xDD - 0xC8) * 2)
	put_proc_addr(processForceJump)       // DD
	put_proc_addr(processRepeatOut)       // DE
	put_proc_addr(processBGMBeginRepeat)  // DF
	put_proc_addr(processBGMSetInst)      //  E0
	put_proc_addr(processBGMPan)          //  E1

	ldx #((0xEF - 0xC8) * 2)
	put_proc_addr(processBGMEndRepeat)  // EF

	rts
}
