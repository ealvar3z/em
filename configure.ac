AC_PREREQ([2.68])
AC_INIT([em], [1.1.0], [55966724+ealvar3z@users.noreply.github.com
])
AM_INIT_AUTOMAKE([-Wall  silent-rules])
AC_CONFIG_SRCDIR([src/em2.c])
AC_CONFIG_HEADERS([config.h])

AC_PROG_CC

AC_CACHE_CHECK([for C23 compiler flag], [em_cv_cc_c23], [
  em_cv_cc_c23=no
  for em_flag in -std=c23 -std=gnu23 -std=c2x -std=gnu2x; do
    em_save_CFLAGS="$CFLAGS"
    CFLAGS="$CFLAGS $em_flag"
    AC_COMPILE_IFELSE(
      [AC_LANG_SOURCE([int main(void) { return 0; }])],
      [em_cv_cc_c23="$em_flag"; break],
      []
    )
    CFLAGS="$em_save_CFLAGS"
  done
])
if test "x$em_cv_cc_c23" != "xno"; then
  CFLAGS="$CFLAGS $em_cv_cc_c23"
fi


AC_CHECK_HEADERS([fcntl.h stdlib.h termios.h unistd.h])

AC_CONFIG_FILES([ Makefile src/Makefile doc/Makefile])
AC_OUTPUT
