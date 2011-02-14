#ifndef LOGFLOAT_H
#define LOGFLOAT_H

int LogFloatInit();
void LogFloatClose();

void LogFloatResults(char *fn);

float LogFloat(float f, char *id, int index, int precision);
#endif
