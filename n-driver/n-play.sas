
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
scope doMainLoop: {

	main_loop:
		jsr waitTimer
		
		bra main_loop

	rts
}

scope waitTimer: {
	// FDh -> T0用上位カウンタ（BGM用）
	// FEh -> T1用上位カウンタ（SE用）

	// BGM用カウンタがアップするまで待つ（アップしたらループを抜ける）
	// その間にSE用カウンタがアップすればSE処理を割り込ませる
	timer_wait_loop:
		// + 出力レベル計算（不要ならここをコメントアウト）
		jsr calcResultLevel
		// + 出力レベル計算ここまで

		// SE用上位カウンタをチェック
		// 0以外（カウントアップ）ならSE処理をここに割り込ませる
		lda $FE // SE用カウンタ値ロード
		beq no_sei
//			>> SE進行
			jsr advanceSETracks
			// レジスタ更新
			 jsr applyTemporaryKeyOnOffFlags
			 jsr transferDSPRegisterShadow
			 jsr resetOneShotRegistersShadow
			 
			jsr checkSE1Invoke
			jsr checkSE2Invoke
		no_sei:
	
		lda $FD // BGM用カウンタ値ロード
		beq timer_wait_loop
	// - - - - - - - - - - - - - - - - - - - - - - - -
	// >> BGM進行
	lda HeaderPauseAtBoot
	bne end_bgmplay
	
	jsr advanceBGMTracks
	inc gGlobalTickCount
	// レジスタ更新
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
	
	sta gVelocityMinus // Vそのまま適用

	lda #6
	jsr advanceATrack
	lda #7
	jsr advanceATrack

fend:
	rts
}

scope advanceBGMTracks: {
	lda #32
	sta gVelocityMinus // Vやや小さめに

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

	// ワーキング変数更新 =============================================
	sta gProcessingTrackIndex
	//  トラックステートのベースオフセット計算
	xcn // Track=x -> +$x0
	sta gProcessingTrackStateBaseOfs

	//  シーケンスデータのベースオフセット取得
	lda gProcessingTrackIndex
	asl
	tax
	lda SeqOffsetTable+1,x // __ Y <- tbl[i*2+1]
	tay                    // _/ 
	lda SeqOffsetTable,x   // A <- tbl[i*2  ]
	stw gSequenceBaseOfs   // Hi=Y, Lo=A

	cpy #$FF // FFxxならトラック無効
	beq no_next_cmd

	jsr decrementBGMGateTime
	jsr decrementBGMRemainDuration
	// ↓ サブルーチン内で次のコマンドを処理するか判断
	// Cフラグが立っていれば処理
	bcc no_next_cmd
		process_cmd_start:
		
		// トラックステートから演奏位置を取得(Y,A 16bit)
		ldx gProcessingTrackStateBaseOfs
		lda gTrackStatusesBase+tofs_PlayHeadPos+1 ,x
		tay
		lda gTrackStatusesBase+tofs_PlayHeadPos   ,x

		// シーケンスデータ位置を計算
		//  合計オフセット算出(トラック単位のオフセット+演奏位置オフセット)
		adw gSequenceBaseOfs
		stw gLongTemp1 // <- 計算用に保持
		//  絶対位置算出
		lda #0
		ldy #((MusicSeqOrigin+ProgramOrigin) >> 8)
		adw gLongTemp1 // オフセット加算
		stw gCurSeqHeadAddr // <- 読み込み位置保持(絶対/16bit)

		// 次のコマンドへのストライド（初期値は1、2バイト以上のコマンドを処理したら増やす）
		lda #1
		sta gCurCommandAdvance

		// 演奏データフェッチ=====================================
		// 3つ先から順番に逆順で取得
		ldy #2
		lda (gCurSeqHeadAddr),y
		sta gCurCommand3

		dey
		lda (gCurSeqHeadAddr),y
		sta gCurCommand2

		dey
		lda (gCurSeqHeadAddr),y
		sta gCurCommand1

		beq null_cmd // コマンド=00なら何もしない（シーケンス終了）
			// A: First command
			jsr processSeqCommand

			// 処理したコマンドのバイト数(gCurCommandAdvanceに格納)だけ先頭位置を進める
			jsr updatePlayHeadPosition
			
			// デュレーションが0のままなら、直ちに次のコマンドを処理
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

	// 直前のノートの残り時間
	lda gTrackStatusesBase+tofs_RemainDur,x
	beq move_next // 既にゼロなら次のコマンドを処理 --------(1)
		// ゼロでなければ1減らす
		dec
		sta gTrackStatusesBase+tofs_RemainDur,x
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
	ldx gProcessingTrackStateBaseOfs

	// 現在の残りゲートタイムを取得
	lda gTrackStatusesBase+tofs_RemainGT,x
	beq no_gt // 0の場合処理しない
	
	cmp #kTieTick // タイ・スラーの場合処理しない
	beq no_gt
		dec
		sta gTrackStatusesBase+tofs_RemainGT,x
		bne no_gt
		
			// ゼロになったのでキーオフ
			jsr setNextKeyOff
		
	no_gt:

	plx
	rts
}

// ◆ キーオフを予約
// in global: gProcessingTrackIndex
scope setNextKeyOff: {
	save_axy()

	// Y <- Bit mask
	ldx gProcessingTrackIndex
	lda BitTable,x // テーブルからトラック番号と対応するビットマスクを取得
	
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
	// YA <- 演奏ヘッド位置(16bit)
	ldx gProcessingTrackStateBaseOfs
	lda gTrackStatusesBase+tofs_PlayHeadPos+1 ,x
	tay
	lda gTrackStatusesBase+tofs_PlayHeadPos   ,x

	// ストライドを加算
	// YA <- old_head + adv
	adw gCurCommandAdvance
	
	// A(Lo), Y(Hi)の順でストア
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

// ◆ 出力音量計算
scope calcResultLevel: {
	save_axy()

	// port addr <- (ch << 4) | 9
	lda gLvMeterCurTrack
	xcn
	tax // XにDSPシャドウ用オフセットを保存(ch << 4)
	ora #$09
	sta pPokeAddr
	// [左]
	// チャンネルのOUTXを取得
	lda pPokeVal
	and #$7F // 符号を捨てて絶対値化
	tay // Yに波高セット
	lda gRegisterShadowBase,x // AにL_volセット
	lsr // Volの値は1/2で計算
	
	// パン適用(左)
	mul // YA <- outx*3
	adw gLvMeterLeftSum // 積算値と合わせる
	stw gLvMeterLeftSum // 積算値を更新

	// [右]
	// チャンネルのOUTXを取得
	lda pPokeVal
	and #$7F // 符号を捨てて絶対値化

	tay // Yに波高セット
	lda gRegisterShadowBase+1,x // AにR_volセット
	lsr // Volの値は1/2で計算

	mul
	adw gLvMeterRightSum // 積算値と合わせる
	stw gLvMeterRightSum// 積算値を更新

	// 対象チャンネル移動
	lda gLvMeterCurTrack
	inc
	cmp HeaderNumTracks
	bcc no_reset
		lda gLvMeterLeftSum+1 // 上位バイトのみ取得(=下位切り捨て)
		sta pIOPort2
		lda gLvMeterRightSum+1 // 上位バイトのみ取得(=下位切り捨て)
		sta pIOPort3
	
		lda #0
		sta gLvMeterLeftSum   // ついでにsumもリセット
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
