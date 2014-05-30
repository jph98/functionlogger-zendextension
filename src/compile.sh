# Enable debugging symbols with -g
rm -f /usr/lib64/php/modules/functionlogger*.so && phpize --clean && phpize && ./configure && make install
