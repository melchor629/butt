#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([butt], [0.1.14], [BUG-REPORT-ADDRESS])
AM_INIT_AUTOMAKE([subdir-objects foreign])

AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

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

AC_CHECK_HEADER([winsock2.h], 
		[
		LIBS="$LIBS -lws2_32"
		host_os="windows"
		],
	       )
AM_CONDITIONAL(WINDOWS, test "$host_os" = "windows")

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

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
