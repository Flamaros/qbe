#include "../all.h"

typedef struct Amd64Op Amd64Op;

enum Amd64Reg {														   // win64 abi
	RAX = RXX+1, /* caller-save */				   // RAX		/* caller-save */
	RCX,										   // RCX
	RDX,										   // RDX
	RSI,										   // R8
	RDI,										   // R9
	R8,											   // R10
	R9,											   // R11
	R10,										   // XMM0
	R11,										   // XMM1
												   // XMM2
	RBX, /* callee-save */						   // XMM3
	R12,										   // XMM4
	R13,										   // XMM5
	R14,										   //
	R15,										   // RBX		/* callee-save */
												   // RBP
	RBP, /* globally live */					   // RDI
	RSP,										   // RSI
												   // RSP
	XMM0, /* sse */								   // R12
	XMM1,										   // R13
	XMM2,										   // R14
	XMM3,										   // R15
	XMM4,										   // XMM6
	XMM5,										   // XMM7
	XMM6,										   // XMM8
	XMM7,										   // XMM9
	XMM8,										   // XMM10
	XMM9,										   // XMM11
	XMM10,										   // XMM12
	XMM11,										   // XMM13
	XMM12,										   // XMM14
	XMM13,										   // XMM15
	XMM14,										   //
	XMM15,										   //
												   //
	NFPR = XMM14 - XMM0 + 1, /* reserve XMM15 */   //
	NGPR = RSP - RAX + 1,						   //
	NGPS = R11 - RAX + 1,						   //
	NFPS = NFPR,								   //
	NCLR = R15 - RBX + 1,

	NFPR_WIN64 = XMM14 - XMM6 + 1, /* reserve XMM15 */
	NGPR_WIN64 = 32, // @Xavier need I count XMM15
	NGPS_WIN64 = 13, // @Xavier caller-save?
	NFPS_WIN64 = NFPR_WIN64,
	NCLR_WIN64 = 19, // @Xavier callee-save ? Need I count XMM15 which is reserved?
};
MAKESURE(reg_not_tmp, XMM15 < (int)Tmp0);

struct Amd64Op {
	char nmem;
	char zflag;
	char lflag;
};

/* targ.c */
extern Amd64Op amd64_op[];

/* sysv.c (abi) */
extern int amd64_sysv_rsave[];
extern int amd64_sysv_rclob[];
bits amd64_sysv_retregs(Ref, int[2]);
bits amd64_sysv_argregs(Ref, int[2]);
void amd64_sysv_abi(Fn *);

/* win64.c (abi) */
extern int amd64_win64_rsave[];
extern int amd64_win64_rclob[];
bits amd64_win64_retregs(Ref, int[2]);
bits amd64_win64_argregs(Ref, int[2]);
void amd64_win64_abi(Fn*);

/* isel.c */
void amd64_isel(Fn *);

/* emit.c */
void amd64_emitfn(Fn *, FILE *);

/* win64_emit.c */
void amd64_win64_emitfn(Fn* fn, FILE* f);
