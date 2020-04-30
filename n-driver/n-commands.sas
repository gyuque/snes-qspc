
// in A: Next command
// in globals: gProcessingTrackIndex
//
// 
scope processSeqCommand: {
	// ** CAUTION **
	// ジャンプ先でrtsするのでここでスタックを触らないこと


	// ノートコマンドは範囲が広いので比較命令で処理
	// ノートはC7hまでなので、AがC8h以上のときスキップ
	cmp #$C8
	bcs no_notecmd
		jsr processSeqNoteCommand
		jmp skip_pcall
	no_notecmd:

	// ノート以外のコマンドはテーブル参照でジャンプ
	sub(#$C8)
	asl
	tax // X <- (cmd - 0xc8) * 2

	jmp (SCmdJumpTable,x)
	// CALLではなくJMPなので注意（ジャンプ先でそのままリターンする）

	skip_pcall:
	rts
}

// C8h (TBL:0)
scope processSeqTieCommand: {
	// タイ・スラーはゲートタイムで表すのでこのコマンドは未使用
	rts
}

// C9h (TBL:1)
scope processSeqRestCommand: {
	// 1バイト目は固定なので処理する内容はなし
	
	// 2バイト目の音長を書き込み終了
	lda gCurCommand2
	jsr writeNoteDuration
	inc gCurCommandAdvance // adv=2

	rts
}

// XX
scope processSeqUndefinedCommand: {
	rts
}

// 演奏位置書き換え（ジャンプ）
// x: トラックステートのオフセット
// コマンドの2バイト目,3バイト目にジャンプ先が格納されていること
macro scope rewrite_playhead() {
	lda gCurCommand2 // 2バイト目=ジャンプ先位置(Lo)
	sta gTrackStatusesBase+tofs_PlayHeadPos,x
	lda gCurCommand3 // 2バイト目=ジャンプ先位置(Hi)
	sta gTrackStatusesBase+tofs_PlayHeadPos+1,x

	// 上でセットした位置から進まないようにする
	lda #0
	sta gCurCommandAdvance
}

// DDh
// in global: gProcessingTrackStateBaseOfs
scope processForceJump: {
	ldx gProcessingTrackStateBaseOfs
	rewrite_playhead()

	// トラックズレチェック用の情報を出力
	phx
		// X <- track no * 2
		lda gProcessingTrackIndex
		asl
		tax
		
		lda gGlobalTickCount
		sta gLoopCheckOutHead,x
	plx

	rts
}

// DEh
// in global: gProcessingTrackStateBaseOfs
scope processRepeatOut: {
	ldx gProcessingTrackStateBaseOfs
	lda gTrackStatusesBase+tofs_LoopCount,x
	bne no_out
		// 最後の周回なので抜ける
		rewrite_playhead()
	jmp endif
	no_out:
		inc gCurCommandAdvance // adv=2
		inc gCurCommandAdvance // adv=3
	endif:

	rts
}

// DFh
// in global: gProcessingTrackStateBaseOfs
scope processSeqBeginRepeat: {
	lda gCurCommand2 // 2バイト目から折り返し回数を取得
	ldx gProcessingTrackStateBaseOfs                // ↓
	sta gTrackStatusesBase+tofs_LoopCount,x // トラックステートに記録

	inc gCurCommandAdvance // adv=2

	rts
}

// EFh
// in global: gProcessingTrackStateBaseOfs
scope processSeqEndRepeat: {
	inc gCurCommandAdvance // adv=2
	inc gCurCommandAdvance // adv=3
	// ↑ リピートを抜ける場合のみこに値を使う。折り返す場合は無効

	ldx gProcessingTrackStateBaseOfs
	lda gTrackStatusesBase+tofs_LoopCount,x // トラックステートからループカウントを取得
	beq exit_repeat // カウントが0ならリピート終了
		dec
		sta gTrackStatusesBase+tofs_LoopCount,x // 1減らしたカウンタを保存
		
		rewrite_playhead()
	exit_repeat:
	
	rts
}

// E0h
// in global: gProcessingTrackIndex
scope processSeqSetInst: {
	inc gCurCommandAdvance // adv=2

	// Y <- register (shadow) offset
	lda gProcessingTrackIndex
	xcn
	tay

	// 2バイト目からINST番号取得
	lda gCurCommand2
	asl
	asl
	tax // ストライド=4なので4倍してXにセット
	
	lda InstPresetTable,x
	sta gRegisterShadowBase+4,y // Set SRCN

	lda InstPresetTable+1,x
	sta gRegisterShadowBase+5,y // Set ADSR(1)

	lda InstPresetTable+2,x
	sta gRegisterShadowBase+6,y // Set ADSR(1)

	rts
}

// E1h Pan
scope processSeqPan: {
	inc gCurCommandAdvance // adv=2

	lda gCurCommand2
	jsr setPannedLevel

	rts
}

// in A: raw position(0-8-16)
// in global: gProcessingTrackStateBaseOfs
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
	ldx gProcessingTrackStateBaseOfs
	sta gTrackStatusesBase+tofs_LPanFactor,x
	
	tya
	sta gTrackStatusesBase+tofs_RPanFactor,x
	
	rts
}

// in A: Next command
// in global: gProcessingTrackIndex, gProcessingTrackStateBaseOfs
scope processSeqNoteCommand: {
	jsr savePreviousNoteGateTime

	pha
		// X <- register (shadow) offset
		// チャンネルに対応するDSPレジスタのオフセット(x0h)をgRegOffsetTempにセットしておく
		lda gProcessingTrackIndex
		xcn
		sta gRegOffsetTemp
	pla

	sub(#$80) // 最小値80h -> 0 とする

	// 音階と対応する周波数をセット
	jsr setChannelFqWithNote

	// 2バイト目(duration)処理
	lda gCurCommand2
	jsr writeNoteDuration
	inc gCurCommandAdvance // adv=2

	// 3バイト目(Q+V)処理
	lda gCurCommand3
	cmp #$80
	bcs not_3rd // コマンドバイトが0x80以上であれば処理しない（現在の仕様ではあり得ないが一応チェック）
		jsr processCmdParamQV
		inc gCurCommandAdvance // adv=3
	not_3rd:
	
	lda gPrevGateTime
	cmp #kTieTick // 直前のノートにタイ・スラーが指示されている場合はキーオンしない
	beq no_keyon
		
		// キーオンフラグセット
		ldx gProcessingTrackIndex
		lda BitTable,x // テーブルからトラック番号と対応するビットマスクを取得
		ora gNextKeyOnFlags
		sta gNextKeyOnFlags // ORして保存

	no_keyon:
	
	rts
}

// in global: gProcessingTrackStateBaseOfs
scope savePreviousNoteGateTime: {
	save_axy()

	ldx gProcessingTrackStateBaseOfs
	lda gTrackStatusesBase+tofs_RemainGT,x
	sta gPrevGateTime
	
	restore_axy()
	rts
}

// in A: Note index(0-)
// in globals: gRegOffsetTemp
scope setChannelFqWithNote: {
	save_axy()

	asl
	tay // Y <- テーブルオフセット
	
	// Pitchレジスタシャドウのオフセット計算（先頭+2）
	ldx gRegOffsetTemp
	inx
	inx
	
	// テーブルの値をピック、レジスタシャドウに書き込み
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
// in global: gProcessingTrackStateBaseOfs
scope writeNoteDuration: {
	jsr modifyNoteDuration

	ldx gProcessingTrackStateBaseOfs
	sta gTrackStatusesBase+tofs_RemainDur,x
	rts
}

// in/out A: raw duration
scope modifyNoteDuration: {
	cmp #97
	bcc no_modify
		// 97以上の場合は7bitに収まらない値が変形されて入っているので元に戻す
		// org = (d-25)*2
		sub(#25)
		asl
	no_modify:
	
	rts
}

// Q(ゲートタイム)とV(ベロシティ)のペアを処理
// in A: cmd byte
// in globals: gRegOffsetTemp, gProcessingTrackStateBaseOfs
scope processCmdParamQV: {
	tax

	// Y <- Q << 1
	// A <- Note Duration
	and #$F0
	bne not_tie
		ldy #kTieTick
	jmp endif_tie
	not_tie:
		asl // 最大値0x70 -> 0xE0   8bit右シフトしたときほぼ1倍となるように。 
		
		tay
		lda gCurCommand2
		jsr modifyNoteDuration

		mul // YA = q * duration
	endif_tie:
	// Aに入っている下位ビットは切り捨て、Yのみ後でトラックステートにセット

	// ベロシティをチャンネルの音量にセット
	txa
	jsr setPannedVelocityToShadow

	// ゲートタイムをセット
	tya
	ldx gProcessingTrackStateBaseOfs
	sta gTrackStatusesBase+tofs_RemainGT,x

	rts
}

// in A: QV byte
scope setPannedVelocityToShadow: {
	save_axy()

	and #$0F
	xcn
	lsr // 最上位ビットは符号なので避ける
	tay
	
	// Y=Velocity
	phy
		// ベロシティとパン係数を掛けてボリュームを決定(L)
		ldx gProcessingTrackStateBaseOfs
		lda gTrackStatusesBase+tofs_LPanFactor,x
		sub(gVelocityMinus)
		mul
		tya
		ldx gRegOffsetTemp
		sta gRegisterShadowBase,x // reg <- (Y * A) / 256
	ply
	// Y=Velocity(again)

	// ベロシティとパン係数を掛けてボリュームを決定(R)
	ldx gProcessingTrackStateBaseOfs
	lda gTrackStatusesBase+tofs_RPanFactor,x
	sub(gVelocityMinus)
	mul
	tya
	ldx gRegOffsetTemp
	sta gRegisterShadowBase+1,x
	
	restore_axy()
	rts
}


// EEh
// in global: gProcessingTrackStateBaseOfs
scope processStopBGMSpecial: {
	jsr stopBGM
	rts
}

// Setup - - - - - - - - - - - - - - - -
macro scope put_proc_addr(proc) {
	lda #({proc} & 0x00ff)
	sta SCmdJumpTable,x
	inx

	lda #(({proc} >> 8) & 0x00ff)
	sta SCmdJumpTable,x
	inx
}

scope init_setupCommandJumpTable: {
	// Clear
	ldx #0
	ldy #48
	loop_begin:
		put_proc_addr(processSeqUndefinedCommand)
		dey
		bne loop_begin

	// Put procs
	
	ldx #0
	put_proc_addr(processSeqTieCommand)  // C8
	put_proc_addr(processSeqRestCommand) // C9

	ldx #((0xDD - 0xC8) * 2)
	put_proc_addr(processForceJump)       // DD
	put_proc_addr(processRepeatOut)       // DE
	put_proc_addr(processSeqBeginRepeat)  // DF
	put_proc_addr(processSeqSetInst)      //  E0
	put_proc_addr(processSeqPan)          //  E1

	ldx #((0xEE - 0xC8) * 2) // EE = Stop BGM special command
	put_proc_addr(processStopBGMSpecial)  // EE

	ldx #((0xEF - 0xC8) * 2)
	put_proc_addr(processSeqEndRepeat)  // EF

	rts
}
