#include "testers.h"

#include "../MusicDocument.h"

void doDocumentTest() {
	MusicDocument doc;

	for (int i = 0; i < 4; ++i) {
		MusicTrack* t = doc.appendTrack();
		t->addByte(0x8C);
		t->addByte(48);
		t->addByte(0x7F);
	}

	doc.generateSequenceImage();
	doc.dumpSequenceBlob();
}
