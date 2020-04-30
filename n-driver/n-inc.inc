constant ProgramOrigin(     $0400)
constant MusicHeaderOrigin( $06F0)
constant MusicSeqOrigin(    $0700)
constant SeqDirOrigin(      $1E80)
constant InstDirOrigin(     $2000)
constant BRRDirOrigin(      $2100)
constant FqTableOrigin(     $2700)
constant BRRBodyOrigin(     $2800)
constant TailPadding(       $F7F0)

constant gDebugOut1($30)
constant gDebugOut2($32)
constant gDebugOut3($34)

constant gSE1PrevVal($38)
constant gSE2PrevVal($3A)

constant gVelocityMinus($3C)

// Temporary and working variables
constant gProcessingTrackIndex($62)
constant gProcessingTrackStateBaseOfs($63)

// vvvv 16bit vvvv
constant gSequenceBaseOfs($64) 
constant gCurSeqHeadAddr($66) 
// ^^^^ 16bit ^^^^
constant gSequenceStrideCounter($68)

constant gCurCommand1(      $6A)
constant gCurCommand2(      $6B)
constant gCurCommand3(      $6C)
constant gCurCommandAdvance($6D)
constant gCurCommandAdvHigh($6E)


constant gPrevGateTime(    $75 )
constant gNextKeyOnFlags(  $76 )
constant gNextKeyOffFlags( $77 )
constant gRegOffsetTemp(   $78 )
constant gLongTemp1(       $7A )

// 90h-9Eh トラックズレチェック情報アウト領域
constant gLoopCheckOutHead( $90 )
constant gLoopCheckOutLast( $9E )
constant gGlobalTickCount(  $A0 )
// A2h- レベルメーター用ワーク
constant gLvMeterCurTrack(  $A2 )
constant gLvMeterLeftSum(   $A4 )
constant gLvMeterRightSum(  $A6 )


// 0100h-01FFh はスタック領域なので潰さないように注意

// 状態保持エリア
constant gTrackStatusesBase( $280)
constant gRegisterShadowBase($300)

// トラックステート内のオフセット
constant tofs_PlayHeadPos($00)
constant tofs_RemainDur($02)
constant tofs_RemainGT($06)
constant tofs_LPanFactor($08)
constant tofs_RPanFactor($09)
constant tofs_LoopCount($0A)

// Track Status Map
// BGM各トラックの状態エリア
// +--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+
// |+0       +1      |+2      |+3      |+4      |+5      |+6      |+8      |+9      |+10     |+11     |+12     |+13     |+14     |
// |Head position    |Rem dur |        |        |        |RemainGT|L_PanFct|R_PanFct|L_LoopCt|        |        |        |        |
// +--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+--------+

constant kTickMax(192)
constant kTieTick(255)
constant kCenterVolumeFactor($80)


// IO Register addresses
constant pSPCControl($F1)

constant pPokeAddr($F2)
constant pPokeVal($F3)

constant pIOPort0($F4)
constant pIOPort1($F5)
constant pIOPort2($F6)
constant pIOPort3($F7)



// in X: DSP port address
macro scope dsp_transfer_x() {
	stx pPokeAddr
	lda gRegisterShadowBase,x
	sta pPokeVal
}

macro scope add(v) {
	clc
	adc {v}
}

macro scope sub(v) {
	sec
	sbc {v}
}

macro save_axy() {
	pha
	phx
	phy
}

macro restore_axy() {
	ply
	plx
	pla
}


macro save_ax() {
	pha
	phx
}

macro restore_ax() {
	plx
	pla
}


macro left_shift_2() {
	asl
	asl
}
