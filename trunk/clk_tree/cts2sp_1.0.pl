#!usr/bin/perl 
#!usr/bin/perl -w
# use the strict mode
use strict;
# Use the Shell enviornment
use Shell;
# Use the time
use Time::gmtime;
# Use switch module
use Switch;
use File::Path;

# Date
my $mydate = gmctime(); 

# Global Variants
# input Option Hash
my %opt_h; 
my $opt_ptr = \%opt_h;
# configurate file hash
my %conf_h;
my $conf_ptr = \%conf_h;
# Monto Carlo
my $mc = 0;
# Tier Num
my @tier_ary;
my $tier_ary_ptr = \@tier_ary;
# Hash for measure results
my %meas_rst;
my $meas_rst_ptr = \%meas_rst;
# Hash for Correlation Matrix
my %corr_mats;
my $corr_mats_ptr = \%corr_mats;


# Configuration file keywords list
# Category for conf file.
# main category : 1st class
my @mctgy;
# sub category : 2nd class
my @sctgy;
# Initialize these categories
@mctgy = ("dir_path",
          "ic_parameters",
          "cts_setup",
          "cts_net_check",
          "cts_net_main",
          "cts_net_sub",
          "cts_abbrev",
          "corr_matrix",
         );
# refer to the keywords of dir_path
@{$sctgy[0]} = ("spice_dir",
                "setup_dir",
                "net_dir",
                "proc_dir",
                "rpt_dir",
                "matrix_dir",
               );
# refer to the keywords of circuit parameters
@{$sctgy[1]} = ("buf_size",
                "num_clk",
                "edge_ratio",
                "pmos_name",
                "nmos_name",
                "pmos_l",
                "nmos_l",
                "pmos_w",
                "nmos_w",
                "vdd",
                "gnd",
                "vh",
                "vl",
                "resolution",
                "skew_from",
                "skew_to",
                "slew_from",
                "slew_to",
                "tsv_model",
                "net_model",
                "temperature",
                "gauss_sigma",
                "noise_amp",
                "noise_freq",
                "noise_phase",
                "noise_delay",
               );
# refer to the keywords of cts_setup
@{$sctgy[2]} = ("tier_src",
           "tvs_bound",
           "clk_freq",
           #"r_buf",
           #"c_buf",
           #"intdly_buf",
           "r_wire",
           "c_wire",
           "r_tsv",
           "c_tsv",
           #"load_buf",
           "wid_lb",
           "d2d_lb",
           "wid_vth",
           "d2d_vth",
           "wid_rw",
           "d2d_rw",
           "wid_cw",
           "d2d_cw",
           "wid_rt",
           "d2d_rt",
           "wid_ct",
           "d2d_ct",
          );
# refer to the keywords of cts netlist check
@{$sctgy[3]} = ("num_sink",
           "num_buf",
           "num_tsv",
           "num_mp",
           "x",
           "y",
           );
# refer to the keywords of cts netlist main
@{$sctgy[4]} = (
           "src",
           "buf",
           "net",
           "mp",
           "tsv",
           "sink",
          );
# refer to the keywords of cts netlist sub
@{$sctgy[5]} = ("coord",
                "cload",
               );
# refer to the keywords of cts abbrevations
@{$sctgy[6]} = ("buf",
                "sink",
                "mp",
                "tsv",
               );
# refer to the keywords of correlation matrix 
@{$sctgy[7]} = ("dev",
                "tier",
                "idx",
                "para",
                "begin_mat",
                "end_mat",
               );

# Netlist file list
my @netfiles;

# Hash reference for analyze netlist
my (%range_h);
my (%buf_h);
my (%net_h);
my (%mp_h);
my (%tsv_h);
my (%src_h);
my (%sink_h);
my ($buf_ptr,$net_ptr,$mp_ptr,$tsv_ptr,$src_ptr,$sink_ptr,$range_ptr) = 
   (\%buf_h,\%net_h,\%mp_h,%tsv_h,\%src_h,\%sink_h,\%range_h);
 

# Debug mode switcher '1' is on
my $debug = 0;


# ----------Subrountines------------#
# Create paths if it does not exist.
sub generate_path($)
{
  my ($mypath) = @_; 
  if (!(-e "$mypath"))
  {
    mkpath "$mypath";
    print "Path($mypath) does not exist...Create it.\n";
  }
  return 1;
}

# Print the usage
sub print_usage()
{
  print "Usage:\n";
  print "      cts2sp [-options <value>]\n";
  print "      Mandatory options: \n";
  print "      -conf : specify the basic configuration files for cts2sp\n";
  print "      -fconf : the configuration file contains CTS file names\n"; 
  print "      Other Options:\n";
  print "      -server_mode : turn on the server mode of HSPICE if supported(Default: off)\n";
  print "      -64bit_mode : turn on the 64-bit HSPICE mode(Default: off)\n";
  print "      -mt <value> : multithreadly run HSPICE, <value> is the number of thread\n";
  print "      -buffer_type <type> : choose whether to use inverter or buffer in the clock tree(type: 1.inv 2.buf)\n";
  print "      -full_check : turn on checking floating nodes(Default:off)\n";
  print "      -dc_analysis : turn on DC analysis\n";
  print "      -power_analysis : turn on power analysis\n";
  print "      -meas_skew : turn on the measurements for clock skew(Default: off)\n";
  print "      -meas_delay : turn on the measurements for delay of each sink(Default: off)\n";
  print "      -wave_post : turn on the Waveform print(Be careful to use this option when Monte Carlo is selected!)\n";
  print "      -power_noise : turn on the power noise(Default:off)\n";
  print "      -mc <value> : Monto Carlo simulations are done,and specify the number of Monte Carlo analysis(Default:off)\n";
  print "      -corr : take the correlations between device variations into account(Default: off)\n";
  print "      -debug : debug mode\n";
  print "      -help : print usage\n";
  exit(1);
  return 1;
}
 
sub spot_option($ $)
{
  my ($start,$target) = @_;
  my ($arg_no,$flag) = (-1,"unfound");
  for (my $iarg = $start; $iarg < $#ARGV+1; $iarg++)
  {
    if ($ARGV[$iarg] eq $target)
    {
      if ("found" eq $flag)
      {
        print "Error: Repeated Arguments!(IndexA: $arg_no,IndexB: $iarg)\n";
        &print_usage();        
      }
      else
      {
        $flag = "found";
        $arg_no = $iarg;
      }
    }
  }
  # return the arg_no if target is found
  # or return -1 when target is missing
  return $arg_no; 
}

sub read_opt_into_hash($ $ $)
{
  my ($opt_name,$opt_with_val,$mandatory) = @_;
  # Check the -$opt_name
  my ($opt_fact) = ("-".$opt_name);
  my ($cur_arg) = (0);
  my ($argfd) = (&spot_option($cur_arg,"$opt_fact"));
  if ($opt_with_val eq "on")
  {
    if (-1 != $argfd)
    {
      if ($ARGV[$argfd+1] =~ m/^-/)
      {
        print "The next argument cannot start with '-'!\n"; 
        print "it implies an option!\n";
      }
      else
      {
        $opt_ptr->{"$opt_name\_val"} = $ARGV[$argfd+1];
        $opt_ptr->{"$opt_name"} = "on";
      }     
    }
    else
    {
      $opt_ptr->{"$opt_name"} = "off";
      if ($mandatory eq "on")
      {
        print "Mandatory option: $opt_fact is missing!\n";
        &print_usage();
      }
    }
  }
  else
  {
    if (-1 != $argfd)
    {
      $opt_ptr->{"$opt_name"} = "on";
    }
    else
    {
      $opt_ptr->{"$opt_name"} = "off";
      if ($mandatory eq "on")
      {
        print "Mandatory option: $opt_fact is missing!\n";
        &print_usage();
      }
    }  
  }
  return 1;
}

# Read options
sub opts_read()
{
  # if no arguments detected, print the usage.
  if (-1 == $#ARGV)
  {
    print "Error : No input arguments!\n";
    print "Try: -help for usage.\n";
    exit(1);
  } 
  else
  {
    # Debug mode: print the ARGVs
    if (1 == $debug)
    {
      print "The input ARGVs are: \n";
      for (my $iargv = 0; $iargv < $#ARGV+1; $iargv++)
      {print "$iargv : $ARGV[$iargv]\n";}
    }  
  } 
  # Read in the options
  my ($cur_arg,$arg_found);
  $cur_arg = 0;
  print "Analyzing your options...\n";
  # Read the options with internal options
  my $argfd;
  # Check help fist 
  $argfd = &spot_option($cur_arg,"-help");
  if (-1 != $argfd)
  {
    print "Help desk:\n";
    &print_usage();
  }  
  # Then Check the debug
  $argfd = &spot_option($cur_arg,"-debug");
  if (-1 != $argfd)
  {
    $opt_ptr->{"debug"} = "on";
    $debug = 1;
  }
  else
  {
    $opt_ptr->{"debug"} = "off";
    $debug  = 0;
  }
  # Check mandatory options
  # Check the -conf
  # Read Opt into Hash(opt_ptr) : "opt_name","with_val","mandatory"
  &read_opt_into_hash("conf","on","on");
  &read_opt_into_hash("fconf","on","on");
  # &read_opt_into_hash("buffer_type","off","off");
  &read_opt_into_hash("mc","on","off");
  &read_opt_into_hash("mt","on","off");
  &read_opt_into_hash("power_noise","off","off");
  &read_opt_into_hash("server_mode","off","off");
  &read_opt_into_hash("full_check","off","off");
  &read_opt_into_hash("power_analysis","off","off");
  &read_opt_into_hash("dc_analysis","off","off");
  &read_opt_into_hash("meas_skew","off","off");
  &read_opt_into_hash("meas_delay","off","off");
  &read_opt_into_hash("wave_post","off","off");
  &read_opt_into_hash("64bit_mode","off","off");
  &read_opt_into_hash("corr","off","off");

  # Check the -buffer_type
  $argfd = &spot_option($cur_arg,"-buffer_type");
  if (-1 != $argfd)
  {
    if ($ARGV[$argfd+1] =~ m/^-/)
    {
      print "The next argument cannot start with '-'!\n"; 
      print "it implies an option!\n";
    }
    else
    {$opt_ptr->{"buffer_type"} = $ARGV[$argfd+1];}     
  }
  else
  {
    print "The buffer type option is missing, set \"buffer\" as default.\n";
    $opt_ptr->{"buffer_type"} = "buf";     
  }

  &opts_check(); 

  return 1;
}
  
# List the options
sub opts_check()
{
  print "List your options\n"; 
  
  while(my ($key,$value) = each(%opt_h))
  {print "$key : $value\n";}

  return 1;
}
 
# Read each line and ignore the comments which starts with given arg
# return the valid information of line
sub read_line($ $)
{
  my ($line,$com) = @_;
  my @chars;
  if (defined($line))
  {
    @chars = split/$com/,$line;
    if (!($line =~ m/[\w\d]/))
    {$chars[0] = undef;}
    if ($line =~ m/^\s*$com/)
    {$chars[0] = undef;}
  }
  else
  {$chars[0] = undef;}
  if (defined($chars[0]))
  {
    $chars[0] =~ s/^(\s+)//g;
    $chars[0] =~ s/(\s+)$//g;
  }  
  return $chars[0];    
}

sub check_keywords_conf()
{
  for (my $imcg = 0; $imcg<$#mctgy+1; $imcg++)
  {
    for (my $iscg = 0; $iscg<$#{$sctgy[$imcg]}+1; $iscg++)
    {
      if (defined($conf_ptr->{$mctgy[$imcg]}->{$sctgy[$imcg]->[$iscg]}->{alias}))
      {
        if (1 == $debug)
        {
          print "Keyword($mctgy[$imcg],$sctgy[$imcg]->[$iscg]) = ";
          print "$conf_ptr->{$mctgy[$imcg]}->{$sctgy[$imcg]->[$iscg]}->{alias}";
          print "\n";
        }
      }
      else
      {die "Error: Keyword($mctgy[$imcg],$sctgy[$imcg]->[$iscg]) is missing!\n";}
    }
  }
  return 1;
}

# Read the configuration file
sub read_conf()
{
  # Read in these key words
  my ($line,$post_line);
  my @equation;
  my $cur = "unknown";
  open (CONF, "< $opt_ptr->{conf_val}") or die "Fail to open $opt_ptr->{conf}!\n";
  print "Reading $opt_ptr->{conf_val}...\n";
  while(defined($line = <CONF>))
  {
    chomp $line;
    $post_line = &read_line($line,"#"); 
    if (defined($post_line))
    {
       if ($post_line =~ m/\[(\w+)\]/) 
       {$cur = $1;}
       elsif ("unknown" eq $cur)
       {
         die "Error: Unknown tags for this line!\n$post_line\n";
       }
       else
       {
         $post_line =~ s/\s//g;
         @equation = split /=/,$post_line;
         $conf_ptr->{$cur}->{$equation[0]}->{alias} = $equation[1];   
       }
    }
  }
  # Check these key words
  print "Read complete!\n";
  &check_keywords_conf(); 
  print "Checking these keywords...";
  print "Successfully\n";
  close(CONF);
  return 1;
}

sub read_fconf()
{
  # Read in file names
  my ($line,$post_line,$cur);
  $cur = 0;
  open (FCONF,"< $opt_ptr->{fconf_val}") or die "Fail to open $opt_ptr->{fconf}!\n";
  print "Reading $opt_ptr->{fconf_val}...\n";
  while(defined($line = <FCONF>))
  {
    chomp $line;
    $post_line = &read_line($line,"#");
    if (defined($post_line))
    {
      $netfiles[$cur] = $post_line;       
      $cur++;
    } 
  }  
  print "Netlist files(total $cur):\n";
  foreach my $temp(@netfiles)
  {print "$temp\n";}
  close(FCONF);
  return 1;
}
 
sub spot_targ_main_category($)
{
  my ($targ) = @_;
  my ($idx_mct);
  for (my $imct = 0; $imct < $#mctgy+1; $imct++)
  {
    if ($mctgy[$imct] eq $targ)
    {$idx_mct = $imct;}
  }
  return $idx_mct;
}

sub set_cts_setup($ $)
{
  my ($alias,$val) = @_;
  my ($idx_mct,$idx_sct);
  # Spot cts_setup category in the main category
  $idx_mct = &spot_targ_main_category("cts_setup"); 
  # Spot the $alias in conf_ptr, give the value.
  for (my $iscg = 0; $iscg<$#{$sctgy[$idx_mct]}+1; $iscg++)
  {
    if ($alias eq $conf_ptr->{cts_setup}->{$sctgy[$idx_mct]->[$iscg]}->{alias})
    {
      $conf_ptr->{cts_setup}->{$sctgy[$idx_mct]->[$iscg]}->{value} = $val;
      if (1 == $debug)
      {
        print "Set value: cts_setup->$sctgy[$idx_mct]->[$iscg] to: ";
        print "$conf_ptr->{cts_setup}->{$sctgy[$idx_mct]->[$iscg]}->{value}\n";
      }
    }
  }  
  return 1; 
}
  
# Fill the conf_ptr->{cts_setup} items
sub read_cts_setup()
{
  my ($line,$post_line,$line_no);
  my @equation;
  $line_no = 1;
  open (FSET,"< $conf_ptr->{dir_path}->{setup_dir}->{alias}") or die "Fail to open $conf_ptr->{dir_path}->{setup_dir}->{alias}!\n";
  print "Reading $conf_ptr->{dir_path}->{setup_dir}->{alias}...\n";
  while(defined($line = <FSET>))
  {
    chomp $line;
    $post_line = &read_line($line,"//");
    if (defined($post_line))
    {
      @equation = split /\s+/,$post_line; 
      if ($#equation != 1)
      {print "Warning: Ilegal delcaration in (line$line_no). Missing arguments.\n";} 
      else
      {&set_cts_setup($equation[0],$equation[1]);}
    } 
    $line_no++;
  }

  close(FSET);
  return 1;
}

sub cal_tier_num($ $)
{
  my ($z1,$z2) = @_;
  my ($flag1,$flag2) = (-1,-1);  
  foreach my $tmp(@tier_ary)
  {
    if ($z1 == $tmp) 
    {$flag1 = 1;}
  }
  if (-1 == $flag1)
  {$tier_ary[$#tier_ary+1]=$z1;} 
  foreach my $tmp(@tier_ary)
  {
    if ($z2 == $tmp) 
    {$flag2 = 1;}
  }
  if (-1 == $flag2)
  {$tier_ary[$#tier_ary+1]=$z2;} 
  return 1;
}

sub match_keyword_ctsnet($ $ $ $ $)
{
  my ($alias,$vals,$curkw,$curpos,$line_no) = @_;
  # Check whether it is in the cts_net_check
  # Spot cts_setup category in the main category
  my $idx_mct = &spot_targ_main_category("cts_net_check"); 
  # Spot the $alias in conf_ptr, give the value.
  for (my $iscg = 0; $iscg<$#{$sctgy[$idx_mct]}+1; $iscg++)
  {
    if ($alias eq $conf_ptr->{cts_net_check}->{$sctgy[$idx_mct]->[$iscg]}->{alias})
    {
      if (($sctgy[$idx_mct]->[$iscg] eq "x")||($sctgy[$idx_mct]->[$iscg] eq "y")) 
      {
        if ($vals =~ /(\d+)\s+(\d+)/)
        {
          $conf_ptr->{cts_net_check}->{$sctgy[$idx_mct]->[$iscg]}->{up} = $1;
          $conf_ptr->{cts_net_check}->{$sctgy[$idx_mct]->[$iscg]}->{down} = $2;
        }
        else
        {
          $conf_ptr->{cts_net_check}->{$sctgy[$idx_mct]->[$iscg]}->{up} = -1;
          $conf_ptr->{cts_net_check}->{$sctgy[$idx_mct]->[$iscg]}->{down} = -1;
          print "Warning: ($alias) is not defined correctly in line($line_no)!\n";
          print "It is set to -1 as default.\n";
        }
      } 
      else
      {
        if ($vals =~ /(\d+)/)
        {$conf_ptr->{cts_net_check}->{$sctgy[$idx_mct]->[$iscg]}->{num} = $1;}
        else
        {
          $conf_ptr->{cts_net_check}->{$sctgy[$idx_mct]->[$iscg]}->{num} = -1;
          print "Warning: ($alias) is not defined correctly in line($line_no)!\n";
          print "It is set to -1 as default.\n";
        }
      }
    }
  }
  # Check whether it is in the cts_net_main
  $idx_mct = &spot_targ_main_category("cts_net_main"); 
  # Spot the $alias in conf_ptr, give the value.
  for (my $iscg = 0; $iscg<$#{$sctgy[$idx_mct]}+1; $iscg++)
  {
    if ($alias eq $conf_ptr->{cts_net_main}->{$sctgy[$idx_mct]->[$iscg]}->{alias})
    {
      if (($sctgy[$idx_mct]->[$iscg] eq "src")) 
      {
        if ($vals =~ /(\d+)\s+(\d+)\s+(\d+)/)
        {
          $conf_ptr->{cts_net_main}->{$sctgy[$idx_mct]->[$iscg]}->{x} = $1;
          $conf_ptr->{cts_net_main}->{$sctgy[$idx_mct]->[$iscg]}->{y} = $2;
          $conf_ptr->{cts_net_main}->{$sctgy[$idx_mct]->[$iscg]}->{z} = $3;
        }
        else
        {die "Error: ($alias) is not defined properly in line($line_no)!\n";}
      }
      else
      {
        $vals =~ s/\s//g;
        ($$curkw,$$curpos) = ($sctgy[$idx_mct]->[$iscg],$vals);
      } 
    }
  }

  # Check whether it is in the cts_net_sub 
  $idx_mct = &spot_targ_main_category("cts_net_sub"); 
  # Spot the $alias in conf_ptr, give the value.
  for (my $iscg = 0; $iscg<$#{$sctgy[$idx_mct]}+1; $iscg++)
  {
    if ($alias eq $conf_ptr->{cts_net_sub}->{$sctgy[$idx_mct]->[$iscg]}->{alias})
    {
      if ((defined($$curkw))&&(defined($$curpos))) 
      {
        if (defined($conf_ptr->{cts_net_main}->{$$curkw}->{$$curpos}->{$sctgy[$idx_mct]->[$iscg]}))
        {print "Warning: Line($line_no) attempt to redefine the $sctgy[$idx_mct]->[$iscg]!\n ";}

        if (("cload" eq $sctgy[$idx_mct]->[$iscg])&&($vals =~ /([0-9e.-]+)/))
        {
          if (defined($conf_ptr->{cts_net_main}->{$$curkw}->{$$curpos}->{$sctgy[$idx_mct]->[$iscg]}))
          {print "Warning: Line($line_no) attempt to redefine the $sctgy[$idx_mct]->[$iscg]!\n ";}
          $conf_ptr->{cts_net_main}->{$$curkw}->{$$curpos}->{$sctgy[$idx_mct]->[$iscg]} = $1;
        }
        elsif ("coord" eq $sctgy[$idx_mct]->[$iscg])
        {
          if (($$curkw eq "buf")||($$curkw eq "mp")||($$curkw eq "sink"))
          {
            $vals =~ /(\d+)\s+(\d+)\s+(\d+)/;
            $conf_ptr->{cts_net_main}->{$$curkw}->{$$curpos}->{$sctgy[$idx_mct]->[$iscg]}->{x1} = $1;
            $conf_ptr->{cts_net_main}->{$$curkw}->{$$curpos}->{$sctgy[$idx_mct]->[$iscg]}->{y1} = $2;
            $conf_ptr->{cts_net_main}->{$$curkw}->{$$curpos}->{$sctgy[$idx_mct]->[$iscg]}->{z1} = $3;
            $conf_ptr->{cts_net_main}->{$$curkw}->{$$curpos}->{$sctgy[$idx_mct]->[$iscg]}->{x2} = $1;
            $conf_ptr->{cts_net_main}->{$$curkw}->{$$curpos}->{$sctgy[$idx_mct]->[$iscg]}->{y2} = $2;
            $conf_ptr->{cts_net_main}->{$$curkw}->{$$curpos}->{$sctgy[$idx_mct]->[$iscg]}->{z2} = $3;
            &cal_tier_num($conf_ptr->{cts_net_main}->{$$curkw}->{$$curpos}->{$sctgy[$idx_mct]->[$iscg]}->{z1},$conf_ptr->{cts_net_main}->{$$curkw}->{$$curpos}->{$sctgy[$idx_mct]->[$iscg]}->{z2});
          }
          elsif ($$curkw eq "net")
          {
            $vals =~ /(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)/;
            $conf_ptr->{cts_net_main}->{$$curkw}->{$$curpos}->{$sctgy[$idx_mct]->[$iscg]}->{x1} = $1;
            $conf_ptr->{cts_net_main}->{$$curkw}->{$$curpos}->{$sctgy[$idx_mct]->[$iscg]}->{y1} = $2;
            $conf_ptr->{cts_net_main}->{$$curkw}->{$$curpos}->{$sctgy[$idx_mct]->[$iscg]}->{x2} = $3;
            $conf_ptr->{cts_net_main}->{$$curkw}->{$$curpos}->{$sctgy[$idx_mct]->[$iscg]}->{y2} = $4;
            $conf_ptr->{cts_net_main}->{$$curkw}->{$$curpos}->{$sctgy[$idx_mct]->[$iscg]}->{z1} = $5;
            $conf_ptr->{cts_net_main}->{$$curkw}->{$$curpos}->{$sctgy[$idx_mct]->[$iscg]}->{z2} = $5;
          }
          elsif ($$curkw eq "tsv") 
          {
            $vals =~ /(\d+)\s+(\d+)\s+(\d+)\s+(\d+)/;
            $conf_ptr->{cts_net_main}->{$$curkw}->{$$curpos}->{$sctgy[$idx_mct]->[$iscg]}->{x1} = $1;
            $conf_ptr->{cts_net_main}->{$$curkw}->{$$curpos}->{$sctgy[$idx_mct]->[$iscg]}->{y1} = $2;
            $conf_ptr->{cts_net_main}->{$$curkw}->{$$curpos}->{$sctgy[$idx_mct]->[$iscg]}->{x2} = $1;
            $conf_ptr->{cts_net_main}->{$$curkw}->{$$curpos}->{$sctgy[$idx_mct]->[$iscg]}->{y2} = $2;
            $conf_ptr->{cts_net_main}->{$$curkw}->{$$curpos}->{$sctgy[$idx_mct]->[$iscg]}->{z1} = $3;
            $conf_ptr->{cts_net_main}->{$$curkw}->{$$curpos}->{$sctgy[$idx_mct]->[$iscg]}->{z2} = $4;
          }
          else
          {print "Warning: Coordinator does not belong to any known element!(Name:$$curkw)";}
        }
        else
        {print "Warning: Ilegal attributes($alias)!...Line($line_no) will be ignored\n";}
      }
      else
      {die "Error: Unable to allocate the $alias in line($line_no)\n";}
    }
  }
     
  return 1;  
}

sub read_cts_netlist($)
{
  my ($line,$post_line,$line_no);
  my @equation;
  my @attrs;
  $line_no = 1;
  my ($fnet_cur) = @_;
  # Define a state to store which keywords is being recording.
  my ($curkw,$curpos);
  # Read netlist file
  open (FNET,"< $fnet_cur") or die "Fail to open $fnet_cur!\n";
  print "Reading netlist description: $fnet_cur ... \n";
  while(defined($line = <FNET>))
  {  
    chomp $line;
    $post_line = &read_line($line,"#");  
    if (defined($post_line))
    {
      $post_line =~ s/(\s*):(\s*)/:/g;
      @equation = split /:/, $post_line;
      # $equation[0] contains the key words, remove spaces
      $equation[0] =~ s/\s//g;
      &match_keyword_ctsnet($equation[0],$equation[1],\$curkw,\$curpos,$line_no); 
    } 
    $line_no++;
  }
  close(FNET);
  print "Finish reading netlist!\n";
  return 1;
}

sub max_min_determine($ $)
{
  my ($a,$b) = @_;
  if ($a>$b) 
  {return ($b,$a);}
  else
  {return ($a,$b);}
}

sub check_in_range($ $ $)
{
  my ($targ,$up,$down) = @_;
  if (($targ >= $up)&&($targ <= $down))
  {return 1;}
  else
  {return 0;}
}
 
# Fill the noise array
sub check_noise_array($)
{
  my ($tier_no,$nary,$paraname) = @_;
  my ($rtvar);
  my @nary_tmp = @{$nary};
  if ($#nary_tmp < 0)
  {die "Error: Invalid noise parameter!(Name: $paraname, Array: $nary)\n";}
  if ($tier_no > $#nary_tmp+1) 
  {
    print "Warning: Number of noise parameter($paraname) is less than number of tier($tier_no)\n";
    print RPT "Warning: Number of noise parameter($paraname) is less than number of tier($tier_no)\n";
    for (my $iary = $#nary_tmp + 1;$iary<$tier_no;$iary++)
    {$nary->[$iary] = $nary->[$#nary_tmp];}   
    print "Warning: Fill the array with the last element($nary->[$#nary_tmp]) in array!\n";
    print RPT "Warning: Fill the array with the last element($nary->[$#nary_tmp]) in array!\n";
  }
    
  for (my $iary = 0;$iary<$tier_no;$iary++)
  {$rtvar=$rtvar."$nary->[$iary],";}   
  

  return $rtvar;
}

sub max_array($)
{
  my ($ary_in) = @_;
  my ($max_in_ary);
  foreach my $tmp(@{$ary_in})
  {
    if ((!defined($max_in_ary))||($max_in_ary<$tmp))
    {
      $max_in_ary = $tmp;
    }
  }
  if ("on" eq $opt_ptr->{debug})
  {print "DEBUG: Max in the array(@{$ary_in}) is $max_in_ary.\n";} 
  return $max_in_ary; 
}
      
sub check_noise_parameters($)
{
  # number of tier is necessary to check the noise parameters
  # because each tier has different noise 
  my ($tier_no) = @_;
  my ($max_tier) = (&max_array($tier_ary_ptr));
  # Split all the parameters
  my @n_amp = split /,/,$conf_ptr->{ic_parameters}->{noise_amp}->{alias};   
  my @n_f = split /,/,$conf_ptr->{ic_parameters}->{noise_freq}->{alias};   
  my @n_ph = split /,/,$conf_ptr->{ic_parameters}->{noise_phase}->{alias};   
  my @n_dly = split /,/,$conf_ptr->{ic_parameters}->{noise_delay}->{alias};   
  # begin check whether array length match the tier number 
  # Or fill the list with the last parameters 
  $conf_ptr->{ic_parameters}->{noise_amp}->{alias} = &check_noise_array($max_tier,\@n_amp,"noise_amp");
  $conf_ptr->{ic_parameters}->{noise_freq}->{alias} = &check_noise_array($max_tier,\@n_f,"noise_freq");
  $conf_ptr->{ic_parameters}->{noise_phase}->{alias} = &check_noise_array($max_tier,\@n_ph,"noise_phase");
  $conf_ptr->{ic_parameters}->{noise_delay}->{alias} = &check_noise_array($max_tier,\@n_dly,"noise_delay");

  return 1;
}

# This sub checks whether there is potential mistakes in the netlist
# For example
# The number of sinks, buffers, TSVs, and merge points match the descriptions.
# The source node is included in the netlist
# Each node does not exceed x and y ranges.
sub check_cts_netlist()
{
  print "Check your netlist...\n";
  print RPT "********Netlist Summary********\n";
  my ($tier_no) = ($#tier_ary+1);
  print RPT "Number of Tier : $tier_no\n";
  print "Number of Tier : $tier_no\n";
  if ("on" eq $opt_ptr->{"debug"})
  {
    print RPT "DEBUG: Tier Array(@tier_ary)\n";
    print "DEBUG : Tier Array(@tier_ary)\n";
  }
  # define some check standards
  my ($x_up,$x_down) = &max_min_determine($conf_ptr->{cts_net_check}->{x}->{up},$conf_ptr->{cts_net_check}->{x}->{down});
  my ($y_up,$y_down) = &max_min_determine($conf_ptr->{cts_net_check}->{y}->{up},$conf_ptr->{cts_net_check}->{y}->{down});
  my ($src_x,$src_y,$src_z) = ($conf_ptr->{cts_net_main}->{src}->{x},
                               $conf_ptr->{cts_net_main}->{src}->{y}, 
                               $conf_ptr->{cts_net_main}->{src}->{z}, 
                              ); 
  if (!defined($src_x))
  {$src_x = -1;}
  if (!defined($src_y))
  {$src_y = -1;}
  if (!defined($src_z))
  {$src_z = -1;}

  my $src_exist = 0;
  # Check  
  &check_noise_parameters($tier_no);
  my $idx_mct = &spot_targ_main_category("cts_net_main"); 
  # Spot the $alias in conf_ptr, give the value.
  if ((!defined($x_up))&&(!defined($x_down))&&(!defined($y_up))&&(!defined($y_down)))
  {print "Warning: The range of Coordinator(X,Y) is undefined! Skip checking the range.\n"}
  for (my $iscg = 0; $iscg<$#{$sctgy[$idx_mct]}+1; $iscg++)
  {
    my $cur = 0;
    my $num_temp = "num_".$sctgy[$idx_mct]->[$iscg];   
    if ("src" ne $sctgy[$idx_mct]->[$iscg])
    {
      while(my ($key,$value) = each(%{$conf_ptr->{cts_net_main}->{$sctgy[$idx_mct]->[$iscg]}}))
      {
        if ($key ne "alias")
        {
          # Count the number
          $cur++;
          # Check whether in range
          # when range of x is not defined
          # Skip the check of range
          if ((defined($x_up))&&(defined($x_down))&&(defined($y_up))&&(defined($y_down)))
          {
            if (0 == &check_in_range($value->{coord}->{x1},$x_up,$x_down))
            {print "Warning: $sctgy[$idx_mct]->[$iscg](Index:$key) coordinate x1($value->{coord}->{x1}) out of range($x_up,$x_down)!\n";}   
            if (0 == &check_in_range($value->{coord}->{x2},$x_up,$x_down))
            {print "Warning: $sctgy[$idx_mct]->[$iscg](Index:$key) coordinate x2($value->{coord}->{x2}) out of range($x_up,$x_down)!\n";}   
            if (0 == &check_in_range($value->{coord}->{y1},$y_up,$y_down))
            {print "Warning: $sctgy[$idx_mct]->[$iscg](Index:$key) coordinate y1($value->{coord}->{y1}) out of range($y_up,$y_down)!\n";}   
            if (0 == &check_in_range($value->{coord}->{y2},$y_up,$y_down))
            {print "Warning: $sctgy[$idx_mct]->[$iscg](Index:$key) coordinate y2($value->{coord}->{y2}) out of range($y_up,$y_down)!\n";}   
          }
          # Check the source
          if ((($src_x+$src_y+$src_z-$value->{coord}->{x1}-$value->{coord}->{y1}-$value->{coord}->{z1}) == 0)||(($src_x+$src_y+$src_z-$value->{coord}->{x2}-$value->{coord}->{y2}-$value->{coord}->{z2}) == 0))
          {
            if (1 == $src_exist)
            {print "Warning: source node defined in multiple lines($sctgy[$idx_mct]->[$iscg],Index:$key)!\n";}
            else
            {$src_exist = 1;}
          }
          # Check Sink has load capacitace
          if (("sink" eq $sctgy[$idx_mct]->[$iscg])&&(!(defined($value->{cload}))))
          {print "Warning: sink($key) does not havd a load capacitance!\n";}
        }
      }
      # print summary
      print RPT "Number of $num_temp : $cur\n";
      # Check number match
      if ("net" ne $sctgy[$idx_mct]->[$iscg])
      {
        if ($cur == $conf_ptr->{cts_net_check}->{$num_temp}->{num})
        {print "Check O.K. for $num_temp(Expected: $conf_ptr->{cts_net_check}->{$num_temp}->{num},Actual: $cur)...\n";}
        else
        {print "Error: $num_temp(Expected: $conf_ptr->{cts_net_check}->{$num_temp}->{num},Actual: $cur)!\n ";}
      }
      else
      {
        $conf_ptr->{cts_net_check}->{$num_temp}->{num} = $cur;
        print "Count the number of $num_temp(Actual:$conf_ptr->{cts_net_check}->{$num_temp}->{num})...\n";
      }
    }
  }
  # Check source flag
  if (0 == $src_exist)
  {
    print "Warning: source node does not connected in the netlist!\n";
  }
  print RPT "********End of Netlist Summary********\n"; 
  print "Checking complete!\n"; 
  return 1;
}

sub search_net_trig($ $)
{
  my ($targ,$stack) = @_;
  my ($trig,$flag);
  $flag = 0;
  # Searching
  while(my ($key,$value) = each(%{$conf_ptr->{cts_net_main}->{net}}))
  {
    # alias is not a vaild sink
    if ($key ne "alias")
    {
      my @nlist = split /_/,$key;
      if ($targ eq $nlist[1])
      {
        if (1 == $flag)
        {
          print "Warning: Multiple driver(net:$key) for $targ!\n";
          print CHKRPT "Warning: Multiple driver(net:$key) for $targ!\n";
        }
        push @{$stack},"net($key)";
        $trig = $nlist[0];
        $flag = 1;
      } 
    } 
  }
  if (0 == $flag)
  {$trig = "none";}
  return $trig;
}

sub spot_trig($ $)
{
  my ($trig) = @_;
  my ($flag) = (0);
  my ($compo,$compo_find);
  my @trig_lst = split /\s*/,$trig;
  
  $compo_find = 0;
  # determine the component from abbreviation
  while(my ($key,$value) = each(%{$conf_ptr->{cts_abbrev}}))
  {
    #if (1 == $debug)
    #{print "Debug: Trig_lst[0]:$trig_lst[0],value:$value->{alias}\n";}
    if ($trig_lst[0] eq $value->{alias})
    {
      $compo = $key; 
      $compo_find = 1;
    } 
  }
  if ($compo_find != 1)
  {$flag = 0;}
  else
  {
    while(my ($key,$value) = each(%{$conf_ptr->{cts_net_main}->{$compo}}))
    {
      if ("$trig_lst[0]$key" eq $trig)
      {
        if (1 == $flag)
        {
          print CHKRPT "Error: find multiple $trig!\n";
          die "Error: find multiple $trig!\n";
        } 
        $flag = 1;
      }
    } 
  }
  return $flag;
}
  
sub check_in_stack($ $)
{
  my ($stack,$trig) = @_;
  my ($flag) = (0);
  foreach my $tmp(@{$stack})
  {
    if ($trig eq $tmp)
    {$flag = 1;}
  }
  return $flag;
}

sub try_trace_back_source($ $)
{
  my ($src_name,$key) = @_;
  # Spot the net that connect this sink
  my ($targ) = ("$conf_ptr->{cts_abbrev}->{sink}->{alias}$key"); 
  my ($trig);
  my ($flag) = (0);
  my @mystack = ($targ);
  while(1)
  {
    $trig = &search_net_trig($targ,\@mystack);
    if ($trig ne "none")
    {
      if (0 == &spot_trig($trig))
      {
       print CHKRPT "Error: $trig does not exist!(PATH:@mystack)\n"; 
       die "Error: $trig does not exist!(PATH:@mystack)\n";
      } 
      else
      {
        # Check loop(if $trig exists in the stack)  
        if (1 == &check_in_stack(\@mystack,$trig))
        { 
          print CHKRPT "Error: Loop detected!(@mystack,$trig)\n";
          die "Error: Loop detected!(@mystack,$trig)\n";
        }
        push @mystack,$trig;
        if ($trig eq $src_name)
        {
          print CHKRPT "Sink($key) is connect to source($src_name)!\n";
          print CHKRPT "Path : @mystack\n";
          $flag = 1;
          last;
        }
        $targ = $trig;
      }
    }
    else
    {
      print "Warning: Floating node($targ) found!\n";
      print CHKRPT "Warning: Floating node($targ) found!\n"; 
      last;
    }
  }
  return $flag;
}

sub check_float_node()
{
  print CHKRPT "\n*****Float Node Summary*****\n";
  print "Check each nodes and path in the netlist...\n";
  my ($src_name) = ("$conf_ptr->{cts_abbrev}->{buf}->{alias}0"); 
  while(my ($key,$value) = each(%{$conf_ptr->{cts_net_main}->{sink}}))
  {
    # alias is not a vaild sink
    if ($key ne "alias")
    {
      if (0 == &try_trace_back_source($src_name,$key))
      {
        print "Warning: Sink node($key) does not connected to $src_name!\n";
        print CHKRPT "Warning: Sink node($key) does not connected to $src_name!\n";
      }
    }
  } 
  print CHKRPT "*****End of Float Node Summary*****\n";
  print "Check complete!\n";
  return 1;
}
 
sub create_hspice_headers($)
{
  my ($spdir) = @_;
  my ($headerdir) = $spdir."headers/";
  print "Generating header files for HSPICE simulation...\n";
  # Create directory for headers
  if (!(-e "$headerdir"))
  {
    mkpath "$headerdir"; 
    print "Path($headerdir) does not exist...Create it.\n";
  }
  my ($sp_tran_time) = ($spdir."headers/trantime_h.sp");
  my ($sp_lib) = ($spdir."headers/cts_lib.sp");
  # Write timing parameters for transition
  my ($icur,$tcur,$vcur) = (0,0,$conf_ptr->{ic_parameters}->{vl}->{alias});
  my ($clk_edge) = 0.5*$conf_ptr->{ic_parameters}->{edge_ratio}->{alias}/$conf_ptr->{cts_setup}->{clk_freq}->{value}; 
  my ($clk_flat) = 0.5*(1-$conf_ptr->{ic_parameters}->{edge_ratio}->{alias})/$conf_ptr->{cts_setup}->{clk_freq}->{value};
  # Write timing parameters for HSPICE simulations.
  print "Write timing parameters to $sp_tran_time ...";
  open (THSP, "> $sp_tran_time") or die "Fail to create $sp_tran_time!\n";
  # Print tags
  $mydate = gmctime();
  print THSP "*****************************\n";
  print THSP "*      Timing Parameters    *\n";
  print THSP "*   Author : Xifan TANG     *\n";
  print THSP "*  Company : LSI Lab, EPFL  *\n";
  print THSP "*  Date    : $mydate   *\n";
  print THSP "*****************************\n";
  # Print main body
  print THSP "*paramter for tran\n";
  print THSP ".param clk_period = \'1/$conf_ptr->{cts_setup}->{clk_freq}->{value}\'\n";
  print THSP ".param T$icur = $tcur\n";
  print THSP ".param V$icur = $vcur\n";
  $icur++;
  $tcur += $clk_flat;
  $vcur = $conf_ptr->{ic_parameters}->{vl}->{alias}; 
  for (my $iclk = 0; $iclk < $conf_ptr->{ic_parameters}->{num_clk}->{alias}; $iclk++)
  {
    print THSP ".param T$icur = $tcur\n";
    print THSP ".param V$icur = $vcur\n";
    $icur++;
    $tcur += $clk_edge;
    $vcur = $conf_ptr->{ic_parameters}->{vh}->{alias}; 
    print THSP ".param T$icur = $tcur\n";
    print THSP ".param V$icur = $vcur\n";
    $icur++;
    $tcur += $clk_flat;
    $vcur = $conf_ptr->{ic_parameters}->{vh}->{alias}; 
    print THSP ".param T$icur = $tcur\n";
    print THSP ".param V$icur = $vcur\n";
    $icur++;
    $tcur += $clk_edge;
    $vcur = $conf_ptr->{ic_parameters}->{vl}->{alias}; 
    print THSP ".param T$icur = $tcur\n";
    print THSP ".param V$icur = $vcur\n";
    $icur++;
    $tcur += $clk_flat;
    $vcur = $conf_ptr->{ic_parameters}->{vl}->{alias}; 
  }  
  print THSP ".param T$icur = $tcur\n";
  print THSP ".param V$icur = $vcur\n";
  print THSP "*end parameter for tran\n";
  close(THSP);
  print "Done\n";
  # Write timing parameters for transition
  my ($process_f) = $conf_ptr->{dir_path}->{proc_dir}->{alias};
  my ($nmos_name) = $conf_ptr->{ic_parameters}->{nmos_name}->{alias};
  my ($pmos_name) = $conf_ptr->{ic_parameters}->{pmos_name}->{alias};
  my ($buf_size) = $conf_ptr->{ic_parameters}->{buf_size}->{alias};
  my ($rw) = $conf_ptr->{cts_setup}->{r_wire}->{value};  
  my ($cw) = $conf_ptr->{cts_setup}->{c_wire}->{value};  
  my ($rtsv) = $conf_ptr->{cts_setup}->{r_tsv}->{value};  
  my ($ctsv) = $conf_ptr->{cts_setup}->{c_tsv}->{value};  
  # Write library file
  print "Write library file to $sp_lib ...";
  open (LBSP, "> $sp_lib") or die "Fail to create $sp_lib!\n";
  # Print tags
  $mydate = gmctime();
  print LBSP "*****************************\n";
  print LBSP "*    Component Library      *\n";
  print LBSP "*   Author : Xifan TANG     *\n";
  print LBSP "*  Company : LSI Lab, EPFL  *\n";
  print LBSP "*  Date    : $mydate   *\n";
  print LBSP "*****************************\n";
  # Print main body
  print LBSP "*****component library for spice netlist*****\n";
  print LBSP ".include \'$process_f\'\n";
  #print LBSP "*****Global nodes*****\n";
  #print LBSP ".global vdd gnd\n";
  print LBSP "*****parameters for NMOS/PMOS*****\n";
  print LBSP ".param l_nmos=$conf_ptr->{ic_parameters}->{nmos_l}->{alias}\n"; 
  print LBSP ".param l_pmos=$conf_ptr->{ic_parameters}->{pmos_l}->{alias}\n"; 
  print LBSP ".param w_nmos=$conf_ptr->{ic_parameters}->{nmos_w}->{alias}\n"; 
  print LBSP ".param w_pmos=$conf_ptr->{ic_parameters}->{pmos_w}->{alias}\n"; 
  # Print the Monto Carlo Simulation
  if ("on" eq $opt_ptr->{mc})
  {
    my $conv_unit = "1e-4";
    my $sigma = $conf_ptr->{ic_parameters}->{gauss_sigma}->{alias};
    print LBSP "*****parameters for Monto Carlo Simulation*****\n"; 
    print LBSP ".param pwid_lb=$conf_ptr->{cts_setup}->{wid_lb}->{value}\n";
    print LBSP ".param pd2d_lb=$conf_ptr->{cts_setup}->{d2d_lb}->{value}\n";
    print LBSP ".param pwid_vth=\'$conf_ptr->{cts_setup}->{wid_vth}->{value}\'\n";
    print LBSP ".param pd2d_vth=\'$conf_ptr->{cts_setup}->{d2d_vth}->{value}\'\n";
    print LBSP ".param pwid_rw=\'$conf_ptr->{cts_setup}->{wid_rw}->{value}*$conv_unit\'\n";
    print LBSP ".param pd2d_rw=\'$conf_ptr->{cts_setup}->{d2d_rw}->{value}*$conv_unit\'\n";
    print LBSP ".param pwid_cw=\'$conf_ptr->{cts_setup}->{wid_cw}->{value}*$conv_unit\'\n";
    print LBSP ".param pd2d_cw=\'$conf_ptr->{cts_setup}->{d2d_cw}->{value}*$conv_unit\'\n";
    print LBSP ".param pwid_rt=$conf_ptr->{cts_setup}->{wid_rt}->{value}\n";
    print LBSP ".param pd2d_rt=$conf_ptr->{cts_setup}->{d2d_rt}->{value}\n";
    print LBSP ".param pwid_ct=$conf_ptr->{cts_setup}->{wid_ct}->{value}\n";
    print LBSP ".param pd2d_ct=$conf_ptr->{cts_setup}->{d2d_ct}->{value}\n";
    # For Die to Die Variation
    my ($rnet,$cnet);
    $rnet = $conf_ptr->{cts_setup}->{r_wire}->{value};
    $cnet = $conf_ptr->{cts_setup}->{c_wire}->{value};
    foreach my $itier(@tier_ary)
    {
      print LBSP "*****Tier $itier Global Variations*****\n";
      print LBSP ".param dvth_$itier=agauss(0,\'pd2d_vth*$sigma\',$sigma)\n";
      print LBSP ".param pchnl_$itier=agauss(0,\'pd2d_lb*$sigma\',$sigma)\n";
      print LBSP ".param nchnl_$itier=agauss(0,\'pd2d_lb*$sigma\',$sigma)\n";
      print LBSP ".param netr_$itier=agauss(0,\'pd2d_rw*$sigma\',$sigma)\n";
      print LBSP ".param netc_$itier=agauss(0\'pd2d_cw*$sigma\',$sigma)\n";
      print LBSP ".param glb_dvth_$itier=dvth_$itier\n";
      print LBSP ".param glb_pchnl_$itier=pchnl_$itier\n";
      print LBSP ".param glb_nchnl_$itier=nchnl_$itier\n";
      print LBSP ".param glb_netr_$itier=netr_$itier\n";
      print LBSP ".param glb_netc_$itier=netc_$itier\n";
    }
    my ($rtsv,$ctsv);
    $rtsv = $conf_ptr->{cts_setup}->{r_tsv}->{value};
    $ctsv = $conf_ptr->{cts_setup}->{c_tsv}->{value};
    print LBSP "*****Global Variation of TSVs*****\n";
    print LBSP ".param tsvr=agauss(0,\'pd2d_rt*$sigma\',$sigma)\n";
    print LBSP ".param tsvc=agauss(0,\'pd2d_ct*$sigma\',$sigma)\n";
    print LBSP ".param glb_tsvr=tsvr\n";
    print LBSP ".param glb_tsvc=tsvc\n";
    print LBSP "*****Local Variations*****\n"; 
    print LBSP ".param loc_dvth=agauss(0,\'pwid_vth*$sigma\',$sigma)\n";
    print LBSP ".param loc_pchnl=agauss(0,\'pwid_lb*$sigma\',$sigma)\n";
    print LBSP ".param loc_nchnl=agauss(0,\'pwid_lb*$sigma\',$sigma)\n";
    print LBSP ".param loc_netr=agauss(0,\'pwid_rw*$sigma\',$sigma)\n";
    print LBSP ".param loc_netc=agauss(0\'pwid_cw*$sigma\',$sigma)\n";
    print LBSP ".param loc_tsvr=agauss(0,\'pwid_rt*$sigma\',$sigma)\n";
    print LBSP ".param loc_tsvc=agauss(0,\'pwid_ct*$sigma\',$sigma)\n";
    #print LBSP "*****End of Monto Carlo parameters*****\n";
  }
  # Print Sub circuits : Buffers, Inverters, Sinks, TSVs, and Nets
  print LBSP "*****Sub circuits*****\n";
  print LBSP "*MOSFET:\n";
  print LBSP "*M<NAME> DRAIN GATE SOURCE BULK <DEVICE> L=<VALUE> W=<VALUE>\n";
  print LBSP ".subckt inv_nsize in out svdd sgnd size=1 nl=l_nmos pl=l_pmos dvth=0\n";
  print LBSP "vshift in in_inv dvth\n";
  print LBSP "Mnmos out in_inv sgnd sgnd $nmos_name L=nl W=\'size*w_nmos\'\n";
  print LBSP "Mpmos out in_inv svdd svdd $pmos_name L=pl W=\'size*w_pmos\'\n";
  print LBSP ".eom inv_nsize\n";
  print LBSP ".subckt buf_nsize in out svdd sgnd size=4 nl=l_nmos pl=l_pmos dvth=0\n";
  print LBSP "Xinv1 in mid svdd sgnd inv_nsize nl=nl pl=pl dvth=dvth\n";
  print LBSP "Xinv2 mid out svdd sgnd inv_nsize size=size nl=nl pl=pl dvth=dvth\n";
  print LBSP ".eom buf_nsize\n";
  # Define the resistance and capacitance per stage in the tsv model
  $rtsv = $conf_ptr->{cts_setup}->{r_tsv}->{value};
  $ctsv = $conf_ptr->{cts_setup}->{c_tsv}->{value};
  # Print the TSV RC model
  print LBSP "*****TSV*****\n";
  print LBSP ".subckt rc_tsv n0 n$conf_ptr->{ic_parameters}->{tsv_model}->{alias} ntier = 1 rtsv=$rtsv ctsv=$ctsv\n";
  for (my $i = 0; $i<$conf_ptr->{ic_parameters}->{tsv_model}->{alias};$i++)
  {
    my $nexti = $i + 1; 
    print LBSP "Ctsvc$i n$i gnd \'ntier*ctsv/$conf_ptr->{ic_parameters}->{tsv_model}->{alias}\'\n";
    print LBSP "Rtsvr$i n$i n$nexti \'ntier*rtsv/$conf_ptr->{ic_parameters}->{tsv_model}->{alias}\'\n";
    if ($nexti == $conf_ptr->{ic_parameters}->{tsv_model}->{alias})
    {print LBSP "Ctsvc$nexti n$nexti gnd \'ntier*ctsv/$conf_ptr->{ic_parameters}->{tsv_model}->{alias}\'\n";}
  }
  print LBSP ".eom rc_tsv\n";
  # Define the resistance and capacitance per stage in the net model
 my $rnet = $conf_ptr->{cts_setup}->{r_wire}->{value};
 my $cnet = $conf_ptr->{cts_setup}->{c_wire}->{value};
  # Print the TSV RC model
  my $conv_unit = "1e-4";
  print LBSP "*****net*****\n";
  print LBSP ".subckt rc_net n0 n$conf_ptr->{ic_parameters}->{net_model}->{alias} netlen = 0 rnet=$rnet cnet=$cnet\n";
  for (my $i = 0; $i<$conf_ptr->{ic_parameters}->{net_model}->{alias};$i++)
  {
    my $nexti = $i + 1; 
    print LBSP "Cnetc$i n$i gnd \'netlen*cnet/$conf_ptr->{ic_parameters}->{net_model}->{alias}\'\n";
    print LBSP "Rnetr$i n$i n$nexti \'netlen*rnet/$conf_ptr->{ic_parameters}->{net_model}->{alias}\'\n";
    if ($nexti == $conf_ptr->{ic_parameters}->{net_model}->{alias})
    {print LBSP "Cnetc$nexti n$nexti gnd \'netlen*cnet/$conf_ptr->{ic_parameters}->{net_model}->{alias}\'\n";}
  }
  print LBSP ".eom rc_net\n";
  # Print the sink model
  print LBSP "*****sink*****\n";
  print LBSP ".subckt rc_sink in csink=0\n";
  print LBSP "Csinkc in gnd \'csink\'\n";
  print LBSP ".eom rc_sink\n";
  print LBSP "*****end component library for spice netlist*****\n";
  close(LBSP);
  print "Done\n";
  return 1; 
}

sub cal_manhattan_dist($ $ $ $)
{
  my ($x1,$y1,$x2,$y2) = @_;
  my ($mdist);
  $mdist = abs($x1-$x2) + abs($y1-$y2);
  return $mdist;
}
    
sub create_hspice_netlist($ $)
{
  my ($mypath,$fname) = @_;
  $fname =~ s/\.(\w+)$//;
  $mypath = $mypath.$fname."/";
  if (!(-e "$mypath"))
  {
    print "$mypath does not exist. Create it...\n";
    mkpath "$mypath";
  }
  my ($fspice) = $mypath.$fname.".sp";
  # Begin Write Hspice file
  print "Write Hspice Netlist: $fspice...\n";
  open (FSP, "> $fspice") or die "Fail to create $fspice!\n";
  # Print tags
  $mydate = gmctime();
  print FSP "CTS circuit $fname HSPICE netlist\n";
  print FSP "*****************************\n";
  print FSP "*       HSPICE Netlist      *\n";
  print FSP "*   Author : Xifan TANG     *\n";
  print FSP "*  Company : LSI Lab, EPFL  *\n";
  print FSP "*  Date    : $mydate   *\n";
  print FSP "*****************************\n";
  # Print main body
  print FSP "*INCLUDE Header files\n";
  print FSP ".include \'../headers/trantime_h.sp\'\n";
  print FSP ".include \'../headers/cts_lib.sp\'\n";
  print FSP "*\n";
  # print FSP ".global vdd gnd\n";
  # print FSP "*\n";
  print FSP ".temp $conf_ptr->{ic_parameters}->{temperature}->{alias}\n";

  if ("on" eq $opt_ptr->{"debug"})
  {
    print FSP ".option captab\n";
  }
  if ("on" eq $opt_ptr->{"mc"})
  {
    print FSP "*Add Options to solve convergence\n*problem in Monte Carlo\n";
    #print FSP ".option Rmin=1e-15\n";
    #print FSP ".option trcon=0\n";
    print FSP ".option MONTECON\n";
    #print FSP "*During each round of Monte Carlo,\n*each model takes different values\n";
    #print FSP ".option MODMONTE=1\n";
  }
  if ("on" eq $opt_ptr->{"dc_analysis"})
  {print FSP ".op\n";}
  if ("on" eq $opt_ptr->{"wave_post"})
  {print FSP ".option POST\n";}
  
  # Scanning Data to be put down(Buffer,Net,Sink,TSV)
  # Spot the $alias in conf_ptr, give the value.
  
  # 1st : put down the nets
  my ($curnet) = (0);
  print FSP "*Nets definition\n";
  while(my ($key,$value) = each(%{$conf_ptr->{cts_net_main}->{net}}))
  {
    # keyword 'num' is the reserved one for number of nets in the list
    if (($key ne "num")&&($key ne "alias"))
    {
      my @nodes = split/_/,$key;
      if ($#nodes+1 != 2)
      {die "Error: net($key) is expected to contain two components!\n";}     
      my ($len) = (&cal_manhattan_dist($value->{coord}->{x1},$value->{coord}->{y1},$value->{coord}->{x2},$value->{coord}->{y2}));
      if (0 == $len)
      {print RPT "Warning: The manhattan distance of net($key) is set to 0!\n";} 

      my ($tier) = ($value->{coord}->{z1});      
      my $sigma = $conf_ptr->{ic_parameters}->{gauss_sigma}->{alias};

      #if (("on" eq $opt_ptr->{corr})&&(defined($corr_mats_ptr->{net}->{wid_rw}->{$tier}))&&(defined($corr_mats_ptr->{net}->{wid_cw}->{$tier}))&&(0 != $len))
      if ("on" eq $opt_ptr->{mc})
      {
        print FSP ".param loc_netr_net$key=loc_netr\n";
        print FSP ".param loc_netc_net$key=loc_netc\n";
      }
        
      print FSP "Xnet$key $nodes[0]\_out $nodes[1]\_in rc_net netlen=$len\n";

      if (("on" eq $opt_ptr->{mc})&&(0 != $len))
      {
        # Define the resistance and capacitance per stage in the net model
        my $rnet = $conf_ptr->{cts_setup}->{r_wire}->{value};
        my $cnet = $conf_ptr->{cts_setup}->{c_wire}->{value};
  
        my $loc_netr = "loc_netr_net$key";
        my $loc_netc = "loc_netc_net$key";
        
        if (("on" eq $opt_ptr->{corr})&&(defined($corr_mats_ptr->{net}->{wid_rw}->{$tier}))&&(defined($corr_mats_ptr->{net}->{wid_cw}->{$tier})))
        {
          $loc_netr = &gen_corr_expr("net",$key,"wid_rw",$tier,"netr");
          $loc_netc = &gen_corr_expr("net",$key,"wid_cw",$tier,"netc");
        }
        print FSP "+ rnet=\'$rnet+glb_netr_$tier+$loc_netr\'\n";
        print FSP "+ cnet=\'$cnet+glb_netc_$tier+$loc_netc\'\n";
      } 
      $curnet++; 
    }
  } 
  if ($curnet != $conf_ptr->{cts_net_check}->{num_net}->{num})
  {die "Error: Expected $conf_ptr->{cts_net_check}->{num_net}->{num} nets in HSPICE file,Actual $curnet! \n";}
  else
  {print "Writing Nets(Total:$curnet)...Done\n";}

  # 2nd : put down the buffer
  my ($curbuf) = (0);
  print FSP "*Buffers definition\n";
  while(my ($key,$value) = each(%{$conf_ptr->{cts_net_main}->{buf}}))
  {
    # Get the abbrevation of buffer
    my ($abb) = ($conf_ptr->{cts_abbrev}->{buf}->{alias});
    # Get buffer size
    my ($buf_size) = ($conf_ptr->{ic_parameters}->{buf_size}->{alias});
    my ($buf_type) = ($opt_ptr->{"buffer_type"});
    # keyword 'num' is the reserved one for number of nets in the list
    if (($key ne "num")&&($key ne "alias"))
    {
      my ($tier) = ($value->{coord}->{z1});      
      my $sigma = $conf_ptr->{ic_parameters}->{gauss_sigma}->{alias};
      
      #if (("on" eq $opt_ptr->{corr})&&(defined($corr_mats_ptr->{buf}->{wid_lb}->{$tier}))&&(defined($corr_mats_ptr->{buf}->{wid_vth}->{$tier})))
      if ("on" eq $opt_ptr->{mc})
      {
        # Define the parameters of local variation for correlation purpose
        print FSP ".param loc_nchnl_buf$key=loc_nchnl\n";
        print FSP ".param loc_pchnl_buf$key=loc_pchnl\n";
        print FSP ".param loc_dvth_buf$key=loc_dvth\n";
      }

      print FSP "Xbuf$key  $abb$key\_in $abb$key\_out vdd$tier gnd $buf_type\_nsize size=$buf_size\n";
      if ("on" eq $opt_ptr->{mc})
      {
        my $loc_nchnl = "loc_nchnl_buf$key";
        my $loc_pchnl = "loc_pchnl_buf$key";
        my $loc_vth_shift = "loc_dvth_buf$key";
        if (("on" eq $opt_ptr->{corr})&&(defined($corr_mats_ptr->{buf}->{wid_lb}->{$tier}))&&(defined($corr_mats_ptr->{buf}->{wid_vth}->{$tier})))
        {
          $loc_nchnl = &gen_corr_expr("buf",$key,"wid_lb",$tier,"nchnl");
          $loc_pchnl = &gen_corr_expr("buf",$key,"wid_lb",$tier,"pchnl");
          $loc_vth_shift = &gen_corr_expr("buf",$key,"wid_vth",$tier,"dvth");
        }
        print FSP "+ nl=\'l_nmos+glb_nchnl_$tier+$loc_nchnl\'\n";
        print FSP "+ pl=\'l_pmos+glb_pchnl_$tier+$loc_pchnl\'\n";
        print FSP "+ dvth=\'glb_dvth_$tier+$loc_vth_shift\'\n";
      } 
      $curbuf++;
    } 
  }
  if ($curbuf != $conf_ptr->{cts_net_check}->{num_buf}->{num})
  {die "Error: Expected $conf_ptr->{cts_net_check}->{num_buf}->{num} buffers in HSPICE file,Actual $curbuf! \n";}
  else
  {print "Writing Buffers(Total:$curbuf)...Done\n";}

  # 3rd : put down the sink  
  my ($cursink) = (0);
  print FSP "*Sinks definition\n";
  while(my ($key,$value) = each(%{$conf_ptr->{cts_net_main}->{sink}}))
  {
    # Get the abbrevation of sink
    my ($abb) = ($conf_ptr->{cts_abbrev}->{sink}->{alias});
    # keyword 'num' is the reserved one for number of nets in the list
    if (($key ne "num")&&($key ne "alias"))
    {
      # Get the capacitance of sink
      my ($csink) = ($value->{cload});
      print FSP "Xsink$key  $abb$key\_in rc_sink csink=$csink\n";
      $cursink++;
    } 
  }
  if ($cursink != $conf_ptr->{cts_net_check}->{num_sink}->{num})
  {die "Error: Expected $conf_ptr->{cts_net_check}->{num_sink}->{num} sinks in HSPICE file,Actual $cursink! \n";}
  else
  {print "Writing Sinks(Total:$cursink)...Done\n";}
  
  # 4th : put down the tsv
  my ($curtsv) = (0);
  print FSP "*TSVs definition\n";
  while(my ($key,$value) = each(%{$conf_ptr->{cts_net_main}->{tsv}}))
  {
    # Get the abbrevation of tsv
    my ($abb) = ($conf_ptr->{cts_abbrev}->{tsv}->{alias});
    # keyword 'num' is the reserved one for number of nets in the list
    if (($key ne "num")&&($key ne "alias"))
    {
      # Get the tier number
      my $ntier = abs($value->{coord}->{z1}-$value->{coord}->{z2});
      print FSP "Xtsv$key $abb$key\_in $abb$key\_out rc_tsv ntier=$ntier\n";
      if ("on" eq $opt_ptr->{mc})
      {
        my $sigma = $conf_ptr->{ic_parameters}->{gauss_sigma}->{alias};
        my $rtsv = $conf_ptr->{cts_setup}->{r_tsv}->{value};
        my $ctsv = $conf_ptr->{cts_setup}->{c_tsv}->{value};
        print FSP "+ rtsv=\'$rtsv+glb_tsvr+loc_tsvr\'\n";
        print FSP "+ ctsv=\'$ctsv+glb_tsvc+loc_tsvc\'\n";
      } 
      $curtsv++;
    } 
  }
  if ($curtsv != $conf_ptr->{cts_net_check}->{num_tsv}->{num})
  {die "Error: Expected $conf_ptr->{cts_net_check}->{num_tsv}->{num} TSVs in HSPICE file,Actual $curtsv! \n";}
  else
  {print "Writing TSVs(Total:$curtsv)...Done\n";}
  # 5th : put down the merge point to resolve the unconnections
  my ($curmp) = (0);
  print FSP "*Merge Points definition\n";
  while(my ($key,$value) = each(%{$conf_ptr->{cts_net_main}->{mp}}))
  {
    # Get the abbrevation of mp
    my ($abb) = ($conf_ptr->{cts_abbrev}->{mp}->{alias});
    # keyword 'num' is the reserved one for number of nets in the list
    if (($key ne "num")&&($key ne "alias"))
    {
      print FSP "Vmp$key $abb$key\_in $abb$key\_out 0\n";
      $curmp++;
    } 
  }
  if ($curmp != $conf_ptr->{cts_net_check}->{num_mp}->{num})
  {die "Error: Expected $conf_ptr->{cts_net_check}->{num_mp}->{num} Merge Points in HSPICE file,Actual $curmp! \n";}
  else
  {print "Writing Merge Points(Total:$curmp)...Done\n";}
  
  # Add the Stimulations
  print "Writing Stimulations...";
  # 1st Add power supply
  print FSP "*Power supply definition\n";
  if ("on" eq $opt_ptr->{"power_noise"})
  {
    # Split all the parameters
    my @n_amp = split /,/,$conf_ptr->{ic_parameters}->{noise_amp}->{alias};   
    my @n_f = split /,/,$conf_ptr->{ic_parameters}->{noise_freq}->{alias};   
    my @n_ph = split /,/,$conf_ptr->{ic_parameters}->{noise_phase}->{alias};   
    my @n_dly = split /,/,$conf_ptr->{ic_parameters}->{noise_delay}->{alias};   
    
    for (my $itier = 0; $itier< ($#tier_ary+1); $itier++)
    {  
      my ($curtier) = ($tier_ary[$itier]);
      print FSP "vsupply$curtier vdd$curtier gnd ";
      print FSP "sin($conf_ptr->{ic_parameters}->{vdd}->{alias} ";
      print FSP "$n_amp[$curtier-1] ";
      print FSP "$n_f[$curtier-1] ";
      print FSP "$n_dly[$curtier-1] ";
      print FSP "0 ";
      print FSP "$n_ph[$curtier-1] ";
      print FSP ")\n";
    }
  }
  else
  {
    for (my $itier = 0; $itier< ($#tier_ary+1); $itier++)
    {  
      my ($curtier) = ($tier_ary[$itier]);
      print FSP "vsupply$curtier vdd$curtier gnd $conf_ptr->{ic_parameters}->{vdd}->{alias}V\n";
      print FSP ".nodeset v(vdd$curtier) $conf_ptr->{ic_parameters}->{vdd}->{alias}V\n";
    }
  }

  #{print FSP "vsupply vdd gnd pwl T0 0 'T1/2' $conf_ptr->{ic_parameters}->{vdd}->{alias}V\n";}
  # 2nd Add input clock source  
  print FSP "*Clock Source definition\n";
  print FSP "vclk_in $conf_ptr->{cts_abbrev}->{buf}->{alias}0_in gnd pwl \n";
  print FSP "+ ";
  my $num_vpoint = $conf_ptr->{ic_parameters}->{num_clk}->{alias}*4+1; 
  for (my $i = 0;$i<$num_vpoint+1;$i++) 
  {print FSP "T$i V$i ";}
  print FSP "\n";
  print "Done\n";

  # Add the transition definition
  print "Writing transition definition...";
  print FSP "*Transition definition\n";
  print FSP ".tran \'T$num_vpoint/($conf_ptr->{ic_parameters}->{num_clk}->{alias}*$conf_ptr->{ic_parameters}->{resolution}->{alias})\' T$num_vpoint "; 
  if ("on" eq $opt_ptr->{mc})
  {
    print FSP "  sweep monte=$opt_ptr->{mc_val}";
  }
  print FSP "\n";
  print "Done\n";

  # Add measurements
  my ($bufabb) = ($conf_ptr->{cts_abbrev}->{buf}->{alias});
  print "Writing measurements...\n";
  print FSP "*Measurements\n";
  if  ("on" eq $opt_ptr->{debug})
  {
    print FSP "*Clock source clk_in measurements\n";
    # Put down the source clock_in measurements 
    print FSP ".measure tran max_clk_in max v($bufabb"."0_in) from=0 to=T$num_vpoint\n"; 
    print FSP ".measure tran min_clk_in min v($bufabb"."0_in) from=0 to=T$num_vpoint\n"; 
    # Put the measurement for Monte Carlo Parameters for DEBUG use
    if ("on" eq $opt_ptr->{mc})
    {
      foreach my $itier(@tier_ary)
      {
        print FSP ".measure delvth_$itier PARAM=vth_shift_$itier\n";
        print FSP ".measure nchannel_$itier PARAM=nchnl_$itier\n";
        print FSP ".measure pchannel_$itier PARAM=pchnl_$itier\n";
        print FSP ".measure res_net_$itier PARAM=netr_$itier\n";
        print FSP ".measure cap_net_$itier PARAM=netc_$itier\n";
      }
      print FSP ".measure res_tsv PARAM=tsvr\n";
      print FSP ".measure cao_tsv PARAM=tsvc\n";
    }  
  }
  if ("on" eq $opt_ptr->{power_analysis})
  {
    print FSP "*Power measurements on VDD\n";
    for (my $itier = 0; $itier< ($#tier_ary+1); $itier++)
    {
      my ($curtier) = ($tier_ary[$itier]);
      print FSP ".measure tran power_avg_vdd$curtier avg p(vsupply$curtier) from=0 to=T$num_vpoint\n";
    }
  }
  if ("on" eq $opt_ptr->{"meas_skew"})
  {
    # Put down the clock skew measurements
    # Clock skew starts from clock_in to each sinks 
    my ($curmsink) = (0); 
    print "Writing measurements for skews between sinks...";
    print FSP "*Measure skew of sinks\n";
    my @sink_lst;
    my ($skew_sinks_cnt) = (0);
    my ($curlst) = (0);
    while(my ($key,$val) = each(%{$conf_ptr->{cts_net_main}->{sink}}))
    {
      if (($key ne "alias")&&($key ne "num"))
      {
        $sink_lst[$curlst] = $key;
        $curlst++;
      }
    }
    for (my $itrig = 0; $itrig<$curlst; $itrig++)
    {
      # Get the abbrevation of sink
      my ($abb) = ($conf_ptr->{cts_abbrev}->{sink}->{alias});
      my ($skew_from_ratio,$skew_to_ratio);
      $skew_from_ratio = $conf_ptr->{ic_parameters}->{skew_from}->{alias};
      $skew_to_ratio = $conf_ptr->{ic_parameters}->{skew_to}->{alias};
      #if (1 == $debug)
      #{print "$abb$itrig is set skew_done!\n";}
      for (my $itarg = $itrig+1; $itarg<$curlst; $itarg++)
      {
        my $vdd =$conf_ptr->{ic_parameters}->{vdd}->{alias};
        my $gnd =$conf_ptr->{ic_parameters}->{gnd}->{alias};
        #if (1 == $debug)
        #{print "Sink Skew-> TRIG:$abb$itrig TARG:$abb$itarg\n";}
        print FSP ".measure tran hold_$abb$itrig\_to_$abb$itarg trig v($abb$itrig\_in) val=\'$skew_from_ratio*$vdd+(1-$skew_from_ratio)*$gnd\' rise=1\n";
          print FSP "+                           targ v($abb$itarg\_in) val=\'$skew_to_ratio*$vdd+(1-$skew_to_ratio)*$gnd\' rise=1\n"; 
          print FSP ".measure tran skew_$abb$itrig\_to_$abb$itarg trig v($abb$itrig\_in) val=\'$skew_from_ratio*$vdd+(1-$skew_from_ratio)*$gnd\' rise=1\n";
          print FSP "+                           targ v($abb$itarg\_in) val=\'$skew_to_ratio*$vdd+(1-$skew_to_ratio)*$gnd\' rise=2\n"; 
          print FSP ".measure setup_$abb$itrig\_to_$abb$itarg PARAM=\'clk_period-skew_$abb$itrig\_to_$abb$itarg\'\n";
          $skew_sinks_cnt++;
      }
    } 
    print "(Total:$skew_sinks_cnt)...Done\n";
  }
  if  ("on" eq $opt_ptr->{meas_delay})
  {
    print "Writing measurements for delay and slew of Sinks...";
    print FSP "*Measure delay and slew of Sinks\n";
    while(my ($key,$value) = each(%{$conf_ptr->{cts_net_main}->{sink}}))
    {
      if (($key ne "alias")&&($key ne "num"))
      {
        # Get the abbrevation of sink
        my ($abb) = ($conf_ptr->{cts_abbrev}->{sink}->{alias});
        my ($skew_from_ratio,$skew_to_ratio);
        my ($slew_from_ratio,$slew_to_ratio);
        my $vdd =$conf_ptr->{ic_parameters}->{vdd}->{alias};
        my $gnd =$conf_ptr->{ic_parameters}->{gnd}->{alias};
        $skew_from_ratio = $conf_ptr->{ic_parameters}->{skew_from}->{alias};
        $skew_to_ratio = $conf_ptr->{ic_parameters}->{skew_to}->{alias};
        $slew_from_ratio = $conf_ptr->{ic_parameters}->{slew_from}->{alias};
        $slew_to_ratio = $conf_ptr->{ic_parameters}->{slew_to}->{alias};

        print FSP ".measure tran delay_$abb$key trig v($bufabb"."0_in) val=\'$skew_from_ratio*$vdd+(1-$skew_from_ratio)*$gnd\' rise=1\n";
        print FSP "+                           targ v($abb$key\_in) val=\'$skew_to_ratio*$vdd+(1-$skew_to_ratio)*$gnd\' rise=1\n"; 
        if  ("on" eq $opt_ptr->{debug})
        {
          print FSP ".measure tran slew_$abb$key trig v($abb$key\_in) val=\'$slew_from_ratio*$vdd+(1-$slew_from_ratio)*$gnd\' rise=1\n";
          print FSP "+                           targ v($abb$key\_in) val=\'$slew_to_ratio*$vdd+(1-$slew_to_ratio)*$gnd\' rise=1\n"; 
        }
      }
    }
    print "Done\n";
  }
  if  ("on" eq $opt_ptr->{debug})
  {
    print "Writing measurements for slews of nets...";
    my %meas_h;
    # measure the slew of net input and output
    while(my ($key,$value) = each(%{$conf_ptr->{cts_net_main}->{net}}))
    {
      if (($key ne "alias")&&($key ne "num"))
      {
        # Get the abbrevation of net
        my ($net_in,$net_out) = split /_/,$key;
        my ($skew_from_ratio,$skew_to_ratio);
        my ($slew_from_ratio,$slew_to_ratio);
        my $vdd =$conf_ptr->{ic_parameters}->{vdd}->{alias};
        my $gnd =$conf_ptr->{ic_parameters}->{gnd}->{alias};
        $net_in = $net_in."_out";
        $net_out = $net_out."_in";
        $skew_from_ratio = $conf_ptr->{ic_parameters}->{skew_from}->{alias};
        $skew_to_ratio = $conf_ptr->{ic_parameters}->{skew_to}->{alias};
        $slew_from_ratio = $conf_ptr->{ic_parameters}->{slew_from}->{alias};
        $slew_to_ratio = $conf_ptr->{ic_parameters}->{slew_to}->{alias};
        # Mark in the hash meas_h    
        # If not exist in the hash, then define this variant.
        if (!defined($meas_h{$net_in}))
        {
          print FSP ".measure tran slew_$net_in trig v($net_in) val=\'$slew_from_ratio*$vdd+(1-$slew_from_ratio)*$gnd\' rise=1\n";
          print FSP "+                               targ v($net_in) val=\'$slew_to_ratio*$vdd+(1-$slew_to_ratio)*$gnd\' rise=1\n"; 
          $meas_h{$net_in} = 1;
        }
        if (!defined($meas_h{$net_out}))
        {
          print FSP ".measure tran slew_$net_out trig v($net_out) val=\'$slew_from_ratio*$vdd+(1-$slew_from_ratio)*$gnd' rise=1\n";
          print FSP "+                               targ v($net_out) val=\'$slew_to_ratio*$vdd+(1-$slew_to_ratio)*$gnd\' rise=1\n"; 
          $meas_h{$net_out} = 1;
        }  
      }
    }
    undef %meas_h;
    print "Done\n";
  }
  # Add ends
  print FSP ".end CTS circuit $fname HSPICE netlist\n";

  close(FSP);
  return 1;
}


sub run_hspice($ $)
{
  my ($mypath,$fname) = @_;
  my ($err_no,$warn_no) = (0,0);
  $fname =~ s/\.(\w+)$//;
  $mypath = $mypath.$fname."/";
  if (!(-e "$mypath"))
  {
    print "$mypath does not exist. Create it...\n";
    mkpath "$mypath";
  }
  my ($fspice) = $mypath.$fname.".sp";
  my ($flis) = $mypath.$fname.".lis";
  my ($hspice_path) = ("hspice");
  my ($mt_def);
  if ($opt_ptr->{"64bit_mode"} eq "on")
  {$hspice_path .= "64";}
  if ("on" eq $opt_ptr->{"mt"})
  {$mt_def = "-mt $opt_ptr->{mt_val} ";}
  my ($server_mode);
  if ("on" eq $opt_ptr->{"server_mode"})
  {$server_mode = "-C";}
  # Use Cshell to run hspice
  `csh -cx '$hspice_path $server_mode -i $fspice -o $flis $mt_def'`;
  #Check errors in the simulation results
  open (FLIS, "< $flis") or die "Fail to open $flis!\n";
  my ($line);
  print RPT "*****HSPICE simulation notes*****\n";
  while(defined($line = <FLIS>))
  {
    chomp $line;
    if ($line =~ /error/i)
    { 
      if (1 == $debug)
      {
        print "Error detected in the lis file!\n";
        print "Info: $line\n";
      }
      print RPT "Error detected in the lis file!\n";
      print RPT "Info: $line\n";
      $err_no++;
    }
    elsif ($line =~ /warn/i)
    {
      if (1 == $debug)
      {print "Warning in HSPICE:\n$line\n";}
      print RPT "Warning in HSPICE:\n$line\n";
      $warn_no++;
    }
  }
  print RPT "HSPICE Simulation completed!(Error:$err_no, Warning:$warn_no)\n";
  print "HSPICE Simulation completed!(Error:$err_no, Warning:$warn_no)\n";
  if ($err_no > 0)
  {exit(1);}
  print "$fname HSPICE has been run successfully!\n";   
  print RPT "$fname HSPICE has been run successfully!\n";   
  print RPT "*****End of HSPICE simulation notes*****\n";

  close(FLIS);
  return 1;
}
   
sub stats_meas($)
{
  my ($item) = @_;
  my ($total,$cnt);
  my ($max,$max_name,$min,$min_name);
  $cnt = 0;
  $total = 0;
  while(my ($key,$value) = each(%{$meas_rst_ptr->{"$item"}}))
  {
    $total = $total + $value;
    $cnt++;
    if ((!defined($max))||($max<$value)) 
    {
      $max_name = $key;
      $max = $value;
    }
    if ((!defined($min))||($min>$value)) 
    {
      $min_name = $key;
      $min = $value;
    }
  }
  my ($avg);
  if ($cnt != 0)
  {$avg = ($total/$cnt);}
  
  print RPT "*****Statistics for $item*****\n";
  print RPT "Total: $total\n";
  print RPT "Counter: $cnt\n";
  print RPT "Max of $item: $max_name, Value : $max\n";
  print RPT "Min of $item: $min_name, Value : $min\n";
  print RPT "Average : $avg\n";
  print RPT "*****End Statistics for $item*****\n";

  return 1;
}  

sub organize_sim_result($ $)
{
  my ($mypath,$fname) = @_;
  $fname =~ s/\.(\w+)$//;
  $mypath = $mypath.$fname."/";
  my ($flis) = $mypath.$fname.".lis";
  my ($fmt0) = $mypath.$fname.".mt0";
  if (($opt_ptr->{power_analysis} eq "on")||($opt_ptr->{meas_skew} eq "on")||($opt_ptr->{meas_delay} eq "on"))
  {
    open (FMT, "< $fmt0") or die "Fail to open $fmt0!\n";
    my ($line);
    my ($org_lines);
    my @meas_name;
    my @meas_val;
    print RPT "*****HSPICE Simulation Results*****\n";
    while(defined($line = <FMT>))
    {
      chomp $line;
      if ($line =~ /^\$/i) 
      {
        #print RPT "$`\n";
      }
      elsif ($line =~ /^\.TITLE/i) 
      {print RPT "$line\n";}
      else
      {
        $line =~ s/^(\s+)//;
        $line =~ s/(\s+)$//;
        if (defined($org_lines))
        {$org_lines = $org_lines." ".$line;}
        else 
        {$org_lines = $line; }

      }
  
    }
    # Split the lines to fill the hash table
    my @temp_lines = split /\s*#\s*/,$org_lines; 
    @meas_name = split /\s+/,$temp_lines[0];
    @meas_val = split /\s+/,$temp_lines[1];
    if ($#meas_name != $#meas_val)
    {
	  print "@meas_name\n\n";
      print "@meas_val\n\n";
      print "Error : Unmatched Number(Meas_Name:$#meas_name, Meas_Val:$#meas_val) in measurement results($fmt0)!\n"; 
      print RPT "Error : Unmatched Number(Meas_Name:$#meas_name, Meas_Val:$#meas_val) in measurement results($fmt0)!\n"; 
      exit(1);
    }
    else
    {
      for (my $imeas = 0; $imeas < ($#meas_name+1); $imeas++)
      {
        # hash table is filled. Then category the skew, delay and power
        if ($meas_name[$imeas] =~ /^skew/)
        {$meas_rst_ptr->{skew}->{$meas_name[$imeas]} = $meas_val[$imeas];} 
        elsif ($meas_name[$imeas] =~ /^power/)
        {$meas_rst_ptr->{power}->{$meas_name[$imeas]} = $meas_val[$imeas];} 
        elsif ($meas_name[$imeas] =~ /^delay/)
        {$meas_rst_ptr->{delay}->{$meas_name[$imeas]} = $meas_val[$imeas];} 
      }
    }
    # Get the maximum, minimum and average
    if ($opt_ptr->{power_analysis} eq "on")
    {  
      &stats_meas("power");  
    }
    if ($opt_ptr->{meas_delay} eq "on")
    {  
      &stats_meas("delay");  
    }
    if ($opt_ptr->{meas_skew} eq "on")
    {  
      &stats_meas("skew");  
    }
    
    print RPT "*****End HSPICE Simulation Results*****\n";
    
    close(FMT);
  }
  return 1;
}
 
# Get alias from conf_ptr->sctgy[]
sub get_alias_from_conf($ $)
{
  my ($mainsct,$targ) = @_;
  my ($rtalias);

  my $idx_mct = &spot_targ_main_category($mainsct); 
  # Spot the $alias in conf_ptr, give the value.
  for (my $iscg = 0; $iscg<$#{$sctgy[$idx_mct]}+1; $iscg++)
  {
    if ($targ eq $conf_ptr->{$mainsct}->{$sctgy[$idx_mct]->[$iscg]}->{alias})
    {$rtalias = $sctgy[$idx_mct]->[$iscg];}
  }
  return $rtalias;
}

# Fill the hash with information
sub fill_cur_info($ $ $)
{
  my ($info_h,$keywd,$val) = @_;
  # Spot corr_matrix category in the main category
  my $idx_mct = &spot_targ_main_category("corr_matrix"); 
  # Spot the $alias in conf_ptr, give the value.
  for (my $iscg = 0; $iscg<$#{$sctgy[$idx_mct]}+1; $iscg++)
  {
    if ($keywd eq $conf_ptr->{corr_matrix}->{$sctgy[$idx_mct]->[$iscg]}->{alias})
    {
      my $temp_val=$val;
      if ($sctgy[$idx_mct]->[$iscg] eq "dev")
      {$temp_val = &get_alias_from_conf("cts_net_main",$val);}
      if ($sctgy[$idx_mct]->[$iscg] eq "para")
      {$temp_val = &get_alias_from_conf("cts_setup",$val);}
      $info_h->{$sctgy[$idx_mct]->[$iscg]} = $temp_val;
      if ("on" eq $opt_ptr->{debug})
      {print "DEBUG: Assign $keywd\'s value ($temp_val) in Matrix file.\n";}
    }
  }
  return 1;
}

# Deal with the correlation file lines
sub read_line_corr_matrix($ $ $)
{
  my ($curline,$inmat,$info_h) = @_;
  my ($endsig) = ($conf_ptr->{corr_matrix}->{end_mat}->{alias});
  if ($$inmat eq "on")
  {
    if ($curline =~ /$endsig/) 
    {$$inmat = "off";}
    else 
    {
      # Give Index list values
      $corr_mats_ptr->{$info_h->{dev}}->{$info_h->{para}}->{$info_h->{tier}}->{idx} = $info_h->{idx}; 
      
      # Split the matrix elements
      $curline =~ s/;$//;
      my @matlines = split /;/,$curline;
      for (my $imln = 0; $imln < ($#matlines+1); $imln++)
      {
        my @mline = split /\s+/,$matlines[$imln];
        if ("on" eq $opt_ptr->{debug})
        {print "DEBUG: ";}
        for (my $ielem = 0; $ielem<($#mline+1); $ielem++)
        {
          $corr_mats_ptr->{$info_h->{dev}}->{$info_h->{para}}->{$info_h->{tier}}->{matrix}->[$info_h->{cur_mat_line}]->[$ielem] = $mline[$ielem]; 
          if ("on" eq $opt_ptr->{debug})
          {print "$mline[$ielem],";}
        }
        $info_h->{cur_mat_line}++;
        if ("on" eq $opt_ptr->{debug})
        {print "\n";}
      }
    }
  } 
  else
  {
    $curline =~ s/(\s*):(\s*)/:/g;
    my @infoline = split /:/,$curline;
    #if ("on" eq $opt_ptr->{debug})
    #{print "DEBUG : InfoLine[0]($infoline[0])\n";}
    if ($infoline[0] eq $conf_ptr->{corr_matrix}->{begin_mat}->{alias})
    {
      $$inmat = "on";
      if ("on" eq $opt_ptr->{debug})
      {print "DEBUG : Matrix Detected!\n";}
      
      $info_h->{cur_mat_line} = 0;
    }
    elsif ($infoline[0] ne $endsig)
    {&fill_cur_info($info_h,$infoline[0],$infoline[1]);}
  }

  return 1;
}
  
# Read the Correlation Matrix File
# Fill the Matrix with 0 if some element is not defined
# For example, the input is "0.2 0.3", the rest will all be "0"
sub read_corr_matrix($)
{
  # Add post-fix to the filename
  my ($mname) = @_;
  $mname =~ s/\.(\w+)$//i;  
  $mname .= ".cmx";
  
  # Read the Correlation Matrix File
  my ($line,$post_line);
  my %cur_corr_info;
  my ($inmat) = ("off");
  open (CMF,"< $mname") or die "Error:Fail to open $mname!\n";

  while(defined($line = <CMF>))  
  {
    chomp $line;
    $post_line = &read_line($line,"#"); 
    if (defined($post_line))
    {&read_line_corr_matrix($post_line,\$inmat,\%cur_corr_info);}
  }

  close(CMF); 
  return 1;
}

# Check the Correlation Matrix File
# Check points :
# A. Device Index and Tiers Match.
# B. Matrix is diagonse and Dimension matches the number of Devices
sub check_corr_matrix()
{

  return 1;
}

sub gen_corr_expr($ $ $ $ $)
{
  my ($dev,$idx,$para,$tier,$label) = @_;
  # two arrays. one for index and the other for matrix
  my @var_idx;
  my ($rtfunc);
  
  @var_idx = split /\s+/,$corr_mats_ptr->{$dev}->{$para}->{$tier}->{idx};   
  
  #Get the corresponding index in the idxlst
  my ($idx_mat);
  for (my $ilst = 0; $ilst<($#var_idx+1); $ilst++)
  {
    if ($idx == $var_idx[$ilst])
    {$idx_mat = $ilst;}
  }
  if (!defined($idx_mat))
  {die "Error: Fail to spot Index($idx) of Device($dev) Parameter($para) Tier($tier) in Array(@var_idx)!\n";}
  
  if ("on" eq $opt_ptr->{debug})
  {print "DEBUG: Index($idx) of Device($dev) Parameter($para) Tier($tier) in Array(@var_idx) is $idx_mat!\n";}
  

  # Form the function
  for (my $ilst = 0; $ilst<($#var_idx+1); $ilst++)
  {
    my $corr_temp = $corr_mats_ptr->{$dev}->{$para}->{$tier}->{matrix}->[$idx_mat]->[$ilst]; 
    if ((defined($corr_temp))&&($corr_temp != 0))
    {
      if ($corr_temp == 1)
      {$rtfunc .= "loc_$label\_$dev$var_idx[$ilst]+";}
      else
      {$rtfunc .= "$corr_temp*loc_$label\_$dev$var_idx[$ilst]+";}
    }
  } 
 
  $rtfunc =~ s/\+$//;   
    
  return $rtfunc;
}
# -----------Main Program-----------#
sub main()
{
  &opts_read();
  &read_conf();
  &read_fconf();
  &read_cts_setup();
  # Prepare the filenames for netlists, sp and lis
  for(my $inetf = 0; $inetf<$#netfiles+1; $inetf++)
  {
    # Name the files
    my ($orgname) = ($netfiles[$inetf]);
    my ($filenet) = ("$conf_ptr->{dir_path}->{net_dir}->{alias}"."$orgname");
    my ($rptfolder) = ("$conf_ptr->{dir_path}->{rpt_dir}->{alias}"."$orgname");
    my ($matname) = ("$conf_ptr->{dir_path}->{matrix_dir}->{alias}"."$orgname");
    
    # Create paths
    &generate_path($conf_ptr->{dir_path}->{rpt_dir}->{alias});
    &generate_path($conf_ptr->{dir_path}->{spice_dir}->{alias});
    $rptfolder =~ s/\.(\w+)$//; 
    &generate_path($rptfolder);
    my ($filerpt) = ("$rptfolder"."/$orgname");
    $filerpt =~ s/\.(\w+)$//;

    # Prepare for report file 
    if ($opt_ptr->{"mc"} eq "on")
    {$filerpt = $filerpt."_mc$opt_ptr->{mc_val}";}
    if ($opt_ptr->{"power_noise"} eq "on")
    {$filerpt = $filerpt."_pn";}
    $filerpt = $filerpt."_summary.rpt";
    my ($node_chk_rpt) = ($filerpt);
    $node_chk_rpt =~ s/_summary\.rpt$/_fullcheck.rpt/;
    open (RPT, "> $filerpt") or die "Fail to create $filerpt!\n";
    open (CHKRPT, "> $node_chk_rpt") or die "Fail to create $node_chk_rpt!\n";
   
    # Begin major program
    # Read CTS netlist 
    &read_cts_netlist($filenet);
    &check_cts_netlist($filerpt);
   
    # Read Correlation Matrix
    if (($opt_ptr->{"corr"} eq "on")&&($opt_ptr->{"mc"} eq "on"))
    {
      print "Reading Correlation Matrix...\n";
      &read_corr_matrix($matname);
      &check_corr_matrix($filerpt);
      print "Done\n";
    }
    # Do checks
    if ($opt_ptr->{"full_check"} eq  "on")
    {&check_float_node($node_chk_rpt);}
    print RPT "*****HSPICE Netlist Generation Summary*****\n";
    &create_hspice_headers($conf_ptr->{dir_path}->{spice_dir}->{alias});
    &create_hspice_netlist($conf_ptr->{dir_path}->{spice_dir}->{alias},$orgname);
    print RPT "*****End of HSPICE Netlist Generation Summary*****\n";
    print "HSPICE Simulation Begin...\n";
    &run_hspice($conf_ptr->{dir_path}->{spice_dir}->{alias},$orgname);
    if ("on" ne $opt_ptr->{"mc"})
    {
      &organize_sim_result($conf_ptr->{dir_path}->{spice_dir}->{alias},$orgname);
    }
    close(RPT);
    close(CHKRPT);
    # Re-initialize 
    # Re-initialize 
    undef  %conf_h;
    undef  %corr_mats;
    # Reread the configuration 
    if ($#netfiles > $inetf)
    {
      &read_conf();
      &read_cts_setup();
    }
  }
  return 1;
}

&main();
exit(0);

