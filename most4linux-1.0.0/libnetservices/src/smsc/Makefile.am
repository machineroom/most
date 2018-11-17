if WITH_NETSERVICES
lib_LTLIBRARIES = libnetservices.la

nodist_libnetservices_la_SOURCES = ads.c \
	ams.c \
	cms.c \
	mcs.c \
	mns.c \
	msval.c \
	msv.c \
	rcs.c \
	scs.c \
	tcs.c

libnetservices_la_CFLAGS = -I${top_srcdir}/libnetservices/include/linux \
	-I${top_srcdir}/libnetservices/include/smsc \
	-I${top_srcdir}/libnetservices/include/user_adjust \
	-I${top_srcdir}/libnetservices/src/linux \
	-I${top_srcdir}

nodist_noinst_HEADERS = ads.h \
	ams.h \
	cms.h \
	mcs.h \
	mns.h \
	msval.h \
	msv.h \
	rcs.h \
	scs.h \
	tcs.h

libnetservices_la_LIBADD = ${top_srcdir}/libnetservices/src/linux/libnsadapt.la
endif
