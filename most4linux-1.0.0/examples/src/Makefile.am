if WITH_ALSA_EXAMPLE
bin_PROGRAMS = ctrl_tx sync_tx sync_rx most_aplay
else
bin_PROGRAMS = ctrl_tx sync_tx sync_rx
endif

ctrl_tx_SOURCES = ctrl-tx.c
ctrl_tx_CFLAGS = -I${top_srcdir}/examples/ \
	-I${top_srcdir}

sync_tx_SOURCES = sync-tx.c
sync_tx_CFLAGS = -I${top_srcdir}/examples/ \
	-I${top_srcdir}
	
sync_rx_SOURCES = sync-rx.c
sync_rx_CFLAGS = -I${top_srcdir}/examples/ \
	-I${top_srcdir}

if WITH_ALSA_EXAMPLE
most_aplay_SOURCES = most-aplay.c
most_aplay_CFLAGS = -I${ALSADIR}/include/ \
	-I${top_srcdir}/examples/ \
	-I${top_srcdir}
most_aplay_LDFLAGS = -L${ALSADIR}/lib/ -lasound
endif

nets_usp_headersdir = $(includedir)/most-nets-usp/
nets_usp_headers_HEADERS = ${top_srcdir}/examples/most-nets-usp/par_cp.h \
	${top_srcdir}/examples/most-nets-usp/registers.h

