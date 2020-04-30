arch snes.smp
include "n-inc.inc"
base ProgramOrigin
output "spcd2n.bin", create

// --------------------- STARTUP ---------------------
lda #$BE
sta $40
lda #$EF
sta $41

jsr initializeAll
jsr initializeNotifyReady
jsr doMainLoop



// 各モジュール埋め込み =============
include "n-init.sas"
include "n-play.sas"
include "n-regshadow.sas"
include "n-commands.sas"
include "n-invoke.sas"
// ==================================
// ▼ データセクション ▼
// 最終的に外からデータを埋めるが、テ
// スト用データを埋め込んでおく
// ==================================

// バイナリ中目印用データ
db 0xEE,0xAA,0xEE,0xAA
// -------------------

// Bit-rise table
BitTable:
db 0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80

PanFactorTable:
	db 0x8F, 0x9F, 0xAF, 0xBF, 0xCF, 0xDF, 0xEF, 0xFF
PanFactorRevTable:
	db 0xFF, 0xEF, 0xDF, 0xCF, 0xBF, 0xAF, 0x9F, 0x8F

// 再生中シーケンスの各トラックオフセット(WORD[])
SeqOffsetTable:
db 0xFF,0xFF
db 0xFF,0xFF
db 0xFF,0xFF
db 0xFF,0xFF

db 0xFF,0xFF
db 0xFF,0xFF
db 0xFF,0xFF
db 0xFF,0xFF

// 各コマンドのハンドラ(初期化時に設定)
SCmdJumpTable:
dw 0x0400,0x0400,0x0400,0x0400 // C8 C9 CA CB | C8:Tie C9:Rest
dw 0x0400,0x0400,0x0400,0x0400 // CC CD CE CF | 
dw 0x0400,0x0400,0x0400,0x0400 //  D0 D1 D2 D3| 
dw 0x0400,0x0400,0x0400,0x0400 //  D4 D5 D6 D7|  CA-DE: Unused
dw 0x0400,0x0400,0x0400,0x0400 //  D8 D9 DA DB| 
dw 0x0400,0x0400,0x0400,0x0400 //  DC DD DE DF| DF: Repeat(start)
dw 0x0400,0x0400,0x0400,0x0400 // E0 E1 E2 E3 | E0:SetInst, E1:Pan
dw 0x0400,0x0400,0x0400,0x0400 // E4 E5 E6 E7 |
dw 0x0400,0x0400,0x0400,0x0400 // E8 E9 EA EB | 
dw 0x0400,0x0400,0x0400,0x0400 // EC ED EE EF | EF: Repeat(back)
dw 0x0400,0x0400,0x0400,0x0400 //  F0 F1 F2 F3| 
dw 0x0400,0x0400,0x0400,0x0400 //  F4 F5 F6 F7|

// ----------------------------------
// Preset data (to be replaced)

origin MusicHeaderOrigin
HeaderNumTracks:
db 0x03

HeaderSEEnabled:
db 0x01

HeaderBGMTimerInterval:
db 0x5A

HeaderPauseAtBoot:
db 0x00

QuickLoadSize:
dw 0x0000

// シーケンスデータ
origin MusicSeqOrigin
insert "assets/test-seq.bin"
insert "assets/test-seq-t2.bin"
insert "assets/test-seq-t3.bin"
insert "assets/test-seq-se.bin"

// シーケンスディレクトリ
// 2B x nTracks
origin SeqDirOrigin
// BGM各トラックのオフセット
SeqDirTable:
db 0x00,0x00 // T0
db 0xD0,0x00 // T1
db 0x60,0x01 // T2
db 0x00,0x00 // T3

db 0x00,0x00 // T4
db 0x00,0x00 // T5
db 0x00,0x00 // T6
db 0x00,0x00 // T7

// SEのオフセット
SeqDirSEPart:
db 0xB0,0x01 // S0
db 0xFF,0xFF // S1

// 音色(BRR番号+ADSR)
// SRCN, ADSR(1), ADSR(2), 00h
origin InstDirOrigin
InstPresetTable:
db 0x00,0xAD,0x30,0x00

// BRRアドレステーブル
origin BRRDirOrigin
insert "assets/test-brrdir.bin"

// ピッチ係数テーブル
origin FqTableOrigin
FqRegTable:
insert "assets/test-fqt.bin"

// BRR実データ領域
origin BRRBodyOrigin
insert "assets/newpiano-brrset.bin"

origin TailPadding
	db "------TAIL------"
