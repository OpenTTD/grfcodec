/* A class for dealing with .NFO files */

#include <ctype.h>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <map>
#include <climits>

#include "info.h"
#include "sprites.h"
#include "error.h"
#include "escapes.h"
#include "grfcomm.h"

#define infoline "%s %d %d %02X %d %d %d %d"
//		<PCX-File> <X> <Y> <info[0..7]>
//		extended if info[0]&8: info[1]*<%d %d> linelen linestart

extern bool _interactive;

int makeint(U8 low, S8 high)
{
	S16 combined;

	combined = (high << 8) | low;
	return combined;
}

void read_file(istream&in,int infover,AllocArray<Sprite>&sprites);

map<string,int> nfo_escapes;

inforeader::inforeader(char *fn)
{
	ifstream f;
	f.open(fn);

	string buffer;
	int infover;

	imgfile = NULL;
	imgname = NULL;


	getline(f,buffer);		// read first line, a comment

	if (strncmp(buffer.c_str(), "// ", 3)) {
		printf("NFO file missing header lines and version info\n");
		exit(1);
	}
	getline(f,buffer);		// second line used to get info version
	if (strncmp(buffer.c_str(), "// (Info version ", 17)) {
		infover = 1;
		f.seekg(0);
	} else
		infover = atoi(buffer.c_str() + 17);

	if (infover > 2)
		getline(f,buffer);	// skip "format: " line

	if (infover > 6) {
		while (strncmp(buffer.c_str(), "// Format: ", 11)) {
			if (!strncmp(buffer.c_str(), "// Escapes: ", 12)) { // Nope. That was an escapes line.
				istringstream esc(buffer);
				string str;
				int byte = 0;
				esc.ignore(12);
				while (esc>>str){
					if(str == "=") byte--;
					else if (str[0] == '@')
						byte = strtol(str.c_str()+1, NULL, 16);
					else nfo_escapes.insert(pair<string, int>(str, byte++));
				}
			}
			getline(f, buffer);	// Try again to skip "format: " line
		}
	}

	colourmap = NULL;

	try{
		read_file(f,infover,nfofile);
	}catch(Sprite::unparseable e){
		printf("%s", e.reason.c_str());
		exit(1);
	}
}

int findescape(string str) {
	for(uint i=0;i<num_esc;i++)
		if(str == escapes[i].str+1)
			return escapes[i].byte;
	map<string,int>::iterator ret = nfo_escapes.find(str);
	if(ret == nfo_escapes.end())
		return -1;
	return ret->second;
}

inforeader::~inforeader()
{
	delete imgfile;
}

void inforeader::PrepareReal(const Real&sprite){
	if ( sprite.reopen() || !imgfile || !imgname || (stricmp(sprite.GetName(), imgname) != 0) ) {
		// new file

		delete imgfile;

		if (_interactive) {
			printf("Loading %s\n", sprite.GetName());
		}

		imgname = sprite.GetName();
		imgfile = MakeReader();
		if (!imgfile) {
			printf("\nError: can't open %s\n", imgname);
			exit(2);
		}

		if (colourmap)
			imgfile->installreadmap(colourmap);

		imgfile->startimage(0, 0, 0, 0, NULL);
	}

	inf = sprite.inf;

	sx = inf.xdim;
	sy = inf.ydim;

	imgfile->startsubimage(sprite.x(), sprite.y(), sx, sy);

	imgsize = (long) sx * (long) sy;
}

pcxread* inforeader::MakeReader()const{
	//if(toupper(name[strlen(name)-1])=='X')//pcx
		return new pcxread(new singlefile(imgname, "rb", NULL));
	//else //png
	//	return new pngread(new singlefile(name, "rb", NULL));
}

int inforeader::getsprite(U8 *sprite)
{
	imgfile->streamgetpixel(sprite, imgsize);
	return 1;
}

void inforeader::installmap(int *map)
{
	colourmap = map;
}

extern int _useexts;

infowriter::infowriter(FILE *info, int maxboxes, int useplaintext)
{
	infowriter::info = info;
	fputs("// Automatically generated by GRFCODEC. Do not modify!\n", info);
	fprintf(info,"// (Info version %d)", _useexts ? 7 : 6);
	if(_useexts) {
		int oldbyte = INT_MAX;
		for(uint i=0;i<num_esc;i++){
			if(escapes[i].action == 9) continue;
			if(escapes[i].byte < oldbyte) {
				fputs("\n// Escapes:", info);
				oldbyte = -1;
			} else if(escapes[i].byte == oldbyte) {
				fprintf(info, " =");
				--oldbyte;
			}
			while (++oldbyte != escapes[i].byte)
				fprintf(info," %s",escapes[0].str+1);
			fprintf(info, " %s", escapes[i].str+1);
		}
	}
	fputs("\n// Format: spritenum pcxfile xpos ypos compression ysize xsize xrel yrel\n", info);

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
				s->info.info,
				s->info.ydim, s->info.xdim,
				s->info.xrel, s->info.yrel);
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
				cfwrite("writing binary include", d->data+3+namelen, d->size-3-namelen, 1, bin);
				delete[]filename;
				fclose(bin);
			} else {
				fprintf(info, "* %d\t", d->size);
				for (j=0; j<d->size; j++) {
					// Readable characters are 32..126 and 158..255
#define istxt(x) (j+(x)<d->size && ((d->data[j+(x)]>=32 && (_useexts || d->data[j+(x)] != 34) && d->data[j+(x)]<127) || d->data[j+(x)] > 158))
					//int thistxt = (d->data[j] >= 32 && d->data[j] < 127) || d->data[j] > 158;
					//int nexttxt = j+1<d->size && ((d->data[j+1]>=32 && d->data[j+1]<127) || d->data[j+1] > 158);

					if (spriteno>1 &&
							istxt(0) && useplaintext && (instr ||
							//two in a row for 8 and E
							(istxt(1) && (d->data[0]==8 || d->data[0]==0xE ||
							//four in a row for everything else.
							(istxt(2) && istxt(3))
							))
							)) {
						if (!instr) {
							fputs(" \"", info); instr = 1;
						}
						if(_useexts && d->data[j]=='\\')
							fputs("\\\\", info);
						else if(_useexts && d->data[j]=='"')
							fputs("\\\"", info);
						else
							fputc(d->data[j], info);
					} else {
						if (instr) {
							fputs("\"", info); instr = 0;
						}
						uint k=0;
						if (_useexts == 2 && spriteno>1) {
							for(;k<num_esc;k++)
								if(escapes[k].byte==d->data[j]&&
									escapes[k].action==d->data[0]&&
									(escapes[k].override?escapes[k].override(d->data,j):escapes[k].pos==j)&&
									(escapes[k].additional==NULL||escapes[k].additional(d->data,d->size))){
										fprintf(info," %s",escapes[k].str);
										break;
									}
						}else
							k = num_esc;
						if(k==num_esc)
							fprintf(info, " %02X", d->data[j]);
					}

					// break lines after 32 non-text characters
					// or at spaces after 32 text characters or
					// after 60 text characters
					count++;
					if ( ((!instr && count>=32) ||
							(instr && ((count>=32 && d->data[j]==' ')
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

void infowriter::addsprite(int x, SpriteInfo info)
{
	if (boxnum >= maxboxes)
		resize(maxboxes*2);

	boxes[boxnum].type = issprite;
	boxes[boxnum].h.sprite.x = x;
	boxes[boxnum].h.sprite.info = info;

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
		fprintf(stderr, "\nError: GRFCodec thinks it has written %d sprites but the info file only got %d\n", count, spriteno);
		exit(2);
	}
}
