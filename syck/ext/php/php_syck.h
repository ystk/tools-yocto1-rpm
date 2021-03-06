/*
  +----------------------------------------------------------------------+
  | PHP Version 4                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2002 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 2.02 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available at through the world-wide-web at                           |
  | http://www.php.net/license/2_02.txt.                                 |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+

  $Id: php_syck.h,v 1.2 2003/09/24 14:36:09 whythluckystiff Exp $ 
*/

#ifndef PHP_SYCK_H
#define PHP_SYCK_H

extern zend_module_entry syck_module_entry;
#define phpext_syck_ptr &syck_module_entry

#ifdef PHP_WIN32
#define PHP_SYCK_API __declspec(dllexport)
#else
#define PHP_SYCK_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(syck);
PHP_MSHUTDOWN_FUNCTION(syck);
PHP_RINIT_FUNCTION(syck);
PHP_RSHUTDOWN_FUNCTION(syck);
PHP_MINFO_FUNCTION(syck);

PHP_FUNCTION(mergekey_init);

PHP_FUNCTION(syck_load);

/* 
  	Declare any global variables you may need between the BEGIN
	and END macros here:     

ZEND_BEGIN_MODULE_GLOBALS(syck)
	int   global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(syck)
*/

/* In every utility function you add that needs to use variables 
   in php_syck_globals, call TSRM_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as SYCK_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define SYCK_G(v) TSRMG(syck_globals_id, zend_syck_globals *, v)
#else
#define SYCK_G(v) (syck_globals.v)
#endif

#endif	/* PHP_SYCK_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * indent-tabs-mode: t
 * End:
 */
