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

#include "debug.h"
#include "expander.h"
#include "KVPair.h"
#include "misc.h"
#include <Utility/iostream_adapter.h>
#include <list>
#include <stdlib.h>
#include <stdio.h>

//=============================================================================

std::string tableRow;

//=============================================================================

static void exitWait(void)
{
	system("PAUSE");
}

//=============================================================================

#define LINESIZE 1024
static bool getNextLine(FILE *fp, char *line, int& frame, int& index) {
	if (!fp)
		return false;
	
	char buf[LINESIZE];
	while (fgets(buf, LINESIZE-1, fp) != NULL)
	{
		int len = strlen(buf);
		if (buf[len-1] == '\n')
			buf[len-1] = '\0';
		if (sscanf(buf, "%d:%d ", &frame, &index) == 2)
		{
			strcpy(line, buf);
			return true;
		}
	}
	
	fclose(fp);
	return false;
}

//=============================================================================

static std::string readInFile(const char *fname) {
	FILE *fp = fopen(fname, "rt");
	if (!fp)
		return "";

	std::string ret;
	char buf[LINESIZE];
	while (fgets(buf, LINESIZE-1, fp) != NULL)
	{
		ret.append(buf);
	}
	
	fclose(fp);
	return ret;
}

//=============================================================================

static FILE *ofp = NULL;

void dumpQueued(void);

static void outputLine(const char *line)
{
	dumpQueued();
	//cout << line << endl;
	if (ofp)
	{
		fputs(line, ofp);
	}
}

//=============================================================================

static void outputLine(int frame, int index, int linkNum,
											 const char *class1, const char *line1,
											 const char *class2, const char *line2, bool same = false)
{
	dumpQueued();
	if (!line1)
		line1 = "&nbsp;";
	if (!line2)
		line2 = "&nbsp;";
	if (!class1)
		class1 = "";
	if (!class2)
		class2 = "";

	Expander e("((", "))");
	e.addExpansion("LEFTCLASS", class1);
	e.addExpansion("LEFTLINE", line1);
	e.addExpansion("RIGHTCLASS", class2);
	e.addExpansion("RIGHTLINE", line2);
	if (same)
	{
		e.addExpansion("NAME", "");
		e.addExpansion("PREV", intToString(linkNum-1));
		e.addExpansion("NEXT", intToString(linkNum));
	}
	else
	{
		e.addExpansion("NAME", intToString(linkNum));
		e.addExpansion("PREV", intToString(linkNum-1));
		e.addExpansion("NEXT", intToString(linkNum+1));
	}

	std::string out;
	e.expand(tableRow, out);
	const char *buf = out.c_str();

	//cout << buf << endl;
	if (ofp)
	{
		fputs(buf, ofp);
	}
}

//=============================================================================

std::list<std::string> queuedLines;
static void queueLine(int frame, int index, const char *line1)
{
	if (!line1)
		line1 = "&nbsp;";

	Expander e("((", "))");
	e.addExpansion("LEFTCLASS", "leftHistory");
	e.addExpansion("LEFTLINE", line1);
	e.addExpansion("RIGHTCLASS", "rightHistory");
	e.addExpansion("RIGHTLINE", line1);
	e.addExpansion("NAME", "");
	e.addExpansion("PREV", intToString(0));
	e.addExpansion("NEXT", intToString(1));

	std::string out;
	e.expand(tableRow, out);

	queuedLines.push_back(out);
	if (queuedLines.size() > 150)
		queuedLines.pop_front();

	//cout << buf << endl;
}

void dumpQueued(void)
{
	while (!queuedLines.empty())
	{
		std::list<std::string>::iterator it = queuedLines.begin();
		const char *buf = (*it).c_str();
		//cout << buf << endl;
		if (ofp)
		{
			fputs(buf, ofp);
		}

		queuedLines.pop_front();
	}
}

//=============================================================================

int main(int argc, char *argv[])
{
	atexit(exitWait);
	const char *inFname[2];
	const char *outFname = "out.html";
	FILE *ifp[2] = {NULL, NULL};
	std::string header, footer;

	if (argc != 7)
	{
		cout << "Usage: munkeeDiff top.html row.html bottom.html in1.txt in2.txt out.txt" << endl;
		header = readInFile("top.html");
		tableRow = readInFile("row.html");
		footer = readInFile("bottom.html");
		inFname[0] = "test1.txt";
		inFname[1] = "test2.txt";
		//return 0;
	}
	else
	{
		header = readInFile(argv[1]);
		tableRow = readInFile(argv[2]);
		footer = readInFile(argv[3]);
		inFname[0] = argv[4];
		inFname[1] = argv[5];
		outFname = argv[6];
	}
	
	ifp[0] = fopen(inFname[0], "rt");
	if (!ifp[0])
	{
		cout << "could not open " << inFname[0] << endl;
		return 1;
	}
	ifp[1] = fopen(inFname[1], "rt");
	if (!ifp[1])
	{
		fclose(ifp[0]);
		cout << "could not open " << inFname[1] << endl;
		return 1;
	}
	
	ofp = fopen(outFname, "wt");
	
	char lastLine[2][LINESIZE];
	int lastFrame[2] = {-1, -1};
	int lastIndex[2] = {-1, -1};
	bool fileOk[2] = {true, true};

	outputLine(header.c_str());

	int linkNum = 1;
	int numDiffs = 0;

	bool seenRight = false;
	bool seenLeft = false;

	while (fileOk[0] || fileOk[1])
	{
		for (int i=0; i<2; ++i)
		{
			if (fileOk[i] == true && lastFrame[i] < 0)
			{
				fileOk[i] = getNextLine(ifp[i], lastLine[i],
					lastFrame[i], lastIndex[i]);
			}
		}
		
		if (fileOk[0] && fileOk[1])
		{
			if (lastFrame[0] < lastFrame[1] ||
				(lastFrame[0]==lastFrame[1] && lastIndex[0] < lastIndex[1]))
			{
				//if (!seenLeft)
					//cout << "Seen left on " << lastFrame[0] << ":" << lastIndex[0] << endl;
				//seenLeft = true;
				if (seenRight && seenLeft)
				{
					outputLine(lastFrame[0], lastIndex[0], linkNum++,
						"leftOnly", lastLine[0], NULL, NULL);
					++numDiffs;
				}
				lastFrame[0] = -1;
			}
			else if (lastFrame[1] < lastFrame[0] ||
				(lastFrame[1]==lastFrame[0] && lastIndex[1] < lastIndex[0]))
			{
				//if (!seenRight)
					//cout << "Seen right on " << lastFrame[1] << ":" << lastIndex[1] << endl;
				//seenRight = true;
				if (seenRight && seenLeft)
				{
					outputLine(lastFrame[1], lastIndex[1], linkNum++,
						NULL, NULL, "rightOnly", lastLine[1]);
					++numDiffs;
				}
				lastFrame[1] = -1;
			}
			else
			{
				int res = strcmp(lastLine[0], lastLine[1]);
				if (res!=0)
				{
					if (!seenLeft || !seenRight)
						cout << "Seen both on " << lastFrame[0] << ":" << lastIndex[0] << endl;
					seenLeft = seenRight = true;
					outputLine(lastFrame[0], lastIndex[0], linkNum++,
						"leftDiff", lastLine[0], "rightDiff", lastLine[1]);
					++numDiffs;
				}
				else
				{
					//if (!seenLeft || !seenRight)
					//	cout << "Seen both on " << lastFrame[0] << ":" << lastIndex[0] << endl;
					//seenLeft = seenRight = true;
					static bool printedFirst = false;
					if (!printedFirst)
					{
						outputLine(lastFrame[0], lastIndex[0], linkNum,
							"leftSame", lastLine[0], "rightSame", lastLine[1], true);
						printedFirst = true;
					}
					else if (seenLeft && seenRight)
					{
						outputLine(lastFrame[0], lastIndex[0], linkNum,
							"leftSame", lastLine[0], "rightSame", lastLine[1], true);
						++numDiffs;
					}
					else
						queueLine(lastFrame[0], lastIndex[0], lastLine[0]);
					//++numDiffs;
				}
				lastFrame[0] = -1;
				lastFrame[1] = -1;
			}
		}
		else if (fileOk[0])
		{
			//if (!seenLeft)
				//cout << "Seen left on " << lastFrame[0] << ":" << lastIndex[0] << endl;
			//seenLeft = true;
			if (seenRight && seenLeft)
			{
				outputLine(lastFrame[0], lastIndex[0], linkNum++,
					"leftOnly", lastLine[0], NULL, NULL);
				++numDiffs;
			}
			lastFrame[0] = -1;
		}
		else if (fileOk[1])
		{
			//if (!seenRight)
				//cout << "Seen right on " << lastFrame[1] << ":" << lastIndex[1] << endl;
			//seenRight = true;
			if (seenRight && seenLeft)
			{
				outputLine(lastFrame[1], lastIndex[1], linkNum++,
					NULL, NULL, "rightOnly", lastLine[1]);
				++numDiffs;
			}
			lastFrame[1] = -1;
		}

		if (numDiffs > 1000)
			break;
	}

	Expander e("((", "))");
	e.addExpansion("LAST", intToString(linkNum-1));
	e.addExpansion("BOTTOM", intToString(linkNum));
	std::string out;
	e.expand(footer, out);
	outputLine(out.c_str());
	
	if (ofp)
	{
		fclose(ofp);
	}
	
	return 0;
}
