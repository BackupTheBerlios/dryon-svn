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

#ifndef __MATCH_H
#define __MATCH_H

#if defined(__FreeBSD__)
#include <sys/types.h>
#endif

#include <string>
#include "config.h"

using namespace std;

bool match_expr(const string &mask, const string &str, string *results= NULL, uint maxresults= 0);
int wc_match(const string &_wildcard, const string &_target);

#endif // __MATCH_H

/**/

