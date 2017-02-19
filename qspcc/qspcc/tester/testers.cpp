#include "testers.h"

#include "../MusicDocument.h"

void doDocumentTest(Embedder* pEmbedder) {
	MusicDocument doc;

	for (int i = 0; i < 4; ++i) {
		MusicTrack* t = doc.appendTrack();
		t->addByte(0x8C);
		t->addByte(48);
		t->addByte(0x7F);
	}

	doc.generateSequenceImage();
	doc.dumpSequenceBlob();

	BytesSourceProxy* pSeqSrc = doc.referSequenceBytesSource();
	fprintf(stderr, "<%d> %02X %02X\n", pSeqSrc->esGetSize(), pSeqSrc->esGetAt(256), pSeqSrc->esGetAt(257));

	pEmbedder->embed(pSeqSrc);
//	pEmbedder->exportToFile("test-out.bin");
}
