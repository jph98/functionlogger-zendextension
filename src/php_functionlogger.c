#ifndef PHP_FUNCTIONLOGGER_H
#define PHP_FUNCTIONLOGGER_H 1
#define PHP_FUNCTIONLOGGER_VERSION "1.0"
#define PHP_FUNCTIONLOGGER_EXTNAME "functionlogger"

PHP_MINIT_FUNCTION(functionlogger);
PHP_MSHUTDOWN_FUNCTION(functionlogger);

PHP_RINIT_FUNCTION(functionlogger);
PHP_RSHUTDOWN_FUNCTION(functionlogger);

PHP_MINFO_FUNCTION(functionlogger);

extern zend_module_entry functionlogger_module_entry;
#define phpext_functionlogger_ptr &functionlogger_module_entry

#endif
