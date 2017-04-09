arch snes.smp
include "s-inc.inc"
base ProgramOrigin
output "spcd2.bin", create

// --------------------- STARTUP ---------------------
// SANITYCHECK
//lda #0
//sta $0101
//insane_loop:
//lda $0100
//bne insane_loop


// レジスタ等初期化 → 初期化完了通知 → メインループの順
jsr initializeAll
jsr initializeNotifyReady
jsr doMainLoop
// -----------------------------------------------------

// MODULES - - - - - - - - -
include "q-main.sas"
include "q-init.sas"
include "q-shadow.sas"
include "q-commands.sas"

// DATA - - - - - - - - - - - - - - - - - -

BitTable:
db 0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80

PanFactorTable:
	db 0x8F, 0x9F, 0xAF, 0xBF, 0xCF, 0xDF, 0xEF, 0xFF
PanFactorRevTable:
	db 0xFF, 0xEF, 0xDF, 0xCF, 0xBF, 0xAF, 0x9F, 0x8F
	
BGMCmdJumpTable:
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

dw 0xCDCD

// DATA SECTIONS (Located) - - - - - - - - -

// Header and meta data
origin MusicHeaderOrigin
HeaderTracksCountByte:
db 0x03
HeaderTrackStride:
db 0x01
HeaderTimerIntervalByte:
db 83

origin MusicSeqOrigin
db 0x00,0x01,0x02,0x03

// 音色(BRR番号+ADSR)
// SRCN, ADSR(1), ADSR(2), 00h
origin InstDirOrigin
InstPresetTable:
db 0x00,0xAD,0x30,0x00

// BRRアドレステーブル
origin BRRDirOrigin
db 0x00,0x28,0x00,0x28

origin FqTableOrigin
FqRegTable:
db 0x00,0x10,0x00,0x10

origin BRRBodyOrigin
db 0x03,0x00,0x00,0x00

origin TailPadding
	db "------TAIL------"
