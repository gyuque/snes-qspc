#include "testers.h"

#include "../MusicDocument.h"
#include "../FrequencyTable.h"
#include "../spcfile/SPCExporter.h"

void doDocumentTest(Embedder* pEmbedder) {
	MusicDocument doc;

	for (int i = 0; i < 4; ++i) {
		MusicTrack* t = doc.appendTrack();
		t->addByte(0x8C);
		t->addByte(48);
		t->addByte(0x7F);
	}

	doc.generateSequenceImage(true);
	doc.dumpSequenceBlob();

	BytesSourceProxy* pSeqSrc = doc.referSequenceBytesSource();
	fprintf(stderr, "<%d> %02X %02X\n", pSeqSrc->esGetSize(), pSeqSrc->esGetAt(256), pSeqSrc->esGetAt(257));

	pEmbedder->embed(true, nullptr, nullptr, pSeqSrc, nullptr, nullptr, nullptr);
//	pEmbedder->exportToFile("test-out.bin");
}

void doFrequencyTableTest() {
	RawFqList fq_ls = generateNotesFqTable(1, 6, true);
	//dumpRawFqTable(fq_ls);
	FqFactorList r_ls = generateFqFactorTable(fq_ls, 500.0);
	//dumpFqFactorTable(r_ls);

	FqRegisterValueList regval_list = generateFqRegisterValueTable(r_ls);
	dumpFqRegisterValueTable(fq_ls, regval_list);
}

void doSPCExporterTest() {
	SPCExporter ex;
	ex.exportToFile("_TestOut-SPC.bin");
}