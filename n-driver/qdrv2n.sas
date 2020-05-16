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



// �e���W���[�����ߍ��� =============
include "n-init.sas"
include "n-play.sas"
include "n-regshadow.sas"
include "n-commands.sas"
include "n-invoke.sas"
// ==================================
// �� �f�[�^�Z�N�V���� ��
// �ŏI�I�ɊO����f�[�^�𖄂߂邪�A�e
// �X�g�p�f�[�^�𖄂ߍ���ł���
// ==================================

// �o�C�i�����ڈ�p�f�[�^
db 0xEE,0xAA,0xEE,0xAA
// -------------------

// Bit-rise table
BitTable:
db 0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80

PanFactorTable:
	db 0x8F, 0x9F, 0xAF, 0xBF, 0xCF, 0xDF, 0xEF, 0xFF
PanFactorRevTable:
	db 0xFF, 0xEF, 0xDF, 0xCF, 0xBF, 0xAF, 0x9F, 0x8F

// �Đ����V�[�P���X�̊e�g���b�N�I�t�Z�b�g(WORD[])
SeqOffsetTable:
db 0xFF,0xFF
db 0xFF,0xFF
db 0xFF,0xFF
db 0xFF,0xFF

db 0xFF,0xFF
db 0xFF,0xFF
db 0xFF,0xFF
db 0xFF,0xFF

// �e�R�}���h�̃n���h��(���������ɐݒ�)
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
db 0x06

HeaderSEEnabled:
db 0x01

HeaderBGMTimerInterval:
db 0x5A

HeaderPauseAtBoot:
db 0x00

QuickLoadSize:
dw 0x0000

// �V�[�P���X�f�[�^
origin MusicSeqOrigin
include "assets/sample-seq.inc"
insert "assets/test-seq-se.bin"

// �V�[�P���X�f�B���N�g��
// 2B x nTracks
origin SeqDirOrigin
// BGM�e�g���b�N�̃I�t�Z�b�g
SeqDirTable:
include "assets/sample-offsets.inc"

// SE�̃I�t�Z�b�g
SeqDirSEPart:
db 0x88,0x02 // S0
db 0xFF,0xFF // S1(terminate)

// ���F(BRR�ԍ�+ADSR)
// SRCN, ADSR(1), ADSR(2), 00h
origin InstDirOrigin
InstPresetTable:
db 0x00,0xAD,0x30,0x00

// BRR�A�h���X�e�[�u��
origin BRRDirOrigin
insert "assets/test-brrdir.bin"

// �s�b�`�W���e�[�u��
origin FqTableOrigin
FqRegTable:
insert "assets/test-fqt.bin"

// BRR���f�[�^�̈�
origin BRRBodyOrigin
insert "assets/newpiano-brrset.bin"

origin TailPadding
	db "------TAIL------"
