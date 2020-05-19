#ifndef HEADER_DH_H
#include <openssl/dh.h>
#endif
DH *get_dh1024()
	{
	static unsigned char dh1024_p[]={
		0x89,0x50,0x85,0x6E,0x25,0x4B,0x2C,0x85,0x28,0x29,0xD8,0xA0,
		0x8F,0xCA,0x67,0x0F,0xD1,0x20,0x1C,0x3F,0xD7,0xD1,0xF8,0x35,
		0xE8,0x54,0x70,0x7D,0xE7,0x20,0xED,0xCE,0x98,0x7B,0x71,0x20,
		0xAE,0x5C,0x5D,0x48,0x3C,0xFF,0x1D,0x3E,0x4C,0xF4,0xAF,0x57,
		0x14,0x56,0xCD,0xED,0x26,0x85,0x81,0x81,0x71,0xBA,0x76,0xA5,
		0x74,0x2F,0x14,0x89,0x43,0xC9,0x36,0xD4,0x37,0xC5,0x50,0x1C,
		0xAF,0x91,0x82,0xDB,0x8A,0x1A,0x5F,0xFD,0xE7,0xA3,0x95,0x0D,
		0xAC,0xAE,0x8A,0xE8,0x38,0x38,0xE6,0x9E,0xA2,0x97,0x76,0x7F,
		0x8E,0xE1,0x97,0x06,0x65,0xB4,0xEF,0x20,0x2B,0x80,0x89,0x00,
		0x97,0x07,0xED,0xDD,0xF7,0x65,0xE4,0x91,0x1D,0x20,0x88,0xFB,
		0x1D,0xFB,0x10,0xF0,0xE0,0xCD,0x50,0xC3,
		};
	static unsigned char dh1024_g[]={
		0x02,
		};
	DH *dh;

	if ((dh=DH_new()) == NULL) return(NULL);
	dh->p=BN_bin2bn(dh1024_p,sizeof(dh1024_p),NULL);
	dh->g=BN_bin2bn(dh1024_g,sizeof(dh1024_g),NULL);
	if ((dh->p == NULL) || (dh->g == NULL))
		{ DH_free(dh); return(NULL); }
	return(dh);
	}
/*
-----BEGIN DH PARAMETERS-----
MIGHAoGBAIlQhW4lSyyFKCnYoI/KZw/RIBw/19H4NehUcH3nIO3OmHtxIK5cXUg8
/x0+TPSvVxRWze0mhYGBcbp2pXQvFIlDyTbUN8VQHK+RgtuKGl/956OVDayuiug4
OOaeopd2f47hlwZltO8gK4CJAJcH7d33ZeSRHSCI+x37EPDgzVDDAgEC
-----END DH PARAMETERS-----
*/
