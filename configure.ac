#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([butt], [0.2.17], [melchor9000@gmail.com])
AM_INIT_AUTOMAKE([subdir-objects foreign])
AC_CANONICAL_HOST

AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_OBJC

LDFLAGS="$LDFLAGS -L/usr/local/lib"

# Checks for libraries.
AC_CHECK_LIB([ogg], [ogg_stream_init], [],
		[AC_MSG_ERROR([**** Could not find libogg     ****])]
	    )
AC_CHECK_LIB([vorbis], [vorbis_info_init], [],
		[AC_MSG_ERROR([**** Could not find libvorbis     ****])]
	    )
AC_CHECK_LIB([vorbisenc], [vorbis_encode_init], [],
		[AC_MSG_ERROR([**** Could not find libvorbisenc     ****])]
	    )
AC_CHECK_LIB([opus], [opus_encoder_create], [],
		[AC_MSG_ERROR([**** Could not find libopus     ****])]
	    )
AC_CHECK_LIB([mp3lame], [lame_init], [],
		[AC_MSG_ERROR([**** Could not find libmp3lame     ****])]

		)
AC_CHECK_LIB([FLAC], [FLAC__stream_encoder_new], [],
		[AC_MSG_ERROR([**** Could not find libFLAC     ****])]
		)
AC_CHECK_LIB([portaudio], [Pa_Initialize], [],
		[AC_MSG_ERROR([**** Could not find libportaudio     ****])]
	    )
AC_CHECK_LIB([pthread], [pthread_create], [],
		[AC_MSG_ERROR([**** Could not find libpthread     ****])]
	    )
AC_CHECK_LIB([samplerate], [src_new], [],
		[AC_MSG_ERROR([**** Could not find samplerate     ****])]
	    )

## Using MinGW, gettext uses libintl, on others, uses libgettextpo
case $host_os in
	mingw*)
		AC_CHECK_LIB([intl], [setlocale], [],
			[AC_MSG_ERROR([**** Could not find intl     ****])]
			)
		;;

	*)
		AC_CHECK_LIB([gettextpo], [setlocale], [],
			[AC_MSG_ERROR([**** Could not find gettextpo     ****])]
			)
        AC_CHECK_LIB([dl], [dlopen], [],
		        [AC_MSG_ERROR([**** Could not find libdl     ****])]
	            )
		;;
esac


AC_CHECK_HEADER([winsock2.h],
		[
		LIBS="$LIBS -lws2_32"
		host_os="windows"
		],
	       )

build_linux=no
build_windows=no
build_mac=no

# Detect the target system
case "${host_os}" in
    linux*)
        build_linux=yes
        AC_MSG_WARN([Linux])
        ;;
    cygwin*|mingw*|windows)
        build_windows=yes
        AC_MSG_WARN([Windows])
        ;;
    darwin*)
        build_mac=yes
        AC_MSG_WARN([macOS])
        ;;
    *)
        AC_MSG_ERROR(["OS $host_os is not supported"])
        ;;
esac

# Pass the conditionals to automake
AM_CONDITIONAL([LINUX], [test "$build_linux" = "yes"])
AM_CONDITIONAL([WINDOWS], [test "$build_windows" = "yes"])
AM_CONDITIONAL([OSX], [test "$build_mac" = "yes"])

AC_CHECK_PROG([FLTKCONFIG],[fltk-config], [fltk-config])

if test "$FLTKCONFIG" = "fltk-config"; then
    AC_CHECK_LIB([fltk], [main],
		    [
		    LIBS="$LIBS `fltk-config --ldflags --use-images`"
		    CXXFLAGS="$CXXFLAGS `fltk-config --cxxflags`"
		    ],
		    [AC_MSG_ERROR([**** Could not find libfltk     ****])]
		)
else
  AC_MSG_ERROR([**** Could not find fltk-config     ****])
fi



# Checks for header files.
AC_PATH_X
AC_CHECK_HEADERS([fcntl.h limits.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([gethostbyaddr gethostbyname memset pow select socket strchr strdup strpbrk strrchr strstr])

# Adds visibility of streaming configuration
AC_ARG_ENABLE([stream_conf],
	AS_HELP_STRING([--disable-stream-conf], [Disable stream configration visibility, for customized clients]))
AS_IF([test "x$enable_stream_conf" = "xno"], [
	AC_DEFINE([VISIBILITY], [0], [Stream conf visibility disabled])
], [
	AC_DEFINE([VISIBILITY], [1], [Stream conf visibility enabled])
])

# Adds support to libfdk-aac
AC_ARG_ENABLE([aac],
	AS_HELP_STRING([--enable-aac], [Enable linking against AAC using the non-free library libfdk-aac]))
AS_IF([test "x$enable_aac" = "xyes"], [
	AC_CHECK_LIB([fdk-aac], [aacEncOpen], [], [
		    AC_MSG_ERROR([libfdk-aac not found])
		])
])

#Add dbus library for Linux
if test "$build_linux"; then
	PKG_CHECK_MODULES([DBUS], [dbus-1], [
		AC_DEFINE([HAVE_DBUS], [1], [Use dbus to get current tracks])
		LIBS="$LIBS $DBUS_LIBS"
		CFLAGS="$LIBS $DBUS_CFLAGS"
		CXXFLAGS="$CXXFLAGS $DBUS_CFLAGS"
	], [
		AC_DEFINE([HAVE_DBUS], [0], [Use dbus to get current tracks])
		AC_MSG_WARN([Support for dbus is disabled])
	])
fi

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
