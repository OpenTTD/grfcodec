/* A class for dealing with .NFO files */

#include <ctype.h>

#include "info.h"
#include "sprites.h"
#include "error.h"

const char *infoline = "%s %d %d %02X %d %d %d %d";
//		<PCX-File> <X> <Y> <info[0..7]>
//		extended if info[0]&8: info[1]*<%d %d> linelen linestart


int makeint(U8 low, S8 high)
{
  S16 combined;

  combined = (high << 8) | low;
  return combined;
}


inforeader::inforeader(FILE *fn)
{
  f = fn;

  pcx = NULL;
  pcxname = NULL;
  buffer = NULL;
  bininclude = NULL;

  fseek(f, 0, SEEK_END);
  filesize = ftell(f);
  fseek(f, 0, SEEK_SET);

  readline();		// read first line, a comment

  if (strncmp(buffer, "// ", 3)) {
	printf("NFO file missing header lines and version info\n");
	exit(1);
  }
  readline();		// second line used to get info version
  if (strncmp(buffer, "// (Info version ", 17)) {
	infover = 1;
	fseek(f, 0, SEEK_SET);
	readline();
  } else
	infover = atoi(buffer + 17);

  if (infover > 2)
	readline();	// skip "format: " line

  lasty = -1;

  colourmap = NULL;
  verbatim_str = 0;
}

inforeader::~inforeader()
{
  if (buffer)
	free(buffer);
  if (pcxname)
	free(pcxname);
  if (bininclude)
	free(bininclude);
  fclose(f);
}

int inforeader::makebufferlarger()
{
  int oldsize;

  if (!buffer) {
	buffersize = 128;
	buffer = (char*) malloc(buffersize);
	if (!buffer) {
		printf("\nOut of memory.\n");
		exit(2);
	}
	return buffersize;
  }

  oldsize = buffersize;
  buffersize *= 2;
  buffer = (char*) realloc(buffer, buffersize);
  if (!buffer) {
	printf("\nOut of memory.\n");
	exit(2);
  }

  return oldsize;
}

int inforeader::readline(char *prepend)
{
  char *pos;
  int readmore;

  if (!buffer)
	makebufferlarger();

  if (prepend) {
	readmore = strlen(prepend);
	while (buffersize < readmore)
		makebufferlarger();

	strncpy(buffer, prepend, buffersize);
	pos = buffer + readmore;
	readmore = buffersize - readmore;
  } else {
	pos = buffer;
	readmore = buffersize;
  }

  while (1) {
	fgets(pos, readmore-1, f);
	if ( feof(f) || strchr(pos, '\n') )
		break;

	// line didn't fit in buffer
	readmore = makebufferlarger();
	pos = buffer + readmore;
	readmore = buffersize - readmore;
  }

  return 1;
}

int inforeader::next(int wantno)
{
  int i;
  char linehead[8], *p;

  if (feof(f)) return 0;

  while (1) {
	i = -1;
	if (infover >= 2)
		fscanf(f, "%5d ", &i);
	if (i == -1) 
		i = wantno;

	fscanf(f, "%6s", linehead);
	if ( (strlen(linehead) == 0) || feof(f) )
		return 0;

	if (!strchr("#/;", linehead[0]))
		break;

	// is a comment
	readline();
  }

  if (i != wantno) {
	printf("\nError: Sprite number mismatch, found %d but wanted %d\n", i, wantno);
	exit(2);
  }

  if (bininclude) {
	free(bininclude);
	bininclude = NULL;
  }

  if (strcmp(linehead, "*") == 0) {
	verbatim = 1;
	fscanf(f, "%hd ", &size);	// read how many bytes
	return 1;
  }
  if (strcmp(linehead, "**") == 0) {
	verbatim = 1;
	readline();		// read rest of line
	p = buffer;
	while (isspace(*p)) { p++; };
	p[strcspn(p, "\r\n")] = 0;
	bininclude = strdup(p);
	return 1;
  }

  verbatim = 0;
  readline(linehead);		// read rest of line
  p = buffer-1;
  while (1) {
	p = strchr(p+1, '.');
	if (!p) {
		printf("\nCan't find file name in line\n%s\n", buffer);
		exit(2);
	}
	if (strnicmp(p, ".pcx ", 4) == 0)
		break;
  }
  p += 4;
  p[0] = 0;
  p++;

  if ( !pcx || !pcxname || (stricmp(buffer, pcxname) != 0) ) {
	// new file

	if (pcx)
		delete(pcx);
	if (pcxname)
		free(pcxname);

	printf("Loading %s\n", buffer);

	pcxname = strdup(buffer);
	if (!pcxname) {
		printf("\nOut of memory.\n");
		exit(2);
	}

	pcx = new pcxread(new singlefile(pcxname, "rb", NULL));
	if (!pcx) {
		printf("\nError: can't open %s\n", pcxname);
		exit(2);
	}

	if (colourmap)
		pcx->installreadmap(colourmap);

	pcx->startimage(0, 0, 0, 0, NULL);
  }

  int x, y;

  if (infover<3) {
	sscanf(p, "%d %d %x %x %x %x %x %x %x %x",
		&x, &y,
		&(intinf[0]), &(intinf[1]), &(intinf[2]), &(intinf[3]),
		&(intinf[4]), &(intinf[5]), &(intinf[6]), &(intinf[7]));
  } else {
	int rx, ry;
	sscanf(p, "%d %d %x %d %d %d %d",
		&x, &y,
		&(intinf[0]), &(intinf[1]),
		&sx, &rx, &ry);
	intinf[2] = sx & 0xff;
	intinf[3] = sx >> 8;
	intinf[4] = rx & 0xff;
	intinf[5] = rx >> 8;
	intinf[6] = ry & 0xff;
	intinf[7] = ry >> 8;
  }

  if (y < lasty) {
	printf("\nError: y is %d, smaller than %d. Y can't decrease!\n", y, lasty);
	exit(2);
  }

  sx = (intinf[3] << 8) | intinf[2];
  sy = intinf[1];

  for (i=0; i<8; i++)
	inf[i] = intinf[i];

  if (infover < 4)
	y++;	// bug, had an extra line at the top

  pcx->startsubimage(x, y, sx, sy);

  imgsize = (long) sx * (long) sy;

  return 1;
}

void inforeader::uncommentstream(int &byte)
{
	while (1) {
		byte = fgetc(f);
		//printf("%c", byte);
		if ( feof(f) ) break;
		if ( byte == '\n') {
			do { byte = fgetc(f); } while (isspace(byte));
			if ( feof(f) ) break;
			if ( (byte == '/') || (byte == '#') || (byte == ';') ) {
				//printf("%c", byte);
			} else {
				ungetc(byte, f);
				break;
			}
		} 
  	}
}

int inforeader::nextverb()
{
  int byte;
  //printf(": ");
  
  checkagain:
  byte = fgetc(f);
  if (byte == '"') {
	//printf("Switched quoting from %d\n", verbatim_str);
	verbatim_str = !verbatim_str;

	if (!verbatim_str) {
		do { byte = fgetc(f); } while (isspace(byte));
		if (byte == '"') {
			verbatim_str = 1;
		} else {
			ungetc(byte, f);
		}
	}
  } else {
	ungetc(byte, f);
  }

  // remove comments from stream, handle comments on multiple lines aswell
  if (!verbatim_str && ( (byte == '/') || (byte == '#') || (byte == ';') ) ) {
	// a comment
	uncommentstream(byte);
	//http://kerneltrap.org/node/553 - And yes, I think it's more readable and much easier...
	goto checkagain;
  }

  if (verbatim_str) {
	byte = fgetc(f);
	//printf("Read quoted byte %c\n", byte);
  } else {
        fscanf(f, "%2x ", &byte);
	//printf("Read unquoted byte %d\n", byte);
  }
  return byte;
}

int inforeader::getsprite(U8 *sprite)
{
  pcx->streamgetpixel(sprite, imgsize);
  return 1;
}

void inforeader::installmap(int *map)
{
	colourmap = map;
}


infowriter::infowriter(FILE *info, int maxboxes, int useplaintext)
{
  infowriter::info = info;
  fputs("// Automatically generated by GRFCODEC. Do not modify!\n", info);
  fputs("// (Info version 6)\n", info);
  fputs("// Format: spritenum pcxfile xpos ypos compression ysize xsize xrel yrel\n", info);

  infowriter::useplaintext = useplaintext;
  infowriter::maxboxes = maxboxes;
  boxes = new box[maxboxes];
  boxnum = 0;
  spriteno = 0;
  for (int i=0; i<maxboxes; i++)
	boxes[i].type = isinvalid;
}

void infowriter::newband(pcxfile *pcx)
{
  int i;
  U16 j;

  for (i=0; i<boxnum; i++) {
	fprintf(info, "%5d ", spriteno++);
	switch (boxes[i].type) {
		case issprite: {
			box::foo::boxsprite *s = &(boxes[i].h.sprite);

			fprintf(info, infoline, pcx->filename(), s->x,
				pcx->subimagey(),
				s->info[0], s->info[1],
				makeint(s->info[2], s->info[3]),
				makeint(s->info[4], s->info[5]),
				makeint(s->info[6], s->info[7]));
			fputs("\n", info);
			break;  }

		case isdata:	{
			box::foo::boxdata *d = &(boxes[i].h.data);
			int instr = 0;
			int count = 0;

			if (d->data[0] == 0xff && d->size>4 && spriteno>1) {	// binary include file
				int namelen = d->data[1];
				char *filename = new char[strlen(pcx->getdirectory())+namelen+1];

				strcpy(filename, pcx->getdirectory());
				strcat(filename, (char*) d->data + 2);
				fprintf(info, "**\t %s\n", filename);

				FILE *bin = fopen(filename, "wb");
				if (!bin) {
					fperror("Cannot write to %s", filename);
					exit(2);
				}
				fwrite(d->data+3+namelen, d->size-3-namelen, 1, bin);
				fclose(bin);
			} else {
			    fprintf(info, "* %d\t", d->size);
			    for (j=0; j<d->size; j++) {
				// Readable characters are 32..126 (excluding " (34)) and 158..255
#define istxt(x) (j+(x)<d->size && ((d->data[j+(x)]>=32 && d->data[j+(x)]!=34 && d->data[j+(x)]<127) || d->data[j+(x)] > 158))
				//int thistxt = (d->data[j] >= 32 && d->data[j] < 127) || d->data[j] > 158;
				//int nexttxt = j+1<d->size && ((d->data[j+1]>=32 && d->data[j+1]<127) || d->data[j+1] > 158);

				if (istxt(0) && useplaintext && (instr || 
						//two in a row for 8 and E
						(istxt(1) && (d->data[0]==8 || d->data[0]==0xE ||
						//four in a row for everything else.
						(istxt(2) && istxt(3))
						))
					)) {
					if (!instr) {
						fputs(" \"", info); instr = 1;
					}
					fputc(d->data[j], info);
				} else {
					if (instr) {
						fputs("\"", info); instr = 0;
					}
					fprintf(info, " %02X", d->data[j]);
				}

				// break lines after 32 non-text characters
				// or at spaces after 32 text characters or
				// after 60 text characters
				count++;
				if ( ((!instr && count>=32) ||
				      (instr && (count>=32 && d->data[j]==' ' 
						|| count>=50)))
				     && j<d->size-1) {
					if (instr) {
						if(istxt(1)){
							fputs("\"\n\t \"", info);
						}else{
							fputs("\"\n\t", info); instr = 0;
						}
					}else
						fputs("\n\t", info);
					count = 0;
				}
			    }

			    if (instr) fputs("\"", info);
			    fputs("\n", info);
			}
			delete(d->data);
			break;	}

		default:
			printf("\nHuh? What kind of box is that?\n");
			exit(2);
	}
  }

  boxnum = 0;

  for (i=0; i<maxboxes; i++)
	boxes[i].type = isinvalid;
}

void infowriter::resize(int newmaxboxes)
{
  printf("Reallocating memory for %d boxes\n", newmaxboxes);
  infowriter::maxboxes = newmaxboxes;
  box *newboxes = new box[newmaxboxes];
  int i;
  for (i=0; i<maxboxes; i++)
	newboxes[i] = boxes[i];
  for (; i<newmaxboxes; i++)
	newboxes[i].type = isinvalid;
  delete(boxes);
  boxes = newboxes;
}

void infowriter::addsprite(int x, U8 info[8])
{
  if (boxnum >= maxboxes)
	resize(maxboxes*2);

  boxes[boxnum].type = issprite;
  boxes[boxnum].h.sprite.x = x;
  memcpy(boxes[boxnum].h.sprite.info, info, 8*sizeof(U8));

  boxnum++;
}

void infowriter::adddata(U16 size, U8 *data)
{
  if (boxnum >= maxboxes)
	resize(maxboxes*2);

  boxes[boxnum].type = isdata;
  boxes[boxnum].h.data.size = size;
  boxes[boxnum].h.data.data = data;

  boxnum++;
}

void infowriter::done(int count)
{
  if (count != spriteno) {
	printf("\nError: GRFCodec thinks it has written %d sprites but the info file only got %d\n", count, spriteno);
	exit(2);
  }
}


