#ifndef PLUGINHEADER_H
#define PLUGINHEADER_H

/*                             FUNCTION PROTOTYPES                           */
int setParams(const char* buffer);
int getParams(char* buffer, int bufferSize);
int run();
const char* queryParamInfo();
clock_t getRunTime();

#endif
