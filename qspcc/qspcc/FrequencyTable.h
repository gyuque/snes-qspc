#ifndef FREQUENCYTABLE_H_INCLUDED
#define FREQUENCYTABLE_H_INCLUDED

#include <vector>

typedef std::vector<double> RawFqList;
typedef std::vector<double> FqFactorList;
typedef std::vector<unsigned short> FqRegisterValueList;
RawFqList generateNotesFqTable(int startOctave, int nOctaves, bool verbose);
FqFactorList generateFqFactorTable(const RawFqList& inList, double baseFq);
FqRegisterValueList generateFqRegisterValueTable(const FqFactorList& inList);

void dumpRawFqTable(const RawFqList& ls);
void dumpFqFactorTable(const FqFactorList& ls);
void dumpFqRegisterValueTable(const RawFqList& rawFqList, const FqRegisterValueList& ls);

#endif