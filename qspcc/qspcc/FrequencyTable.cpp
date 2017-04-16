#include <math.h>
#include <stdio.h>
#include "FrequencyTable.h"

// Še‰¹ŠK‚Ìü”g”‚ğŒvZ
RawFqList generateNotesFqTable(int startOctave, int nOctaves, bool verbose) {
	RawFqList ls;
	if (verbose) {
		fprintf(stderr, "Base octave=%d\n", startOctave);
	}

	for (int k = 0; k < nOctaves; ++k) {
		for (int i = 0; i < 12; ++i) {
			const double f = (55.0*0.5) * pow(2.0, (12.0*(double)(startOctave + k) + 3.0 + (double)i) / 12.0); // A+3
			ls.push_back(f);
		}
	}

	return ls;
}

void dumpRawFqTable(const RawFqList& ls) {
	int oct = 0;

	const size_t n = ls.size();
	for (size_t i = 0; i < n; ++i) {
		if ((i % 12) == 0) {
			fprintf(stderr, "%d | ", oct);
		}

		const double& f = ls[i];
		if (f < 100.0) { fprintf(stderr, " "); }
		if (f < 1000.0) { fprintf(stderr, " "); }

		fprintf(stderr, "%4.2f", f);

		if ((i % 12) == 11) {
			++oct;
			fprintf(stderr, "\n");
		} else {
			fprintf(stderr, " ");
		}
	}
}

// Še‰¹ŠK‚Ìü”g”‚Æƒx[ƒXü”g”‚Ì”ä‚ğŒvZ
FqFactorList generateFqFactorTable(const RawFqList& inList, double baseFq) {
	FqFactorList retList;

	const size_t n = inList.size();
	for (size_t i = 0; i < n; ++i) {
		const double r = inList[i] / baseFq;
		retList.push_back(r);
	}

	return retList;
}

void dumpFqFactorTable(const FqFactorList& ls) {
	const size_t n = ls.size();

	for (size_t i = 0; i < n; ++i) {
		fprintf(stderr, "%f", ls[i]);

		if ((i % 12) == 11) {
			fprintf(stderr, "\n");
		} else {
			fprintf(stderr, " ");
		}
	}
}


// DSP—pƒŒƒWƒXƒ^’l¶¬
FqRegisterValueList generateFqRegisterValueTable(const FqFactorList& inList) {
	FqRegisterValueList reg_ls;

	const int org = 4096;

	const size_t n = inList.size();
	for (size_t i = 0; i < n; ++i) {
		const int rv = (int)(0.4 + inList[i] * (double)org);
		if (rv > 0x3FFF) {
			reg_ls.push_back(0x3FFF);
		} else {
			reg_ls.push_back(rv);
		}
	}

	return reg_ls;
}


void dumpFqRegisterValueTable(const RawFqList& rawFqList, const FqRegisterValueList& ls) {
	int oct = 0;
	fprintf(stderr, "--+-------------------------------------------------------------------------------------------------\n");

	const size_t n = ls.size();
	for (size_t i = 0; i < (n*2); ++i) {
		if ((i % 24) == 0) {
			fprintf(stderr, "%d | ", oct);
		} else if ((i % 24) == 12) {
			fprintf(stderr, "  | ", oct);
		}

		bool phase = (i % 24) > 11;
		int entryIndex = (oct * 12) + (i % 12);

		const double& f = rawFqList[entryIndex];
		const unsigned short& r = ls[entryIndex];

		if (!phase) {
			if (f < 100.0) { fprintf(stderr, " "); }
			if (f < 1000.0) { fprintf(stderr, " "); }
			fprintf(stderr, "%4.2f", f);
		} else {
			fprintf(stderr, " %04X  ", r);
		}

		if ((i % 12) == 11) {
			fprintf(stderr, "\n");
		} else {
			fprintf(stderr, " ");
		}

		if ((i % 24) == 23) {
			++oct;
			fprintf(stderr, "--+-------------------------------------------------------------------------------------------------\n");
		}
	}

	const int dataSize = n * 2;
	fprintf(stderr, "Table size: %d(%4Xh)Bytes\n", dataSize, dataSize);
}