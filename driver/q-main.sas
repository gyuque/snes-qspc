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
	// FDh -> T0用上位カウンタ（BGM用）
	// FEh -> T1用上位カウンタ（SE用）

	// BGM用カウンタがアップするまで待つ（アップ後プロシージャを抜ける）
	// その間にSE用カウンタがアップすればSE処理を割り込ませる
	timer_wait_loop:
		// SE用上位カウンタをチェック
		// 0以外（カウントアップ）ならSE処理をここに割り込ませる
		lda $FE
		beq no_sei
			jsr processSystemCommand
			// jsr seProc
		no_sei:
	
		lda $FD // BGM用カウンタチェック
		beq timer_wait_loop
		
	// SE用カウンタをインクリメント
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

	lda #0 // トラックインデックス（スタート）
	ldy HeaderTracksCountByte // 処理トラック数（8ch - SE割り当て数 までにすること）
	
	sta gNextKeyOnFlags  // キーオンフラグ(一時)を0で初期化
	sta gNextKeyOffFlags // キーオフフラグ(一時)を0で初期化
	sta gSequenceStrideCounter // シーケンストラックのストライドを積算するカウンタ（0で初期化）

	track_loop:
		jsr advanceABGMTrack

		jsr advanceStrideCounter
		inc // トラックインデックス +1
		dey // カウンタ -1
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

	// ワーキング変数更新 =============================================
	sta gCurBGMTrackIndex
	//  トラックステート、シーケンスデータのベースオフセットをそれぞれ計算
	xcn
	sta gCurBGMTrackStateBaseOfs

	// トラックのオフセットとしてcounter << 8をセット
	ldy gSequenceStrideCounter
	lda #0
	stw gCurBGMTrackSequenceBaseOfs // Hi=Y, Lo=A

	jsr decrementBGMGateTime
	jsr decrementBGMRemainDuration
	// ↓ サブルーチン内で次のコマンドを処理するか判断
	// Cフラグが立っていれば処理
	bcc no_next_cmd
		process_cmd_start:

		// トラックステートから演奏位置を取得(Y,A 16bit)
		ldx gCurBGMTrackStateBaseOfs
		lda gBGMTrackStatusesBase+tofs_PlayHeadPos+1 ,x
		tay
		lda gBGMTrackStatusesBase+tofs_PlayHeadPos   ,x

		adw gCurBGMTrackSequenceBaseOfs
		stw gLongTemp1 // <- トラック単位のオフセット+演奏位置
		lda #0
		ldy #((MusicSeqOrigin+ProgramOrigin) >> 8)
		adw gLongTemp1 // + シーケンスデータ領域のベース
		stw gCurBGMTrackHeadAddr // <- 演奏データの読み込み位置(絶対/16bit)

		// 次のコマンドへのストライド（初期値は1、2バイト以上のコマンドを処理したら増やす）
		lda #1
		sta gCurCommandAdvance

		// 演奏データフェッチ=====================================
		ldy #2
		lda (gCurBGMTrackHeadAddr),y
		sta gCurCommand3
		
		dey
		lda (gCurBGMTrackHeadAddr),y
		sta gCurCommand2

		dey
		lda (gCurBGMTrackHeadAddr),y
		sta gCurCommand1

		beq null_cmd // コマンド=00なら何もしない（シーケンス終了）
			// A: First command
			jsr processBGMCommand

			// 処理したコマンドのバイト数(gCurCommandAdvanceに格納)だけ先頭位置を進める
			jsr updateBGMTrackHeadPosition
			
			// デュレーションが0のままなら、直ちに次のコマンドを処理
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

	// 直前のノートの残り時間
	lda gBGMTrackStatusesBase+tofs_RemainDur,x
	beq move_next // 既にゼロなら次のコマンドを処理 --------(1)
		// ゼロでなければ1減らす
		dec
		sta gBGMTrackStatusesBase+tofs_RemainDur,x
		beq move_next // 新しい値が0なら次を処理 -----------(2)

		plx
		clc // <- Cフラグクリア(次コマンドを処理しない)
		rts
	// -------------------------------------------------

	move_next:
		plx
		sec // <- Cフラグセット(次コマンド処理を指示)
		rts
}

scope decrementBGMGateTime: {
	phx
	ldx gCurBGMTrackStateBaseOfs

	// 現在の残りゲートタイムを取得
	lda gBGMTrackStatusesBase+tofs_RemainGT,x
	beq no_gt // 0の場合処理しない
	
	cmp #kTieTick // タイ・スラーの場合処理しない
	beq no_gt
		dec
		sta gBGMTrackStatusesBase+tofs_RemainGT,x
		bne no_gt
		
			// ゼロになったのでキーオフ
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
	lda BitTable,x // テーブルからトラック番号と対応するビットマスクを取得
	
	ora gNextKeyOffFlags
	sta gNextKeyOffFlags

	restore_axy() 
	rts
}

scope updateBGMTrackHeadPosition: {
	// YA <- 演奏ヘッド位置(16bit)
	ldx gCurBGMTrackStateBaseOfs
	lda gBGMTrackStatusesBase+tofs_PlayHeadPos+1 ,x
	tay
	lda gBGMTrackStatusesBase+tofs_PlayHeadPos   ,x
	
	adw gCurCommandAdvance
	// YA <- old_head + adv
	
	// A(Lo), Y(Hi)の順でストア
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

