PHP_ARG_ENABLE(functionlogger, whether to enable function logger, [ --enable-functionlogger   Enable the function logger extension])
if test "$PHP_FUNCTIONLOGGER" = "yes"; then
  AC_DEFINE(HAVE_FUNCTIONLOGGER, 1, [Whether you have function logger])
  PHP_NEW_EXTENSION(functionlogger, functionlogger.c, $ext_shared)
fi
