/**********************************************************************

  intern.h -

  $Author: matz $
  $Date: 2004/10/30 15:33:01 $
  created at: Thu Jun 10 14:22:17 JST 1993

  Copyright (C) 1993-2003 Yukihiro Matsumoto
  Copyright (C) 2000  Network Applied Communication Laboratory, Inc.
  Copyright (C) 2000  Information-technology Promotion Agency, Japan

**********************************************************************/

/* 
 * Functions and variables that are used by more than one source file of
 * the kernel.
 */

#include <time.h>
#include <sys/types.h>

#if defined(_WIN32)
#include <winsock.h>
#endif


#define ID_ALLOCATOR 1

/* array.c */
EXPORT void rb_mem_clear _((register VALUE*, register long));
EXPORT VALUE rb_assoc_new _((VALUE, VALUE));
EXPORT VALUE rb_check_array_type _((VALUE));
EXPORT VALUE rb_ary_new _((void));
EXPORT VALUE rb_ary_new2 _((long));
EXPORT VALUE rb_ary_new3 __((long,...));
EXPORT VALUE rb_ary_new4 _((long, const VALUE *));
EXPORT VALUE rb_ary_freeze _((VALUE));
EXPORT VALUE rb_ary_aref _((int, VALUE*, VALUE));
EXPORT void rb_ary_store _((VALUE, long, VALUE));
EXPORT VALUE rb_ary_dup _((VALUE));
EXPORT VALUE rb_ary_to_ary _((VALUE));
EXPORT VALUE rb_ary_to_s _((VALUE));
EXPORT VALUE rb_ary_push _((VALUE, VALUE));
EXPORT VALUE rb_ary_pop _((VALUE));
EXPORT VALUE rb_ary_shift _((VALUE));
EXPORT VALUE rb_ary_unshift _((VALUE, VALUE));
EXPORT VALUE rb_ary_entry _((VALUE, long));
EXPORT VALUE rb_ary_each _((VALUE));
EXPORT VALUE rb_ary_join _((VALUE, VALUE));
EXPORT VALUE rb_ary_print_on _((VALUE, VALUE));
EXPORT VALUE rb_ary_reverse _((VALUE));
EXPORT VALUE rb_ary_sort _((VALUE));
EXPORT VALUE rb_ary_sort_bang _((VALUE));
EXPORT VALUE rb_ary_delete _((VALUE, VALUE));
EXPORT VALUE rb_ary_delete_at _((VALUE, long));
EXPORT VALUE rb_ary_clear _((VALUE));
EXPORT VALUE rb_ary_plus _((VALUE, VALUE));
EXPORT VALUE rb_ary_concat _((VALUE, VALUE));
EXPORT VALUE rb_ary_assoc _((VALUE, VALUE));
EXPORT VALUE rb_ary_rassoc _((VALUE, VALUE));
EXPORT VALUE rb_ary_includes _((VALUE, VALUE));
EXPORT VALUE rb_ary_cmp _((VALUE, VALUE));
EXPORT VALUE rb_protect_inspect _((VALUE(*)(ANYARGS),VALUE,VALUE));
EXPORT VALUE rb_inspecting_p _((VALUE));
EXPORT VALUE rb_check_array_value _((VALUE));
EXPORT VALUE rb_values_at _((VALUE, long, int, VALUE*, VALUE(*) _((VALUE,long))));
/* bignum.c */
EXPORT VALUE rb_big_clone _((VALUE));
EXPORT void rb_big_2comp _((VALUE));
EXPORT VALUE rb_big_norm _((VALUE));
EXPORT VALUE rb_uint2big _((unsigned long));
EXPORT VALUE rb_int2big _((long));
EXPORT VALUE rb_uint2inum _((unsigned long));
EXPORT VALUE rb_int2inum _((long));
EXPORT VALUE rb_cstr_to_inum _((const char*, int, int));
EXPORT VALUE rb_str_to_inum _((VALUE, int, int));
EXPORT VALUE rb_cstr2inum _((const char*, int));
EXPORT VALUE rb_str2inum _((VALUE, int));
EXPORT VALUE rb_big2str _((VALUE, int));
EXPORT long rb_big2long _((VALUE));
#define rb_big2int(x) rb_big2long(x)
EXPORT unsigned long rb_big2ulong _((VALUE));
#define rb_big2uint(x) rb_big2ulong(x)
#if HAVE_LONG_LONG
EXPORT VALUE rb_ll2inum _((LONG_LONG));
EXPORT VALUE rb_ull2inum _((unsigned LONG_LONG));
EXPORT LONG_LONG rb_big2ll _((VALUE));
EXPORT unsigned LONG_LONG rb_big2ull _((VALUE));
#endif  /* HAVE_LONG_LONG */
EXPORT void rb_quad_pack _((char*,VALUE));
EXPORT VALUE rb_quad_unpack _((const char*,int));
EXPORT VALUE rb_dbl2big _((double));
EXPORT double rb_big2dbl _((VALUE));
EXPORT VALUE rb_big_plus _((VALUE, VALUE));
EXPORT VALUE rb_big_minus _((VALUE, VALUE));
EXPORT VALUE rb_big_mul _((VALUE, VALUE));
EXPORT VALUE rb_big_divmod _((VALUE, VALUE));
EXPORT VALUE rb_big_pow _((VALUE, VALUE));
EXPORT VALUE rb_big_and _((VALUE, VALUE));
EXPORT VALUE rb_big_or _((VALUE, VALUE));
EXPORT VALUE rb_big_xor _((VALUE, VALUE));
EXPORT VALUE rb_big_lshift _((VALUE, VALUE));
EXPORT VALUE rb_big_rand _((VALUE, double*));
/* class.c */
EXPORT VALUE rb_class_boot _((VALUE));
EXPORT VALUE rb_class_new _((VALUE));
EXPORT VALUE rb_mod_init_copy _((VALUE, VALUE));
EXPORT VALUE rb_class_init_copy _((VALUE, VALUE));
EXPORT VALUE rb_singleton_class_clone _((VALUE));
EXPORT void rb_singleton_class_attached _((VALUE,VALUE));
EXPORT VALUE rb_make_metaclass _((VALUE, VALUE));
EXPORT void rb_check_inheritable _((VALUE));
EXPORT VALUE rb_class_inherited _((VALUE, VALUE));
EXPORT VALUE rb_define_class_id _((ID, VALUE));
EXPORT VALUE rb_module_new _((void));
EXPORT VALUE rb_define_module_id _((ID));
EXPORT VALUE rb_mod_included_modules _((VALUE));
EXPORT VALUE rb_mod_include_p _((VALUE, VALUE));
EXPORT VALUE rb_mod_ancestors _((VALUE));
EXPORT VALUE rb_class_instance_methods _((int, VALUE*, VALUE));
EXPORT VALUE rb_class_public_instance_methods _((int, VALUE*, VALUE));
EXPORT VALUE rb_class_protected_instance_methods _((int, VALUE*, VALUE));
EXPORT VALUE rb_class_private_instance_methods _((int, VALUE*, VALUE));
EXPORT VALUE rb_obj_singleton_methods _((int, VALUE*, VALUE));
EXPORT void rb_define_method_id _((VALUE, ID, VALUE (*)(ANYARGS), int));
EXPORT void rb_frozen_class_p _((VALUE));
EXPORT void rb_undef _((VALUE, ID));
EXPORT void rb_define_protected_method _((VALUE, const char*, VALUE (*)(ANYARGS), int));
EXPORT void rb_define_private_method _((VALUE, const char*, VALUE (*)(ANYARGS), int));
EXPORT void rb_define_singleton_method _((VALUE, const char*, VALUE(*)(ANYARGS), int));
EXPORT VALUE rb_singleton_class _((VALUE));
/* compar.c */
EXPORT int rb_cmpint _((VALUE, VALUE, VALUE));
NORETURN(void rb_cmperr _((VALUE, VALUE)));
/* enum.c */
/* error.c */
EXPORT int ruby_nerrs;
EXPORT VALUE rb_exc_new _((VALUE, const char*, long));
EXPORT VALUE rb_exc_new2 _((VALUE, const char*));
EXPORT VALUE rb_exc_new3 _((VALUE, VALUE));
NORETURN(void rb_loaderror __((const char*, ...)));
NORETURN(void rb_name_error __((ID, const char*, ...)));
NORETURN(void rb_invalid_str _((const char*, const char*)));
void rb_compile_error __((const char*, ...));
void rb_compile_error_append __((const char*, ...));
NORETURN(void rb_load_fail _((const char*)));
NORETURN(void rb_error_frozen _((const char*)));
void rb_check_frozen _((VALUE));
/* eval.c */
RUBY_EXTERN struct RNode *ruby_current_node;
void ruby_set_current_source _((void));
EXPORT NORETURN(void rb_exc_raise _((VALUE)));
NORETURN(void rb_exc_fatal _((VALUE)));
VALUE rb_f_exit _((int,VALUE*));
VALUE rb_f_abort _((int,VALUE*));
void rb_remove_method _((VALUE, const char*));
#define rb_disable_super(klass, name) ((void)0)
#define rb_enable_super(klass, name) ((void)0)
#define HAVE_RB_DEFINE_ALLOC_FUNC 1
EXPORT void rb_define_alloc_func _((VALUE, VALUE (*)(VALUE)));
EXPORT void rb_undef_alloc_func _((VALUE));
void rb_clear_cache _((void));
void rb_clear_cache_by_class _((VALUE));
void rb_alias _((VALUE, ID, ID));
void rb_attr _((VALUE,ID,int,int,int));
int rb_method_boundp _((VALUE, ID, int));
VALUE rb_dvar_defined _((ID));
VALUE rb_dvar_curr _((ID));
VALUE rb_dvar_ref _((ID));
void rb_dvar_asgn _((ID, VALUE));
void rb_dvar_push _((ID, VALUE));
VALUE *rb_svar _((int));
VALUE rb_eval_cmd _((VALUE, VALUE, int));
int rb_respond_to _((VALUE, ID));
void rb_interrupt _((void));
VALUE rb_apply _((VALUE, ID, VALUE));
void rb_backtrace _((void));
ID rb_frame_last_func _((void));
VALUE rb_obj_instance_eval _((int, VALUE*, VALUE));
VALUE rb_mod_module_eval _((int, VALUE*, VALUE));
EXPORT void rb_load _((VALUE, int));
void rb_load_protect _((VALUE, int, int*));
NORETURN(void rb_jump_tag _((int)));
int rb_provided _((const char*));
void rb_provide _((const char*));
VALUE rb_f_require _((VALUE, VALUE));
VALUE rb_require_safe _((VALUE, int));
EXPORT void rb_obj_call_init _((VALUE, int, VALUE*));
EXPORT VALUE rb_class_new_instance _((int, VALUE*, VALUE));
VALUE rb_block_proc _((void));
VALUE rb_f_lambda _((void));
VALUE rb_proc_new _((VALUE (*)(ANYARGS/* VALUE yieldarg[, VALUE procarg] */), VALUE));
EXPORT VALUE rb_protect _((VALUE (*)(VALUE), VALUE, int*));
void rb_set_end_proc _((void (*)(VALUE), VALUE));
void rb_mark_end_proc _((void));
void rb_exec_end_proc _((void));
void ruby_finalize _((void));
NORETURN(void ruby_stop _((int)));
EXPORT int ruby_cleanup _((int));
EXPORT int ruby_exec _((void));
void rb_gc_mark_threads _((void));
void rb_thread_start_timer _((void));
void rb_thread_stop_timer _((void));
void rb_thread_schedule _((void));
void rb_thread_wait_fd _((int));
int rb_thread_fd_writable _((int));
void rb_thread_fd_close _((int));
int rb_thread_alone _((void));
void rb_thread_polling _((void));
void rb_thread_sleep _((int));
void rb_thread_sleep_forever _((void));
VALUE rb_thread_stop _((void));
VALUE rb_thread_wakeup _((VALUE));
VALUE rb_thread_run _((VALUE));
VALUE rb_thread_kill _((VALUE));
VALUE rb_thread_create _((VALUE (*)(ANYARGS), void*));
void rb_thread_interrupt _((void));
void rb_thread_trap_eval _((VALUE, int, int));
void rb_thread_signal_raise _((char*));
int rb_thread_select _((int, fd_set *, fd_set *, fd_set *, struct timeval *));
void rb_thread_wait_for _((struct timeval));
VALUE rb_thread_current _((void));
VALUE rb_thread_main _((void));
VALUE rb_thread_local_aref _((VALUE, ID));
VALUE rb_thread_local_aset _((VALUE, ID, VALUE));
void rb_thread_atfork _((void));
/* file.c */
int eaccess _((const char*, int));
VALUE rb_file_s_expand_path _((int, VALUE *));
void rb_file_const _((const char*, VALUE));
int rb_find_file_ext _((VALUE*, const char* const*));
VALUE rb_find_file _((VALUE));
char *rb_path_next _((const char *));
char *rb_path_skip_prefix _((const char *));
char *rb_path_last_separator _((const char *));
char *rb_path_end _((const char *));
/* gc.c */
NORETURN(void rb_memerror __((void)));
int ruby_stack_check _((void));
int ruby_stack_length _((VALUE**));
char *rb_source_filename _((const char*));
void rb_gc_mark_locations _((VALUE*, VALUE*));
void rb_mark_tbl _((struct st_table*));
void rb_mark_hash _((struct st_table*));
void rb_gc_mark_maybe _((VALUE));
void rb_gc_mark _((VALUE));
void rb_gc_force_recycle _((VALUE));
EXPORT void rb_gc _((void));
void rb_gc_copy_finalizer _((VALUE,VALUE));
void rb_gc_finalize_deferred _((void));
void rb_gc_call_finalizer_at_exit _((void));
VALUE rb_gc_enable _((void));
VALUE rb_gc_disable _((void));
EXPORT VALUE rb_gc_start _((void));
/* hash.c */
EXPORT void st_foreach _((struct st_table *, int (*)(), unsigned long));
EXPORT void rb_hash_foreach _((VALUE, int (*)(), VALUE));
EXPORT VALUE rb_hash _((VALUE));
EXPORT VALUE rb_hash_new _((void));
EXPORT VALUE rb_hash_freeze _((VALUE));
EXPORT VALUE rb_hash_aref _((VALUE, VALUE));
EXPORT VALUE rb_hash_aset _((VALUE, VALUE, VALUE));
EXPORT VALUE rb_hash_delete_if _((VALUE));
EXPORT VALUE rb_hash_delete _((VALUE,VALUE));
EXPORT int rb_path_check _((char*));
EXPORT int rb_env_path_tainted _((void));
/* io.c */
#define rb_defout rb_stdout
EXPORT  VALUE rb_fs;
EXPORT  VALUE rb_output_fs;
EXPORT  VALUE rb_rs;
EXPORT  VALUE rb_default_rs;
EXPORT  VALUE rb_output_rs;
VALUE rb_io_write _((VALUE, VALUE));
VALUE rb_io_gets _((VALUE));
VALUE rb_io_getc _((VALUE));
VALUE rb_io_ungetc _((VALUE, VALUE));
VALUE rb_io_close _((VALUE));
VALUE rb_io_eof _((VALUE));
VALUE rb_io_binmode _((VALUE));
VALUE rb_io_addstr _((VALUE, VALUE));
VALUE rb_io_printf _((int, VALUE*, VALUE));
VALUE rb_io_print _((int, VALUE*, VALUE));
VALUE rb_io_puts _((int, VALUE*, VALUE));
VALUE rb_file_open _((const char*, const char*));
VALUE rb_gets _((void));
void rb_write_error _((const char*));
void rb_write_error2 _((const char*, long));
/* marshal.c */
VALUE rb_marshal_dump _((VALUE, VALUE));
VALUE rb_marshal_load _((VALUE));
/* numeric.c */
void rb_num_zerodiv _((void));
VALUE rb_num_coerce_bin _((VALUE, VALUE));
VALUE rb_num_coerce_cmp _((VALUE, VALUE));
VALUE rb_num_coerce_relop _((VALUE, VALUE));
EXPORT VALUE rb_float_new _((double));
EXPORT VALUE rb_num2fix _((VALUE));
EXPORT VALUE rb_fix2str _((VALUE, int));
EXPORT VALUE rb_dbl_cmp _((double, double));
/* object.c */
EXPORT int rb_eql _((VALUE, VALUE));
EXPORT VALUE rb_any_to_s _((VALUE));
EXPORT VALUE rb_inspect _((VALUE));
EXPORT VALUE rb_obj_is_instance_of _((VALUE, VALUE));
EXPORT VALUE rb_obj_is_kind_of _((VALUE, VALUE));
EXPORT VALUE rb_obj_alloc _((VALUE));
EXPORT VALUE rb_obj_clone _((VALUE));
EXPORT VALUE rb_obj_dup _((VALUE));
EXPORT VALUE rb_obj_init_copy _((VALUE,VALUE));
EXPORT VALUE rb_obj_taint _((VALUE));
EXPORT VALUE rb_obj_tainted _((VALUE));
EXPORT VALUE rb_obj_untaint _((VALUE));
EXPORT VALUE rb_obj_freeze _((VALUE));
EXPORT VALUE rb_obj_id _((VALUE));
EXPORT VALUE rb_obj_class _((VALUE));
EXPORT VALUE rb_class_real _((VALUE));
EXPORT VALUE rb_class_inherited_p _((VALUE, VALUE));
EXPORT VALUE rb_convert_type _((VALUE,int,const char*,const char*));
EXPORT VALUE rb_check_convert_type _((VALUE,int,const char*,const char*));
EXPORT VALUE rb_to_int _((VALUE));
EXPORT VALUE rb_Integer _((VALUE));
EXPORT VALUE rb_Float _((VALUE));
EXPORT VALUE rb_String _((VALUE));
EXPORT VALUE rb_Array _((VALUE));
EXPORT double rb_cstr_to_dbl _((const char*, int));
EXPORT double rb_str_to_dbl _((VALUE, int));
/* parse.y */
EXPORT  int   ruby_sourceline;
EXPORT  char *ruby_sourcefile;
int ruby_yyparse _((void));
ID rb_id_attrset _((ID));
void rb_parser_append_print _((void));
void rb_parser_while_loop _((int, int));
int ruby_parser_stack_on_heap _((void));
void rb_gc_mark_parser _((void));
int rb_is_const_id _((ID));
int rb_is_instance_id _((ID));
int rb_is_class_id _((ID));
int rb_is_local_id _((ID));
int rb_is_junk_id _((ID));
VALUE rb_backref_get _((void));
void rb_backref_set _((VALUE));
VALUE rb_lastline_get _((void));
void rb_lastline_set _((VALUE));
VALUE rb_sym_all_symbols _((void));
/* process.c */
int rb_proc_exec _((const char*));
VALUE rb_f_exec _((int,VALUE*));
int rb_waitpid _((int,int*,int));
void rb_syswait _((int));
VALUE rb_proc_times _((VALUE));
VALUE rb_detach_process _((int));
/* range.c */
EXPORT VALUE rb_range_new _((VALUE, VALUE, int));
EXPORT VALUE rb_range_beg_len _((VALUE, long*, long*, long, int));
EXPORT VALUE rb_length_by_each _((VALUE));
/* re.c */
int rb_memcmp _((char*,char*,long));
int rb_memcicmp _((char*,char*,long));
long rb_memsearch _((char*,long,char*,long));
VALUE rb_reg_nth_defined _((int, VALUE));
VALUE rb_reg_nth_match _((int, VALUE));
VALUE rb_reg_last_match _((VALUE));
VALUE rb_reg_match_pre _((VALUE));
VALUE rb_reg_match_post _((VALUE));
VALUE rb_reg_match_last _((VALUE));
VALUE rb_reg_new _((const char*, long, int));
VALUE rb_reg_match _((VALUE, VALUE));
VALUE rb_reg_match2 _((VALUE));
int rb_reg_options _((VALUE));
void rb_set_kcode _((const char*));
const char* rb_get_kcode _((void));
/* ruby.c */
EXPORT  VALUE rb_argv;
EXPORT  VALUE rb_argv0;
EXPORT void rb_load_file _((const char*));
EXPORT void ruby_script _((const char*));
EXPORT void ruby_prog_init _((void));
EXPORT void ruby_set_argv _((int, char**));
EXPORT void ruby_process_options _((int, char**));
EXPORT void ruby_load_script _((void));
EXPORT void ruby_init_loadpath _((void));
EXPORT void ruby_incpush _((const char*));
/* signal.c */
VALUE rb_f_kill _((int, VALUE*));
void rb_gc_mark_trap_list _((void));
#ifdef POSIX_SIGNAL
#define posix_signal ruby_posix_signal
void posix_signal _((int, RETSIGTYPE (*)(int)));
#endif
void rb_trap_exit _((void));
void rb_trap_exec _((void));
const char *ruby_signal_name _((int));
/* sprintf.c */
EXPORT VALUE rb_f_sprintf _((int, VALUE*));
/* string.c */
EXPORT VALUE rb_str_new _((const char*, long));
EXPORT VALUE rb_str_new2 _((const char*));
EXPORT VALUE rb_str_new3 _((VALUE));
EXPORT VALUE rb_str_new4 _((VALUE));
EXPORT VALUE rb_str_new5 _((VALUE, const char*, long));
EXPORT VALUE rb_tainted_str_new _((const char*, long));
EXPORT VALUE rb_tainted_str_new2 _((const char*));
EXPORT VALUE rb_str_buf_new _((long));
EXPORT VALUE rb_str_buf_new2 _((const char*));
EXPORT VALUE rb_str_buf_append _((VALUE, VALUE));
EXPORT VALUE rb_str_buf_cat _((VALUE, const char*, long));
EXPORT VALUE rb_str_buf_cat2 _((VALUE, const char*));
EXPORT VALUE rb_obj_as_string _((VALUE));
EXPORT VALUE rb_check_string_type _((VALUE));
EXPORT VALUE rb_str_dup _((VALUE));
EXPORT VALUE rb_str_locktmp _((VALUE));
EXPORT VALUE rb_str_unlocktmp _((VALUE));
EXPORT VALUE rb_str_dup_frozen _((VALUE));
EXPORT VALUE rb_str_plus _((VALUE, VALUE));
EXPORT VALUE rb_str_times _((VALUE, VALUE));
EXPORT VALUE rb_str_substr _((VALUE, long, long));
EXPORT void rb_str_modify _((VALUE));
EXPORT VALUE rb_str_freeze _((VALUE));
EXPORT VALUE rb_str_resize _((VALUE, long));
EXPORT VALUE rb_str_cat _((VALUE, const char*, long));
EXPORT VALUE rb_str_cat2 _((VALUE, const char*));
EXPORT VALUE rb_str_append _((VALUE, VALUE));
EXPORT VALUE rb_str_concat _((VALUE, VALUE));
EXPORT int rb_str_hash _((VALUE));
EXPORT int rb_str_cmp _((VALUE, VALUE));
EXPORT VALUE rb_str_upto _((VALUE, VALUE, int));
EXPORT void rb_str_update _((VALUE, long, long, VALUE));
EXPORT VALUE rb_str_inspect _((VALUE));
EXPORT VALUE rb_str_dump _((VALUE));
EXPORT VALUE rb_str_split _((VALUE, const char*));
EXPORT void rb_str_associate _((VALUE, VALUE));
EXPORT VALUE rb_str_associated _((VALUE));
EXPORT void rb_str_setter _((VALUE, ID, VALUE*));
EXPORT VALUE rb_str_intern _((VALUE));
/* struct.c */
EXPORT VALUE rb_struct_new __((VALUE, ...));
EXPORT VALUE rb_struct_define __((const char*, ...));
EXPORT VALUE rb_struct_alloc _((VALUE, VALUE));
EXPORT VALUE rb_struct_aref _((VALUE, VALUE));
EXPORT VALUE rb_struct_aset _((VALUE, VALUE, VALUE));
EXPORT VALUE rb_struct_getmember _((VALUE, ID));
EXPORT VALUE rb_struct_iv_get _((VALUE, char*));
EXPORT VALUE rb_struct_s_members _((VALUE));
EXPORT VALUE rb_struct_members _((VALUE));
/* time.c */
EXPORT VALUE rb_time_new _((time_t, time_t));
/* variable.c */
EXPORT VALUE rb_mod_name _((VALUE));
EXPORT VALUE rb_class_path _((VALUE));
EXPORT void rb_set_class_path _((VALUE, VALUE, const char*));
EXPORT VALUE rb_path2class _((const char*));
EXPORT void rb_name_class _((VALUE, ID));
EXPORT VALUE rb_class_name _((VALUE));
EXPORT void rb_autoload _((VALUE, ID, const char*));
EXPORT void rb_autoload_load _((VALUE, ID));
EXPORT VALUE rb_autoload_p _((VALUE, ID));
EXPORT void rb_gc_mark_global_tbl _((void));
EXPORT VALUE rb_f_trace_var _((int, VALUE*));
EXPORT VALUE rb_f_untrace_var _((int, VALUE*));
EXPORT VALUE rb_f_global_variables _((void));
EXPORT void rb_alias_variable _((ID, ID));
EXPORT struct st_table* rb_generic_ivar_table _((VALUE));
EXPORT void rb_copy_generic_ivar _((VALUE,VALUE));
EXPORT void rb_mark_generic_ivar _((VALUE));
EXPORT void rb_mark_generic_ivar_tbl _((void));
EXPORT void rb_free_generic_ivar _((VALUE));
EXPORT VALUE rb_ivar_get _((VALUE, ID));
EXPORT VALUE rb_ivar_set _((VALUE, ID, VALUE));
EXPORT VALUE rb_ivar_defined _((VALUE, ID));
EXPORT VALUE rb_iv_set _((VALUE, const char*, VALUE));
EXPORT VALUE rb_iv_get _((VALUE, const char*));
EXPORT VALUE rb_attr_get _((VALUE, ID));
EXPORT VALUE rb_obj_instance_variables _((VALUE));
EXPORT VALUE rb_obj_remove_instance_variable _((VALUE, VALUE));
EXPORT void *rb_mod_const_at _((VALUE, void*));
EXPORT void *rb_mod_const_of _((VALUE, void*));
EXPORT VALUE rb_const_list _((void*));
EXPORT VALUE rb_mod_constants _((VALUE));
EXPORT VALUE rb_mod_remove_const _((VALUE, VALUE));
EXPORT int rb_const_defined _((VALUE, ID));
EXPORT int rb_const_defined_at _((VALUE, ID));
EXPORT int rb_const_defined_from _((VALUE, ID));
EXPORT VALUE rb_const_get _((VALUE, ID));
EXPORT VALUE rb_const_get_at _((VALUE, ID));
EXPORT VALUE rb_const_get_from _((VALUE, ID));
EXPORT void rb_const_set _((VALUE, ID, VALUE));
EXPORT VALUE rb_mod_constants _((VALUE));
EXPORT VALUE rb_mod_const_missing _((VALUE,VALUE));
EXPORT VALUE rb_cvar_defined _((VALUE, ID));
#define RB_CVAR_SET_4ARGS 1
EXPORT void rb_cvar_set _((VALUE, ID, VALUE, int));
EXPORT VALUE rb_cvar_get _((VALUE, ID));
EXPORT void rb_cv_set _((VALUE, const char*, VALUE));
EXPORT VALUE rb_cv_get _((VALUE, const char*));
EXPORT void rb_define_class_variable _((VALUE, const char*, VALUE));
EXPORT VALUE rb_mod_class_variables _((VALUE));
EXPORT VALUE rb_mod_remove_cvar _((VALUE, VALUE));
/* version.c */
void ruby_show_version _((void));
void ruby_show_copyright _((void));
