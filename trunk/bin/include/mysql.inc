#pragma library MySQL

const mysql_res:no_res= mysql_res:0;
const mysql_res:invalid_res= mysql_res:-1;

native mysql_close(mysql_db:id);
native mysql_connect(&mysql_db:id, const host[], const login[], const pass[], const database_name[]);
native mysql_count_results(mysql_res:res);
native mysql_free_result(mysql_res:res);
native mysql_res:mysql_query(mysql_db:id, const format[], ...);
native mysql_get_DATEfield(mysql_res:result_index, array_index, const field_name[], buff[], buff_maxsize= sizeof buff, const dateformat[]);
native mysql_get_INTfield(mysql_res:result_index, array_index, const field_name[]);
native mysql_get_field(mysql_res:result_index, array_index, const field_name[], buff[], buff_maxsize= sizeof buff);
