#include "all.h"


void
nasminit(enum Asm asmmode)
{
}

void
nasmemitlnk(char* n, Lnk* l, char* s, FILE* f)
{
	if (l->sec) {
		fprintf(f, "\tSECTION %.*s", (int)strlen(l->sec) - 2, &l->sec[1]);
		if (l->secf)
			fprintf(f, ", %s", l->secf);
	}
	else {
		fputs(s, f);
	}
	fputc('\n', f);
	if (l->export)
		fprintf(f, "\tglobal %s\n", n);
	fprintf(f, "%s:\n", n);
}

void
nasmemitfntail(char* fn, FILE* f)
{
	//if (gasasm == Gaself) {
	//	fprintf(f, ".type %s, @function\n", fn);
	//	fprintf(f, ".size %s, .-%s\n", fn, fn);
	//}
}

void
nasmemitdat(Dat* d, FILE* f)
{
	static char* dtoa[] = {
		[DB] = "\tdb", // byte 8bits
		[DH] = "\tdw", // word 16bits (on all win platforms)
		[DW] = "\tdd", // double word 32 bits
		[DL] = "\tdq"  // quad word 64 bits
	};
	static int64_t zero;

	switch (d->type) {
	case DStart:
		zero = 0;
		break;
	case DEnd:
		if (zero != -1) {
			nasmemitlnk(d->name, d->lnk, ".bss", f);
			fprintf(f, "\t.fill %"PRId64",1,0\n", zero);
		}
		break;
	case DZ:
		if (zero != -1)
			zero += d->u.num;
		else
			fprintf(f, "\t.fill %"PRId64",1,0\n", d->u.num);
		break;
	default:
		if (zero != -1) {
			nasmemitlnk(d->name, d->lnk, ".data", f);
			if (zero > 0)
				fprintf(f, "\t.fill %"PRId64",1,0\n", zero);
			zero = -1;
		}
		if (d->isstr) {
			if (d->type != DB)
				err("strings only supported for 'b' currently");
			fprintf(f, "\tdb\t\t%s\n", d->u.str);
		}
		else if (d->isref) {
			fprintf(f, "%s %s%+"PRId64"\n",
				dtoa[d->type], d->u.ref.name,
				d->u.ref.off);
		}
		else {
			fprintf(f, "%s\t\t%"PRId64"\n",
				dtoa[d->type], d->u.num);
		}
		break;
	}
}

typedef struct Asmbits Asmbits;

struct Asmbits {
	char bits[16];
	int size;
	Asmbits* link;
};

static Asmbits* stash;

int
nasmstash(void* bits, int size)
{
	Asmbits** pb, * b;
	int i;

	assert(size == 4 || size == 8 || size == 16);
	for (pb = &stash, i = 0; (b = *pb); pb = &b->link, i++)
		if (size <= b->size)
			if (memcmp(bits, b->bits, size) == 0)
				return i;
	b = emalloc(sizeof * b);
	memcpy(b->bits, bits, size);
	b->size = size;
	b->link = 0;
	*pb = b;
	return i;
}

void
nasmemitfin(FILE* f)
{
	Asmbits* b;
	char* p;
	int sz, i;
	double d;

	if (!stash)
		return;
	fprintf(f, "; floating point constants\n.data\n");
	for (sz = 16; sz >= 4; sz /= 2)
		for (b = stash, i = 0; b; b = b->link, i++) {
			if (b->size == sz) {
				fprintf(f,
					".balign %d\n"
					"%sfp%d:",
					sz, AT.loc, i
				);
				for (p = b->bits; p < &b->bits[sz]; p += 4)
					fprintf(f, "\n\t.int %"PRId32,
						*(int32_t*)p);
				if (sz <= 8) {
					if (sz == 4)
						d = *(float*)b->bits;
					else
						d = *(double*)b->bits;
					fprintf(f, " ; %f\n", d);
				}
				else
					fprintf(f, "\n");
			}
		}
	while ((b = stash)) {
		stash = b->link;
		free(b);
	}
}

AsmTarget AT_nasm = {
	.name = "nasm",
	.loc = "",
	.sym = "",
	.start_comment = "; ",
	.end_comment = "",

	.init = nasminit,
	.emitlnk = nasmemitlnk,
	.emitfntail = nasmemitfntail,
	.emitdat = nasmemitdat,
	.stash = nasmstash,
	.emitfin = nasmemitfin,
};
