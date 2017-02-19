#include "MMLUtility.h"

// Note Indices
// ==============================================
// 0   1   2   3   4   5   6   7   8   9   10  11
//     |       |           |       |       |
// | - | - | - | - | - | - | - | - | - | - | - |
// C   C+  D   D+  E   F   F+  G   G+  A   A+  B
// Do  Do# Re  Re# Mi  Fa  Fa# So  So# La  La# Si

//                                       A  B   C  D  E  F  G
static const int sKeyToIndexTable[7] = { 9, 11, 0, 2, 4, 5, 7 }; // start from A


// ���� c,d,e... ���C���f�b�N�X�ɕϊ�(0-11)
// c- �� -1 ��Ԃ��A���̃I�N�^�[�u��b�Ƃ��Ĉ����B
int fromKeyNameToIndex(const std::string& name) {
	char k = name.at(0);
	int i;

	if (k == 'r' || k == 'R') {
		return KEY_NAME_REST;
	}

	if (k >= 'a' && k <= 'g') {
		i = sKeyToIndexTable[ (int)(k - 'a') ];
	} else if (k >= 'A' && k <= 'G') {
		i = sKeyToIndexTable[ (int)(k - 'A') ];
	} else {
		return KEY_NAME_NOT_FOUND;
	}

	// +/- ���t���Ă��邩�H�i���̑O�ɁA2�����ڂ����邩�`�F�b�N�j
	if (name.size() > 1) {
		char z = name.at(1);
		if (z == '+') {
			++i;
		} else if (z == '-') {
			--i;
		}
	}

	return i;
}

// ����(N������+�t�_)���g�[�N���񂩂琶��
NoteLength calcNoteLengthFromTokens(const MMLTokenList& tokList, int startPos) {
	NoteLength nl;
	nl.dots = 0;
	nl.N = 0;
	nl.tuplet = 0;

	const int nTokens = (int)(tokList.size());
	if (nTokens > startPos) {
		for (int i = startPos; i < nTokens; ++i) {
			const MMLTokenStruct& tk = tokList[i];
			if (TT_INTEGER == tk.type) {
				nl.N = tk.intVal;
			} else if (TT_DOTS == tk.type) {
				nl.dots = tk.intVal;
			}
		}
	}

	return nl;
}

// TickCount - - - - - - - - -
// �� Tick����192=�S�����Ƃ��Đ�Β��ŕ\�����A7bit�Ɏ��߂邽�߂�2������(96)��蒷���l�ɂ͉��H������
// �������F modified_tick = tick/2 + 25
// ���t���F tick = (modified_tick - 25) * 2
// 1  = 192 -> 121
// 2..= 168 -> 109
// 2. = 144 -> 97
// 2  = 96�i���̂܂܂�OK�j
// 4  = 48�i���̂܂܂�OK�j

#define TICK_MAX 192

int calcTickCount(const NoteLength& nl) {
	int t = TICK_MAX / nl.N;

	int subLen = t / 2;
	for (int i = 0; i < nl.dots; ++i) {
		t += subLen;

		subLen /= 2;
	}

	return t;
}

uint8_t generateCompressedTicks(DriverTick ot) {
	if (ot <= 96) {
		return (uint8_t)ot;
	}

	int mt = (ot / 2) + 25;
	if (mt > 126) { mt = 126; }

	return (uint8_t)mt;
}

// For Q and Velocity bits
uint8_t generateCompressedQ(int q) {
	// �Q�[�g�^�C����3bits(00h-07h)�ɉ������߂�
	// MAX 16 -> 7
	// MIN 1  -> 0
	if (q > 16) { q = 16; }
	else if (q < 1) { q = 1; }

	return (q - 1) / 2;
}
uint8_t generateCompressedVelocity(int v) {
	// �x���V�e�B��4bits(00h-0Fh)�ɉ������߂�
	if (v > 16) { v = 16; }
	else if (v < 1) { v = 1; }

	return v - 1;
}

uint8_t generateQVbits(int rawQ, int rawV) {
	// Q 3bit, V 4bit ��OR���ĕԂ�
	// �ŏ�ʃr�b�g�͏��0�i1�ł���Ύ��̃m�[�g�Ƃ݂Ȃ����j

	return (generateCompressedQ(rawQ) << 4) | generateCompressedVelocity(rawV);
}



std::string cleanStringLiteral(const std::string& original) {
	std::regex re1("^\"");
	std::regex re2("\"$");

	std::string mid = std::regex_replace(original.c_str(), re1, "");
	return std::regex_replace(mid.c_str(), re2, "");
}