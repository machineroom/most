if WITH_NETSERVICES
noinst_LTLIBRARIES = libnsadapt.la

libnsadapt_la_SOURCES = events.c \
	extensions.c \
	init.c \
	par_cp.c \
	service.c

libnsadapt_la_CFLAGS = -I${top_srcdir} \
	-I${top_srcdir}/most-kernel \
	-I${top_srcdir}/libnetservices/include \
	-I${top_srcdir}/libnetservices/include/linux \
	-I${top_srcdir}/libnetservices/include/smsc \
	-I${top_srcdir}/libnetservices/include/user_adjust \
	-pipe

noinst_HEADERS = global.h \
	msv_linux.h

libnsadapt_la_LIBADD = -lpthread
endif
