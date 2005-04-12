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

#ifndef CFG_H
#define CFG_H

#include <string>
#include <map>


using namespace std;

class ConfigFileReader
{
protected:
	char filename[100];
	map<string,string> data;

public:
	ConfigFileReader(const char *);
	ConfigFileReader(){}
	~ConfigFileReader();
	void setFile(const char*);
	int readFile();

	bool isDefined(const char*);

	int readIntKey(const char*);
	string readStringKey(const char*);
	char readCharKey(const char*, int);
	bool readBoolKey(const char*);
};

// used to save plugin data
class DataFile : public ConfigFileReader
{
public:
	void saveFile();
	void setStringKey(string,string);
};

#endif // CFG_H

/**/


