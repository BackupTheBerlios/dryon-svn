/*
********************************************************************************
*                                 Dryon project
********************************************************************************
*
*  This is free software under the GPL license.
*  Since this software uses Small from Compuphase, it also use its license.
*  Copy of the two licenses are in the main dir (gpl.txt and amx_license.txt
*  Coded by Ammous Julien known as Anthalir
*
********************************************************************************
*/

#ifndef _UTILS_H
#define _UTILS_H

#include "time.h"
#include <string>
#include <vector>


using namespace std;

int createFolder(const char *path);
bool fileExist(const char *path);

bool is_numeric(char c);
void changeEXT(char *src, char *newext);
void TokenizeWithStr(const string& str, vector<string>& tokens, const string& str_delimiter);
int parseDateTime(string date_str, string time_str, struct tm &ret);

//bool matchregexp(const char *regexp, const char *str);
string trim(const string &s);
string buildFilename(const string &base, const string &ext);

int MakeConnection(const char *host, int port);

#endif // _UTILS_H


/**/

