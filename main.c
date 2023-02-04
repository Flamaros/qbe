#include "all.h"
#if defined(_WIN32)
#	include "win32_config.h"
#else
#	include "config.h"
#endif
#include <ctype.h>
#include <getopt.h>

Target T;
AsmTarget AT;

char debug['Z'+1] = {
	['P'] = 0, /* parsing */
	['M'] = 0, /* memory optimization */
	['N'] = 0, /* ssa construction */
	['C'] = 0, /* copy elimination */
	['F'] = 0, /* constant folding */
	['A'] = 0, /* abi lowering */
	['I'] = 0, /* instruction selection */
	['L'] = 0, /* liveness */
	['S'] = 0, /* spilling */
	['R'] = 0, /* reg. allocation */
};

#if defined(_WIN32)
extern Target T_amd64_win64;
extern AsmTarget AT_nasm;

static Target* tlist[] = {
	&T_amd64_win64,
	0
};
#else
extern Target T_amd64_sysv;
extern Target T_arm64;
extern Target T_rv64;
extern AsmTarget AT_gas;

static Target *tlist[] = {
	&T_amd64_sysv,
	&T_arm64,
	&T_rv64,
	0
};
#endif

static FILE *outf;
static int dbg;

static void
data(Dat *d)
{
	if (dbg)
		return;
	if (d->type == DEnd) {
		fprintf(outf, "%send data%s\n\n", AT.start_comment, AT.end_comment);
		freeall();
	}
	AT.emitdat(d, outf);
}

static void
func(Fn *fn)
{
	uint n;

	if (dbg)
		fprintf(stderr, "**** Function %s ****", fn->name);
	if (debug['P']) {
		fprintf(stderr, "\n> After parsing:\n");
		printfn(fn, stderr);
	}
	fillrpo(fn);
	fillpreds(fn);
	filluse(fn);
	memopt(fn);
	filluse(fn);
	ssa(fn);
	filluse(fn);
	ssacheck(fn);
	fillalias(fn);
	loadopt(fn);
	filluse(fn);
	ssacheck(fn);
	copy(fn);
	filluse(fn);
	fold(fn);
	T.abi(fn);
	fillpreds(fn);
	filluse(fn);
	T.isel(fn);
	fillrpo(fn);
	filllive(fn);
	fillloop(fn);
	fillcost(fn);
	spill(fn);
	rega(fn);
	fillrpo(fn);
	simpljmp(fn);
	fillpreds(fn);
	fillrpo(fn);
	assert(fn->rpo[0] == fn->start);
	for (n=0;; n++)
		if (n == fn->nblk-1) {
			fn->rpo[n]->link = 0;
			break;
		} else
			fn->rpo[n]->link = fn->rpo[n+1];
	if (!dbg) {
		T.emitfn(fn, outf);
		AT.emitfntail(fn->name, outf);
		fprintf(outf, "%send function %s%s\n\n", AT.start_comment, fn->name, AT.end_comment);
	} else
		fprintf(stderr, "\n");
	freeall();
}

int
main(int ac, char *av[])
{
	Target **t;
	FILE *inf, *hf;
	char *f, *sep;
	int c, asmmode;

	asmmode = Defasm;
	T = Deftgt;
	AT = Defasmtgt;
	outf = stdout;
	while ((c = getopt(ac, av, "hd:o:G:t:")) != -1)
		switch (c) {
		case 'd':
			for (; *optarg; optarg++)
				if (isalpha(*optarg)) {
					debug[toupper(*optarg)] = 1;
					dbg = 1;
				}
			break;
		case 'o':
			if (strcmp(optarg, "-") != 0) {
				outf = fopen(optarg, "w");
				if (!outf) {
					fprintf(stderr, "cannot open '%s'\n", optarg);
					exit(1);
				}
			}
			break;
		case 't':
			if (strcmp(optarg, "?") == 0) {
				puts(T.name);
				exit(0);
			}
			for (t=tlist;; t++) {
				if (!*t) {
					fprintf(stderr, "unknown target '%s'\n", optarg);
					exit(1);
				}
				if (strcmp(optarg, (*t)->name) == 0) {
					T = **t;
					break;
				}
			}
			break;
		case 'G':
			if (strcmp(optarg, "e") == 0)
				asmmode = Gaself;
			else if (strcmp(optarg, "m") == 0)
				asmmode = Gasmacho;
			else {
				fprintf(stderr, "unknown gas flavor '%s'\n", optarg);
				exit(1);
			}
			break;
		case 'h':
		default:
			hf = c != 'h' ? stderr : stdout;
			fprintf(hf, "%s [OPTIONS] {file.ssa, -}\n", av[0]);
			fprintf(hf, "\t%-11s prints this help\n", "-h");
			fprintf(hf, "\t%-11s output to file\n", "-o file");
			fprintf(hf, "\t%-11s generate for a target among:\n", "-t <target>");
			fprintf(hf, "\t%-11s ", "");
			for (t=tlist, sep=""; *t; t++, sep=", ")
				fprintf(hf, "%s%s", sep, (*t)->name);
			fprintf(hf, "\n");
			fprintf(hf, "\t%-11s generate gas (e) or osx (m) asm\n", "-G {e,m}");
			fprintf(hf, "\t%-11s dump debug information\n", "-d <flags>");
			exit(c != 'h');
		}

	AT.init(asmmode);

	do {
		f = av[optind];
		if (!f || strcmp(f, "-") == 0) {
			inf = stdin;
			f = "-";
		} else {
			inf = fopen(f, "r");
			if (!inf) {
				fprintf(stderr, "cannot open '%s'\n", f);
				exit(1);
			}
		}
		parse(inf, f, data, func);
		fclose(inf);
	} while (++optind < ac);

	if (!dbg) {
		AT.emitfin(outf);
	}

	exit(0);
}
