/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* Copyright (C) Electronic Arts Canada Inc. 1994-1998.  All rights reserved. */

/* ABSTRACT
gimex.h - Graphics IMport EXport (GIMEX) v2.26
@ */

#ifndef __GIMEX_H
#define __GIMEX_H 1

#ifdef __cplusplus
extern "C" {
#endif

#define GimexVersion "2.26"

/****************************************************************************/
/* Data Types                                                               */
/****************************************************************************/

/* ARGB structure used for palettes/pixels */

#ifndef _ARGB_T
    #define _ARGB_T
    #if defined(_MSC_VER) || defined(__WATCOMC__) || defined(PSX) || defined(DC)

        typedef struct
        {
            unsigned char b,g,r,a;
        } ARGB;

    #elif defined(SGI)

        typedef struct
        {
            unsigned char a,b,g,r;
        } ARGB;

    #else /* Mac */

        typedef struct
        {
            unsigned char a,r,g,b;
        } ARGB;

    #endif
#endif

/* Info structure describing bitmaps */

#define GIMEX_FRAMENAME_SIZE    32
#define GIMEX_COMMENT_SIZE      256
#define GIMEX_COLOURTBL_SIZE    256
#define GIMEX_HOTSPOTTBL_SIZE   256
#define GIMEX_HOTSPOTTBL_VALUES 2

typedef struct
{
    long signature;         /* signature of gimex ie 'tga ' (optional) */
    long size;              /* size of GINFO structure */
    int  version;           /* version number of GINFO structure (200) */
    int  framenum;          /* current frame */
    int  width;             /* width of bitmap in pixels */
    int  height;            /* height of bitmap in pixels */
    int  bpp;               /* bits per pixel (8 or 32) */
    int  originalbpp;       /* bits per pixel in original image (1 to 32) */
    int  startcolour;       /* first colour in palette */
    int  numcolours;        /* number of colours in original indexed palette */
    ARGB colourtbl[GIMEX_COLOURTBL_SIZE]; /* 8 bit palette */
    int  subtype;           /* internal format sub-type 0-default */
    int  packed;            /* type of packing on original image. 0 none, 1 run, n other */
    int  quality;           /* quality of lossy packing 0..100 */
    int  framesize;         /* size of frame in bytes */
    int  alphabits;         /* number of bits in alpha channel */
    int  redbits;           /* number of bits in red channel */
    int  greenbits;         /* number of bits in green channel */
    int  bluebits;          /* number of bits in blue channel */
    int  centerx;           /* center point relative to upper left corner */
    int  centery;
    int  defaultx;          /* default coordinate point */
    int  defaulty;
    int  numhotspots;       /* number of hot spots defined */
    char framename[GIMEX_FRAMENAME_SIZE];   /* null terminated name of frame/image */
    char comment[GIMEX_COMMENT_SIZE];       /* null terminated multiline user comment */
    int  hotspottbl[GIMEX_HOTSPOTTBL_SIZE][GIMEX_HOTSPOTTBL_VALUES];/* up to 256 hot spots, XY pairs relative to upperleft */
    float dpi;              /* dots per inch ie 72.0 */
    int  reserved[3];       /* reserved for future use - set to zero */
} GINFO;

#ifndef _GSTREAM_T
#define _GSTREAM_T  GIMEX
typedef void GSTREAM;       /* handle used for file functions */
#endif

typedef struct
{
    long signature;         /* signature of gimex ie 'tga ' (optional) */
    long size;              /* size of GINSTANCE structure */
    int  frames;            /* Number of frames in file */
    int  framenum;          /* current frame (optional) */
    GSTREAM *gstream;       /* stream pointer for file */
    char *gref;             /* gimex reference to additional memory used by module (optional) */
} GINSTANCE;

/* Info structure describing bitmaps */

#define MAXMACTYPES 8
#define MAXEXTENSIONS 8

#define GIMEX_EXTENSION_SIZE    8
#define GIMEX_AUTHORSTR_SIZE    32
#define GIMEX_VERSIONSTR_SIZE   8
#define GIMEX_SHORTTYPESTR_SIZE 8
#define GIMEX_WORDTYPESTR_SIZE  16
#define GIMEX_LONGTYPESTR_SIZE  32

typedef struct
{
    long signature;                 /* signature of gimex ie 'tga ' (optional) */
    long size;                      /* size of GABOUT structure */
    int  version;                   /* version number of GABOUT structure (200) */
    unsigned int canimport     :1;  /* supports importing */
    unsigned int canexport     :1;  /* supports exporting */
    unsigned int importpacked  :2;  /* max import packed field 0..3 */
    unsigned int exportpacked  :2;  /* max export packed field 0..3  */
    unsigned int import8       :1;  /* supports importing 8 bit indexed */
    unsigned int export8       :1;  /* supports exporting 8 bit indexed */
    unsigned int import32      :1;  /* supports importing 32 bit direct rgb */
    unsigned int export32      :1;  /* supports exporting 32 bit direct rgb */
    unsigned int multiframe    :1;  /* supports multiple frames */
    unsigned int multifile     :1;  /* format requires additional files or resource fork */
    unsigned int multisize     :1;  /* supports different size per frame */
    unsigned int framebuffer   :1;  /* module requires memory to buffer entire frame */
    unsigned int external      :1;  /* module requires external tool or plugin */
    unsigned int usesfile      :1;  /* module is file based vs ads/printer/generator */
    unsigned int singlepalette :1;  /* limited to a single palette per file */
    unsigned int greyscale     :1;  /* use maxcolours for number of levels */
    unsigned int startcolour   :1;  /* supports start colour */
    unsigned int dotsubtype    :1;  /* subtype based on extension */
    unsigned int resizable     :1;  /* read will respect ginfo width & height */
    unsigned int pad           :11; /* pad bitfield to 32 bit boundary for inter compiler compatibility */
    int  maxcolours;                /* only use in 8 bit, 0 if module does not care */
    int  maxframename;              /* maximum characters in ginfo framename */
    int  defaultquality;            /* default pack quality */
    long mactype[MAXMACTYPES];      /* mac file system types used */
    char extensions[MAXEXTENSIONS][GIMEX_EXTENSION_SIZE]; /* null terminated extensions with '.' */
    char authorstr[GIMEX_AUTHORSTR_SIZE];        /* name of gimex module author */
    char versionstr[GIMEX_VERSIONSTR_SIZE];      /* version number of gimex module ie 1.00 */
    char shorttypestr[GIMEX_SHORTTYPESTR_SIZE];  /* 3 or 4 character type string ie TGA */
    char wordtypestr[GIMEX_WORDTYPESTR_SIZE];    /* single word type string ie Targa */
    char longtypestr[GIMEX_LONGTYPESTR_SIZE];    /* full name of data format ie True Vision Targa */
} GABOUT;

/* Bitmap structure (optional) */

typedef struct
{
    GINFO *ginfo;
    char  *image;
    int   rowbytes;
} GBITMAP;

#define GMAKEID(a,b,c,d) (((long)(a)<<24)|((long)(b)<<16)|((long)(c)<<8)|(long)(d))

#ifndef gmin
#define gmin(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef gmax
#define gmax(a,b) ((a)>(b)?(a):(b))
#endif

#if !defined(GCALL)
#if defined(_MSC_VER) || defined(__WATCOMC__)
#define GCALL __stdcall
#else
#define GCALL
#endif
#endif

/* disable unreferenced parameters warnings */
#if defined(__WATCOMC__)
#pragma warning 202 999999
#endif
#if defined(_MSC_VER)
#pragma warning(disable : 4100)
#endif

#if 0
/****************************************************************************/
/* Gimex Module Example Prototypes                                          */
/****************************************************************************/

/* Example Information Functions */

GABOUT * GCALL aboutbmp(void);
int      GCALL isbmp(GSTREAM *g);

/* Example Import Functions */

int     GCALL openbmp(GINSTANCE **gx, GSTREAM *g, char *pathname);
GINFO * GCALL infobmp(GINSTANCE *gx, int framenum);
int     GCALL readbmp(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
int     GCALL closebmp(GINSTANCE *gx);

/* Example Export Functions */

int     GCALL wopenbmp(GINSTANCE **gx, GSTREAM *g, char *pathname, int numframes);
int     GCALL writebmp(GINSTANCE *gx, GINFO *ginfo, char *source, int rowbytes);
int     GCALL wclosebmp(GINSTANCE *gx);

#endif

/****************************************************************************/
/* Application Module Prototypes                                            */
/****************************************************************************/

/* File Stream Functions */

GSTREAM * GCALL gopen(const char *pathname);
GSTREAM * GCALL gwopen(const char *pathname);
int       GCALL gclose(GSTREAM *g);
int       GCALL gread(GSTREAM *g, void *buf, long size);
int       GCALL gwrite(GSTREAM *g, void *buf, long size);
int       GCALL gseek(GSTREAM *g, long offset);
long      GCALL glen(GSTREAM *g);
long      GCALL gtell(GSTREAM *g);

/* Memory Functions */

void *  GCALL galloc(long size);
int     GCALL gfree(void *memptr);
void          gputm(void *memptr, unsigned long val, int numbytes);
void          gputi(void *memptr, unsigned long val, int numbytes);
unsigned long ggetm(void *memptr, int numbytes);
unsigned long ggeti(void *memptr, int numbytes);

/****************************************************************************/
/* Watcom Memory Functions                                                  */
/****************************************************************************/

#if defined(__WATCOMC__) && !defined(__NOINLINE__)
#pragma aux ggeti = \
    "mov 	eax,[eax+ecx-4]" \
    "neg    ecx" \
    "lea    ecx,32[ecx*8]" \
    "shr    eax,cl" \
    parm   [eax] [ecx] \
    modify [eax ecx] \
    value  [eax];

#pragma aux ggetm = \
    ".586" \
    "mov 	eax,[eax]" \
    "bswap  eax" \
    "neg    ecx" \
    "lea    ecx,32[ecx*8]" \
    "shr    eax,cl" \
    parm   [eax] [ecx] \
    modify [eax ecx] \
    value  [eax];

unsigned long bswap(unsigned long val);
#pragma aux bswap = "bswap eax" parm [eax] modify [eax] value [eax];

#define gputm(putmdest,putmdata,putmbytes) \
      (((int)(putmbytes)==4) ? ((void)(*((unsigned long *) (putmdest)) = bswap((unsigned long)(putmdata)))) \
    : (((int)(putmbytes)==1) ? ((void)(*((unsigned char *) (putmdest)) = (unsigned char)(putmdata))) \
    : (((int)(putmbytes)==2) ? ((void)(*((unsigned short *) (putmdest)) = (unsigned short)(bswap((unsigned long)(putmdata))>>16))) \
    : (((int)(putmbytes)==3) ? ((void)(*((unsigned char *) (putmdest)+2) = (unsigned char)(putmdata)),(void)(*((unsigned short *) (putmdest)) = (unsigned short)(bswap((unsigned long)(putmdata))>>8))) \
    : (void)0))))

#define gputi(putidest,putidata,putibytes) \
      (((int)(putibytes)==4) ? ((void)(*((unsigned long *) (putidest)) = ((unsigned long)(putidata)))) \
    : (((int)(putibytes)==1) ? ((void)(*((unsigned char *) (putidest)) = (unsigned char)(putidata))) \
    : (((int)(putibytes)==2) ? ((void)(*((unsigned short *) (putidest)) = (unsigned short)(putidata))) \
    : (((int)(putibytes)==3) ? ((void)(*((unsigned short *) (putidest)) = (unsigned short)(putidata)),(void)(*((unsigned char *) (putidest)+2) = (unsigned char)((unsigned long)(putidata)>>16))) \
    : (void)0))))

#endif /* __WATCOMC__ */

#ifdef __cplusplus
}
#endif

#endif /* __GIMEX_H */
/* END ABSTRACT */



