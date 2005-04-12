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

#ifndef __TOKENS_H
#define __TOKENS_H

void Tokenize(const string& str, vector<string>& tokens, const string& delimiters);
void TokenizeWithStr(const string& str, vector<string>& tokens, const string& str_delimiter);

#endif // __TOKENS_H
/**/

