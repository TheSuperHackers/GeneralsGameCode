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

/* Copyright (C) Electronic Arts Canada Inc. 1994-2002.  All rights reserved. */

/* ABSTRACT
gimex.h - Primary header file for the GIMEX API.
@ */

/*------------------------------------------------------------------*/
/*                                                                  */
/*                 Graphics IMport EXport API v3.46                 */
/*                                                                  */
/*                      by Frank Barchard, EAC                      */
/*                                                                  */
/*                   Header Module - Dec 05, 2002                   */
/*                                                                  */
/*------------------------------------------------------------------*/
/*                                                                  */
/* Version Date   SE History                                        */
/* ------- ------ -- -------                                        */
/*    3.01 ------ FB gref void *                                    */
/*    3.02 ------ FB const GIMEX_write                              */
/*    3.10 ------ FB 16 bpp import                                  */
/*    3.11 ------ FB gbitmap->image void *                          */
/*    3.12 ------ FB about importstream, exportstream, maxredbits et*/
/*    3.13 ------ FB about->movie                                   */
/*    3.14 ------ FB GFUNCTIONS table declared                      */
/*    3.15 ------ FB about->mipmap                                  */
/*    3.16 ------ FB about->font                                    */
/*    3.17 ------ FB linux use intel ordered ARGB                   */
/*    3.20 ------ FB 64 bit file system (GPOS), removed ARGB16      */
/*    3.21 ------ FB 64 bit backwards compatibility with 32         */
/*    3.22 ------ FB 32 bit file implementation for efficiency      */
/*    3.23 ------ FB bitfield warning removed                       */
/*    3.24 010426 FB GIMEX_read/write take void * prototyped here   */
/*    3.25 010427 FB about width/height max,align and obsolete      */
/*    3.26 010501 FB INT64 for gcc 2.95.3 on ps2 use long           */
/*    3.27 010504 FB removed _ARGB_T - use ARGB. add GC reference   */
/*    3.30 010614 LC update for 330 release                         */
/*    3.31 010628 FB GIMEX_COMMENT_SIZE 1024                        */
/*    3.32 011009 FB about->file64 for 64 bit file sizes            */
/*    3.40 011022 FB bool returns, ggetm inline, 512 GIMEX_FRAME... */
/*    3.41 020225 FB metalbin 240 on ps2                            */
/*    3.44 020307 FB bumped for next release. no api change         */
/*    3.45 020927 FB geti 3 bounds safe/purify                      */
/*    3.46 021205 FB about firstextension                           */
/*                                                                  */
/*------------------------------------------------------------------*/

#ifndef __GIMEX_H
#define __GIMEX_H 1

#pragma once

#if defined(_MSC_VER)
#pragma warning(disable : 4100)
/* warning C4100: unreferenced parameter */
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define GIMEX_VERSION 346
#define GIMEX_PATCH   0

/****************************************************************************/
/* Data Types                                                               */
/****************************************************************************/

/* ARGB structure used for palettes/pixels */

// experimental float channel

#ifdef GIMEXFLOAT
typedef float GCHANNEL;
#else
typedef unsigned char GCHANNEL;
#endif

#ifndef ARGB
#define ARGB ARGB
#if defined(_MSC_VER) || defined(__i386__)

    typedef struct
    {
        GCHANNEL b,g,r,a;
    } ARGB;

#elif defined(__R5900) || defined(SGI) /* PS2 */

    typedef struct
    {
        GCHANNEL r,g,b,a;
    } ARGB;

#else /* GameCube/Mac */

    typedef struct
    {
        GCHANNEL a,r,g,b;
    } ARGB;
#endif
#endif

#if !defined(GPOS)
#if defined(_MSC_VER)
typedef __int64 GPOS;
#elif defined(__R5900)
typedef long GPOS;
#else
typedef long long GPOS;
#endif
#endif

/* Info structure describing bitmaps */

#define GIMEX_FRAMENAME_SIZE    512
#define GIMEX_COMMENT_SIZE      1024
#define GIMEX_COLOURTBL_SIZE    256
#define GIMEX_HOTSPOTTBL_SIZE   1024
#define GIMEX_HOTSPOTTBL_VALUES 2

typedef struct
{
    int signature;         /* signature of gimex ie 'tga ' (optional) */
    int size;              /* size of GINFO structure */
    int version;           /* version number of GINFO structure (300) */
    int framenum;          /* current frame */
    int width;             /* width of bitmap in pixels */
    int height;            /* height of bitmap in pixels */
    int bpp;               /* bits per pixel (8, 16 or 32) */
    int originalbpp;       /* bits per pixel in original image (1 to 32) */
    int startcolour;       /* first colour in palette */
    int numcolours;        /* number of colours in original indexed palette */
    ARGB colourtbl[GIMEX_COLOURTBL_SIZE]; /* 8 bit palette */
    int subtype;           /* internal format sub-type 0-default */
    int packed;            /* type of packing on original image. 0 none, 1 run, n other */
    int quality;           /* quality of lossy packing 0..100 */
    int framesize;         /* size of frame in bytes */
    int alphabits;         /* number of bits in alpha channel */
    int redbits;           /* number of bits in red channel */
    int greenbits;         /* number of bits in green channel */
    int bluebits;          /* number of bits in blue channel */
    int centerx;           /* center point relative to upper left corner */
    int centery;
    int defaultx;          /* default coordinate point */
    int defaulty;
    int numhotspots;       /* number of hot spots defined */
    char framename[GIMEX_FRAMENAME_SIZE];   /* null terminated name of frame/image */
    char comment[GIMEX_COMMENT_SIZE];       /* null terminated multiline user comment */
    int hotspottbl[GIMEX_HOTSPOTTBL_SIZE][GIMEX_HOTSPOTTBL_VALUES];/* up to 256 hot spots, XY pairs relative to upperleft */
    float dpi;             /* dots per inch ie 72.0 */
    float fps;             /* frame per second (one over duration).  15.0 is typical */
    int reserved[3];       /* reserved for future use - set to zero */
} GINFO;

typedef struct GSTREAM GSTREAM; /* handle used for file functions */

typedef struct
{
    int signature;         /* signature of gimex ie '.tga' (optional) */
    int size;              /* size of GINSTANCE structure */
    int frames;            /* Number of frames in file */
    int framenum;          /* current frame (optional) */
    GSTREAM *gstream;      /* stream pointer for file */
    void *gref;            /* gimex reference to additional memory used by module (optional) */
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
    int signature;                 /* signature of gimex ie 'tga ' (optional) */
    int size;                      /* size of GABOUT structure */
    int version;                   /* version number of GABOUT structure (200) */
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
    unsigned int external      :1;  /* uses external resources */
	unsigned int usesfile      :1;  /* module is file based vs ads/printer/generator */
    unsigned int globalpalette :1;  /* limited to a single palette per file */
    unsigned int greyscale     :1;  /* use maxcolours for number of levels */
    unsigned int startcolour   :1;  /* supports start colour */
    unsigned int dotsubtype    :1;  /* subtype based on extension */
    unsigned int resizable     :1;  /* read will respect ginfo width & height */
    unsigned int reserved2     :1;  /* reserved for future use */
    unsigned int reserved3     :1;  /* reserved for future use */
    unsigned int importstream  :1;  /* supports open with GIMEX_NOFRAMECOUNT */
    unsigned int exportstream  :1;  /* will ignore frame count on export */
    unsigned int movie         :1;  /* this is a movie format (as opposed to mipmaps, fonts or multipage) */
    unsigned int mipmap        :1;  /* mipmaps are supported and will constrain sizes */
    unsigned int font          :1;  /* this is a font format */
    unsigned int obsolete      :1;  /* this format is obsolete */
    unsigned int file64        :1;  /* this format supports 64 bit file sizes */
    unsigned int firstextension:1;  /* use first extension when assigning default name */
	unsigned int pad           :1;  /* pad bitfield to 32 bit boundary for inter compiler compatibility */
    int  maxcolours;                /* only use in 8 bit, 0 if module does not care */
    int  maxframename;              /* maximum characters in ginfo framename */
    int  defaultquality;            /* default pack quality */
    int mactype[MAXMACTYPES];      /* mac file system types used */
    char extensions[MAXEXTENSIONS][GIMEX_EXTENSION_SIZE]; /* null terminated extensions with '.' */
    char authorstr[GIMEX_AUTHORSTR_SIZE];        /* name of gimex module author */
    char versionstr[GIMEX_VERSIONSTR_SIZE];      /* version number of gimex module ie 1.00 */
    char shorttypestr[GIMEX_SHORTTYPESTR_SIZE];  /* 3 or 4 character type string ie TGA */
    char wordtypestr[GIMEX_WORDTYPESTR_SIZE];    /* single word type string ie Targa */
    char longtypestr[GIMEX_LONGTYPESTR_SIZE];    /* full name of data format ie True Vision Targa */
    unsigned int maxalphabits:8;    /* maximum supported number of bits in alpha channel */
    unsigned int maxredbits  :8;    /* maximum supported number of bits in red channel */
    unsigned int maxgreenbits:8;    /* maximum supported number of bits in green channel */
    unsigned int maxbluebits :8;    /* maximum supported number of bits in blue channel */
    unsigned int maxwidth;          /* maximum width in pixels */
    unsigned int maxheight;         /* maximum height in pixels */
    unsigned int alignwidth;        /* width must be multiple of this in pixels */
    unsigned int alignheight;       /* height must be multiple of this in pixels */
    unsigned int pad2[4];
} GABOUT;

/* Bitmap structure (optional) */

typedef struct
{
    GINFO *ginfo;
    void  *image;
    int   rowbytes;
} GBITMAP;

#ifndef GMAKEID
#define GMAKEID(a,b,c,d) (((int)(a)<<24)|((int)(b)<<16)|((int)(c)<<8)|(int)(d))
#endif

#ifndef gmin
#define gmin(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef gmax
#define gmax(a,b) ((a)>(b)?(a):(b))
#endif

#if !defined(GCALL)
#if defined(_MSC_VER) && !defined(_XBOX)
#define GCALL __stdcall
#else
#define GCALL
#endif
#endif


#if !defined(__cplusplus) && !defined(bool)
#define bool bool
typedef enum{ False=0x0, True=0x1}bool;
#endif

typedef struct GFUNCTIONS
{
    GABOUT * (GCALL * GIMEX_about)(void);
    int      (GCALL * GIMEX_is)(GSTREAM *g);
    int      (GCALL * GIMEX_open)(GINSTANCE **gx, GSTREAM *g, const char *pathname,bool framecountflag);
    GINFO *  (GCALL * GIMEX_info)(GINSTANCE *gx, int framenum);
    int      (GCALL * GIMEX_read)(GINSTANCE *gx, GINFO *ginfo, char *dest, int rowbytes);
    int      (GCALL * GIMEX_close)(GINSTANCE *gx);
    int      (GCALL * GIMEX_wopen)(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
    int      (GCALL * GIMEX_write)(GINSTANCE *gx, const GINFO *ginfo, char *source, int rowbytes);
    int      (GCALL * GIMEX_wclose)(GINSTANCE *gx);
} GFUNCTIONS;

extern struct GFUNCTIONS gfunctions[];

/****************************************************************************/
/* Gimex Module Example Prototypes                                          */
/****************************************************************************/

/* valid values to be passed to framecountflag parameter for the GIMEX_open function */
#ifdef __cplusplus
#define GIMEX_FRAMECOUNT   true
#define GIMEX_NOFRAMECOUNT false
#else
#define GIMEX_FRAMECOUNT   0x1
#define GIMEX_NOFRAMECOUNT 0x0
#endif
/* Information Functions */

GABOUT *GCALL GIMEX_about(void);
int     GCALL GIMEX_is(GSTREAM *g);

/* Import Functions */

#ifdef __cplusplus
int     GCALL GIMEX_open(GINSTANCE **gx, GSTREAM *g, const char *pathname, bool framecountflag=GIMEX_FRAMECOUNT);
#else
int     GCALL GIMEX_open(GINSTANCE **gx, GSTREAM *g, const char *pathname, bool framecountflag);
#endif
GINFO * GCALL GIMEX_info(GINSTANCE *gx, int framenum);
bool    GCALL GIMEX_read(GINSTANCE *gx, const GINFO *ginfo, void *dest, int rowbytes);
bool    GCALL GIMEX_close(GINSTANCE *gx);

/* Example Export Functions */

bool    GCALL GIMEX_wopen(GINSTANCE **gx, GSTREAM *g, const char *pathname, int numframes);
bool    GCALL GIMEX_write(GINSTANCE *gx, const GINFO *ginfo, const void *source, int rowbytes);
bool    GCALL GIMEX_wclose(GINSTANCE *gx);

/****************************************************************************/
/* Application Module Prototypes                                            */
/****************************************************************************/

/* File Stream Functions */

GSTREAM * GCALL gopen(const char *pathname);
GSTREAM * GCALL gwopen(const char *pathname);
bool      GCALL gclose(GSTREAM *g);
int       GCALL gread(GSTREAM *g, void *buf, int size);
int       GCALL gwrite(GSTREAM *g, const void *buf, int size);
bool      GCALL gseek(GSTREAM *g, GPOS offset);
GPOS      GCALL glen(GSTREAM *g);
GPOS      GCALL gtell(GSTREAM *g);

/* Memory Functions */

#if !defined(galloc)
#include <stdlib.h>
#define galloc malloc
//void *  GCALL galloc(int size);
#endif
//bool   GCALL gfree(void *memptr);
#define gfree free

#ifdef __cplusplus
}
#endif

/****************************************************************************/
/* Inline C++ (cross platform)                                              */
/****************************************************************************/

#ifdef _MSC_VER
#pragma warning(disable: 4514) // unreferenced inline function has been removed
#endif

/* get motorola memory */
static __inline unsigned int ggetm(const void *src, int bytes)
{
    if (bytes==1)
        return (unsigned int) *(const unsigned char *) src;
#if defined(__APPLE__) || (defined(__MWERKS__) && defined(__PPCGEKKO__))
    else if (bytes==2)
        return (unsigned int) *(const unsigned short *) src;
    else if (bytes==4)
        return (unsigned int) *(const unsigned int *) src;
#else
    else if (bytes==2)
        return (((unsigned int) *(const unsigned char *) src)<<8) | (((unsigned int) *((const unsigned char *) src+1)));
    else if (bytes==4)
        return (((unsigned int) *(const unsigned char *) src)<<24) | (((unsigned int) *((const unsigned char *) src+1))<<16) | (((unsigned int) *((const unsigned char *) src+2))<<8) | (((unsigned int) *((const unsigned char *) src+3)));
#endif
    else if (bytes==3)
        return (((unsigned int) *(const unsigned char *) src)<<16) | (((unsigned int) *((const unsigned char *) src+1))<<8) | (((unsigned int) *((const unsigned char *) src+2)));
    return 0;
}

/* get intel memory */

static __inline unsigned int ggeti(const void *src, int bytes)
{
    if (bytes==1)
        return (unsigned int) *(const unsigned char *) src;
#if defined(_MSC_VER) || defined(__i386__)
    else if (bytes==2)
        return (unsigned int) *(const unsigned short *) src;
    else if (bytes==4)
        return (unsigned int) *(const unsigned int *) src;
#else
    else if (bytes==2)
        return (((unsigned int) *((const unsigned char *) src+1))<<8) | (((unsigned int) *(const unsigned char *) src+0));
    else if (bytes==4)
        return (((unsigned int) *((const unsigned char *) src+3))<<24) | (((unsigned int) *((const unsigned char *) src+2))<<16) | (((unsigned int) *((const unsigned char *) src+1))<<8) | (((unsigned int) *(const unsigned char *) src+0));
#endif
    else if (bytes==3)
        return (((unsigned int) *((const unsigned char *) src+2))<<16) | (((unsigned int) *((const unsigned char *) src+1))<<8) | (((unsigned int) *(const unsigned char *) src+0));
    return 0;
}

/* put motorolla memory */

static __inline void gputm(void *dst, unsigned int data, int bytes)
{
    if (bytes==1)
    {
        ((unsigned char *) dst)[0] = (unsigned char) data;
    }
#if defined(__APPLE__) || (defined(__MWERKS__) && defined(__PPCGEKKO__))
    else if (bytes==2)
    {
        ((unsigned short *) dst)[0] = (unsigned short) data;
    }
    else if (bytes==4)
    {
        ((unsigned int *) dst)[0] = (unsigned int) data;
    }
#else
    else if (bytes==2)
    {
        ((unsigned char *) dst)[0] = (unsigned char) (data>>8);
        ((unsigned char *) dst)[1] = (unsigned char) data;
    }
    else if (bytes==4)
    {
        ((unsigned char *) dst)[0] = (unsigned char) (data>>24);
        ((unsigned char *) dst)[1] = (unsigned char) (data>>16);
        ((unsigned char *) dst)[2] = (unsigned char) (data>>8);
        ((unsigned char *) dst)[3] = (unsigned char) data;
    }
#endif
    else if (bytes==3)
    {
        ((unsigned char *) dst)[0] = (unsigned char) (data>>16);
        ((unsigned char *) dst)[1] = (unsigned char) (data>>8);
        ((unsigned char *) dst)[2] = (unsigned char) data;
    }
}

/* put intel memory */

static __inline void gputi(void *dst, unsigned int data, int bytes)
{
    if (bytes==1)
    {
        ((unsigned char *) dst)[0] = (unsigned char) data;
    }
#if defined(_MSC_VER) || defined(__i386__)
    else if (bytes==2)
    {
        ((unsigned short *) dst)[0] = (unsigned short) data;
    }
    else if (bytes==4)
    {
        ((unsigned int *) dst)[0] = (unsigned int) data;
    }
#else
    else if (bytes==2)
    {
        ((unsigned char *) dst)[0] = (unsigned char) data;
        ((unsigned char *) dst)[1] = (unsigned char) (data>>8);
    }
    else if (bytes==4)
    {
        ((unsigned char *) dst)[0] = (unsigned char) data;
        ((unsigned char *) dst)[1] = (unsigned char) (data>>8);
        ((unsigned char *) dst)[2] = (unsigned char) (data>>16);
        ((unsigned char *) dst)[3] = (unsigned char) (data>>24);
    }
#endif
    else if (bytes==3)
    {
        ((unsigned char *) dst)[0] = (unsigned char) data;
        ((unsigned char *) dst)[1] = (unsigned char) (data>>8);
        ((unsigned char *) dst)[2] = (unsigned char) (data>>16);
    }
}

#endif /* __GIMEX_H */

/* END ABSTRACT */
