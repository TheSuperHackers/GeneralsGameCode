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

/************************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S				 ****
 ************************************************************************************************
 *																								*
 *                 Project Name : Command & Conquer											 	*
 *																							 	*
 *                     $Archive:: /Renegade Setup/Autorun/GetCD.h                                        $Author:: Denzil_l														*
 *                     $Modtime:: 1/08/02 3:38p                                                                                         $Revision:: 20															*
 *																								*
 *----------------------------------------------------------------------------------------------*
 * Functions:																					*
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

#pragma once

#ifndef GETCD_H
#define GETCD_H


extern const char *	_CD_Volume_Label[];
extern int		_Num_Volumes;



#ifdef NOT_FOR_WIN95

/* ==================================================================== */
/* Defines */
/* ==================================================================== */
#define	CHLEFT				0
#define	CHRIGHT				1
#define	CHBOTH				2
#define	AUDIO_START_MIN		1
#define	AUDIO_START_SEC		44

struct SEGSEL {
	unsigned short seg ;
	unsigned short sel ;
};

extern "C" int 	DPMI_real_alloc	( UINT , SEGSEL * , USHORT * ) ;
extern "C" int 	DPMI_real_free	( SEGSEL  ) ;
extern "C" void	DPMI_real_intr	( int , union REGS * , struct SREGS * );
extern "C" void	DPMI_real_call	( void * funct , union REGS * , struct SREGS * );

/* ==================================================================== */
/* Data structures */
/* ==================================================================== */

//-----------------------------------------------------------------------------
// Audio Track Info request block
//-----------------------------------------------------------------------------
#pragma pack(push, 1)
struct	TinfoType {
	UBYTE		Length;
	UBYTE		SubCd;
	UBYTE		Command;
	UWORD		Status;
	UBYTE		Rsvd[8];
	UBYTE		MDescr;

	UWORD		TrnsAdOff;
	UWORD		TrnsAdSeg;

	UWORD		CntTrns;
	UWORD		StSect;

	UWORD		VolIDOff;
	UWORD		VolIDSeg;

	UBYTE		TrInfo;
	UBYTE		Track;
	ULONG		Start;
	UBYTE		TrCtrl;
};
#pragma pack(pop)

//-----------------------------------------------------------------------------
// Audio Track Status Control Block
//-----------------------------------------------------------------------------
#pragma pack(push, 1)
struct	StatType {
	UBYTE		Length;
	UBYTE		SubCd;
	UBYTE		Command;
	UWORD		Status;
	UBYTE		Rsvd[8];
	UBYTE		MDescr;

	UWORD		TrnsAdOff;
	UWORD		TrnsAdSeg;

	UWORD		CntTrns;
	UWORD		StSect;

	UWORD		VolIDOff;
	UWORD		VolIDSeg;

	UBYTE		StatInfo;
	UWORD		Stat;
	ULONG		Start;
	ULONG		End;
};
#pragma pack(pop)

//-----------------------------------------------------------------------------
// Audio Track Volume control block
//-----------------------------------------------------------------------------
#pragma pack(push, 1)
struct	VolmType	{
	UBYTE		Length;
	UBYTE		SubCd;
	UBYTE		Command;
	UWORD		Status;
	UBYTE		Rsvd[8];
	UBYTE		MDescr;

	UWORD		TrnsAdOff;
	UWORD		TrnsAdSeg;

	UWORD		CntTrns;
	UWORD		StSect;

	UWORD		VolIDOff;
	UWORD		VolIDSeg;

	UBYTE		TrInfo;
	UBYTE		In0;
	UBYTE		Vol0;
	UBYTE		In1;
	UBYTE		Vol1;
	UBYTE		In2;
	UBYTE		Vol2;
	UBYTE		In3;
	UBYTE		Vol3;
};
#pragma pack(pop)

//-----------------------------------------------------------------------------
// Audio Track Play request block
//-----------------------------------------------------------------------------
#pragma pack(push, 1)
struct	PlayType {
	UBYTE		Length;
	UBYTE		SubCd;
	UBYTE		Command;
	UWORD		Status;
	UBYTE		Rsvd[8];
	UBYTE		AddrMd;
	ULONG		Start;
	ULONG		CntSect;
};
#pragma pack(pop)


//-----------------------------------------------------------------------------
// Audio Track Stop request block
//-----------------------------------------------------------------------------
#pragma pack(push, 1)
struct	StopType {
	UBYTE		Length;
	UBYTE		SubCd;
	UBYTE		Command;
	UWORD		Status;
	UBYTE		Rsvd[8];
};
#pragma pack(pop)

#endif	//NOT_FOR_WIN95


/****************************************************************************
 * GetCDClass -- object which will return logical CD drive					*
 *																			*
 * HISTORY:																	*
 *   06/04/1994 SW : Created.												*
 *==========================================================================*/

#define MAX_CD_DRIVES	26
#define NO_CD_DRIVE		-1

class	GetCDClass
{
	public:
		GetCDClass	( void );							// This is the default constructor
		~GetCDClass	( void );							// This is the destructor

		inline	int		Get_First_CD_Drive		( void );
		inline	int		Get_Next_CD_Drive		( void );
		inline	int		Get_Number_Of_Drives	( void )	{ return( CDCount ); };
		inline	int		Get_Index				( void )	{ return( CDIndex ); };
		inline	void	Reset_Index				( void )	{ CDIndex = 0; };

		int		Get_CD_Drive_For_This_Volume	( const char *volume_name );
		const char *	Get_Volume_For_This_CD_Drive	( const char *path, char *volume_name );
		const char *	Get_Volume_Label				( int index );

	protected:

		int	CDDrives[ MAX_CD_DRIVES ];					//Array containing CD drive letters
		int	CDCount;									//Number of available CD drives
		int	CDIndex;									//Index of current location
};

/****************************************************************************
 * GCDC::Get_First_CD_Drive -- return the number of the first CD drive		*
 *																			*
 * INPUT:		none														*
 *																			*
 * OUTPUT:		logical drive number 										*
 *																			*
 * WARNINGS:																*
 *																			*
 * HISTORY:																	*
 *   05/26/1994 SW : Created.												*
 *   12/4/95    ST : fixed for Win95										*
 *==========================================================================*/

inline int GetCDClass::Get_First_CD_Drive(void)
{
	CDIndex = 0;
	return (Get_Next_CD_Drive());
}

/************************************************************************************
 * GCDC::Get_Next_CD_Drive -- return the logical drive number of the next CD drive	*
 *																												*
 * INPUT:    Nothing																						*
 *																												*
 * OUTPUT:   Logical drive number of a cd drive or -1 if none								*
 *																												*
 * WARNINGS: None																							*
 *																												*
 * HISTORY:																									*
 *    5/21/96 3:50PM ST : Created																	*
 *==================================================================================*/

inline int GetCDClass::Get_Next_CD_Drive(void)
{
	if (CDCount){
		if (CDIndex >= CDCount) {
			CDIndex = 0;
		}
		return (CDDrives[CDIndex++]);
	} else {
		return (-1);
	}
}


/************************************************************************************
 * RedBookClass -- adds red book functionality													*
 *																												*
 *	this class inherits from GetCDClass and adds red book play functionality			*
 *																												*
 *																												*
 * HISTORY:																									*
 *   06/04/1994 SW : Created.																			*
 *==================================================================================*/

#ifdef NOT_FOR_WIN95
class RedBookClass : public GetCDClass
{
	public:

		RedBookClass(void);					// This is the default constructor
		~RedBookClass(void);					// This is the destructor
		ULONG		RedToHS(ULONG i);
		ULONG		MSFtoRed(UBYTE m, UBYTE s, UBYTE f);
		void		FullCDVolume(UBYTE chan);
		void		PlayTrack(UWORD track);
		void		Play_CD_MSL(UWORD min_sec, UWORD len);
		void		PlayMSF(UBYTE startM, UBYTE startS, UBYTE startF,
						UBYTE endM, UBYTE endS, UBYTE endF, UBYTE chan);
		UWORD		CheckCDMusic(void);
		void		StopCDMusic(void);

	private:

		SEGSEL	Tinfo_addrp;
		SEGSEL	Stat_addrp;
		SEGSEL	Stop_addrp;
		SEGSEL	Volm_addrp;
		SEGSEL	Play_addrp;

		StopType 	Stop;
		PlayType	Play;
		VolmType	Volm;
		StatType	Stat;
		TinfoType	Tinfo;
};

#endif	//NOT_FOR_WIN95

/***************************** End of Playcd.h ****************************/

extern GetCDClass		CDList;

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------
bool  CD_Volume_Verification ( int cd_drive, char *volume_label, char *volume_to_find );


#endif // PLAYCD_H
