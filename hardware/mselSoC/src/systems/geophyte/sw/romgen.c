/*
   Copyright 2015, Google Inc.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

void fatal_error(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fprintf(stderr, "\n");
	exit(1);
}

void usage(const char *prog_name)
{
	exit(0);
}

int main(int argc, char *argv[])
{
	char *ifname, *ofname;
	int i;
	unsigned long istart, iend, ilen, ostart, ipos, fsize;
	unsigned int val, def_val, def_set;
	unsigned char buf[4];
	FILE *in_fd, *out_fd;

	ifname = ofname = NULL;
	istart = iend = ilen = ostart = 0;
	def_set = 0;

	for(i = 1; i < argc; i++)
	{
		if(strcmp(argv[i], "--help") == 0
			|| strcmp(argv[i], "-h") == 0)
		{
			usage(argv[0]);
		}
		if(strcmp(argv[i], "--input") == 0
			|| strcmp(argv[i], "-i") == 0)
		{
			i++;
			if(i >= argc)
				fatal_error("insufficent arguments");
			ifname = argv[i];
			continue;
		}
		if(strcmp(argv[i], "--output") == 0
			|| strcmp(argv[i], "-o") == 0)
		{
			i++;
			if(i >= argc)
				fatal_error("insufficent arguments");
			ofname = argv[i];
			continue;
		}
		if(strcmp(argv[i], "--istart") == 0
			|| strcmp(argv[i], "-is") == 0)
		{
			i++;
			if(i >= argc)
				fatal_error("insufficent arguments");
			istart = strtoul(argv[i], NULL, 0);
			continue;
		}
		if(strcmp(argv[i], "--iend") == 0
			|| strcmp(argv[i], "-ie") == 0)
		{
			i++;
			if(i >= argc)
				fatal_error("insufficent arguments");
			iend = strtoul(argv[i], NULL, 0);
			continue;
		}
		if(strcmp(argv[i], "--ilen") == 0
			|| strcmp(argv[i], "-il") == 0)
		{
			i++;
			if(i >= argc)
				fatal_error("insufficent arguments");
			ilen = strtoul(argv[i], NULL, 0);
			continue;
		}
		if(strcmp(argv[i], "--ostart") == 0
			|| strcmp(argv[i], "-os") == 0)
		{
			i++;
			if(i >= argc)
				fatal_error("insufficent arguments");
			ostart = strtoul(argv[i], NULL, 0);
			continue;
		}
		if(strcmp(argv[i], "--def-val") == 0
			|| strcmp(argv[i], "-dv") == 0)
		{
			i++;
			if(i >= argc)
				fatal_error("insufficent arguments");
			def_val = strtoul(argv[i], NULL, 0);
			def_set++;
			continue;
		}
		fatal_error("invalid argument %s", argv[i]);
	}

	if(!ifname)
		fatal_error("input filename not specified");
	if(!ofname)
		fatal_error("output filename not specified");
	
	in_fd = fopen(ifname, "rb");
	if(!in_fd)
		fatal_error("failed to open input file: %s", ifname);
	fseek(in_fd, 0, SEEK_END);
	fsize = ftell(in_fd);
	fseek(in_fd, 0, SEEK_SET);

	if(!ilen && !iend)
		iend = fsize;
	if(iend)
	{
		if(iend < istart)
			fatal_error("iend < istart");
		ilen = iend - istart;
	}
	else
		iend = istart + ilen;

	if(iend - istart > fsize)
		fatal_error("specified size larger that input file size");
	if(!ilen)
		fatal_error("zero input size");

	out_fd = fopen(ofname, "w");
	if(!out_fd)
		fatal_error("failed to open output file: %s", ofname);

	fseek(in_fd, istart, SEEK_SET);

	for(ipos = 0; ipos < ilen; ipos += sizeof(buf))
	{
		memset(buf, 0, sizeof(buf));
		fread(buf, sizeof(buf[0]), sizeof(buf), in_fd);

		val = buf[0];
		val <<= 8;
		val |= buf[1];
		val <<= 8;
		val |= buf[2];
		val <<= 8;
		val |= buf[3];
		if(def_set && val == def_val)
			continue;
		fprintf(out_fd, "%u: wb_dat_o <= 32'h%02x%02x%02x%02x;\n",
			(ipos + ostart) / 4, buf[0], buf[1], buf[2], buf[3]);
	}
	fclose(out_fd);
	fclose(in_fd);
	return 0;
}

