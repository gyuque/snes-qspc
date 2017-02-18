#ifndef MMLUTILITY_H_INCLUDED
#define MMLUTILITY_H_INCLUDED

#include <string>
#include <stdint.h>
#include "MMLTokenizer.h"

#define KEY_NAME_NOT_FOUND (-99)
#define KEY_NAME_REST      (999)

int fromKeyNameToIndex(const std::string& name);
NoteLength calcNoteLengthFromTokens(const MMLTokenList& tokList, int startPos);
int calcTickCount(const NoteLength& nl);
uint8_t generateCompressedTicks(DriverTick ot);

// For Q and Velocity bits
uint8_t generateCompressedQ(int q);
uint8_t generateCompressedVelocity(int v);
uint8_t generateQVbits(int rawQ, int rawV);

#endif