/*
 *  Copyright(c) Siemens AG, Muenchen, Germany, 2005, 2006, 2007
 *                           Steve Kreyer   <steve.kreyer@web.de>
 *                           Bernhard Walle <bernhard.walle@gmx.de>
 *                           Gernot Hillier <gernot.hillier@siemens.com>
 *                           All rights reserved.
 *
 * ----------------------------------------------------------------------------
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See the
 * License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Siemens code.
 * 
 * The Initial Developer of the Original Code is Siemens AG.
 * Portions created by the Initial Developer are Copyright (C) 2005-06
 * the Initial Developer. All Rights Reserved.
 * ----------------------------------------------------------------------------
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <alsa/asoundlib.h>
#include "most-nets-usp/par_cp.h"
#include "most-nets-usp/registers.h"

/* !!! This is the pcm device name. Set it to your needs !!!*/
#define PCM_DEVICE_NAME "plughw:1,0"

#define WAVE_PCM_TAG_VALUE 0x01
#define IS_WAVE_PCM_CODED(tag) \
	(tag == WAVE_PCM_TAG_VALUE)
#define IS_S16LE_SAMPLE_FORMAT(bits_per_sample) \
	(bits_per_sample == 16)
#define IS_44100_HZ_SAMPLED_WAVE(sample_rate) \
	(sample_rate == 44100)

#define CHANNEL_TOSTRING(chan) \
	((chan == 1)? \
	 "Mono" \
	 : "Stereo")
#define SAMPLE_FORMAT_TOSTRING(format) \
	((format != SND_PCM_FORMAT_S16_LE)? \
	 "Unsigned 8 bit" \
	 : "Signed 16 bit Little Endian")

#define XCR_MASTER 0x80
#define XCR_SLAVE 0x00

typedef unsigned short word;
typedef unsigned char byte;
typedef unsigned int doubleword;

/* wave file header */
typedef struct {
	doubleword riff;
	doubleword file_length;
	doubleword wave;
	doubleword fmt;
	doubleword fmt_length;
	word fmt_tag;
	word channels;
	doubleword sample_rate;
	doubleword bytes_per_second;
	word block_align;
	word bits_per_sample;
	doubleword header_sig;
	doubleword data_length;
} wave_header_t;

/* global variable definitions */

/* Handle for the PCM device */ 
snd_pcm_t *g_pcm_handle;

/* name of the pci device */
wave_header_t g_wave_header;

unsigned int g_sample_format = SND_PCM_FORMAT_S16_LE;
unsigned int g_channels = 2;
unsigned int g_sample_rate = 44100;
unsigned int g_frame_length = 4;

int                     InstID = 0;
int                     g_nets_fd = 0;
int                     g_mode = XCR_SLAVE;

void print_help(void) 
{
	fprintf(stderr, "Usage: most-aplay file.wav\n\n");
}

int init_pcm_dev(const char* pcm_dev_name) 
{
	snd_pcm_hw_params_t *hwparams;
	unsigned int exact_rate;

	/* Allocate the snd_pcm_hw_params_t structure on the stack. */
	snd_pcm_hw_params_alloca(&hwparams);

	/* read out wave file setting from wave file header*/
	g_sample_rate = g_wave_header.sample_rate;
	if (g_wave_header.bits_per_sample == 8)
		g_sample_format = SND_PCM_FORMAT_U8;
	else
		g_sample_format = SND_PCM_FORMAT_S16_LE;
	if (g_wave_header.channels == 1) {
		g_channels = 1;
		if (g_sample_format == SND_PCM_FORMAT_S16_LE)
			g_frame_length = 2;
		else
			g_frame_length = 1;
	}
	else {
		g_channels = 2;
		if (g_sample_format == SND_PCM_FORMAT_S16_LE)
			g_frame_length = 4;
		else
			g_frame_length = 2;
	}

	/* open the pcm device with default mode (blocking) */
	if (snd_pcm_open(&g_pcm_handle, 
				pcm_dev_name, 
				SND_PCM_STREAM_PLAYBACK, 0) < 0) {
		fprintf(stderr, "Error opening PCM device %s.\n", pcm_dev_name);
		goto err;
	}

	/* Init hwparams with full configuration space */
	if (snd_pcm_hw_params_any(g_pcm_handle, hwparams) < 0) {
		fprintf(stderr, "Can not configure PCM device %s.\n", pcm_dev_name);
		goto err;
	}

	/* Set access type to interleaved mode */
	if (snd_pcm_hw_params_set_access(g_pcm_handle, 
				hwparams, 
				SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {

		fprintf(stderr, "Error setting access.\n");
		goto err;
	}

	/* Set sample format */
	if (snd_pcm_hw_params_set_format(g_pcm_handle, 
				hwparams, 
				g_sample_format) < 0) {
		fprintf(stderr, "Error setting format.\n");
		goto err;
	}

	/* Set sample rate. If the exact rate is not supported */
	/* by the hardware, use nearest possible rate.         */ 
	exact_rate = g_sample_rate;
	if (snd_pcm_hw_params_set_rate_near(g_pcm_handle, 
				hwparams, 
				&exact_rate, 
				0) < 0) {
		fprintf(stderr, "Error setting rate.\n");
		goto err;
	}
	if (g_sample_rate != exact_rate) {
		fprintf(stderr, "The rate %d Hz is not supported by your hardware.\n\
				==> Using %d Hz instead.\n", g_sample_rate, exact_rate);
	}

	/* Set number of channels */
	if (snd_pcm_hw_params_set_channels(g_pcm_handle, 
				hwparams, 
				g_channels) < 0) {
		fprintf(stderr, "Error setting channels.\n");
		goto err;
	}

	/* Apply HW parameter settings to */
	/* PCM device and prepare device  */
	if (snd_pcm_hw_params(g_pcm_handle, hwparams) < 0) {
		fprintf(stderr, "Error setting HW params.\n");
		goto err;
	}
	return 0;

err:
	return -1;
}

int play_pcm_data(byte* pcm_data, const char* file_name)
{
	int bytes_written = 0;
	printf("Playing WAVE '%s' : %s, Rate %d HZ, %s\n",
			file_name, 
			SAMPLE_FORMAT_TOSTRING(g_sample_format),
			g_sample_rate,
			CHANNEL_TOSTRING(g_channels));

	/* write the data */
	while ((bytes_written = snd_pcm_writei(g_pcm_handle, pcm_data,
					g_wave_header.data_length / 
					g_frame_length)) < 0) {
		snd_pcm_prepare(g_pcm_handle);
		fprintf(stderr, "<<<<<<<<<<<<<<< Buffer Underrun >>>>>>>>>>>>>>>\n");
	}
	/* wait til pending frames are played and then stop pcm device */
	snd_pcm_drain(g_pcm_handle);
	return 0;
}

int read_wave_file_header(int fd)
{
	int bytes_read = 0;

	/* read the wave header from the given file descriptor */
	if ((bytes_read = read(fd, &g_wave_header, sizeof(wave_header_t))) < 0){
		perror("Error while reading wave header information");
		goto err;
	}
	/* check if wave container is PCM coded */
	if (!IS_WAVE_PCM_CODED(g_wave_header.fmt_tag)) {
		fprintf(stderr, "most_aplay: can't play not PCM-coded WAVE-files\n");
		goto err;
	}

	if (!IS_44100_HZ_SAMPLED_WAVE(g_wave_header.sample_rate)) {
		fprintf(stderr, 
				"most_aplay: Only can play 44100 HZ sampled wave files!\n");
		goto err;
	}
	if (!IS_S16LE_SAMPLE_FORMAT(g_wave_header.bits_per_sample)) {
		fprintf(stderr, 
				"most_aplay: Only can play wave files with 16 bits per sample!\n");
		goto err;
	}
	return bytes_read;
err:
	return -1;
}

void sync_in_connect(void)
{
	static unsigned char buffer[128] = {
		/*  0 */   0x47, 0x46, 0x45, 0x44, 0x04, 0x05, 0x06, 0x07,
		/*  8 */   0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
		/* 10 */   0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
		/* 18 */   0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
		/* 20 */   0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
		/* 28 */   0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
		/* 30 */   0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
		/* 38 */   0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
		/* 40 */   0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8,
		/* 48 */   0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8,
		/* 50 */   0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8,
		/* 58 */   0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8,
		/* 60 */   0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8,
		/* 68 */   0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8,
		/* 70 */   0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8,
		/* 78 */   0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8  };

	regwriteblock(g_nets_fd, 0, 128, buffer);
}

int init8104(void)
{

	char            buffer[32];
	unsigned char pll_input_select = 0;
	unsigned char   block[] = {
		0x00, 0x04, 0x04, 0x00, 0x00, 0x7f, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00 };

	snprintf(buffer, sizeof(buffer), "/dev/mostnets%d", InstID);
	g_nets_fd = open(buffer, O_RDWR);
	if (g_nets_fd < 0) {
		perror("Could not open NetServices");
		return -1;
	}

	/* if device mode is configured to work in slave mode
	 * the PLL input select (CM1.MX[0:1] bits) must be set to 00 */
	if (g_mode == XCR_SLAVE)
		pll_input_select = 0x00;
	/* otherwise set PLL input select to 0x2 */
	else
		pll_input_select = 0x2;

	/* Interrupt Enable register -- no interrupts */
	regwrite(g_nets_fd, MOSTREG_IE /* 0x88 */, 0xff);

	/* Node Address Low Register */
	regwrite(g_nets_fd, MOSTREG_NAL /* 0x8b */, 0xfe);

	/* Transceiver Status Register */
	sleep(1);
	regwrite(g_nets_fd, MOSTREG_XSR /* 0x81 */, 0x40);

	/* Transceiver Control Register */
	regwrite(g_nets_fd, 
			MOSTREG_XCR /* 0x80 */, 
			0x62 | g_mode /* ->setting master or slave mode */);

	/* Transceiver Status Register */
	regwrite(g_nets_fd, MOSTREG_XSR /* 0x81 */, 0x40);

	/* Transceiver Status Register2 */
	regwrite(g_nets_fd, MOSTREG_XSR2 /* 0x97 */, 0x02);

	/* Source Data Control Register 1 */
	regwrite(g_nets_fd, MOSTREG_SDC1 /* 0x82 */, 0x3);

	/* Packet Priority Register 1 */
	regwrite(g_nets_fd, MOSTREG_PPI /* 0xf2 */, 0x1);

	/* Clock Manager Register */
	regwrite(g_nets_fd, MOSTREG_CM1 /* 0x83 */, 0x54 | pll_input_select);

	/* source data control register 2 */
	regwrite(g_nets_fd, MOSTREG_SDC2 /* 0x8c */, 0xa8);

	/* source data control register 3 */
	regwrite(g_nets_fd, MOSTREG_SDC3 /* 0x8d */, 0x0);

	/* clock manager 4 register */
	regwrite(g_nets_fd, MOSTREG_CM4 /* 0x93 */, 0xc3); /* 0xc0 */

	/* Synchronous Bandwidth Control Register */
	regwrite(g_nets_fd, MOSTREG_SBC /* 0x96 */, 0x04); /* new */

	/* Transceiver Status Register */
	sleep(1);
	regwrite(g_nets_fd, MOSTREG_XSR /* 0x81 */, 0x40);

	/* Synchronous Bandwidth Control Register */
	regwrite(g_nets_fd, MOSTREG_SBC /* 0x96 */, 0xf); /* new */

	/* deallocate all */
	regwrite(g_nets_fd, 0x3d6, 0x7f);
	regwrite(g_nets_fd, 0x3d2, 0x1);

	/* writeblock */
	regwriteblock(g_nets_fd, 0xc0, 21, block);

	/* set node address */
	regwrite(g_nets_fd, MOSTREG_NAH /* 0x8a */, 0xab);
	regwrite(g_nets_fd, MOSTREG_NAL /* 0x8b */, 0xcd);

	/* deallocate all */
	regwrite(g_nets_fd, 0x3d6, 0x7f);
	regwrite(g_nets_fd, 0x3d2, 0x1);

	/* Message control Register */
	regwrite(g_nets_fd, MOSTREG_MSGC /* 0x85 */, 0x80);

	/* wait */
	sleep(1);

	return 0;
}

void cleanup(int fd, byte* pcm_data) 
{
	if (fd && fd != STDIN_FILENO)
		close(fd);
	if (pcm_data)
		munmap(pcm_data, g_wave_header.data_length);
}

int main(int argc, char** argv) 
{
	int fd = 0;
	const char* pcm_dev_name = PCM_DEVICE_NAME;
	byte* pcm_data = NULL;

	if (argc == 2)
		fd = open(argv[1], O_RDONLY | O_NDELAY);
	else {
		print_help();
		goto err;
	}

	if (fd < 0) {
		perror("Error while opening audiofile");
		goto err;
	}

	bzero(&g_wave_header, sizeof(wave_header_t));
	if (read_wave_file_header(fd) < 0)
		goto err;

	if (init_pcm_dev(pcm_dev_name) < 0)
		goto err;

	/* Mapping audiofile to virtual memory */
	pcm_data = mmap(NULL, 
			g_wave_header.file_length + 8, 
			PROT_READ, 
			MAP_SHARED, 
			fd, 
			0);
	if (pcm_data == MAP_FAILED) {
		perror("Error while mapping file into virtual memory");
		goto err;
	}
	/* point the mapped memory pointer to the pcm data */
	pcm_data += sizeof(wave_header_t);

	printf("Initializing the MOST controller...");
	fflush(stdout);
	if (init8104() < 0) {
		goto err;
	}
	printf(" done.\n");
	sync_in_connect();

	/* wait */
	sleep(1);

	if (play_pcm_data(pcm_data, argv[1]))
		goto err;

	cleanup(fd, pcm_data);
	return 0;

err:
	cleanup(fd, pcm_data);
	return 1;
}
