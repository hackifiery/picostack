AC_INIT([picostack], [0.1])
AM_INIT_AUTOMAKE([foreign])
AC_PROG_CC

AX_CHECK_COMPILE_FLAG([-Wall], [CFLAGS+=" -Wall"])
AX_CHECK_COMPILE_FLAG([-Wextra], [CFLAGS+=" -Wextra"])
AX_CHECK_COMPILE_FLAG([-pedantic], [CFLAGS+=" -Wpedantic"])
AX_CHECK_COMPILE_FLAG([-Wno-unused-function], [CFLAGS+=" -Wno-unused-function"])
AX_CHECK_COMPILE_FLAG([-Werror], [CFLAGS+=" -Werror"])
AX_CHECK_COMPILE_FLAG([-std=gnu99], [CFLAGS+=" -std=gnu99"])
AX_CHECK_COMPILE_FLAG([-Wno-unused-result], [CFLAGS+=" -Wno-unused-result"])
AX_CHECK_COMPILE_FLAG([-Wno-empty-body], [CFLAGS+=" -Wno-empty-body"])

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT