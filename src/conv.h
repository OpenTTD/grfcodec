
// convert between DOS and Windows GRF files

// map DOS to Windows
int palmap0[] = {
	   0, 215, 216, 136,  88, 106,  32,  33,	// 0..7
	  40, 245,  10,  11,  12,  13,  14,  15,	// 8..15
	  16,  17,  18,  19,  20,  21,  22,  23,	// 16..23
	  24,  25,  26,  27,  28,  29,  30,  31,	// 24..31
	  53, 107,  34,  35,  36,  37,  38,  39,	// 32..39
	 178,  41,  42,  43,  44,  45,  46,  47,	// 40..47
	  48,  49,  50,  51,  52,  53,  54,  55,	// 48..55
	  56,  57,  58,  59,  60,  61,  62,  63,	// 56..63
	  64,  65,  66,  67,  68,  69,  70,  71,	// 64..71
	  72,  73,  74,  75,  76,  77,  78,  79,	// 72..79
	  80,  81,  82,  83,  84,  85,  86,  87,	// 80..87
	  96,  89,  90,  91,  92,  93,  94,  95,	// 88..95
	  96,  97,  98,  99, 100, 101, 102, 103,	// 96..103
	 104, 105,  53, 107, 108, 109, 110, 111,	// 104..111
	 112, 113, 114, 115, 116, 117, 118, 119,	// 112..119
	 120, 121, 122, 123, 124, 125, 126, 127,	// 120..127
	 128, 129, 130, 131, 132, 133, 134, 135,	// 128..135
	 170, 137, 138, 139, 140, 141, 142, 143,	// 136..143
	 144, 145, 146, 147, 148, 149, 150, 151,	// 144..151
	 152, 153, 154, 155, 156, 157, 158, 159,	// 152..159
	 160, 161, 162, 163, 164, 165, 166, 167,	// 160..167
	 168, 169, 170, 171, 172, 173, 174, 175,	// 168..175
	 176, 177, 178, 179, 180, 181, 182, 183,	// 176..183
	 184, 185, 186, 187, 188, 189, 190, 191,	// 184..191
	 192, 193, 194, 195, 196, 197, 198, 199,	// 192..199
	 200, 201, 202, 203, 204, 205, 206, 207,	// 200..207
	 208, 209, 210, 211, 212, 213, 214,   7,	// 208..215
	   8,   9, 246, 247, 248, 249, 250, 251,	// 216..223
	 252, 253, 254, 230, 231, 227, 228, 229,	// 224..231
	 236, 237, 238, 232, 233, 234, 235, 184,	// 232..239
	  70, 241, 242, 244, 243, 220, 221, 217,	// 240..247
	 218, 219, 226, 222, 223, 224, 225,  15,	// 248..255
};

// map Windows to DOS
int palmap1[] = {
	   0, 180,   2,   3,   4,   5,   6, 215,	// 0..7
	 216, 219,  10,  11,  12,  13,  14,  15,	// 8..15
	  16,  17,  18,  19,  20,  21,  22,  23,	// 16..23
	  24,  25,  26,  27,  28,  29,  30,  31,	// 24..31
	   6,   7,  34,  35,  36,  37,  38,  39,	// 32..39
	   8,  41,  42,  43,  44,  45,  46,  47,	// 40..47
	  48,  49,  50,  51,  52,  53,  54,  55,	// 48..55
	  56,  57,  58,  59,  60,  61,  62,  63,	// 56..63
	  64,  65,  66,  67,  68,  69,  70,  71,	// 64..71
	  72,  73,  74,  75,  76,  77,  78,  79,	// 72..79
	  80,  81,  82,  83,  84,  85,  86,  87,	// 80..87
	   4,  89,  90,  91,  92,  93,  94,  95,	// 88..95
	  96,  97,  98,  99, 100, 101, 102, 103,	// 96..103
	 104, 105,   5, 107, 108, 109, 110, 111,	// 104..111
	 112, 113, 114, 115, 116, 117, 118, 119,	// 112..119
	 120, 121, 122, 123, 124, 125, 126, 127,	// 120..127
	 128, 129, 130, 131, 132, 133, 134, 135,	// 128..135
	   3, 137, 138, 139, 140, 141, 142, 143,	// 136..143
	 144, 145, 146, 147, 148, 149, 150, 151,	// 144..151
	 152, 153, 154, 155, 156, 157, 158, 159,	// 152..159
	 160, 161, 162, 163, 164, 165, 166, 167,	// 160..167
	 168, 169, 170, 171, 172, 173, 174, 175,	// 168..175
	 176, 177, 178, 179, 180, 181, 182, 183,	// 176..183
	 184, 185, 186, 187, 188, 189, 190, 191,	// 184..191
	 192, 193, 194, 195, 196, 197, 198, 199,	// 192..199
	 200, 201, 202, 203, 204, 205, 206, 207,	// 200..207
	 208, 209, 210, 211, 212, 213, 214,   1,	// 208..215
	   2, 247, 248, 249, 245, 246, 251, 252,	// 216..223
	 253, 254, 250, 229, 230, 231, 227, 228,	// 224..231
	 235, 236, 237, 238, 232, 233, 234, 239,	// 232..239
	 240, 241, 242, 244, 243,   9, 218, 219,	// 240..247
	 220, 221, 222, 223, 224, 225, 226, 255,	// 248..255
};
