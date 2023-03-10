/*
 * Automatic truncate - tool to truncate away zeros from end of a file
 *
 * Copyright (C) 2014 Jolla Ltd. All rights reserved.
 * Contact: Kalle Jokiniemi <kalle.jokiniemi@jolla.com>
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

/* Needed for O_LARGEFILE */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <getopt.h>
#include <errno.h>
#include <inttypes.h>
#include <sys/stat.h>

/* Use 128kB search chunk size (4096 * 32) */
#define SEARCH_CHUNK_SIZE	131072
#define BLOCK_SIZE		4096

static struct option options[] = {
	{"help",     no_argument,       0, 'h'},
	{0, 0, 0, 0}
};

static void short_help()
{
	printf("  atruncate [OPTIONS] FILE [DEST_FILE]\n");
}

static void print_help()
{
	printf("  atruncate - Remove contiguous zero bytes from EOF\n");
	printf("  Usage:\n");
	short_help();
	printf("    FILE      - the source file to truncate\n");
	printf("    DEST_FILE - Destination file, if omitted, FILE is used\n");
	printf("\n  OPTIONS:\n");
	printf("  --help, -h      Print this help\n");
}

void find_last_byte(off_t *result, int fd, off_t start, off_t end)
{
	char buf[SEARCH_CHUNK_SIZE];
	char byte = 0;
	int i = 1;
	int j = 0;
	off_t cur_offset;
	size_t size = SEARCH_CHUNK_SIZE;
	ssize_t count;

	memset((void *)buf, 0, SEARCH_CHUNK_SIZE);

	while (byte == 0 || byte == EOF) {
		if ((end - i * SEARCH_CHUNK_SIZE) < 0) {
			/* Cannot read full chunk, only read what is left */
			cur_offset = start;
			size = end % SEARCH_CHUNK_SIZE;
		} else {
			cur_offset = end - i * SEARCH_CHUNK_SIZE;
		}

		count = pread(fd, buf, size, cur_offset);

		if (count < 0) {
			perror("Could not read file");
		} else if (count != size) {
			printf("warning: read %d bytes, expected to get %u\n",
				(unsigned int) count, (unsigned int) size);
			size = count;
		}

		/* Let's check for any non-zero data */
		for (j = size - 1; j >= 0; j--) {
			if (buf[j] != 0) {
				cur_offset += j;
				*result = cur_offset + 1;
				return;
			}
		}
		i++;
	}
}

/* Find the BLOCK_SIZE aligned position after last non-zero byte in the file */
off_t find_last_block(const char *f)
{
	struct stat props;
	int ret = 0;
	off_t current_offset = 0;
	int fd;

	fd = open(f, O_RDONLY | O_LARGEFILE);
	if (fd < 0) {
		perror("Could not open source file");
		current_offset = fd;
		goto find_exit;
	}

	ret = stat(f, &props);
	if (ret) {
		printf("Could not stat file\n");
		current_offset = ret;
		goto find_exit;
	}

	find_last_byte(&current_offset, fd, 0, props.st_size);

	/* Align to next block size */
	if (current_offset % BLOCK_SIZE) {
		current_offset += BLOCK_SIZE;
		current_offset = current_offset - (current_offset % BLOCK_SIZE);
	}

	close(fd);
find_exit:
	return current_offset;
}

static int do_truncate(char *src, char *dst)
{
	off_t size = 0;
	char cp_command[128];

	size = find_last_block(src);
	if (size < 0) {
		printf("Could not find last byte\n");
		return -1;
	}

	if (!strcmp(src, dst)) {
		if (truncate(src, size)) {
			printf("Could not truncate %s\n", src);
			return -1;
		}
	} else {
		sprintf(cp_command, "cp -f %s  %s", src, dst);

		if (system(cp_command))
			return -1;

		if (truncate(dst, size)) {
			printf("Could not truncate %s\n", dst);
			return -1;
		}
	}

	return 0;
}

int main (int argc, char *argv[])
{
	int c, option_index = 0;
	char *source_file;
	char *dest_file;
	while (1) {
		c = getopt_long (argc, argv, "h", options, &option_index);

		if (c == -1)
			break;

		switch (c) {
		case 'h':
			print_help();
			break;
		}
	}

	if (optind < argc) {
		source_file = argv[optind++];
	} else {
		printf("Please provide source FILE\n");
		goto fail_help;
	}

	if (optind < argc)
		dest_file = argv[optind++];
	else
		dest_file = source_file;

	if (do_truncate(source_file, dest_file))
		goto fail;

	return EXIT_SUCCESS;
fail_help:
	short_help();
fail:
	return EXIT_FAILURE;
}
