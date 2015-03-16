#!usr/bin/perl -w
use strict;
use Shell;
#Use the time
use Time::gmtime;
# Use switch module
use Switch;
use File::Path;
use Cwd;
use FileHandle;

#Get Date
my $mydate = gmctime();
# Current Path
my $cwd = getcwd();

# Global Variants
# input Option Hash
my %opt_h; 
my $opt_ptr = \%opt_h;
# configurate file hash
my %conf_h;
my $conf_ptr = \%conf_h;
# reports has
my %rpt_h;
my $rpt_ptr = \%rpt_h;

# Benchmarks
my %benchmarks;
my $benchmarks_ptr = \%benchmarks;

# Configuration file keywords list
# Category for conf file.
# main category : 1st class
my @mctgy;
# sub category : 2nd class
my @sctgy;
# Initialize these categories
@mctgy = (#"dir_path",
          "flow_settings",
          "rpt_tags",
         );
# refer to the keywords of dir_path
@{$sctgy[0]} = ("flow_type", # Combinatioinal or Sequential
                "benchmark_prefix",#prefix of benchmark verilog files
                "report_prefix", # Design Compiler Report Prefix 
                "set_dont_use", # Set don't use in Design Compiler
                "read_library", # read_lib
                "target_library", #
                "link_library", # 
                "dc_shell_name", # 
                "clk_name", # name of clk in designs
                "critical_path", #valid for combinational and sequential
                "binary_search_mini_step",
                "max_area",
                "max_leakage_power",
                "map_effort",
                "area_effort",
               );
# refer to the keywords of flow_type
@{$sctgy[1]} = ("dc_rpt_tags",
                );

# ----------Subrountines------------#
# Print TABs and strings
sub tab_print($ $ $)
{
  my ($FILE,$str,$num_tab) = @_;
  my ($my_tab) = ("    ");
  
  for (my $i = 0; $i < $num_tab; $i++) {
    print $FILE "$my_tab";
  }
  print $FILE "$str"; 
}

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
  print "      dc_flow [-options <value>]\n";
  print "      Mandatory options: \n";
  print "      -conf : specify the basic configuration files for dc_flow\n";
  print "      -benchmark : the configuration file contains benchmark file names\n"; 
  print "      -rpt : CSV file consists of data\n";
  print "      Other Options:\n";
  print "      -binary_search: use binary searching to determine the optimal critical path\n";
  print "      -reduce_error_to_warning: reduce the errors in DC report to warnings.\n";
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

# Specify in the input list,
# 1. Option Name
# 2. Whether Option with value. if yes, choose "on"
# 3. Whether Option is mandatory. If yes, choose "on"
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
  # Then Check the debug with highest priority
  $argfd = &spot_option($cur_arg,"-debug");
  if (-1 != $argfd)
  {
    $opt_ptr->{"debug"} = "on";
  }
  else
  {
    $opt_ptr->{"debug"} = "off";
  }
  # Check mandatory options
  # Check the -conf
  # Read Opt into Hash(opt_ptr) : "opt_name","with_val","mandatory"
  &read_opt_into_hash("conf","on","on");
  &read_opt_into_hash("benchmark","on","on");
  &read_opt_into_hash("rpt","on","on");
  
  # Check Optional Options
  &read_opt_into_hash("binary_search","off","off");
  &read_opt_into_hash("reduce_error_to_warning","off","off");

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

# Check each keywords has been defined in configuration file
sub check_keywords_conf()
{
  for (my $imcg = 0; $imcg<$#mctgy+1; $imcg++)
  {
    for (my $iscg = 0; $iscg<$#{$sctgy[$imcg]}+1; $iscg++)
    {
      if (defined($conf_ptr->{$mctgy[$imcg]}->{$sctgy[$imcg]->[$iscg]}->{val}))
      {
        if ("on" eq $opt_ptr->{debug})
        {
          print "Keyword($mctgy[$imcg],$sctgy[$imcg]->[$iscg]) = ";
          print "$conf_ptr->{$mctgy[$imcg]}->{$sctgy[$imcg]->[$iscg]}->{val}";
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
         $conf_ptr->{$cur}->{$equation[0]}->{val} = $equation[1];   
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

sub read_benchmarks()
{
  # Read in file names
  my ($line,$post_line,$cur);
  $cur = 0;
  open (FCONF,"< $opt_ptr->{benchmark_val}") or die "Fail to open $opt_ptr->{benchmark}!\n";
  print "Reading $opt_ptr->{benchmark_val}...\n";
  while(defined($line = <FCONF>))
  {
    chomp $line;
    $post_line = &read_line($line,"#");
    if (defined($post_line))
    {
      my @benchmark_info = split /,/,$post_line;
      $benchmarks_ptr->{$cur}->{name} = $benchmark_info[0];       
      $benchmarks_ptr->{$cur}->{top} = $benchmark_info[1];
      # Check optional in benchmark configuration
      if (($opt_ptr->{binary_search} eq "off")&&(defined($benchmark_info[2]))) {
         $benchmarks_ptr->{$cur}->{crit_path} = $benchmark_info[2];
      }
      if (($opt_ptr->{binary_search} eq "off")&&(!(defined($benchmarks_ptr->{$cur}->{crit_path})))) {
        die "Error: Missing critical path definition in benchmark file($opt_ptr->{benchmark_val})!\n";
      }
      # Check optional clock name
      if (("seq" eq $conf_ptr->{flow_settings}->{flow_type}->{val})&&(defined($benchmark_info[3]))) {
        $benchmarks_ptr->{$cur}->{clk_name} = $benchmark_info[3];
      }
      if (("seq" eq $conf_ptr->{flow_settings}->{flow_type}->{val})&&(!(defined($benchmarks_ptr->{$cur}->{clk_name})))) {
        die "Error: Missing clk_name definition in benchmark file($opt_ptr->{benchmark_val})!\n";
      }
      $cur++;
    } 
  }  
  print "Benchmarks(total $cur):\n";
  $benchmarks_ptr->{total} = $cur;
  close(FCONF);

  for (my $i=0; $i < $benchmarks_ptr->{total}; $i++) {
    print "No.: $i, Verilog: $benchmarks_ptr->{$i}->{name}, Top: $benchmarks_ptr->{$i}->{top}\n";
  }

  return 1;
}

sub generate_TCL($ $ $ $ $ $)
{
  my ($tcl_script,$verilog_file,$verilog_top,$critical_path,$dc_rpt_file,$cur) = @_;
  my ($tclfh) = FileHandle->new;

  if ($tclfh->open(" > $tcl_script")) {
    # Print Head
    print $tclfh "#-----------------------#\n";  
    print $tclfh "#  Design Compiler TCL  #\n";
    print $tclfh "#  Author: Xifan TANG   #\n";
    print $tclfh "#      EPFL IC LSI      #\n";
    print $tclfh "# Date: $mydate #\n";
    print $tclfh "#-----------------------#\n";  
    print $tclfh "\n# Variables Declaration\n";
    if ("seq" eq $conf_ptr->{flow_settings}->{flow_type}->{val}) {
      print $tclfh "set CLK_NAME $benchmarks_ptr->{$cur}->{clk_name}\n";
    }
    #print $tclfh "set CRITICAL_PATH $conf_ptr->{flow_settings}->{critical_path}->{val} ;# [ns]\n";
    print $tclfh "set CRITICAL_PATH $critical_path ;# [ns]\n";
    print $tclfh "\n# Clear previous designs\n";
    print $tclfh "remove_design -all\n";
    print $tclfh "\n# Standard library settings\n"; 
    print $tclfh "read_lib $conf_ptr->{flow_settings}->{read_library}->{val}\n";
    print $tclfh "\n";
    print $tclfh "set target_library $conf_ptr->{flow_settings}->{target_library}->{val}\n";
    print $tclfh "set link_library $conf_ptr->{flow_settings}->{link_library}->{val}\n";

    print $tclfh "\n# Don't use some components\n";
    my @dont_use = split /\|/,$conf_ptr->{flow_settings}->{set_dont_use}->{val};
    foreach my $dont_use_tmp(@dont_use) {
      print $tclfh "set_dont_use $dont_use_tmp\n";
    }
    print $tclfh "\n";
  
    print $tclfh "\n#--Compile $verilog_file--#\n";
    print $tclfh "remove design\n";
    print $tclfh "analyze -f verilog $conf_ptr->{flow_settings}->{benchmark_prefix}->{val}$verilog_file\n";
    print $tclfh "elaborate $verilog_top\n";
    print $tclfh "set_max_leakage_power $conf_ptr->{flow_settings}->{max_leakage_power}->{val}\n";
    print $tclfh "set_max_area $conf_ptr->{flow_settings}->{max_area}->{val}\n";
    if ("seq" eq $conf_ptr->{flow_settings}->{flow_type}->{val}) {
      print $tclfh "create_clock \$CLK_NAME -period \$CRITICAL_PATH\n";
    }
    print $tclfh "link\n";
    print $tclfh "compile -map_effort $conf_ptr->{flow_settings}->{map_effort}->{val} -area_effort $conf_ptr->{flow_settings}->{area_effort}->{val}\n";
    print $tclfh "report_timing > $conf_ptr->{flow_settings}->{report_prefix}->{val}$dc_rpt_file\n";
    print $tclfh "report_area >> $conf_ptr->{flow_settings}->{report_prefix}->{val}$dc_rpt_file\n";
    print $tclfh "report_power >> $conf_ptr->{flow_settings}->{report_prefix}->{val}$dc_rpt_file\n";
    print $tclfh "report_reference >> $conf_ptr->{flow_settings}->{report_prefix}->{val}$dc_rpt_file\n";
    print $tclfh "remove_design\n";
    print $tclfh "\n#--END Compile $verilog_file--#\n";
    print $tclfh "\n";
    print $tclfh "\n";
    print $tclfh "exit\n";
    print $tclfh "\n";
    print $tclfh "#-----END Design Compiler TCL-----#\n";
  }
  else {
    die "Error: Fail to create $tcl_script";
  }
  close($tclfh);
  return $tcl_script;
}

sub run_TCL($)
{
  my ($tcl_script) = @_;
  `csh -cx '$conf_ptr->{flow_settings}->{dc_shell_name}->{val} -f $tcl_script'`;
}

sub check_dc_rpt($)
{
  my ($dc_rpt) = @_;
  my ($dcrptfh) = FileHandle->new;
  my ($line,$line_no);

  $line_no = 0;
  if ($dcrptfh->open(" < $conf_ptr->{flow_settings}->{report_prefix}->{val}$dc_rpt")) {
    while(defined($line = <$dcrptfh>)) {
      chomp $line;
      $line_no++;
      if ($line =~ m/error/i) {
        if ("on" eq $opt_ptr->{reduce_error_to_warning}) {
          print "Warning: Found error in Design Compiler report($dc_rpt,[LINE $line_no])!\n";
        }
        else {
          die "Error: Found error in Design Compiler report($dc_rpt,[LINE $line_no])!\n";
        }
      }
    }    
  }
  else {
    die "Fail to open Design Compiler Report: $conf_ptr->{flow_settings}->{report_prefix}->{val}$dc_rpt!\n";
  }

  close($dcrptfh);   
}

sub extract_results($ $ $)
{
  my ($tag,$dc_rpt_file,$crit_path) = @_;
  my ($line,$line_no);
  my @rpt_tags = split /\|/,$conf_ptr->{rpt_tags}->{dc_rpt_tags}->{val};

  $line_no = 0;
  open (FRPT," < $conf_ptr->{flow_settings}->{report_prefix}->{val}$dc_rpt_file") or die "Fail to open $conf_ptr->{flow_settings}->{report_prefix}->{val}$dc_rpt_file!\n"; 
  while (defined($line = <FRPT>)) {
    chomp $line; 
    $line =~ s/\s//g;
    foreach my $rpt_tag(@rpt_tags) {
      # Special dealing with "slack"
      if (($rpt_tag eq "slack")&&($line =~ m/$rpt_tag\(([A-Za-z]+)\)([\-\+\d\.]+)/)) {
        #print "Line: $line\n";
        $rpt_ptr->{$tag}->{$rpt_tag}->{status} = $1;
        $rpt_ptr->{$tag}->{$rpt_tag}->{slack_val} = $2;
        #print "Slack Status=$rpt_ptr->{$tag}->{$rpt_tag}->{status}, Value=$rpt_ptr->{$tag}->{$rpt_tag}->{slack_val}\n";
        $rpt_ptr->{$tag}->{$rpt_tag}->{val} = $crit_path - $rpt_ptr->{$tag}->{$rpt_tag}->{slack_val};
        # Check VIOLATE or MET
        if ($rpt_ptr->{$tag}->{$rpt_tag}->{status} ne "MET") {
          return 0;
        }
      }
      elsif (($rpt_tag ne "slack")&&($line =~ m/$rpt_tag[\:\=]([\w\.]+)/)) {
        $rpt_ptr->{$tag}->{$rpt_tag}->{val} = $1;
      }
    } 
  }

  close(FRPT); 
  return 1;
}

sub run_dc_one_benchmark($ $ $ $)
{
  my ($verilog_file,$verilog_top,$critical_path,$cur) = @_;
  my ($tcl_script,$dc_rpt_file);
  my ($DC_RUN_DIR) = ("./DC_RUN/");
  my ($ret) = (0);

  # Enter into DC_DUN_DIR  
  chdir $DC_RUN_DIR; 
  &generate_path($DC_RUN_DIR);

  my ($tcl_prefix) = ("../BIN/");
  &generate_path($tcl_prefix);

  # Generate TCL script
  my ($verilog_name) = ($verilog_file);
  $verilog_name =~ s/\.v$//;     

  $tcl_script = "$tcl_prefix$verilog_name\_dc\.tcl";
  $dc_rpt_file = "$verilog_name\.dcrpt" ;

  &generate_TCL($tcl_script,$verilog_file,$verilog_top,$critical_path,$dc_rpt_file,$cur);

  &run_TCL($tcl_script);
  
  &check_dc_rpt($dc_rpt_file);

  if (1 == &extract_results($verilog_name,$dc_rpt_file,$critical_path)) {
    $ret = 1;
  }

  # Return to Current Directory
  chdir $cwd;

  return $ret;
}

sub binary_search_run_dc_one_benchmark($ $ $)
{
  my ($verilog_file,$verilog_top,$cur) = @_;
  my ($critical_path);
  my ($last_success,$last_fail);
  my ($mini_step) = ($conf_ptr->{flow_settings}->{binary_search_mini_step}->{val});

  $critical_path = $conf_ptr->{flow_settings}->{critical_path}->{val};  

  # Check initial critical_path is a success or failure
  if (1 == &run_dc_one_benchmark($verilog_file,$verilog_top,$critical_path,$cur)) {
    # Success, Scale down to find a failure
    $last_success = $critical_path;
    $critical_path = $critical_path/2;
    print "Info: Initial success...\n";
    while (1 == &run_dc_one_benchmark($verilog_file,$verilog_top,$critical_path,$cur)) {
      print "Info: Try critical path = $critical_path Success!\n";
      $last_success = $critical_path;
      $critical_path = $critical_path/2;
    }
    $last_fail = $critical_path;
  }
  else {
    # Failure, explode to find a success
    $last_fail = $critical_path;
    $critical_path = $critical_path*2;
    print "Info: Initial fail...\n";
    while (0 == &run_dc_one_benchmark($verilog_file,$verilog_top,$critical_path,$cur)) {
      print "Info: Try critical path = $critical_path Fail\n";
      $last_fail = $critical_path;
      $critical_path = $critical_path*2;
    }
    $last_success = $critical_path;
  }
 
  print "Info: [Binary search] Last success = $last_success, Last fail = $last_fail, Mini_step: $mini_step\n";

  # Binary search
  while (abs($last_success - $last_fail) > $mini_step) {
    $critical_path = ($last_success+$last_fail)/2;
    print "Info: [Binary search] Try critical_path = $critical_path\n";
    if (1 == &run_dc_one_benchmark($verilog_file,$verilog_top,$critical_path,$cur)) {
      $last_success = $critical_path;
    }
    else {
      $last_fail = $critical_path;
    }
    print "Info: [Binary search] Last success = $last_success, Last_fail = $last_fail\n";
  }

  print "Info: [Binary search] Success = $last_success\n";
  # Finish Re-run last_success to extract the final result
  &run_dc_one_benchmark($verilog_file,$verilog_top,$last_success,$cur);
  
}

sub run_dc_one_benchmark_options($ $ $)
{
  my ($verilog_file,$verilog_top,$cur) = @_;
  my ($critical_path);

  if ($opt_ptr->{binary_search} eq "off") {
    $critical_path = $benchmarks_ptr->{$cur}->{crit_path}; 
    &run_dc_one_benchmark($verilog_file,$verilog_top,$critical_path,$cur);
  }
  else { #Binary searching the optimal critical path
    &binary_search_run_dc_one_benchmark($verilog_file,$verilog_top,$cur);
  }
}

sub run_dc()
{
  my ($verilog_file,$verilog_top);
  my ($pid) = (1);

  # Process Benchmark one by one
  for (my $i=0; $i < $benchmarks_ptr->{total}; $i++) {
    $verilog_file = $benchmarks_ptr->{$i}->{name}; 
    $verilog_top = $benchmarks_ptr->{$i}->{top}; 
    &run_dc_one_benchmark_options($verilog_file,$verilog_top,$i);
  }
}

sub gen_frpt()
{
  my @rpt_tags = split /\|/,$conf_ptr->{rpt_tags}->{dc_rpt_tags}->{val};
  
  my ($rptfh) = FileHandle->new;
  if ($rptfh->open("> $opt_ptr->{rpt_val}")) { 
    print $rptfh "Library,$conf_ptr->{flow_settings}->{read_library}->{val}\n";
    print $rptfh "Verilog,"; 
    for(my $ikw = 0; $ikw<$#rpt_tags+1; $ikw++) {
      print $rptfh "$rpt_tags[$ikw],";
    }
    print $rptfh "\n";
    for (my $i=0; $i < $benchmarks_ptr->{total}; $i++) {
      my ($verilog_name) = $benchmarks_ptr->{$i}->{name};
      $verilog_name =~ s/\.v$//;
      print $rptfh "$benchmarks_ptr->{$i}->{name},"; 
      for(my $ikw = 0; $ikw<$#rpt_tags+1; $ikw++) {
        print $rptfh "$rpt_ptr->{$verilog_name}->{$rpt_tags[$ikw]}->{val},";
      }
      print $rptfh "\n";
    }
  }
  else {
    die "Fail to create $opt_ptr->{rpt_val}!\n";
  }
  close($rptfh);
  return 1;
}

sub main()
{
  &opts_read(); 
  &read_conf();
  &read_benchmarks();

  &run_dc();

  &gen_frpt();

  return 1;
}
 
&main();
exit(1);
