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
#undef close

#include <iostream>
#include <fstream>
#include <string.h>
#include <stdio.h>
#include "cfg.h"
#include "match.h"
#include "log.h"



ConfigFileReader::ConfigFileReader(const char *fname)
{
	strncpy(filename, fname, sizeof(filename));
}

ConfigFileReader::~ConfigFileReader()
{
	//Debug("ConfigFileReader::~ConfigFileReader()\n");
}

bool ConfigFileReader::isDefined(const char *key)
{
	string skey= string(key);
	map<string,string>::iterator it;

	it= data.find(skey);
	if( it == data.end() )
		return false;
	else
		return true;
}

int ConfigFileReader::readIntKey(const char *key)
{
	string s= readStringKey(key);
	if( s=="undefined" )
		return 0;
	else
		return atoi(s.c_str());
}

string ConfigFileReader::readStringKey(const char *key)
{
	string skey= string(key);
	map<string,string>::iterator it;

	it= data.find(skey);
	if( it == data.end() )
		return "undefined";

	return (*it).second;
}

char ConfigFileReader::readCharKey(const char *key, int n)
{
	string tmp= readStringKey(key);
	return tmp[n];
}

bool ConfigFileReader::readBoolKey(const char *key)
{
	string tmp= readStringKey(key);
	if( (tmp == "true") || (tmp == "1") )
		return true;
	else
		return false;
}

void ConfigFileReader::setFile(const char *newfile)
{
	strncpy(filename, newfile, sizeof(filename));
}

// get data from the file
int ConfigFileReader::readFile()
{
	char line[2000];
	fstream file;

	file.open(filename, ios::in);
	if( file.bad() )
	{
		//Error("[CONFIG] Unable to read file %s\n", filename);
		return -1;
	}
	data.clear();

	// read all the lines
	while( file.getline(line, sizeof(line)-1) )
	{
		// comment or blank line
		if( ((line[0]=='/') && (line[1]=='/')) || !strcmp(line, "") )
			continue;

		string res[2];
		if( match_expr("* = *", line, res, 2) )
		{
			//Debug("[CONFIG(\"%s\")] %s(%s)\n", filename, res[0].c_str(), res[1].c_str());
			string &tmp= data[res[0]];
			tmp= res[1];

			// remove \r if exists
			uint n;
			if( (n= tmp.find('\r')) != string::npos )
				tmp.erase(n,1);
		}
	}

	file.close();
	return 0;
}


/*************************************/
/*************************************/

void DataFile::saveFile()
{
	if( data.size() > 0 )
	{
		// open the file and remove its content
		FILE *file= fopen(filename, "w");

		Output("Saving '%s' (%d vars)...\n", filename, data.size());

		for(map<string,string>::iterator it= data.begin(); it!=data.end(); it++ )
		{
			fprintf(file, "%s = %s\n", it->first.c_str(), it->second.c_str());
			//Debug("\"%s\" = \"%s\"\n", it->first.c_str(), it->second.c_str());
		}

		fclose(file);
	}
	else
	{
		//Output("Saving \"%s\"... nothing to save\n", filename);
	}
}

void DataFile::setStringKey(string key, string value)
{
	data[key]= value;
}


#ifdef TEST
int main()
{
	ConfigFileReader cfg("saves/test.sma.dat");
	cfg.readFile();
	//cout << "my_str: " << cfg.readStringKey("my_str") << endl;
	//cout << "irc_command_prefix: " << cfg.readStringKey("irc_command_prefix") << endl;
	//cout << "bob: " << cfg.readStringKey("bob") << endl << endl;
}
#endif

/**/


