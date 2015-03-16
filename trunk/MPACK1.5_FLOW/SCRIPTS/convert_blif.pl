#!usr/bin/perl -w
use strict;
use Shell;
#Use the time
use Time::gmtime;

#Get Date
my $mydate = gmctime();

my ($fname,$frpt);

sub print_usage()
{
  print "Usage:\n";
  print "      perl <script_name.pl> [-options]\n";
  print "      Options:(Mandatory!)\n";
  print "              -i <input_blif_path>\n";
  print "              -o <output_blif_path>\n";
  print "\n";
  return 1;
}

sub opts_read()
{
  if (-1 == $#ARGV)
  {
    print "Error: No input argument!\n";
    &print_usage();
    exit(1); 
  }
  else
  {
    for (my $iargv = 0; $iargv < $#ARGV+1; $iargv++)
    {
      if ("-i" eq $ARGV[$iargv]) 
      {$fname = $ARGV[$iargv+1];}
      elsif ("-o" eq $ARGV[$iargv]) 
      {$frpt = $ARGV[$iargv+1];}
    }
  } 
  return 1;
}

sub scan_blif()
{
  my ($line);
  my @tokens;
  
  # Open src file
  open(FIN, "< $fname") or die "Fail to open $fname!\n";  
  # Open des file
  open(FOUT, "> $frpt") or die "Fail to open $frpt!\n";  
  while(defined($line = <FIN>)) {
    chomp $line; 
    # Check if this line start with ".latch", which we cares only
    @tokens = split('\s+',$line);
    if ($tokens[0] eq ".names") {
      if (($#tokens - 1) == 3) {
        print FOUT ".gate CARRY a=$tokens[1] b=$tokens[2] c=$tokens[3] O=$tokens[4]\n"; 
      }
      elsif (($#tokens - 1) == 2) {
        print FOUT ".gate AND a=$tokens[1] b=$tokens[2] O=$tokens[3]\n"; 
      }
      elsif (($#tokens - 1) == 1) {
        $line = <FIN>;
        if ($line =~ m/^0/) {
          print FOUT ".gate INV a=$tokens[1] O=$tokens[2]\n";
        }
        else {
          print FOUT ".gate BUF a=$tokens[1] O=$tokens[2]\n";
        } 
      }
    }
    else {
      print FOUT "$line\n";
    }
  }
  close(FIN);
  close(FOUT);
  return 1;
}

sub main()
{
  &opts_read(); 
  &scan_blif();
  return 1;
}
 
&main();
exit(1);
