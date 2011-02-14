//LogFloat.c
//Logs max, min and precision of a floating point value
#include <stdio.h>
#include "LogFloat.h"

struct LogFloatEntry {
	char id[128];
	float min, max, precision;
	int counter;
};

#define MAX_LF_ENTRIES 1000
struct LogFloatEntry lf_entries[MAX_LF_ENTRIES];
int	lf_total=0;

_inline struct LogFloatEntry *_lfAdd(char *id) {
	//Adds a new entry.
	//NULL == Array full.

	if (lf_total==MAX_LF_ENTRIES) return NULL;

	strcpy(lf_entries[lf_total].id, id);
	lf_entries[lf_total].max=lf_entries[lf_total].min=0.0f;
	lf_entries[lf_total].precision=1.0f;
	lf_entries[lf_total].counter=0;

	return &lf_entries[lf_total++];
}

_inline struct LogFloatEntry *_lfFind(char *id) {
	//Finds an existing entry.
	//NULL == No matching id

	struct LogFloatEntry *cur=&lf_entries[0], *last=&lf_entries[lf_total];
	while (cur<=last) {
		//Change this is case insensitive matching is desired.
		if (!strcmp(id, cur->id)) return cur;
		cur++;
	}

	return NULL;
}

_inline struct LogFloatEntry *_lfGet(char *id) {
	//Gets an entry. Returns an entry if it already exists, otherwise a new entry is created.
	struct LogFloatEntry *cur=_lfFind(id);

	if (!cur) cur=_lfAdd(id);
	return cur;
}

int LogFloatInit() {

	lf_total=0;
	return 1;
}

void LogFloatClose() {
	lf_total=0;
}

void LogFloatResults(char *fn) {
	int i;
	FILE *f=fopen(fn, "w+");

	if (f) {
		fprintf(f, "ID\tMin\tMax\tPrecision\tCount\n\n");
		for (i=0; i<lf_total; i++) 
			fprintf(f, "%s:\t%f\t%f\t%e\t%d\n", lf_entries[i].id, lf_entries[i].min, lf_entries[i].max, lf_entries[i].precision, lf_entries[i].counter);
		fprintf(f, "\nTotal items logged: %d", lf_total);

		fclose(f);
	}
}

float LogFloat(float f, char *id, int index, int precision) {
	//Params:
	//	      f: Float value to be logged
	//       id: string ID (name) of the float variable
	//    index: index of f in the event that f is a member of an array. Specify index < 0 for no index.
	//precision: Indicates max precision truncation to be applied to the return float. If no precision
	//           truncating is desired, set precision < 0.

	//Return value:
	//if precision >=0 && <= 32, return value is f truncated to the fixed point precision "precision".
	//otherwise return value == f.

	struct LogFloatEntry *cur;
	float tmp;

	if (index>=0) {		
		char newId[256];
		sprintf(newId, "%s[%d]", id, index);
		cur=_lfGet(newId);
	} else cur=_lfGet(id);

	if (cur) {
		cur->counter++;
		if (cur->max<f) cur->max=f;
		if (cur->min>f) cur->min=f;
		tmp=(float)fabs(f);
		if (tmp<1.0f&&f!=0.0f)
			if (tmp<cur->precision) cur->precision=tmp;
	} //else Error: we ran out of entry storage

	if (precision>=0) {
		float f2=(float)(2^precision);
		tmp=f*f2;
		return tmp/f2;
	}

	return f;
}