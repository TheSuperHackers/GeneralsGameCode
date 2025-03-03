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

// MapSettings.cpp : implementation file
//

#define DEFINE_TIME_OF_DAY_NAMES
#define DEFINE_WEATHER_NAMES

#include "StdAfx.h"
#include "WorldBuilder.h"
#include "MapSettings.h"
#include "Common/GlobalData.h"
#include "Common/WellKnownKeys.h"
#include "Compression.h"

/////////////////////////////////////////////////////////////////////////////
// MapSettings dialog


MapSettings::MapSettings(CWnd* pParent /*=NULL*/)
	: CDialog(MapSettings::IDD, pParent)
{
	//{{AFX_DATA_INIT(MapSettings)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void MapSettings::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(MapSettings)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(MapSettings, CDialog)
	//{{AFX_MSG_MAP(MapSettings)
	ON_CBN_SELENDOK(IDC_MAP_TIMEOFDAY, OnChangeMapTimeofday)
	ON_CBN_SELENDOK(IDC_MAP_WEATHER, OnChangeMapWeather)
	ON_EN_CHANGE(IDC_MAP_TITLE, OnChangeMapTitle)
	ON_CBN_SELENDOK(IDC_MAP_COMPRESSION, OnChangeMapCompression)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// MapSettings message handlers

void MapSettings::OnChangeMapTimeofday() 
{
}

void MapSettings::OnChangeMapWeather() 
{
}

BOOL MapSettings::OnInitDialog() 
{
	CDialog::OnInitDialog();

	Int i;

	CComboBox *timeofday = (CComboBox*)GetDlgItem(IDC_MAP_TIMEOFDAY);
	timeofday->ResetContent();
	for (i = TIME_OF_DAY_FIRST; i < TIME_OF_DAY_COUNT; i++)
	{
		timeofday->AddString(TimeOfDayNames[i]);
	}
	timeofday->SetCurSel(TheGlobalData->m_timeOfDay-TIME_OF_DAY_FIRST);
	
	
	CComboBox *weather = (CComboBox*)GetDlgItem(IDC_MAP_WEATHER);
	weather->ResetContent();
	for (i = 0; i < WEATHER_COUNT; i++)
	{
		weather->AddString(WeatherNames[i]);
	}
	weather->SetCurSel(TheGlobalData->m_weather);

	Dict *worldDict = MapObject::getWorldDict();
	Bool exists = false;
	AsciiString mapName = worldDict->getAsciiString(TheKey_mapName, &exists);
	CEdit *mapTitle = (CEdit *)GetDlgItem(IDC_MAP_TITLE);
	if (exists)
	{
		mapTitle->SetWindowText(mapName.str());
	}
	else
	{
		mapTitle->SetWindowText("");
	}

	CComboBox *compressionComboBox = (CComboBox*)GetDlgItem(IDC_MAP_COMPRESSION);
	compressionComboBox->ResetContent();
	for (i = COMPRESSION_MIN; i <= COMPRESSION_MAX; i++)
	{
		compressionComboBox->AddString(CompressionManager::getCompressionNameByType((CompressionType)i));
	}
	exists = FALSE;
	Int index = worldDict->getInt(TheKey_compression, &exists);
	if (!exists)
		index = CompressionManager::getPreferredCompression();
	compressionComboBox->SetCurSel(index);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void MapSettings::OnOK() 
{
	CComboBox *timeofday = (CComboBox*)GetDlgItem(IDC_MAP_TIMEOFDAY);
	CComboBox *weather = (CComboBox*)GetDlgItem(IDC_MAP_WEATHER);
	TimeOfDay tod = (TimeOfDay) (timeofday->GetCurSel()+TIME_OF_DAY_FIRST);
	Weather theWeather = (Weather) weather->GetCurSel();

	TheWritableGlobalData->setTimeOfDay(tod);
	TheWritableGlobalData->m_weather = theWeather;

	CEdit *mapTitle = (CEdit *)GetDlgItem(IDC_MAP_TITLE);
	char munkee[256];
	AsciiString mapName;
	mapTitle->GetWindowText(munkee, 256);
	mapName = munkee;
	Dict *worldDict = MapObject::getWorldDict();
	worldDict->setAsciiString(TheKey_mapName, mapName);

	CComboBox *compressionComboBox = (CComboBox*)GetDlgItem(IDC_MAP_COMPRESSION);
	CompressionType compType = (CompressionType) (compressionComboBox->GetCurSel()+COMPRESSION_MIN);
	worldDict->setInt(TheKey_compression, compType);

	CDialog::OnOK();
}

void MapSettings::OnChangeMapTitle() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	
}

void MapSettings::OnChangeMapCompression() 
{
	// TODO: Add your control notification handler code here
	
}
