#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.60)
AC_INIT(most4linux, 1.0.0)
AC_CONFIG_HEADERS([config/config.h])
AC_CONFIG_AUX_DIR([config/])
AC_PREFIX_DEFAULT([/usr/local/most4linux])
AM_INIT_AUTOMAKE(dist-bzip2)
AM_INIT_AUTOMAKE(dist-zip)

AH_TEMPLATE(DEBUG, Prints debugging output in the most driver)
AH_TEMPLATE(TRACE, Prints trace informations in netservices adaption lib)
AH_TEMPLATE(REG_ACCESS_DEBUG, Prints debug messages on all low-level register accesses. 
							   This makes the driver slow, so use this with care!)
AH_TEMPLATE(IOCTL_DEBUG, Prints debug messages for the ioctl calls)
AH_TEMPLATE(DEVFUNC_DEBUG, Prints debug messages for the OS-8104 register access)
AH_TEMPLATE(IRQ_DEBUG, Prints debug messages in the interrupt services routine.
					   Please not that this only works if the buffer size is reasonable
					   high because printk() in a interrupt service rotine slows down 
					   the interrupt handling dramatically)
AH_TEMPLATE(RT_IRQ_DEBUG, Same as IRQ_DEBUG but for the real-time interrupt service routine)
AH_TEMPLATE(RXBUF_DEBUG, Prints debug messages for the receive buffer)
AH_TEMPLATE(TXBUF_DEBUG, Prints debug messages for the transmit buffer)
AH_TEMPLATE(SYNC_DEBUG, Prints debug messages in the synchronous module)
AH_TEMPLATE(NETS_DEBUG, Prints debug messages in the NetServices module)
AH_TEMPLATE(SERIAL_RT_DEBUG, Debugging over the serial port in real-time.
							  This is useful if the real-time kernel "hangs"
							  so that Linux never gets the chance to print the 
							  kernelmessage on the screen or serial interface. 
							  The load script is prepared to load the real-time 
							  driver which is supplied with RTAI. But please take 
							  care that this only works if Linux doesn't occupy the serial port)
AH_TEMPLATE(MEASURING_PCI, Used to measure the interrupt latency)
AH_TEMPLATE(MEASURING_SCHED, Used to measure the scheduling latency)
AH_TEMPLATE(ALSA_DEBUG, Prints debugging output in the ALSA driver)
AH_TEMPLATE(RT_RTDM, Enables real-time support in driver)

AC_ARG_ENABLE(debug,
			  AC_HELP_STRING([--enable-debug], [Prints debugging output in most driver and in netservices adaption lib]),
			  AC_DEFINE(DEBUG)
			  debug=enabled,
			  debug=disabled)
AC_ARG_ENABLE(trace,
			  AC_HELP_STRING([--enable-trace], [Prints trace informations in netservices adaption lib]),
			  AC_DEFINE(TRACE)
			  trace=enabled,
			  trace=disabled)

AC_ARG_ENABLE(reg_access_debug,
			  AC_HELP_STRING([--enable-reg_access_debug], 
			  				 [Prints debug messages on all low-level register accesses
							   This makes the driver slow, so use this with care!]),
			  AC_DEFINE(REG_ACCESS_DEBUG)
			  reg_access_debug=enabled,
			  reg_access_debug=disabled)
AC_ARG_ENABLE(ioctl_debug, 
			  AC_HELP_STRING([--enable-ioctl_debug], [Prints debug messages for the ioctl calls]),
			  AC_DEFINE(IOCTL_DEBUG)
			  ioctl_debug=enabled,
			  ioctl_debug=disabled)
AC_ARG_ENABLE(devfunc_debug,
			  AC_HELP_STRING([--enable-devfunc_debug], 
			  				 [Prints debug messages for the OS-8104 register access]),
			  AC_DEFINE(DEVFUNC_DEBUG)
			  devfunc_debug=enabled,
			  devfunc_debug=disabled)
AC_ARG_ENABLE(irq_debug, 
			  AC_HELP_STRING([--enable-irq_debug], 
			  				 [Prints debug messages in the interrupt services routine]),
			  AC_DEFINE(IRQ_DEBUG)
			  irq_debug=enabled,
			  irq_debug=disabled)
AC_ARG_ENABLE(rt_irq_debug, 
			  AC_HELP_STRING([--enable-rt_irq_debug], 
			  				[Same as --enable-irq_debug but for the real-time interrupt service routine]),
			  AC_DEFINE(RT_IRQ_DEBUG)
			  rt_irq_debug=enabled,
			  rt_irq_debug=disabled)
AC_ARG_ENABLE(rxbuf_debug, 
			  AC_HELP_STRING([--enable-rxbuf_debug], [Prints debug messages for the receive buffer]),
			  AC_DEFINE(RXBUF_DEBUG)
			  rxbuf_debug=enabled,
			  rxbuf_debug=disabled)
AC_ARG_ENABLE(txbuf_debug, 
			  AC_HELP_STRING([--enable-txbuf_debug], [Prints debug messages for the transmit buffer]),
			  AC_DEFINE(TXBUF_DEBUG)
			  txbuf_debug=enabled,
			  txbuf_debug=disabled)
AC_ARG_ENABLE(sync_debug, 
			  AC_HELP_STRING([--enable-sync_debug], [Prints debug messages in the synchronous module]),
			  AC_DEFINE(SYNC_DEBUG)
			  sync_debug=enabled,
			  sync_debug=disabled)
AC_ARG_ENABLE(nets_debug, 
			  AC_HELP_STRING([--enable-nets_debug], [Prints debug messages in the NetServices module]),
			  AC_DEFINE(NETS_DEBUG)
			  nets_debug=enabled,
			  nets_debug=disabled)

AC_ARG_ENABLE(serial_rt_debug, 
			  AC_HELP_STRING([--enable-serial_rt_debug], [Debugging over the serial port in real-time]),
			  AC_DEFINE(SERIAL_RT_DEBUG)
			  serial_rt_debug=enabled,
			  serial_rt_debug=disabled)
dnl AC_ARG_ENABLE(measuring_pci, 
dnl			  AC_HELP_STRING([--enable-measuring_pci], [Used to measure the interrupt latency]),
dnl			  AC_DEFINE(MEASURING_PCI)
dnl			  measuring_pci=enabled,
dnl			  measuring_pci=disabled)
dnl
dnl AC_ARG_ENABLE(measuring_sched, 
dnl			  AC_HELP_STRING([--enable-measuring_sched], [Used to measure the scheduling latency]),
dnl			  AC_DEFINE(MEASURING_SCHED)
dnl			  measuring_sched=enabled,
dnl			  measuring_sched=disabled)
AC_ARG_ENABLE(alsa_debug, 
			  AC_HELP_STRING([--enable-alsa_debug], [Prints debugging output in the ALSA driver]),
			  AC_DEFINE(ALSA_DEBUG)
			  alsa_debug=enabled,
			  alsa_debug=disabled)

AC_ARG_ENABLE([realtime], 
			  AC_HELP_STRING([--enable-realtime=EXTENSION],
			  				 [building driver with real-time support. 
							  Possible values for EXTENSION are disable, 
							  xenomai or rtai (disable)]),
			  [	case ${enableval} in 
			  		disable | xenomai | rtai) 
						RT=${enableval}
						AC_DEFINE(RT_RTDM);;
					*) 
						AC_MSG_ERROR(You must supply an valid argument to --enable-realtime);;
			  	esac],
			  RT=disabled)
AC_ARG_WITH([realtime-extensiondir],
			  AC_HELP_STRING([--with-realtime-extensiondir=DIR],
			  				 [specify the path to the real-time extension 
							  installation directory (/usr/realtime)]),
			  [	case ${withval} in 
					"" | y | ye | yes | n | no) 
						AC_MSG_ERROR(You must supply an argument to --with-realtime-extension-path);;
			  		*) 
						RT_PATH=${withval};;
			 	esac],
			  RT_PATH=/usr/realtime)
AC_ARG_WITH([kerneldir],
			  AC_HELP_STRING([--with-kerneldir=DIR],
			  				 [specify the path to the linux kernel source tree (/lib/modules/$(uname -r)/build)]),
			  [	case ${withval} in 
					"" | y | ye | yes | n | no) 
						AC_MSG_ERROR(You must supply an argument to --with-kerneldir);;
			  		*) 
						KERNELDIR=${withval};;
			 	esac],
			  KERNELDIR=/lib/modules/$(uname -r)/build)

AC_ARG_WITH([alsalib],
			  AC_HELP_STRING([--with-alsalib=DIR],
			  				 [specify the path to the alsalib installation prefix (/usr/local)]),
			  [	case ${withval} in 
					"" | y | ye | yes | n | no) 
						AC_MSG_ERROR(You must supply an argument to --with-alsalib);;
			  		*) 
						ALSADIR=${withval};;
			 	esac],
			  ALSADIR=/usr/local)

AC_ARG_WITH([moduledir],
			  AC_HELP_STRING([--with-moduledir=DIR],
			  				 [specify the path to where to install the driver modules (PREFIX/modules)]),
			  [	case ${withval} in 
					"" | y | ye | yes | n | no) 
						AC_MSG_ERROR(You must supply an argument to --with-moduledir);;
			  		*) 
						MODULE_INSTALL_DIR=${withval};;
			 	esac],
			  MODULE_INSTALL_DIR='${exec_prefix}/modules')

# Checks for programs.
AC_PROG_AWK
AC_PROG_CC
if test ${RT} != "disabled"; then
	if test ${RT} = "xenomai"; then
		AC_PATH_PROG([RTEXT_CONF], [xeno-config], "notfound", $RT_PATH/bin)
		TOOL=xeno-config
	else if test ${RT} = "rtai"; then
		AC_PATH_PROG([RTEXT_CONF], [rtai-config], "notfound", $RT_PATH/bin)
		TOOL=rtai-config
	fi
	fi
	if test ${RTEXT_CONF} = "notfound"; then
		AC_MSG_ERROR(Cannot find $TOOL in $RT_PATH/bin.
		Please make sure that the realtime extension is installed or that the 
		correct path is specified with the option --with-realtime-extensiondir)
	fi
fi

#check for kernel headers
if ! test -r ${KERNELDIR}/Makefile; then
	AC_MSG_ERROR(Cannot find the kbuild system of the Linux kernel.
		Please make sure that the Linux kernel headers and or the Linux
		kernel source is installed or make sure that the the option 
		--with-kerneldir=DIR points to the right location.)
fi

if test ${RT} = "xenomai"; then
	KMOD_CFLAGS="$(${RTEXT_CONF} --xeno-cflags)"
else if test ${RT} = "rtai"; then
	KMOD_CFLAGS="$(${RTEXT_CONF} --module-cflags)"
fi
fi

CFLAGS="${CFLAGS} -Wall -Wno-unused"
AC_SUBST(RT)
AC_SUBST(KMOD_CFLAGS)
AC_SUBST(MODULE_INSTALL_DIR)
AC_SUBST(KERNELDIR)


# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([fcntl.h limits.h stdlib.h string.h strings.h sys/ioctl.h sys/param.h sys/time.h unistd.h])

ac_CFLAGS_SAVE=${CFLAGS}
CFLAGS="${CFLAGS} -I${ALSADIR}/include"
AC_CHECK_HEADERS([alsa/asoundlib.h],[HAVE_ALSA=1],
AC_MSG_WARN("Unable to find ALSA headers. 
			Please make sure alsa-lib is installed or specify the installation prefix to --with-alsalib=DIR.
			Otherwise the alsa example in the examples directory will not be compiled"),
[[#if HAVE_ALSA_ASOUNDLIB_H
# include <alsa/asoundlib.h>
#endif
]])
CFLAGS=${ac_CFLAGS_SAVE}

if test x${HAVE_ALSA} = x1; then
	ac_LDFLAGS_SAVE=${LDFLAGS}
	LDFLAGS="${LDFLAGS} -L${ALSADIR}/lib"
	AC_CHECK_LIB(asound, main, [HAVE_ALSA_LIB=1],
AC_MSG_WARN("Unable to find libasound.
			Please make sure alsa-lib is installed or specify the installation prefix to --with-alsalib=DIR.
			Otherwise the alsa example in the examples directory will not be compiled")
	)
	LDFLAGS=${ac_LDFLAGS_SAVE}
	AC_SUBST(ALSADIR)
fi

if test x${HAVE_ALSA_LIB} = x1; then
	ALSA_OUTPUT=enabled
else
	ALSA_OUTPUT=disabled
fi

AM_CONDITIONAL(WITH_ALSA_EXAMPLE, [test x${HAVE_ALSA_LIB} = x1])

#now check for some source code files of the MOST netservices exists in --with-netservices directory
#to detect if netservices can be compiled
NETSERVICES_DIR=${srcdir}/libnetservices/src/smsc
AC_CHECK_FILE(${NETSERVICES_DIR}/mns.c, HAVE_MNS_C=1)
if test x${HAVE_MNS_C} = x1; then
	AC_CHECK_FILE(${NETSERVICES_DIR}/cms.c, HAVE_CMS_C=1)
fi
# final test
if test x${HAVE_CMS_C} = x1; then
	AC_CHECK_FILE(${NETSERVICES_DIR}/msv.c, HAVE_NETSERVICES_SOURCE=1)
fi
AM_CONDITIONAL(WITH_NETSERVICES, [test x${HAVE_NETSERVICES_SOURCE} = x1])
if test x${HAVE_NETSERVICES_SOURCE} = x1; then
	NS_OUTPUT=enabled
else
	NS_OUTPUT=disabled
fi


# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_SSIZE_T
AC_HEADER_TIME
AC_TYPE_UINT64_T
AC_C_VOLATILE

# Checks for library functions.
AC_PROG_GCC_TRADITIONAL
AC_PROG_LIBTOOL
AC_FUNC_MALLOC
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_STRTOD
AC_CHECK_FUNCS([floor gettimeofday memmove memset select strcasecmp strdup strerror uname])
AC_CONFIG_LINKS(most-kernel/Makefile:most-kernel/Makefile.kbuild)
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([most-kernel/GNUmakefile])
AC_CONFIG_FILES([doc/Makefile])
AC_CONFIG_FILES([libnetservices/Makefile])
AC_CONFIG_FILES([libnetservices/include/Makefile])
AC_CONFIG_FILES([libnetservices/include/linux/Makefile])
AC_CONFIG_FILES([libnetservices/include/user_adjust/Makefile])
AC_CONFIG_FILES([libnetservices/include/smsc/Makefile])
AC_CONFIG_FILES([libnetservices/src/Makefile])
AC_CONFIG_FILES([libnetservices/src/linux/Makefile])
AC_CONFIG_FILES([libnetservices/src/smsc/Makefile])
AC_CONFIG_FILES([examples/Makefile])
AC_CONFIG_FILES([examples/src/Makefile])

AM_CONDITIONAL(RT_SUPPORT, [test x${RT} != xdisabled])
AC_OUTPUT

if test ${MODULE_INSTALL_DIR} = '${exec_prefix}/modules'; then
	MODULE_INSTALL_DIR_OUT=${prefix}/modules
else
	MODULE_INSTALL_DIR_OUT=${MODULE_INSTALL_DIR}
fi

cat << EOF

Config files successfully generated by ./configure

Install prefix						${prefix}
Install path for driver modules				${MODULE_INSTALL_DIR_OUT}
Linux kernel source tree				${KERNELDIR}
Build driver with real-time support			${RT}
Compile MOST NetServices library			${NS_OUTPUT}
Compile with ALSA example				${ALSA_OUTPUT}
Print debug messages					${debug}
Print trace messages					${trace}
Print debug messages on all register accesses	 	${reg_access_debug}
Print debug messages for ioctl syscall			${ioctl_debug}
Print debug messages for OS-8104 register access	${devfunc_debug}
Print debug messages in non real-time ISRs		${irq_debug}
Print debug messages in real-time ISRs			${rt_irq_debug}
Print debug messages for receive buffer			${rxbuf_debug}
Print debug messages for transmit buffer		${txbuf_debug}
Print debug messages in sync module			${sync_debug}
Print debug messages in NetServices module		${nets_debug}
Print debug messages in alsa driver			${alsa_debug}
Debugging over serial port in realtime			${serial_rt_debug}
dnl Measure interrupt latency				${measuring_pci}
dnl Measure schedule latency				${measuring_sched}

Now type 'make' to compile and afterwards 'make install' to install the driver
EOF
