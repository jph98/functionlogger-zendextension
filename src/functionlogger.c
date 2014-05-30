#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "php.h"
#include "php_ini.h"
#include "php_functionlogger.h"

// TRSM (Thread Safe Resource Management) or ZTS (Zend Thread Safety)
#include "TSRM.h"
#include "SAPI.h"
#include "main/php_ini.h"
#include "ext/standard/head.h"
#include "ext/standard/html.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"
#include "php_globals.h"
#include "main/php_output.h"
#include "ext/standard/php_var.h"
#include "zend_extensions.h"

HashTable *globalstats;

// Define our method interception function
#if PHP_VERSION_ID < 50500
/* Pointer to the original execute function */
static ZEND_DLEXPORT void (*_zend_execute) (zend_op_array *ops TSRMLS_DC);

/* Pointer to the origianl execute_internal function */
static ZEND_DLEXPORT void (*_zend_execute_internal) (zend_execute_data *data,
                           int ret TSRMLS_DC);
#else
/* Pointer to the original execute function */
static void (*_zend_execute_ex) (zend_execute_data *execute_data TSRMLS_DC);

/* Pointer to the origianl execute_internal function */
static void (*_zend_execute_internal) (zend_execute_data *data,
                      struct _zend_fcall_info *fci, int ret TSRMLS_DC);
#endif

static function_entry functionlogger_functions[] = {    
    {NULL, NULL, NULL}
};

// Define the MODULE ENTRY function
zend_module_entry functionlogger_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
    STANDARD_MODULE_HEADER,
#endif
    PHP_FUNCTIONLOGGER_EXTNAME,
    functionlogger_functions,
    PHP_MINIT(functionlogger),
    PHP_MSHUTDOWN(functionlogger),
    PHP_RINIT(functionlogger),
    PHP_RSHUTDOWN(functionlogger),
    PHP_MINFO(functionlogger),
#if ZEND_MODULE_API_NO >= 20010901
    PHP_FUNCTIONLOGGER_VERSION,
#endif
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_FUNCTIONLOGGER
ZEND_GET_MODULE(functionlogger)
#endif

int php_print_contents_function(int *val, int num_args, va_list args, zend_hash_key *hash_key) {
        
    TSRMLS_FETCH();

    zend_printf("\tEntry key: [%s] value: [%i]:\n", hash_key->arKey, *val);

    return ZEND_HASH_APPLY_KEEP;
}

/*
 * Function logger handles function interception for PHP
 */
#if PHP_VERSION_ID < 50500
ZEND_DLEXPORT void functionlogger_execute (zend_op_array *ops TSRMLS_DC) {
#else
ZEND_DLEXPORT void functionlogger_execute_ex (zend_execute_data *execute_data TSRMLS_DC) {

    // Operations array contains zend_ops with following structure:
    // struct _zend_op {
    //   opcode_handler_t handler;
    //   znode result;
    //   znode op1;
    //   znode op2;
    //   ulong extended_value;
    //   uint lineno;
    //   zend_uchar opcode;
    // };

    zend_op_array *ops = execute_data->op_array;
#endif

    struct timeval start, end;
    gettimeofday(&start, NULL);     

    zend_function      *curr_func;        
    char *func;
    char *file = (char *) ops->filename;    

    zend_execute_data *data = EG(current_execute_data);

    if (data) {
        curr_func = data->function_state.function;
        func = curr_func->common.function_name;    
    } else {
        func = "none";
    }

    char *sep = "|";
    size_t key_length = malloc(strlen(file) + strlen(sep) + strlen(func) + 1);
    char *key = (char*) malloc(sizeof(char) * key_length);
    snprintf(key, key_length, "%s%s%s", file, sep, func);    

    char *varvalue;
    
    // TODO: Check to see whether the function exists in the hash already
    if (zend_hash_exists(globalstats, key, strlen(key)) ) {

        //zend_printf("\n\tKey already present, incrementing hit count");
        int *previous_count;
        zend_hash_find(globalstats, key, strlen(key), (void **) &previous_count);        
        zend_printf("\n\nPrevious count %i ", *previous_count);
        ++(*previous_count);
        
        //zend_hash_update(globalstats, key, strlen(key), &previous_count, sizeof(zval), NULL);

    } else {        
        int hit_count = 0;
        zend_hash_add(globalstats, key, strlen(key), &hit_count, sizeof(zval), NULL);
        //zend_printf("\n\tKey not present, adding new: %i \n", zend_hash_num_elements(globalstats));
        free(key);
    }

    gettimeofday(&end, NULL);

    double elapsedTime = (end.tv_sec - start.tv_sec) * 1000.0;
    elapsedTime += (end.tv_usec - start.tv_usec) / 1000.0;
    
    // All is well, just output to screen    
    zend_printf("Function call time taken: %.2f ms \n", elapsedTime);

    // MAIN: Perform execution
#if PHP_VERSION_ID < 50500
  _zend_execute(ops TSRMLS_CC);
#else
  _zend_execute_ex(execute_data TSRMLS_CC);

  

#endif            

}

/*
 * Module init callback.
 */
PHP_MINIT_FUNCTION(functionlogger) {
    
    // Replace zend_execute with our proxy
#if PHP_VERSION_ID < 50500
    _zend_execute = zend_execute;
    zend_execute  = functionlogger_execute;
#else
    _zend_execute_ex = zend_execute_ex;
    zend_execute_ex  = functionlogger_execute_ex;
#endif

    zend_printf("\nMINIT functionlogger code usage extension initialised.\n");

    return SUCCESS;
}

/**
 * Module shutdown callback.
 */
PHP_MSHUTDOWN_FUNCTION(functionlogger) {

    zend_printf("\nMSHUTDOWN functionlogger code usage extension shutdown\n");

    return SUCCESS;
}

/**
 * Request initialisation
 */
PHP_RINIT_FUNCTION(functionlogger) {

    zend_printf("\nRINIT functionlogger request initialisation. \n");

    // TODO: Increase the size of the hashtable based on the number of functions expected
    ALLOC_HASHTABLE(globalstats);
    zend_hash_init(globalstats, 50, NULL, ZVAL_PTR_DTOR, 0);

    return SUCCESS;
}

/**
 * Request shutdown
 */
PHP_RSHUTDOWN_FUNCTION(functionlogger) {

    zend_printf("\nRSHUTDOWN functionlogger request shutdown. Global stats size: %i \n", zend_hash_num_elements(globalstats));    

    // TODO: Iterate through hash and write each line to a CSV File
    zend_printf("\n* Dumping globalstats:\n");
    zend_hash_apply_with_arguments(globalstats, php_print_contents_function, 0);
    zend_printf("* Done dumping globalstats\n\n");
    FREE_HASHTABLE(globalstats);

    return SUCCESS;
}

PHP_MINFO_FUNCTION(functionlogger)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "functionlogger", "enabled");
    php_info_print_table_row(2, "Version", "0.1");    
    php_info_print_table_end();
    DISPLAY_INI_ENTRIES();
}