#
#	Command & Conquer Generals Zero Hour™
#	Copyright 2025 Electronic Arts Inc.
#
#	This program is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

sub ShowWarning;
sub ShowError;

$noxScript = shift || 'C:\Nox\Game\nox.str';
$outFile = shift;

open(STR, $noxScript) || die "Can't open Nox string file: $!\n";

if ($outFile)
{
	open(STDOUT, ">$outFile") || die "Can't open output file: $!\n";
}
select((select(STDOUT), $| = 1)[0]);

$state = 0;
$line = 0;
$strcount = 0;
$strMaxCount = 32;
$lastFileName = "";
$lastFileNameLine = 0;
$ignoreErrors = 0;
$errCount = 0;
%labels = ();


while (<STR>)
{
	$strline = $_;
	$line++;
	$strline =~ s/\/\/.*//g;
	$strline =~ s/^\s*//g;
	$strline =~ s/\s*$//g;
	if (! ($strline eq ""))
	{
		if ($state == 0)
		{
			if (! ($strline =~ /^[^\"][^ :]*:[^ :]*[^\"]$/))
			{
				ShowError($line, $strline, "Expecting 'file:name'\n");
			}
			else
			{
				if (defined ($labels{$strline}))
				{
					ShowWarning($line, $strline, "Duplicate label at line $labels{$strline}\n");
				}

				$labels{$strline} = $line;
				$lastFileName = $strline;
				$lastFileNameLine = $line;
				$strcount = 0;			
				$ignoreErrors = 0;
				$state = 1;
			}
		}

		elsif ($state == 1)
		{
			if (++$strcount >= $strMaxCount)
			{
				ShowError($line, $strline, "Too many strings ($strcount) - suspected missing END from section $lastFileName on line $lastFileNameLine\n");
			}
			elsif ($strline =~ /^END$/i)
			{
				$state = 0;				
			}
			elsif ($strline =~ /\/n/i)
			{
				ShowWarning($line, $strline, "'/n'? This should probably be '\\n' instead\n");
			}
			elsif ($strline =~ /^\"[^\"]*[^\"]$/)
			{
				$state = 2;
			}
			else
			{
				$strline =~ s/\"\s*={0,1}\s*[a-zA-Z0-9_\.]*[^\"]$/\"/i;

				if (! ($strline =~ /^\".*\"$/))
				{
					if ($strline =~ /^[^\"][^ :]*:[^ :]*[^\"]$/)
					{
						ShowError($line, $strline, "Missing quote - suspected missing END from section $lastFileName on line $lastFileNameLine\n");
					}
					else
					{
						ShowError($line, $strline, "Missing quote\n");
					}
				}
			}
		}

		elsif ($state == 2)
		{
			$strline =~ s/\"\s*={0,1}\s*[a-zA-Z0-9_\.]*[^\"]$/\"/i;
			if (++$strcount >= $strMaxCount)
			{
				ShowError($line, $strline, "Too many strings ($strcount) - suspected missing END from section $lastFileName on line $lastFileNameLine\n");
			}
			elsif ($strline =~ /\/n/i)
			{
				ShowWarning($line, $strline, "'/n'? This should probably be '\\n' instead\n");
			}
			elsif ($strline =~ /^[^\"].*\"$/)
			{
				$state = 1;
			}
			elsif (! ($strline =~ /^[^\"].*[^\"]$/))
			{
				ShowError($line, $strline, "Extra quote character found at start of line\n");
			}
			elsif ($strline =~ /^END$/i)
			{
				ShowWarning($line, $strline, "Suspiciously placed 'END'. Maybe missing an end-quote from previous section\n");
			}
		}
	}
}

if ($state != 0)
{
	ShowError($line, $strline, "Missing END\n");
}

print STDOUT "$line lines processed\n";

close(STR);

if ($errCount == 0)
{
	open(RESULT, ">strcheck.err") || die "Can't open output file: $!\n";
	print RESULT "OK";
	close (RESULT);

}

exit($errCount);

sub ShowWarning
{
	my ($errLine, $errString, $errDesc) = @_;

	if (length($errString) > 32)
	{
		$errString = substr($errString, 0, 32) . "...";
	}

	if ($ignoreErrors == 0)
	{
		print STDOUT "$noxScript($errLine) : warning: '$errString' : $errDesc";
	}

	$state = 0;
	$ignoreErrors = 1;
}

sub ShowError
{
	my ($errLine, $errString, $errDesc) = @_;

	if (length($errString) > 32)
	{
		$errString = substr($errString, 0, 32) . "...";
	}

	if ($ignoreErrors == 0)
	{
		print STDOUT "$noxScript($errLine) : error: '$errString' : $errDesc";
		$errCount++;
	}

	$state = 0;
	$ignoreErrors = 1;
}
