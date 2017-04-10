#ifndef MML_TYPES_H_INCLUDED
#define MML_TYPES_H_INCLUDED

#include <stdint.h>
#include <vector>
typedef std::vector<uint8_t> ByteList;

#define PANPOT_MIN  0
#define PANPOT_MID  8
#define PANPOT_MAX 16

typedef struct _NoteLength {
	int N;
	int dots;
	int tuplet;
} NoteLength;



typedef enum _MMLTokenType {
	TT_BLANK,
	TT_TEMPO_CMD,
	TT_NSHIFT_CMD,
	TT_OCT_CMD,
	TT_LEN_CMD,
	TT_VELO_CMD,
	TT_QSET_CMD,
	TT_OINC_CMD,
	TT_ODEC_CMD,
	TT_NOTE,
	TT_INTEGER,
	TT_NEG_INT,
	TT_DOTS,
	TT_TERM,  //  ;
	TT_SLASH, //  /

	TT_CMB_START, // {
	TT_CMB_END,   // }

	TT_LcREP_START, // /:
	TT_LcREP_END,   // :/

	TT_MAC_IDENT,
	TT_PANPOT,  // @p
	TT_EQUAL,   // =
	TT_ATMARK,  // @
	TT_AMP,     // &
	TT_STRLIT,
	TT_USING,
	TT_TITLE,
	TT_ARTIST,
	TT_OCTREV
} MMLTokenType;

typedef enum _MMLExpressionType {
	MX_TEMPO,
	MX_NSHIFT,
	MX_INSTCHG,
	MX_OCTSET,
	MX_LENSET,
	MX_VELOSET,
	MX_QSET,
	MX_OINC,
	MX_ODEC,
	MX_NOTE,
	MX_PANPOT,
	MX_TERM,
	MX_SLASH,

	MX_CMB_START, // {
	MX_CMB_END,   // }

	MX_LcREP_START, // /:n
	MX_LcREP_END,   // :/

	MX_USINGDECL,
	MX_TITLEDECL,
	MX_ARTISTDECL,
	MX_MACRODEF,
	MX_MACROUSE
} MMLExpressionType;



typedef enum {
	INSTLD_OK = 0,

	INSTLD_NOT_SET = -1,
	INSTLD_DIR_NOTFOUND = -2,
	INSTLD_MANIFEST_NOTFOUND = -3,
	INSTLD_BAD_MANIFEST = -4,
	INSTLD_BRR_NOTFOUND = -5
} InstLoadResult;

#endif