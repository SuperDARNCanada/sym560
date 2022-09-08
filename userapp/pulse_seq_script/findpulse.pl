#!/usr/bin/perl

# File : 	findpulse.pl
# Author: 	Bradley Krug
# Modified:	Feb 21 (added some comments)
# Description:	Script used to condense and structure the raw timestamp text files created
#		by the manual or automatic version of sym560_cmdline.  The script looks to 
#		see if the timestamps belong to a pulse sequence (such as a SuperDARN pulse
#		sequence) as defined by the @psep variable below.  The @psep variable is an
#		array of the seperation in ms between pulses in the sequence.  Pulses that
#		are part of a sequence get grouped together.  If a sequence is detected but
#		certain pulses are missing then the pulse is labelled as MISSING.  If a 
#		timestamp is determined not to be part of a pulse sequence then it is given
#		the label N for Not part of sequence.
#
#		An example of the raw timestamp text files structure is:
#		...
#		YEAR = 2008
#	        DAY = 050
#       	TIME = 19:28 UTC
#        	SEC = 30.4788532
#
#       	YEAR = 2008
#        	DAY = 050
#       	TIME = 19:28 UTC
#        	SEC = 30.4878127
#		...
#		and the resulting condensed files have the form:
#		
#		...
#		PULSES TIMESTAMPED FOR
#  		YEAR: 2008
#  		DAY: 032
#
#		MISSING
#		MISSING
#		16:29:53.1279494
#		16:29:53.1309487
#		16:29:53.1354495
#		16:29:53.1414494
#		16:29:53.1579484
#		16:29:53.1594489

#		16:29:53.1965083
#		16:29:53.2175084
#		16:29:53.2295090
#		16:29:53.2325082
#		16:29:53.2370087
#		16:29:53.2430086
#		16:29:53.2595083
#		16:29:53.2610088
#		...
#
#		The script can be run manually, in which case the name of the input
#		and output files will be requested.  The script can also be provided 
#		with the arguments auto and a filename (findpulse.pl auto timestamp.txt)
#		which will skip over the prompts thus allowing the script to be run
#		automatically.  This is how the sym560 auto function calls this script.

use strict;
use warnings;

#Global variables 
#array containing pulse seperations in milliseconds
my @psep = (21.0, 12.0 , 3.0, 4.5, 6.0, 16.5, 1.5);
my $curpulse = 1;		#keeps track of which pulse we are at

my ($pulsenum1, $pulsenum2) = {};	#used to pass back the pulse numbers in check_sequence	
my $ret;			#used to check return status of functions
my (%tstamp1, %tstamp2);		#containers for timestamp information
my ($hr1, $hr2, $min1, $min2, $sec1, $sec2,$yr,$day);	#scalars for easy printing of timestamps
my $cnt;
my $infile;
my $outfile;
my $hourflag = -1;

#If there is at least two arguments
if ($#ARGV > 0) {
	if ($ARGV[0] eq "auto") {
		$infile = $ARGV[1];
	}
	else {
		print "Invalid Argument\n";
		print "USAGE: findpulse.pl auto inputfile\n"; 
	}
}
else {
	#prompt user for name of raw data file
	$infile = &promptUser("Enter raw data input file ");
}

open(my $RAWDATA, $infile);

# get first 2 timestamps
%tstamp1 = &get_timestamp($RAWDATA);
if ($tstamp1{"year"} == 0) {
	print "\nNO TIMESTAMPS FOUND\n";
	close($RAWDATA);
	exit 0;
}
%tstamp2 = &get_timestamp($RAWDATA);
if ($tstamp2{"year"} == 0) {
	$hr1 = $tstamp1{"hour"};
	$min1 = $tstamp1{"minute"};
	$sec1 = $tstamp1{"second"};
	print "\nOnly 1 timestamp found : $hr1:$min1:$sec1\n";
	close($RAWDATA);
	exit 0;
}

#get date and remove leading whitespace
$yr = $tstamp1{"year"};
$day = $tstamp1{"day"};
$yr =~ s/^\s+//;
$day =~ s/^\s+//;

#determine the output file name
if ($#ARGV > 0) {
	if ($ARGV[0] eq "auto") {
		#$outfile = "pulses_".$yr."_$day.txt";
		# Get the hour, minute, second of first timestamp for the output file
		$hr1 = $tstamp1{"hour"};
		$min1 = $tstamp1{"minute"};
		$sec1 = $tstamp1{"second"};
		$hr1 =~ s/^\s+//;
		$min1 =~ s/^\s+//;
		$sec1 =~ s/^\s+//;
		$outfile = "pulses_".$yr."_".$day."_$hr1$min1.txt"
	}
}
else {
	#prompt user for name of outfile
	$outfile = &promptUser("Enter timestamp output file ", "pulses_".$yr."_"."$day.txt");
}

#check if file already exists
if (-e $outfile) {
	open(OUTFILE, ">>$outfile");	#open and append
}
else {
	open(OUTFILE, ">$outfile");	#open for write, overwrite
}

# printing header information
print OUTFILE "OUTPUT FORMAT IF A PULSE SEQUENCE IS FOUND: \n";
print OUTFILE "   PULSE 1 (HH:MM:SS.mmmuuun)\n";
print OUTFILE "   PULSE 2\n";
print OUTFILE "   ...\n";
print OUTFILE "   PULSE N\n\n";
print OUTFILE "IF A PULSE IS MISSING THEN 'MISSING' IS INSERTED\n";
print OUTFILE "IF A PULSE IS NOT PART OF A SEQUENCE THEN 'N' IS PRINTED BEFORE THE TIME\n\n";

#loop until end of input file reached
while(!(eof($RAWDATA))) {

	$yr = $tstamp1{"year"};
	$day= $tstamp1{"day"};
	$hr1 = $tstamp1{"hour"};
	$min1 = $tstamp1{"minute"};
	$sec1 = $tstamp1{"second"};
	$hr2 = $tstamp2{"hour"};
	$min2 = $tstamp2{"minute"};
	$sec2 = $tstamp2{"second"};
	
	# print year and day once an hour
	if (($hourflag == -1) || ($hourflag != $hr1)) {
		print OUTFILE "\n\nPULSES TIMESTAMPED FOR\n  YEAR: $yr\n  DAY: $day\n\n";
		$hourflag = $hr1;
	}
	
	#remove leading whitespace
	$sec1 =~ s/^\s+//;
	$sec2 =~ s/^\s+//;

	# do a check to see if they are part of a sequence
	$ret = &check_sequence(\%tstamp1, \%tstamp2, \@psep, \$pulsenum1, \$pulsenum2);
	
	# if the pulses aren't a part of a sequence then print the first one and move on.
	if ($ret == 1) {
		print OUTFILE "\nN $hr1:$min1:$sec1\n";
		%tstamp1 = %tstamp2;
		%tstamp2 = &get_timestamp($RAWDATA);
		if ($tstamp2{"year"} == 0) {
			print OUTFILE "\n\nEOF Encountered\n\n";
			close(OUTFILE);
			close($RAWDATA);
			exit 0;
		}
		next;
	}
	
	print OUTFILE "\n";
	#if pulsenum1 is not the first pulse print the previous ones as missing
	if ($pulsenum1 != 1) {
		for ($cnt = 1; $cnt < $pulsenum1; $cnt++) {
			#print OUTFILE "PULSE $cnt = MISSING\n";
			print OUTFILE "MISSING\n";
		}
	}
	#print first timestamp
	#print OUTFILE "PULSE $pulsenum1 = $hr1:$min1:$sec1\n";
	print OUTFILE "$hr1:$min1:$sec1\n";
	#if second timestampt is pulsenum1 + 1 then print missing for the in between pulses
	if ($pulsenum2 != $pulsenum1 + 1) {
		for ($cnt = $pulsenum1 + 1; $cnt < $pulsenum2; $cnt++) {
			#print OUTFILE "PULSE $cnt = MISSING\n";
			print OUTFILE "MISSING\n";
		}
	}
	#print second timestamp 
	#print OUTFILE "PULSE $pulsenum2 = $hr2:$min2:$sec2\n";
	print OUTFILE "$hr2:$min2:$sec2\n";
	
	#If second timestamp is the last pulse then move on to next iteration of loop
	if ($pulsenum2 == @psep + 1) {
		# get the next 2 timestamps
		%tstamp1 = &get_timestamp($RAWDATA);
		if ($tstamp1{"year"} == 0) {
			print OUTFILE "\nEOF\n";
			close(OUTFILE);
			close($RAWDATA);
			exit 0;
		}
		
		%tstamp2 = &get_timestamp($RAWDATA);
		if ($tstamp2{"year"} == 0) {
			$hr1 = $tstamp1{"hour"};
			$min1 = $tstamp1{"minute"};
			$sec1 = $tstamp1{"second"};
			$sec1 =~ s/^\s+//;
			print OUTFILE "\nN $hr1:$min1:$sec1\n";
			print OUTFILE "\nEOF ENCOUNTERED\n";
			close(OUTFILE);
			close($RAWDATA);
			exit 0;
		}
		next;
	}
	
	#otherwise loop through printing the rest of the sequence
	#for ($cnt = $pulsenum2; $cnt < @sep + 1; $cnt++) {
	while ($pulsenum2 < @psep+1) {
		%tstamp1 = %tstamp2;
		%tstamp2 = &get_timestamp($RAWDATA);
		#exit if eof encountered
		if ($tstamp2{"year"} == 0) {
			print OUTFILE "\nEOF ENCOUNTERED\n";
			close(OUTFILE);
			close($RAWDATA);
			exit 0;
		}
		
		$hr1 = $tstamp1{"hour"};
		$min1 = $tstamp1{"minute"};
		$sec1 = $tstamp1{"second"};
		$hr2 = $tstamp2{"hour"};
		$min2 = $tstamp2{"minute"};
		$sec2 = $tstamp2{"second"};
		$sec1 =~ s/^\s+//;
		$sec2 =~ s/^\s+//;

		$ret = &check_sequence(\%tstamp1, \%tstamp2, \@psep, \$pulsenum1, \$pulsenum2);
		
		#if the pulse sequence is unfinished
		if ($ret == 1) {
			print OUTFILE "REMAINING PULSES IN SEQUENCE ARE MISSING\n";
			%tstamp1 = %tstamp2;
			
			%tstamp2 = &get_timestamp($RAWDATA);
			if ($tstamp2{"year"} == 0) {
				print OUTFILE "\nEOF\n";
				close(OUTFILE);
				close($RAWDATA);
				exit 0;
			}
			$pulsenum2 = @psep+1;
		}
		
		#print MISSING for any pulses in between the two
		if ($pulsenum2 != $pulsenum1 + 1) {
			my $incnt;
			for ($incnt = $pulsenum1 + 1; $incnt < $pulsenum2; $incnt++) {
				#print OUTFILE "PULSE $incnt = MISSING\n";
				print OUTFILE "MISSING\n";
			}
		}
		#print OUTFILE "PULSE $pulsenum2 = $hr2:$min2:$sec2\n";
		print OUTFILE "$hr2:$min2:$sec2\n";
		#if that was the 8th pulse then get the next two timestamps
		if ($pulsenum2 == @psep+1) {
		
			# get next2 timestamps
			%tstamp1 = &get_timestamp($RAWDATA);
			if ($tstamp1{"year"} == 0) {
				print OUTFILE "\nEOF\n";
				close(OUTFILE);
				close($RAWDATA);
				exit 0;
			}
			
			%tstamp2 = &get_timestamp($RAWDATA);
			if ($tstamp2{"year"} == 0) {
				$hr1 = $tstamp1{"hour"};
				$min1 = $tstamp1{"minute"};
				$sec1 = $tstamp1{"second"};
				$sec1 =~ s/^\s+//;
				print OUTFILE "\nN $hr1:$min1:$sec1\n";
				print OUTFILE "\nEOF\n";
				close(OUTFILE);
				close($RAWDATA);
				exit 0;
			}
		}
	}
	
}

######################################################################################
#  Subroutine: 	check_sequence
#
#      Inputs:	$fhandle - filehandle for raw data input file
#		$ts1 - address of first timestamp
#		$ts2 - address of second timestamp
#		@sep - array containing pulse seperations in milliseconds
#		$pnum1 - address that will be passed back the pulse number of first
#			timestamp (0 if it is not part of sequence).
#		$pnum2 - same as num1 but for ts2 instead of ts1
#
#     Returns:	0 if pulses were part of same sequence
#	     	1 if pulses were not part of same sequence
#
# Description:	Checks two timestamps (ts1 and ts2) to see if they are part
#               of the sequence and returns which pulses in the sequence they are
#		via variables pnum1 and pnum2.
sub check_sequence {
	my $ts1 = shift;
	my $ts2 = shift;
	my $pulsep = shift;
	my $pnum1 = shift;
	my $pnum2 = shift;
	my ($i, $j);
	my $range = 0.00005;	#+/- range that pulse seperation is considered acceptable
	my $totalsep = 0;
	
	my @sep = @{$pulsep};
	
	my $sec1 = $ts1->{"second"};
	my $sec2 = $ts2->{"second"};
	if ($sec2 < $sec1) {
		$sec2 = $sec2 + 60;
	}
	
	#initialize the pulse numbers
	${$pnum1} = 1;
	${$pnum2} = 2;
	
	#loop until proper pulse number for timestamps is found
	for ($i = 0; $i < @sep; $i++) {
		$totalsep = ($sep[$i])*0.001;
		#loop checking all possible seperations for $ts1  = pulse # i+1 
		for ($j = $i; $j < @sep; $j++) {
			if (($sec2 > ($sec1+($totalsep-$range))) && ($sec2 < ($sec1+($totalsep+$range)))) {
				${$pnum1} = $i+1;
				${$pnum2} = $j+2;
				return 0;
			}
			else {
				#add the next seperation to the total seperation
				if ($j != (@sep - 1)) {
					$totalsep = $totalsep + (($sep[$j+1])*0.001)
				}
			}
		}
	}
	
	return 1;
}
######################################################################################



######################################################################################
#  Subroutine: 	get_timestamp
#
#      Inputs:	$fhandle - filehandle for raw data input file
#
#     Returns:	Timestamp data in the form of a hash with the keys:
#		year, day, hour, minute and second
#		Timestamp "year" will be 0 if EOF was reached
#
# Description:	This routine reads in lines from a provided filehandle
#		until it detects a timestamp (a line containing the string YEAR).
#		The resulting timestamp data is read in and passed back.
#		This function assumes the filehandle contains timestamps of the form:
#		YEAR = 2008
#	        DAY = 050
#       	TIME = 19:28 UTC
#        	SEC = 30.4788532
#
#       	YEAR = 2008
#        	DAY = 050
#       	TIME = 19:28 UTC
#        	SEC = 30.4878127
sub get_timestamp {
	my $fhandle = shift;
	
	#declare local variables;
	my $yearcheck = 0;
	my $rawline;
	my @splitstr;
	my $curyear;
	my $curday;
	my $curtime;
	my $curhour;
	my $curminute;
	my $cursecond;
	my %timestamp;
	
	# read in lines until you reach the beginning of a timestamp
	while ($yearcheck == 0) {
		if (eof($fhandle)) {
			%timestamp = ("year", 0);
			return %timestamp;
		}
		
		$rawline = <$fhandle>;
		chomp($rawline);
		if ($rawline =~ /YEAR/) {
			$yearcheck = 1;
		}
	}
	
	#get the year
	@splitstr = split(/=/, $rawline);
	$curyear = $splitstr[1];
	
	#get the day
	$rawline = <$fhandle>;
	chomp($rawline);
	@splitstr = split(/=/, $rawline);
	$curday = $splitstr[1];
	
	#get the time (hour and minute)
	$rawline = <$fhandle>;
	chomp($rawline);
	@splitstr = split(/=/, $rawline);
	$curtime = $splitstr[1];
	@splitstr = split(/:/, $curtime);
	$curhour = $splitstr[0];
	$curminute = $splitstr[1];
	@splitstr = split(/ /, $curminute); #remove the UTC symbol
	$curminute = $splitstr[0];
	
	#get the seconds
	$rawline = <$fhandle>;
	chomp($rawline);
	@splitstr = split (/=/, $rawline);
	$cursecond = $splitstr[1];
	
	%timestamp = ("year", $curyear,
			"day", $curday,
			"hour", $curhour,
			"minute", $curminute,
			"second", $cursecond);
	return %timestamp;
}
######################################################################################


#----------------------------------------------------------------------#
#  Copyright 1998 by DevDaily Interactive, Inc.  All Rights Reserved.  #
#----------------------------------------------------------------------#

#----------------------------(  promptUser  )-----------------------------#
#                                                                         #
#  FUNCTION:	promptUser                                                #
#                                                                         #
#  PURPOSE:	Prompt the user for some type of input, and return the    #
#		input back to the calling program.                        #
#                                                                         #
#  ARGS:	$promptString - what you want to prompt the user with     #
#		$defaultValue - (optional) a default value for the prompt #
#                                                                         #
#-------------------------------------------------------------------------#

sub promptUser {

   #-------------------------------------------------------------------#
   #  two possible input arguments - $promptString, and $defaultValue  #
   #  make the input arguments local variables.                        #
   #-------------------------------------------------------------------#

   my $promptString = shift;
   my $defaultValue = shift;

   #-------------------------------------------------------------------#
   #  if there is a default value, use the first print statement; if   #
   #  no default is provided, print the second string.                 #
   #-------------------------------------------------------------------#

   if ($defaultValue) {
      print $promptString, "[", $defaultValue, "]: ";
   } else {
      print $promptString, ": ";
   }

   $| = 1;               # force a flush after our print
   $_ = <STDIN>;         # get the input from STDIN (presumably the keyboard)


   #------------------------------------------------------------------#
   # remove the newline character from the end of the input the user  #
   # gave us.                                                         #
   #------------------------------------------------------------------#

   chomp;

   #-----------------------------------------------------------------#
   #  if we had a $default value, and the user gave us input, then   #
   #  return the input; if we had a default, and they gave us no     #
   #  no input, return the $defaultValue.                            #
   #                                                                 # 
   #  if we did not have a default value, then just return whatever  #
   #  the user gave us.  if they just hit the <enter> key,           #
   #  the calling routine will have to deal with that.               #
   #-----------------------------------------------------------------#

   if ($defaultValue) {
      return $_ ? $_ : $defaultValue;    # return $_ if it has a value
   } else {
      return $_;
   }
}
