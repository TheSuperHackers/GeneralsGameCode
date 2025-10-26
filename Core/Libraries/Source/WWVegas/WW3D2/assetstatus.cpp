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

#include "assetstatus.h"
#include "hashtemplate.h"
#include "wwstring.h"
#include "RAWFILE.H"

AssetStatusClass AssetStatusClass::Instance;

const char* ReportCategoryNames[AssetStatusClass::REPORT_COUNT]={
	"LOAD_ON_DEMAND_ROBJ",
	"LOAD_ON_DEMAND_HANIM",
	"LOAD_ON_DEMAND_HTREE",
	"MISSING_ROBJ",
	"MISSING_HANIM",
	"MISSING_HTREE"
};

AssetStatusClass::AssetStatusClass()
	:
	Reporting (true),
	LoadOnDemandReporting(false)
{
}

AssetStatusClass::~AssetStatusClass()
{
#ifdef WWDEBUG
	if (Reporting) {
		StringClass report("Load-on-demand and missing assets report\n\n");
		for (int i=0;i<REPORT_COUNT;++i) {
			report+="Category: ";
			report+=ReportCategoryNames[i];
			report+="\n\n";

			HashTemplateIterator<StringClass,int> ite(ReportHashTables[i]);
			for (ite.First();!ite.Is_Done();ite.Next()) {
				report+=ite.Peek_Key();
				int count=ite.Peek_Value();
				if (count>1) {
					StringClass tmp(0,true);
					tmp.Format("\t(reported %d times)",count);
					report+=tmp;
				}
				report+="\n";
			}
			report+="\n";
		}
		if (report.Get_Length()) {
			RawFileClass raw_log_file("asset_report.txt");
			raw_log_file.Create();
			raw_log_file.Open(RawFileClass::WRITE);
			raw_log_file.Write(report,report.Get_Length());
			raw_log_file.Close();
		}
	}
#endif
}

void AssetStatusClass::Add_To_Report(int index, const char* name)
{
	StringClass lower_case_name(name,true);
	_strlwr(lower_case_name.Peek_Buffer());
	// This is a bit slow - two accesses to the same member, but currently there's no better way to do it.
	int count=ReportHashTables[index].Get(lower_case_name);
	count++;
	ReportHashTables[index].Set_Value(lower_case_name,count);
}

void AssetStatusClass::Report_Load_On_Demand_RObj(const char* name)
{
	if (LoadOnDemandReporting) Add_To_Report(REPORT_LOAD_ON_DEMAND_ROBJ,name);
}

void AssetStatusClass::Report_Load_On_Demand_HAnim(const char* name)
{
	if (LoadOnDemandReporting) Add_To_Report(REPORT_LOAD_ON_DEMAND_HANIM,name);
}

void AssetStatusClass::Report_Load_On_Demand_HTree(const char* name)
{
	if (LoadOnDemandReporting) Add_To_Report(REPORT_LOAD_ON_DEMAND_HTREE,name);
}

void AssetStatusClass::Report_Missing_RObj(const char* name)
{
	Add_To_Report(REPORT_MISSING_ROBJ,name);
}

void AssetStatusClass::Report_Missing_HAnim(const char* name)
{
	Add_To_Report(REPORT_MISSING_HANIM,name);
}

void AssetStatusClass::Report_Missing_HTree(const char* name)
{
	Add_To_Report(REPORT_MISSING_HTREE,name);
}

