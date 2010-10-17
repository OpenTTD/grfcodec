// Escapes.h
// The data required for writing the various not-quoted escape sequences.
//
// This file is shared between GRFCodec and NFORenum.
// GRFCodec defines GRFCODEC. NFORenum does not.


#ifndef GRFCODEC

#define START_ESCAPES()\
	const struct esc{\
		char byte;\
		char*str;\
		char action;\
		uint pos;\
	}escapes[]={

#define ESCAPE(byte,str,action,off)\
	{(char)0x##byte,(char*)("\\" str),(char)0x##action,off},

#define ESCAPE_79(byte,str,off)\
	ESCAPE(byte,str,7,off)

#define ESCAPE_AD(byte,str,action,off,adtl)\
	{(char)0x##byte,(char*)("\\" str),(char)0x##action,off},

#define ESCAPE_OVR(byte,str,action,ovr)\
	{(char)0x##byte,(char*)("\\" str),(char)0x##action,0},

#else	/* GRFCODEC */

#define START_ESCAPES()\
	const struct esc{\
		char byte;\
		char*str;\
		char action;\
		uint pos;\
		bool (*additional)(const U8*,uint);\
		bool (*override)(const U8*,uint);\
	}escapes[]={

#define ESCAPE(byte,str,action,off)\
	{(char)0x##byte,(char*)("\\" str),(char)0x##action,off,NULL,NULL},

#define ESCAPE_79(byte,str,off)\
	ESCAPE(byte,str,7,off) ESCAPE(byte,str,9,off)

#define ESCAPE_AD(byte,str,action,off,adtl)\
	{(char)0x##byte,(char*)("\\" str),(char)0x##action,off,__ESC_AD__##adtl,NULL},

#define ESCAPE_OVR(byte,str,action,ovr)\
	{(char)0x##byte,(char*)("\\" str),(char)0x##action,0,NULL,__ESC_OVR__##ovr},

#define CALLBACK_AD(name)\
	bool __ESC_AD__##name(const U8*data,uint len)
#define CALLBACK_OVR(name)\
	bool __ESC_OVR__##name(const U8*data,uint pos)

#endif /* GRFCODEC */

#define END_ESCAPES() };\
	static const unsigned int num_esc=sizeof(escapes)/sizeof(escapes[0]);

#ifdef GRFCODEC

// ***********************************************************************
// Define callback functions for ESCAPE_* here
// Use the same value for the argument to the CALLBACK_* macro and the last
// argument to ESCAPE_*
// The signature for CALLBACK_AD is:
// bool func(const U8*data, uint len)
// The signature for CALLBACK_OVR is:
// bool func(const U8*data, uint pos)
// ***********************************************************************

CALLBACK_AD(IsGRM){
	return len==9&&data[2]==0&&data[4]==0xFE&&data[5]==0xFF;
}

CALLBACK_OVR(Is2Op){
	if(pos<7||!(data[3]&0x80)||data[3]==0x80||data[3]==0x83)return false;
	uint w = 1<<((data[3]>>2)&3);
	uint loc=4;//Start at first <var>
	while(true){//read <var> [<param>] <varadjust> [<op> ...]. loc reports byte to be checked.
		if((data[loc++/*<var>*/]&0xE0)==0x60)loc++;//<param>
		if(loc>=pos)return false;
		if(!(data[loc]&0x20))return false;//end of advanced
		if(data[loc++/*<shift>*/]&0xC0)
			loc+=2*w;//<add> and <div>/<mod>
		loc+=w;//<and>
		if(loc==pos)return true;//This is an operation byte
		if(loc++/*<op>*/>pos)return false;//past proposed op byte
	}
}

#endif /* GRFCODEC */

START_ESCAPES()

// ***********************************************************************
// Define escape sequences here.
// The arguments to ESCAPE* are:
//   byte:   the byte value for this escape sequence, in hex.
//   str:    the string for this escape sequence, excluding the \.
//   action: the action for this escape sequence, eg 2, 7, D, &c.
//           Note that 7 and 9 are usually paired, with ESCAPE_79, which does
//           not take an action argument.
//   off:    the offset in the action where this escape sequence appears
// ESCAPE_AD takes an additional argument; the CALLBACK to call if the
// byte, action, and off checks all pass.
// EXCAPE_OVR takes an argument in place of off, the CALLBACK to call
// instead of running the off check.
// ***********************************************************************

ESCAPE_OVR(00,"2+",2,Is2Op)
ESCAPE_OVR(01,"2-",2,Is2Op)
ESCAPE_OVR(02,"2<",2,Is2Op)
ESCAPE_OVR(03,"2>",2,Is2Op)
ESCAPE_OVR(04,"2u<",2,Is2Op)
ESCAPE_OVR(05,"2u>",2,Is2Op)
ESCAPE_OVR(06,"2/",2,Is2Op)
ESCAPE_OVR(07,"2%",2,Is2Op)
ESCAPE_OVR(08,"2u/",2,Is2Op)
ESCAPE_OVR(09,"2u%",2,Is2Op)
ESCAPE_OVR(0A,"2*",2,Is2Op)
ESCAPE_OVR(0B,"2&",2,Is2Op)
ESCAPE_OVR(0C,"2|",2,Is2Op)
ESCAPE_OVR(0D,"2^",2,Is2Op)
ESCAPE_OVR(0E,"2sto",2,Is2Op)
ESCAPE_OVR(0E,"2s",2,Is2Op)
ESCAPE_OVR(0F,"2rst",2,Is2Op)
ESCAPE_OVR(0F,"2r",2,Is2Op)
ESCAPE_OVR(10,"2psto",2,Is2Op)
ESCAPE_OVR(11,"2ror",2,Is2Op)
ESCAPE_OVR(11,"2rot",2,Is2Op)
ESCAPE_OVR(12,"2cmp",2,Is2Op)
ESCAPE_OVR(13,"2ucmp",2,Is2Op)
ESCAPE_OVR(14,"2<<",2,Is2Op)
ESCAPE_OVR(15,"2u>>",2,Is2Op)
ESCAPE_OVR(16,"2>>",2,Is2Op)

ESCAPE_79(00,"71",3)
ESCAPE_79(01,"70",3)
ESCAPE_79(02,"7=",3)
ESCAPE_79(03,"7!",3)
ESCAPE_79(04,"7<",3)
ESCAPE_79(05,"7>",3)
ESCAPE_79(06,"7G",3)
ESCAPE_79(07,"7g",3)
ESCAPE_79(08,"7gG",3)
ESCAPE_79(09,"7GG",3)
ESCAPE_79(0A,"7gg",3)
ESCAPE_79(0B,"7c",3)
ESCAPE_79(0C,"7C",3)

ESCAPE(00,"D=",D,2)
ESCAPE(01,"D+",D,2)
ESCAPE(02,"D-",D,2)
ESCAPE(03,"Du*",D,2)
ESCAPE(04,"D*",D,2)
ESCAPE(05,"Du<<",D,2)
ESCAPE(06,"D<<",D,2)
ESCAPE(07,"D&",D,2)
ESCAPE(08,"D|",D,2)
ESCAPE(09,"Du/",D,2)
ESCAPE(0A,"D/",D,2)
ESCAPE(0B,"Du%",D,2)
ESCAPE(0C,"D%",D,2)

ESCAPE_AD(00,"DR",D,3,IsGRM)
ESCAPE_AD(01,"DF",D,3,IsGRM)
ESCAPE_AD(02,"DC",D,3,IsGRM)
ESCAPE_AD(03,"DM",D,3,IsGRM)
ESCAPE_AD(04,"DnF",D,3,IsGRM)
ESCAPE_AD(05,"DnC",D,3,IsGRM)
ESCAPE_AD(06,"DO",D,3,IsGRM)

END_ESCAPES()
