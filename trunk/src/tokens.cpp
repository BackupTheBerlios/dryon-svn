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

#include "config.h"
#include <vector>
#include <string>

using namespace std;

void Tokenize(const string& str, vector<string>& tokens, const string& delimiters)
{
	tokens.clear();

    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while( (pos != string::npos)  || (lastPos != string::npos ) )
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos= str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos= str.find_first_of(delimiters, lastPos);
    }
}

// tokenize with a string as a delimiter
void TokenizeWithStr(const string& str, vector<string>& tokens, const string& str_delimiter)
{
	tokens.clear();
	unsigned int start= 0, end;

	end= str.find(str_delimiter, 0);

	while( start < str.length() )
	{
		tokens.push_back(str.substr(start, end - start));
		start= end + str_delimiter.length();
		end= str.find(str_delimiter, start);
		if( end==string::npos )
			end= str.size();
	}
}


/**/


