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

/*!
Extension module
add functions to communicates with MySQL database
*/

#define EXTENSION_MODULE

#include "config.h"
#include "natives.h"
#include "Small/amx_script.h"
#include "utils.h"
#include "log.h"
#include <vector>
#include <map>
#include <string>
#include <mysql/mysql.h>

extern char buildString_buffer[1024];

extern "C"
{
	Logs EXPORT *amx_log;
	Logs EXPORT *error_log;
	Logs EXPORT *debug_log;
}

/****h* Small/MySQL_Extension
* LIBRARY
*	MySQL
* INCLUDE FILE
*	mysql.inc
* INCLUDE CONTENT
*	const mysql_res:no_res= mysql_res:0;
*	const mysql_res:invalid_res= mysql_res:-1;
* MODULE DESCRIPTION
*
*	database communication
*	special tags used:
*		- mysql_res : result identifier
*		- mysql_db  : db identifier
***/

/*
	mysql_connect(int &id, string host, string login, string pass, string db_name)
	mysql_query(int id, string format, ...)
	mysql_count_results(int id)
	mysql_get_field(int id, int result_index, string field_name, string &buff, int maxsize)

*/

const int MAX_MYSQL_CONNECTIONS= 5;	//! number of connections to database (for all scripts)
const int MAX_MYSQL_RESULTS= 5;		//! results kept in memory, first is index 1 (0 reserved)

// database connection
struct Mysql_Con
{
	//bool connected;
	MYSQL db;
	uint results_used;
	map<uint, map<string,string> > results[MAX_MYSQL_RESULTS];
	Script *plugin_owner;
};

uint con_used= 0x01;
Mysql_Con con_list[MAX_MYSQL_CONNECTIONS];

bool isConIDValid(cell id)
{
	return (((con_used & (1<<id)) != 0) && (id < MAX_MYSQL_CONNECTIONS));
}

// last 16 bits are res id, the firsts are con_id
uint encodeResID(cell con_id, cell res_id )
{
	if( (res_id & ~0xFFFF) != 0 )
		Error("res_id too high: %d\n", res_id);

	return (con_id<<16) | res_id;
}

void decodeResID(cell eid, cell &con_id, cell &res_id)
{
	res_id= eid & 0xFFFF;
	con_id= eid >> 16;
}

// we assume here that con_id is VALID
bool isResIDValid(cell con_id, cell res_id)
{
	Mysql_Con &con= con_list[con_id];
	return ( (res_id!=0) && ((con.results_used & (1<<res_id)) != 0) && (res_id < MAX_MYSQL_RESULTS));
}

uint countFreeResultSlots(cell con_id)
{
	uint ret= 0;
	Mysql_Con &con= con_list[con_id];

	// find an unused index
	for(int i= 0; i< MAX_MYSQL_RESULTS; i++ )
	{
		if( (con.results_used & (1<<i)) == 0 )
			ret++;
	}

	return ret;
}

/****f* MySQL_Extension/mysql_connect
* USAGE
*	mysql_connect(&mysql_db:id, const host[], const login[], const pass[], const database_name[])
* DESCRIPTION
*	open a connection to a MySQL database
* RETURN VALUE
*	bool: false if connection fails
* INPUTS
*	id            : will receive the connection identifier if connection is a success
*	host          : ip of host
*	login         : login
*	pass          : & password
*	database_name : database to use after login
***/
NATIVE(_connect)
{
	char *host, *login, *pass, *dbname;
	MYSQL *ret;
	cell *cstr;
	int current;
	Script *plugin;

	// find an unused index
	for(current= 0; current< MAX_MYSQL_CONNECTIONS; current++ )
	{
		if( (con_used & (1<<current)) == 0 )
			break;
	}

	CHECK(current< MAX_MYSQL_CONNECTIONS, 0, "mysql_connect: too many connections (%d) !\n", current);

	amx_GetUserData(amx, AMX_USERTAG('P','L','U','G'), (void**)&plugin);
	copyStringFromAMX(amx, params[2], &host);
	copyStringFromAMX(amx, params[3], &login);
	copyStringFromAMX(amx, params[4], &pass);
	copyStringFromAMX(amx, params[5], &dbname);
	amx_GetAddr(amx, params[1], &cstr);

	*cstr= current;

	// initialisation
	con_list[current].results_used= 0;
	//con_list[current].results.clear();

	mysql_init(&con_list[current].db);
	uint timeout= 1;
	mysql_options(&con_list[current].db, MYSQL_OPT_CONNECT_TIMEOUT, (char*)&timeout);
	ret= mysql_real_connect(&con_list[current].db, host, login, pass, dbname, 0, NULL, 0);
	if( ret!=NULL )
	{
		//con_list[current].connected= true;
		con_used|= (1<<current);
	}
	else
	{
		//const char *msg= mysql_error(&con_list[current].db);
		//Debug("Connection error: %s\n", msg);
	}

	con_list[current].plugin_owner= plugin;

	timeout= 0;
	mysql_options(&con_list[current].db, MYSQL_OPT_CONNECT_TIMEOUT, (char*)&timeout);

	delete [] host;
	delete [] login;
	delete [] pass;
	delete [] dbname;
	return (ret!=NULL);
}

/****f* MySQL_Extension/mysql_close
* USAGE
*	mysql_close(mysql_db:id)
* DESCRIPTION
*	close a connection opened with %mysql_connect%
* INPUTS
*	id : connection identifier returned by %mysql_connect%
***/
NATIVE(_close)
{
	CHECK(isConIDValid(params[1]), 0, "mysql_close: id invalid: %d (or connection already closed)\n", params[1]);
	mysql_close(&con_list[params[1]].db);
	con_used&= ~(1<<params[1]);
	return 0;
}

/****f* MySQL_Extension/mysql_count_results
* USAGE
*	mysql_count_results(mysql_res:res)
* RETURN VALUE
*	integer : number of lines in the result
* INPUTS
*	res : result identifier returned by %mysql_query%
***/
NATIVE(_count_results)
{
	cell con_id;
	cell res_id;

	decodeResID(params[1], con_id, res_id);

	CHECK(isConIDValid(con_id), 0, "mysql_count_results: connection id invalid: %d\n", con_id);
	CHECK(isResIDValid(con_id, res_id), 0, "mysql_count_results: result id invalid: %d\n", res_id);

	return con_list[con_id].results[res_id].size();
}

/****f* MySQL_Extension/mysql_free_result
* USAGE
*	mysql_free_result(mysql_res:res)
* DESCRIPTION
*	free the memory used by the result
* INPUTS
*	res : result identifier returned by %mysql_query%
***/
NATIVE(_free_result)
{
	cell con_id;
	cell res_id;

	decodeResID(params[1], con_id, res_id);

	CHECK(isConIDValid(con_id), 0, "mysql_free_result: connection id invalid: %d\n", con_id);
	CHECK(isResIDValid(con_id, res_id), 0, "mysql_free_result: result id invalid: %d\n", res_id);

	Mysql_Con &con= con_list[con_id];

	con.results_used &= ~(1<<res_id);
	return 0;
}

/****f* MySQL_Extension/mysql_query
* USAGE
*	mysql_res:mysql_query(mysql_db:id, const format[], ...)
* DESCRIPTION
*	execute a query on the database
* RETURN VALUE
*	a result identifier
* INPUTS
*	id     : connection identifier returned by %mysql_connect%
*	format : %VarArgFormat%
***/
NATIVE(_query)
{
	MYSQL_RES *res;
	MYSQL_ROW row;
	MYSQL_FIELD *fields;
	cell *cstr;
	uint rows_count, fields_count;
	cell con_id= params[1];
	int res_id;
	cell ret= 0;

	CHECK(isConIDValid(con_id), -1, "mysql_query: id invalid: %d\n", con_id);
	CHECK(!mysql_ping(&con_list[con_id].db), -1, "mysql_query: connection lost (%d)\n", con_id);

	Mysql_Con &con= con_list[con_id];

	// look for an unused result slot
	for(res_id= 1; res_id< MAX_MYSQL_RESULTS; res_id++)
	{
		if( (con.results_used & (1<<res_id)) == 0  )
			break;
	}

	con.results[res_id].clear();

	CHECK( res_id< MAX_MYSQL_RESULTS, -1, "mysql_query: no more space free to store result data\n", 0);

	amx_GetAddr(amx, params[2], &cstr);
	buildString(amx, cstr, &params[3], (params[0]/sizeof(cell))-2);

	//Debug("MySQL Query: \"%s\"\n", buildString_buffer);

	CHECK(!mysql_query(&con.db, buildString_buffer), -1, "mysql_query: query failed: \"%s\"\n", buildString_buffer );

	res= mysql_store_result(&con.db);

	// do nothing if query shouldnt return a result
	if( res!=NULL )
	{
		rows_count= mysql_num_rows(res);

		if( rows_count > 0 )
		{
			fields_count= mysql_num_fields(res);
			fields = mysql_fetch_fields(res);

			for(uint i= 0; i< rows_count; i++)
			{
				row= mysql_fetch_row(res);

				for(uint j= 0; j< fields_count; j++)
				{
					string tmp= "";
					if( row[j]!=0x00 )
						tmp= row[j];

					con.results[res_id][i][fields[j].name]= tmp;
					//Debug(" '%s' = '%s'\n", fields[j].name, row[j]);
				}
			}

			con.results_used|= (1<<res_id);
			ret= res_id;
		}
	}
	else if( mysql_field_count(&con.db) > 0 )
	{
		Error("\"%s\" should return data but returned nothing\n");
	}

	if( ret!=0 )
		ret= encodeResID(con_id, ret);

	mysql_free_result(res);
	return ret;
}

/****h* MySQL_Extension/FieldConversions
* MODULE DESCRIPTION
*	used to retrieve field data
***/

/****f* FieldConversions/mysql_get_field
* USAGE
*	mysql_get_field(mysql_res:result_index, array_index, const field_name[], buff[], buff_maxsize= sizeof buff)
* DESCRIPTION
*	used to retrieve value of a result field as a string
* INPUTS
*	result_index : result identifier returned by %mysql_query%
*	array_index  : result line ( use %mysql_count_results% to get the line count )
*	field_name   : field name ( case sensitive )
*	buff         : where to store the result
*	buff_maxsize : %Natives%
***/
NATIVE(_get_field)
{
	cell con_id;
	cell res_id;
	cell array_index= params[2];

	decodeResID(params[1], con_id, res_id);


	char *field_name;
	CHECK(isConIDValid(con_id), -1, "mysql_get_field: id invalid: %d\n", con_id);
	CHECK(isResIDValid(con_id, res_id), -1, "mysql_get_field: res_id invalid: %d\n", res_id);

	Mysql_Con &con= con_list[con_id];
	CHECK(IN_RANGE(array_index, 0, con.results[res_id].size()), -1, "mysql_get_field: array index invalid: %d\n", array_index);

	copyStringFromAMX(amx, params[3], &field_name);

	map<string,string> &m= con.results[res_id][array_index];
	map<string,string>::iterator it= m.find(field_name);
	if( it!=m.end() )
		copyStringToAMX(amx, params[4], const_cast<char*>(it->second.c_str()), params[5]-1, 0);

	delete [] field_name;
	return 0;
}

/****f* FieldConversions/mysql_get_INTfield
* USAGE
*	mysql_get_INTfield(mysql_res:result_index, array_index, const field_name[])
* DESCRIPTION
*	used to retrieve value of a result field as an integer
* RETURN VALUE
*	integer : field data converted to integer
* INPUTS
*	result_index : result identifier returned by %mysql_query%
*	array_index  : result line ( use %mysql_count_results% to get the line count )
*	field_name   : field name ( case sensitive )
***/
NATIVE(_get_INTfield)
{

	cell ret= 0;
	cell con_id;
	cell res_id;
	cell array_index= params[2];
	char *field_name;

	decodeResID(params[1], con_id, res_id);

	CHECK(isConIDValid(con_id), -1, "mysql_get_INTfield: id invalid: %d\n", con_id);
	CHECK(isResIDValid(con_id, res_id), -1, "mysql_get_INTfield: res_id invalid: %d\n", res_id);

	Mysql_Con &con= con_list[con_id];
	CHECK(IN_RANGE(array_index, 0, con.results[res_id].size()), -1, "mysql_get_INTfield: array index invalid: %d\n", array_index);

	copyStringFromAMX(amx, params[3], &field_name);

	map<string,string> &m= con.results[res_id][array_index];
	map<string,string>::iterator it= m.find(field_name);
	if( it!=m.end() )
		ret= atoi( it->second.c_str() );

	delete [] field_name;
	return ret;

}

/****f* FieldConversions/mysql_get_DATEfield
* USAGE
*	mysql_get_DATEfield(mysql_res:result_index, array_index, const field_name[], buff[], buff_maxsize= sizeof buff, const dateformat[])
* DESCRIPTION
*	used to retrieve value of a result field as a formatted date string (DATETIME fields)
* INPUTS
*	result_index : result identifier returned by %mysql_query%
*	array_index  : result line ( use %mysql_count_results% to get the line count )
*	field_name   : field name ( case sensitive )
*	buff         : where to store the result
*	buff_maxsize : %Natives%
*	dateformat   : format string ( will be passed as is to strftime system call
*
*					Here is a non-exhaustive list of the available options:
*					%d = day of the month ("01"-"31")
*					%e = day of the month (" 1"-"31")
*					%m = month ("01"-"12")
*					%F = "%Y-%m-%d"
*					%H = hour ("00"-"23") 24h
*					%I = hour ("01"-"12") 12h
*					%M = minutes ("00"-"59")
*					%S = seconds ("00"-"60")
*					%R = "%H:%M"
*					%T = "%H:%M:%S"
*
*					%n = newline
*					%t = tabulation
***/
NATIVE(_get_DATEfield)
{
	cell con_id;
	cell res_id;
	cell array_index= params[2];
	char *field_name, *format;

	decodeResID(params[1], con_id, res_id);

	CHECK(isConIDValid(con_id), -1, "mysql_get_DATEfield: id invalid: %d\n", con_id);
	CHECK(isResIDValid(con_id, res_id), -1, "mysql_get_DATEfield: index invalid: %d\n", res_id);

	Mysql_Con &con= con_list[con_id];
	CHECK(IN_RANGE(array_index, 0, con.results[res_id].size()), -1, "mysql_get_DATEfield: array index invalid: %d\n", array_index);

	copyStringFromAMX(amx, params[3], &field_name);
	copyStringFromAMX(amx, params[6], &format);

	map<string,string> &m= con.results[res_id][array_index];
	map<string,string>::iterator it= m.find(field_name);
	if( it!=m.end() )
{
		// YYYY-MM-DD HH:MM:SS => DD/MM/YYY HH:MM
		string date[6], out;
		struct tm tm_s;
		char buff[100];

		parseMySQLDateTime(it->second, tm_s);
		strftime(buff, sizeof(buff)-1, format, &tm_s);
		copyStringToAMX(amx, params[4], buff, params[5]-1, 0);
	}

	delete [] field_name;
	delete [] format;
	return 0;
}

/**** Dynamic extension interface ****/

extern "C"
{
	void EXPORT dryon_MySQL_Init(Script *_p)
	{
		SmallScript *p= (SmallScript*)_p;

		AMX_NATIVE_INFO db_natives[]= {
			{"mysql_connect",		_connect},
			{"mysql_close",			_close},
			{"mysql_query",			_query},
			{"mysql_free_result",	_free_result},
			{"mysql_count_results",	_count_results},
			{"mysql_get_field", 	_get_field},
			{"mysql_get_DATEfield",	_get_DATEfield},
			{"mysql_get_INTfield",	_get_INTfield},
			{NULL,NULL}
	};

		amx_Register(p->getAMX(), db_natives, -1);
	}

	void EXPORT dryon_MySQL_Cleanup(Script *p)
	{
		for(uint i= 0; i< MAX_MYSQL_CONNECTIONS; i++)
		{
			if( isConIDValid(i) )
			{
				Mysql_Con &con= con_list[i];
				mysql_close(&con.db);
				con_used&= ~(1<<i);
			}
		}
	}

	void EXPORT dryon_MySQL_UnloadPlugin(Script *p)
	{
		for(uint i= 0; i< MAX_MYSQL_CONNECTIONS; i++)
		{
			if( isConIDValid(i) )
			{
				Mysql_Con &con= con_list[i];
				if( con.plugin_owner == p )
				{
					mysql_close(&con.db);
					con_used&= ~(1<<i);
				}
			}
		}
	}

	void EXPORT dryon_MySQL_EndOfAMXCall(Script *p)
	{
		uint i, c= 0;
		// look for connections opened by this plugin
		for(i= 0; i< MAX_MYSQL_CONNECTIONS; i++)
		{
			if( isConIDValid(i) )
			{
				Mysql_Con &con= con_list[i];
				if( con.plugin_owner == p )
				{
					for(uint j= 0; j< MAX_MYSQL_RESULTS; j++)
					{
						if( isResIDValid(i, j) )
							c++;
					}

					con.results_used= 0;
				}
			}
		}

		if( c > 0 )
		{
			AmxDebug("%sresults not freed for %s: %d%s\n", COLOR_RED, p->getName(), c, COLOR_RESET);
		}
	}
}

/**/


