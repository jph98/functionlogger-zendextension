functionlogger-zendextension
----------------------------

A fairly simple Zend extension that maintains counts of function calls made from PHP.

Uses zend_hash to maintain an in-memory hashmap that *could* be flushed at the end of the request.

See: http://zendguru.wordpress.com/2009/05/10/php-extensions-understanding-and-working-with-hash-api-part-1/
