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

#include "natives.h"
#include "amx.h"
#include "utils.h"
#include "tokens.h"
#include <vector>
#include <string>
#include <bitset>
#include <algorithm>

/****h* Natives/StringsFuncs
* MODULE DESCRIPTION
*	string manipulation
***/

/****f* StringsFuncs/substr
* USAGE
*	substr(const str[], from, to, out[], out_maxsize= sizeof out)
* DESCRIPTION
*	extract part of a string
* INPUTS
*	str         : source string
*	from        : first character index
*	to          : last character index
*	out         : destination string
*	out_maxsize : Natives
* SINCE
*	0.1
***/
NATIVE(_substr)
{
	int i;
	char *original, *out;
	int from= params[2], to= params[3];

	CHECK(from < to, -1, "substr: from >= to !!\n", 0);

	copyStringFromAMX(amx, params[1], &original);

	out= new char[to-from+1];

	for(i= 0; (original[from+i]!='\0') && (i< to-from); i++)
		out[i]= original[from+i];

	i= (i>params[5])?params[5]:i;
	out[i]= '\0';
	copyStringToAMX(amx, params[4], out, params[5]-1);

	delete [] original;
	delete [] out;
	return 0;
}

/****f* StringsFuncs/strstr
* USAGE
*	strstr(const where[], const what[])
* DESCRIPTION
*	look for what in where and return starting index if found
* RETURN VALUE
*	integer : starting index if what is found, -1 else
* INPUTS
*	where : string in which to look
*	what  : string to search for in where
* SINCE
*	0.7
***/
NATIVE(_strstr)
{
	cell ret;
	char *tmp;
	char *where, *what;
	copyStringFromAMX(amx, params[1], &where);
	copyStringFromAMX(amx, params[2], &what);

	if( (tmp=strstr(where, what)) != NULL )
	{
		ret= tmp-where;
	}
	else
	{
		ret= -1;
	}

	delete [] where;
	delete [] what;
	return ret;
}

/****f* StringsFuncs/strstri
* USAGE
*	strstri(const where[], const what[])
* DESCRIPTION
*	look for what in where and return starting index if found
*	case insensitive version
* RETURN VALUE
*	integer : starting index if what is found, -1 else
* INPUTS
*	where : string in which to look
*	what  : string to search for in where
* SINCE
*	0.7
***/
NATIVE(_strstri)
{
	cell ret;
	char *tmp;
	char *where, *what;
	copyStringFromAMX(amx, params[1], &where);
	copyStringFromAMX(amx, params[2], &what);

	if( (tmp=strcasestr(where, what)) != NULL )
	{
		ret= tmp-where;
	}
	else
	{
		ret= -1;
	}

	delete [] where;
	delete [] what;
	return ret;
}


/****f* StringsFuncs/strnmatch
* USAGE
*	bool:strnmatch(const s1[], const s2[], len)
* RETURN VALUE
*	bool : true if both strings are the same
* INPUTS
*	s1 		: first string
*	s2 		: second string
*	len     : number of characters to compare
* SINCE
*	0.1
***/

/****f* StringsFuncs/strmatch
* MACRO
*	bool:strmatch(const s1[], const s2[])
* RETURN VALUE
*	bool : true if both strings are the same
* INPUTS
*	s1 		: first string
*	s2 		: second string
* NOTES
*	function implemented in amxbot.inc
*	using strnmatch
* SINCE
*	0.1
***/
NATIVE(_strnmatch)
{
	cell ret;
	char *str1, *str2;
	copyStringFromAMX(amx, params[1], &str1);
	copyStringFromAMX(amx, params[2], &str2);

	if( params[3]==-1 )
		ret= strcmp(str1, str2);
	else
		ret= strncmp(str1, str2, params[3]);

	delete [] str1;
	delete [] str2;
	return (ret==0);
}

/****f* StringsFuncs/strnmatchi
* USAGE
*	bool:strnmatchi(const s1[], const s2[], len)
* RETURN VALUE
*	bool : true if both strings are the same (case insensitive)
* INPUTS
*	s1 		: first string
*	s2 		: second string
*	len     : number of characters to compare
* SINCE
*	0.6
***/

/****f* StringsFuncs/strmatchi
* MACRO
*	bool:strmatchi(const s1[], const s2[])
* RETURN VALUE
*	bool : true if both strings are the same
* INPUTS
*	s1 		: first string
*	s2 		: second string
* NOTES
*	function implemented in amxbot.inc
*	using strnmatchi (case insensitive)
* SINCE
*	0.6
***/
NATIVE(_strnmatchi)
{
	cell ret;
	char *str1, *str2;
	copyStringFromAMX(amx, params[1], &str1);
	copyStringFromAMX(amx, params[2], &str2);

	if( params[3]==-1 )
		ret= strcasecmp(str1, str2);
	else
		ret= strncasecmp(str1, str2, params[3]);

	delete [] str1;
	delete [] str2;
	return (ret==0);
}

/****f* StringsFuncs/strncpy
* USAGE
*	strncpy(dest[], const src[], dest_maxsize= sizeof dest, bool:packed= false)
* DESCRIPTION
*	copy src string in dest string
* INPUTS
*	dest         : destination string
*	src          : source string
*	dest_maxsize : Natives
*	packed       : do you want the dest string to be packed ?
* SINCE
*	0.1
***/
NATIVE(_strncpy)
{
	char *src;
	copyStringFromAMX(amx, params[2], &src);
	copyStringToAMX(amx, params[1], src, params[3], params[4]);

	delete [] src;
	return 0;
}

/****f* StringsFuncs/snprintf
* USAGE
*	snprintf(dest[], dest_maxsize= sizeof dest, const format[], ...)
* DESCRIPTION
*	print a formatted string in dest
* INPUTS
*	dest         : destination string
*	dest_maxsize : Natives
*	format       : VarArgFormat
* SINCE
*	0.1
***/
NATIVE(_snprintf)
{
	cell *cstr;
	amx_GetAddr(amx, params[3], &cstr);

	buildString(amx, cstr, &params[4], (params[0]/sizeof(cell))-3);
	copyStringToAMX(amx, params[1], buildString_buffer, params[2]-1);

	return 0;
}

/****f* StringsFuncs/strtonum
* USAGE
*	strtonum(const src[])
* DESCRIPTION
*	convert number stored in a string to an integer
* RETURN
*	integer : the number
* INPUTS
*	src : the string where the number is
* REMARKS
*	if the string does not contains a number result will be 0
* SINCE
*	0.6
***/
NATIVE(_strtonum)
{
	cell ret;
	char *src;
	copyStringFromAMX(amx, params[1], &src);

	ret= atoi(src);

	delete [] src;
	return ret;
}

/****f* StringsFuncs/str_explode
* USAGE
*	string_array:str_explode(const str[], const sep[], mode= 0)
* DESCRIPTION
*	cut str in parts with sep
* RETURN VALUE
*	string_array : %String_Array%
* INPUTS
*	str : the original string
*	sep : the separator
*	mode: determine of sep is used :
*			0= string will be cut with any character found in sep
*			1= string will be cut with the whole string sep as separator
* SINCE
*	0.6
***/
NATIVE(_explode)
{
	cell &mode= params[3];
	int id= string_array::create();
	CHECK(id!=-1, -1, "str_explode: couldn't create string array, maybe they are all used..\n", 0);
	char *src, *sep;
	copyStringFromAMX(amx, params[1], &src);
	copyStringFromAMX(amx, params[2], &sep);

	vector<string> parts;

	if( mode==0 )
	{
		Tokenize(src, parts, sep);
	}
	else
	{
		TokenizeWithStr(src, parts, sep);
	}

	for(uint i= 0; i< parts.size(); i++)
		string_array::add(id, parts[i]);

	delete [] src;
	delete [] sep;
	return id;
}

/////////////////////////////////////////////////////////////////////////////////
// string array functions
/////////////////////////////////////////////////////////////////////////////////

/****c* StringsFuncs/String_Array
* MODULE DESCRIPTION
*	manipulation of arrays of string
***/
namespace string_array
{
	// number of arrays useable by all the scripts
	const int MAX_ARRAYCOUNT= 100;

	// arrays 0...9 are reserved for function parameters (see amxplugin.cpp)
	//uint list_used= 0x1FF;
	bitset<MAX_ARRAYCOUNT> list_used( 0x1FF );
	vector<string> string_list[MAX_ARRAYCOUNT];
	vector<string>::iterator it;

	//int current= 10;

	bool isArrayIDValid(cell id)
	{
		return ((id >= 0) && list_used.test(id) && (id < MAX_ARRAYCOUNT));
		//return ((list_used & (1<<id)) != 0) && (id < MAX_ARRAYCOUNT);
	}

	string join(int id, string sep, int first/*= 0*/)
	{
		string out;
		vector<string> &vec= string_list[id];
		for(uint i= first; i< vec.size(); i++)
		{
			out.append(vec[i]);

			if( i< vec.size()-1 )
				out.append(sep);
		}

		return out;
	}

	bool add(int id, string value)
	{
		if( value!="" )
		{
			//it= find(string_list[id].begin(), string_list[id].end(), value);
			//if( it==string_list[id].end() )
			//{
				string_list[id].push_back(value);
				return true;
			//}
		}

		return false;
	}

	void clear(int id)
	{
		string_list[id].clear();
	}

	int create()
	{
		int current;
		// find an unused index
		for(current= 0; current< MAX_ARRAYCOUNT; current++ )
		{
			if( !list_used.test(current) )
				break;
		}

		if( current < MAX_ARRAYCOUNT )
		{
			clear(current);
			//list_used|= (1<<current);
			list_used.set(current);
			return current;
		}

		return -1;
	}

/****f* String_Array/array_valid
* USAGE
*	bool:array_valid(string_array:id)
* RETURN VALUE
*	boolean : true is id is a valid string array
* INPUTS
*	id : string array returned by %array_create%, %str_replace% or %mask_getlist%
* SINCE
*	0.6
***/
	NATIVE(_valid)
	{
		cell ret= 0;

		if( isArrayIDValid(params[1]) )
			ret= 1;

		return ret;
	}

/****f* String_Array/array_create
* USAGE
*	string_array:array_create()
* RETURN VALUE
*	integer : array identifier required for all string array functions
* SINCE
*	0.1
***/
	NATIVE(_create)
	{
		int current= create();
		CHECK(current < MAX_ARRAYCOUNT, -1, "array_create: no more space\n", 0);
		return current;
	}

/****f* String_Array/array_destroy
* USAGE
*	array_destroy(string_array:id)
* DESCRIPTION
*	mark the array as free so it can be reused
* INPUTS
*	id 	  : array id ( from array_create )
* SINCE
*	0.3
***/
	NATIVE(_destroy)
	{
		cell string_id= params[1];

		CHECK(isArrayIDValid(string_id), -1, "array_destroy: invalid id (%d)\n", string_id);

		//list_used&= ~(1<<string_id);
		list_used.reset(string_id);
		return 0;
	}

/****f* String_Array/array_add
* USAGE
*	array_add(string_array:id, const value[])
* DESCRIPTION
*	add a value in the array
* INPUTS
*	id 	  : array id ( from array_create )
*	value : string to add
* NOTES
*	empty string and character ',' not allowed
* SINCE
*	0.1
***/
	NATIVE(_add)
	{
		char *val;
		cell string_id= params[1];

		CHECK(isArrayIDValid(string_id), -1, "array_add: invalid id (%d)\n", string_id);

		copyStringFromAMX(amx, params[2], &val);

		add(string_id, val);
		delete [] val;
		return 0;
	}

/****f* String_Array/array_remove
* USAGE
*	array_remove(string_array:id, const value[])
* DESCRIPTION
*	remove first occurence of a string from array
* INPUTS
*	id    : array id ( from array_create )
*	value : string to remove from array
* SINCE
*	0.1
***/
	NATIVE(_remove)
	{
		char *val;
		cell string_id= params[1];

		CHECK(isArrayIDValid(string_id), -1, "array_remove: invalid id (%d)\n", string_id);

		copyStringFromAMX(amx, params[2], &val);

		it= find(string_list[string_id].begin(), string_list[string_id].end(), val);
		if( it!=string_list[string_id].end() )
			string_list[string_id].erase(it);

		delete [] val;
		return 0;
	}

/****f* String_Array/array_replace
* USAGE
*	array_replace(string_array:id, const oldvalue[], const newvalue[])
* DESCRIPTION
*	replace first occurence of a string with another
* INPUTS
*	id       : array id ( from array_create )
*	oldvalue : look for this string
*	newvalue : and replace it by this one
* NOTES
*	empty string and character ',' not allowed
* SINCE
*	0.1
***/
	NATIVE(_replace)
	{
		char *oldval, *newval;
		cell string_id= params[1];

		CHECK(isArrayIDValid(string_id), -1, "array_replace: invalid id (%d)\n", string_id);

		copyStringFromAMX(amx, params[2], &oldval);
		copyStringFromAMX(amx, params[3], &newval);

		// empty string not allowed and ',' not allowed
		if( strcmp(newval, "") && (strchr(newval, ',')==NULL) )
			replace(string_list[string_id].begin(), string_list[string_id].end(), oldval, newval);

		delete [] oldval;
		delete [] newval;
		return 0;
	}

/****f* String_Array/array_isIn
* USAGE
*	bool:array_isIn(string_array:id, const value[])
* RETURN VALUE
*	bool : true if value is found in array
* INPUTS
*	id    : array id ( from array_create )
*	value : string to look for
* SINCE
*	0.1
***/
	NATIVE(_isIn)
	{
		char *val;
		cell string_id= params[1];

		CHECK(isArrayIDValid(string_id), 0, "array_isIn: invalid id (%d)\n", string_id);

		copyStringFromAMX(amx, params[2], &val);

		it= find(string_list[string_id].begin(), string_list[string_id].end(), val);

		delete [] val;
		return (it!=string_list[string_id].end());
	}

/****f* String_Array/array_get
* USAGE
*	array_get(string_array:id, position, out[], out_maxsize= sizeof out)
* INPUTS
*	id           : array id ( from array_create )
*	position     : index of the wanted string
*	out          : destination string
*	out_maxsize  : Natives
* SINCE
*	0.1
***/
	NATIVE(_get)
	{
		cell string_id= params[1];
		cell position= params[2];
		cell max_len= params[4];

		CHECK(isArrayIDValid(string_id), -1, "array_get: invalid id (%d)\n", string_id);
		CHECK(position < (cell)string_list[string_id].size(), -1, "array_get: pos out of range: %d\n", position);
		CHECK(position >= 0, -1, "array_get: pos out of range: %d\n", position);

		copyStringToAMX(amx, params[3], const_cast<char*>((string_list[string_id])[position].c_str()), max_len-1);
		return 0;
	}

/****f* String_Array/array_size
* USAGE
*	array_size(string_array:id)
* RETURN VALUE
*	integer : size of the array
* INPUTS
*	id : array id ( from array_create )
* SINCE
*	0.1
***/
	NATIVE(_getsize)
	{
		cell string_id= params[1];

		CHECK(isArrayIDValid(string_id), -1, "array_size: invalid id (%d)\n", string_id);
		return (cell)string_list[string_id].size();
	}

/****f* String_Array/array_join
* USAGE
*	array_join(string_array:id, const separator[], start_index, out[], out_maxsize= sizeof out)
* DESCRIPTION
*	build a string with the elements of one array
* INPUTS
*	id          : array id ( from array_create )
*	separator   : will be put between each value
*	start_index : starting index in the array
*	out         : destination string
*	out_maxsize : Natives
* SINCE
*	0.1
***/
	NATIVE(_join)
	{
		string str;
		char *sep;
		cell string_id= params[1];
		cell start_index= params[3];
		cell max_len= params[5];

		CHECK(isArrayIDValid(string_id), -1, "array_join: invalid id (%d)\n", string_id);
		CHECK(max_len > start_index, -1, "array_join: start_index >= maxlen: %d\n", start_index);
		copyStringFromAMX(amx, params[2], &sep);

		str= join(string_id, sep, start_index);
		copyStringToAMX(amx, params[4], const_cast<char*>(str.c_str()), max_len-1);

		delete [] sep;
		return 0;
	}

};

void registerNatives_String(AMX *amx)
{
	AMX_NATIVE_INFO db_natives[]= {
		/* strings array */
		{"array_create", 	string_array::_create},
		{"array_destroy",	string_array::_destroy},
		{"array_add", 		string_array::_add},
		{"array_remove", 	string_array::_remove},
		{"array_replace", 	string_array::_replace},
		{"array_isIn", 		string_array::_isIn},
		{"array_get", 		string_array::_get},
		{"array_size", 		string_array::_getsize},
		{"array_join", 		string_array::_join},
		{"array_valid",		string_array::_valid},

		/* string manipulation */
		{"strnmatch", 		_strnmatch},
		{"strnmatchi",		_strnmatchi},
		{"substr", 			_substr},
		{"strncpy", 		_strncpy},
		{"snprintf",		_snprintf},
		{"strtonum",		_strtonum},
		{"str_explode",		_explode},
		{"strstr",			_strstr},
		{"strstri",			_strstri},
		{NULL,NULL}
	};

	amx_Register(amx, db_natives, -1);
}


