#!usr/bin/perl -w
# use the strict mode
use strict;
# Use the Shell enviornment
use Shell;
# Use the time
use Time::gmtime;
# Use switch module
#use Switch;
use File::Path;
use Cwd;
use FileHandle;
# Multi-thread support
use threads;
use threads::shared;

# Date
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
my @benchmark_names;
my %benchmarks;
my $benchmarks_ptr = \%benchmarks;
my %task_status;
my $task_status_ptr = \%task_status;

# Configuration file keywords list
# Category for conf file.
# main category : 1st class
my @mctgy;
# sub category : 2nd class
my @sctgy;
# Initialize these categories
@mctgy = ("dir_path",
          "task_conf",
          "csv_tags",
         );
# refer to the keywords of dir_path
@{$sctgy[0]} = ("result_dir",
                "shell_script_name",
                "top_netlist_postfix",
                "mux_testbench_postfix",
               );
# refer to the keywords of flow_type
@{$sctgy[1]} = ("auto_check",
                );
# refer to the keywords of csv_tags
@{$sctgy[2]} = ("top_netlist_leakage_power_tags",
                "top_netlist_dynamic_power_tags",
                "mux_testbench_leakage_power_prefix_tags",
                "mux_testbench_dynamic_power_prefix_tags",
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
  print "      run_fpga_spice.pl [-options <value>]\n";
  print "      Mandatory options: \n";
  print "      -conf <file> : specify the basic configuration files for run_fpga_spice\n";
  print "      -task <file> : the configuration file contains benchmark file names\n"; 
  print "      -rpt <file> : CSV file consists of data\n";
  print "      Other Options:\n";
  print "      -parse_mux_testbench: parse the results in mux_testbench\n";
  print "      -multi_thread <int>: turn on multi-thread mode, specify the number of processors could be pushed\n";
  print "      -sim_leakage_power_only : simulate leakage power only.\n";
  print "      -parse_results_only : only parse HSPICE simulation results\n";
  print "      -debug : debug mode\n";
  print "      -help : print usage\n";
  exit(1);
  return 1;
}
 
sub spot_option($ $) {
  my ($start,$target) = @_;
  my ($arg_no,$flag) = (-1,"unfound");
  for (my $iarg = $start; $iarg < $#ARGV+1; $iarg++) {
    if ($ARGV[$iarg] eq $target) {
      if ("found" eq $flag) {
        print "Error: Repeated Arguments!(IndexA: $arg_no,IndexB: $iarg)\n";
        &print_usage();        
      } else {
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
sub read_opt_into_hash($ $ $) {
  my ($opt_name,$opt_with_val,$mandatory) = @_;
  # Check the -$opt_name
  my ($opt_fact) = ("-".$opt_name);
  my ($cur_arg) = (0);
  my ($argfd) = (&spot_option($cur_arg,"$opt_fact"));
  if ($opt_with_val eq "on") {
    if (-1 != $argfd) {
      if ($ARGV[$argfd+1] =~ m/^-/) {
        print "The next argument cannot start with '-'!\n"; 
        print "it implies an option!\n";
      } else {
        $opt_ptr->{"$opt_name\_val"} = $ARGV[$argfd+1];
        $opt_ptr->{"$opt_name"} = "on";
      }     
    } else {
      $opt_ptr->{"$opt_name"} = "off";
      if ($mandatory eq "on") {
        print "Mandatory option: $opt_fact is missing!\n";
        &print_usage();
      }
    }
  } else {
    if (-1 != $argfd) {
      $opt_ptr->{"$opt_name"} = "on";
    } else {
      $opt_ptr->{"$opt_name"} = "off";
      if ($mandatory eq "on") {
        print "Mandatory option: $opt_fact is missing!\n";
        &print_usage();
      }
    }  
  }
  return 1;
}

# Read options
sub opts_read() {
  # if no arguments detected, print the usage.
  if (-1 == $#ARGV) {
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
  if (-1 != $argfd) {
    print "Help desk:\n";
    &print_usage();
  }  
  # Then Check the debug with highest priority
  $argfd = &spot_option($cur_arg,"-debug");
  if (-1 != $argfd) {
    $opt_ptr->{"debug"} = "on";
  } else {
    $opt_ptr->{"debug"} = "off";
  }
  # Check mandatory options
  # Check the -conf
  # Read Opt into Hash(opt_ptr) : "opt_name","with_val","mandatory"
  &read_opt_into_hash("conf","on","on");
  &read_opt_into_hash("task","on","on");
  &read_opt_into_hash("rpt","on","on");
  &read_opt_into_hash("parse_mux_testbench","off","off");
  &read_opt_into_hash("sim_leakage_power_only","off","off");
  &read_opt_into_hash("parse_results_only","off","off");
  &read_opt_into_hash("multi_thread","on","off");

  &print_opts(); 

  return 1;
}
  
# List the options
sub print_opts() {
  print "List your options\n"; 
  
  while(my ($key,$value) = each(%opt_h)) {
    print "$key : $value\n";
  }

  return 1;
}


# Read each line and ignore the comments which starts with given arg
# return the valid information of line
sub read_line($ $) {
  my ($line,$com) = @_;
  my @chars;
  if (defined($line)) {
    @chars = split/$com/,$line;
    if (!($line =~ m/[\w\d]/)) {
      $chars[0] = undef;
    }
    if ($line =~ m/^\s*$com/) {
      $chars[0] = undef;
    }
  } else {
    $chars[0] = undef;
  }
  if (defined($chars[0])) {
    $chars[0] =~ s/^(\s+)//g;
    $chars[0] =~ s/(\s+)$//g;
  }  
  return $chars[0];    
}

# Check each keywords has been defined in configuration file
sub check_keywords_conf() {
  for (my $imcg = 0; $imcg<$#mctgy+1; $imcg++) {
    for (my $iscg = 0; $iscg<$#{$sctgy[$imcg]}+1; $iscg++) {
      if (defined($conf_ptr->{$mctgy[$imcg]}->{$sctgy[$imcg]->[$iscg]}->{val})) {
        if ("on" eq $opt_ptr->{debug}) {
          print "Keyword($mctgy[$imcg],$sctgy[$imcg]->[$iscg]) = ";
          print "$conf_ptr->{$mctgy[$imcg]}->{$sctgy[$imcg]->[$iscg]}->{val}";
          print "\n";
        }
      } else {
        die "Error: Keyword($mctgy[$imcg],$sctgy[$imcg]->[$iscg]) is missing!\n";
      }
    }
  }
  return 1;
}

# Read the configuration file
sub read_conf() {
  # Read in these key words
  my ($line,$post_line);
  my @equation;
  my $cur = "unknown";
  open (CONF, "< $opt_ptr->{conf_val}") or die "Fail to open $opt_ptr->{conf}!\n";
  print "Reading $opt_ptr->{conf_val}...\n";
  while(defined($line = <CONF>)) {
    chomp $line;
    $post_line = &read_line($line,"#"); 
    if (defined($post_line)) {
      if ($post_line =~ m/\[(\w+)\]/) {
        $cur = $1;
      } elsif ("unknown" eq $cur) {
        die "Error: Unknown tags for this line!\n$post_line\n";
      } else {
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

sub read_benchmarks() {
  # Read in file names
  my ($line,$post_line,$cur);
  $cur = 0;
  open (FCONF,"< $opt_ptr->{task_val}") or die "Fail to open $opt_ptr->{task}!\n";
  print "Reading $opt_ptr->{task_val}...\n";
  while(defined($line = <FCONF>)) {
    chomp $line;
    $post_line = &read_line($line,"#");
    if (defined($post_line)) {
      $post_line =~ s/\s+//g;
      my @tokens = split(",",$post_line);
      # first is the benchmark name, 
      #the second is the channel width, if applicable
      if ($#tokens < 2) {
        die "ERROR: invalid definition for a benchmark! At least include 3 tokens!\n";
      }
      if ($tokens[0]) {
        $benchmark_names[$cur] = $tokens[0];       
      } else {
        die "ERROR: invalid definition for benchmarks!\n";
      }    
      $benchmarks_ptr->{"$benchmark_names[$cur]"}->{spice_netlist_prefix} = $tokens[1];
      $benchmarks_ptr->{"$benchmark_names[$cur]"}->{spice_dir} = $tokens[2];
      $cur++;
    } 
  }  
  print "Benchmarks(total $cur):\n";
  foreach my $temp(@benchmark_names) {
    print "$temp\n";
  }
  close(FCONF);
  return 1;
}

# Input program path is like "~/program_dir/program_name"
# We split it from the scalar
sub split_prog_path($) { 
  my ($prog_path) = @_;
  my @path_elements = split /\//,$prog_path;
  my ($prog_dir,$prog_name);
 
  $prog_name = $path_elements[$#path_elements]; 
  $prog_dir = $prog_path;
  $prog_dir =~ s/$prog_name$//g;
    
  return ($prog_dir,$prog_name);
}

# TODO: Check settings
sub check_fpga_spice() {
  # Format the dir_path 
  # Format SPICE prefix
}

# Initialize the status of all tasks 
sub init_tasks_status() {
  foreach my $bm(@benchmark_names) {
    $task_status_ptr->{$bm}->{status} = "wait";
    $task_status_ptr->{$bm}->{thread_id} = undef;
  } 
}

sub check_all_fpga_spice_tasks_done() {
  foreach my $bm(@benchmark_names) {
    if ("done" ne $task_status_ptr->{$bm}->{status}) {
      return 0;
    }
  } 
  return 1;
}

sub mark_all_fpga_spice_tasks_done() {
  foreach my $bm(@benchmark_names) {
    $task_status_ptr->{$bm}->{status} = "done";
  } 
  return 1;
}

sub print_tasks_status() {
  my ($num_jobs_running, $num_jobs_to_run, $num_jobs_finish, $num_jobs) = (0, 0, 0, 0); 
 
  foreach my $benchmark(@benchmark_names) {
    # Count the number of jobs
    $num_jobs++; 
    # Count to do jobs
    if ("wait" eq $task_status_ptr->{$benchmark}->{status}) {
      $num_jobs_to_run++;
      next;
    }
    if ("running" eq $task_status_ptr->{$benchmark}->{status}) {
      # Count running jobs
      $num_jobs_running++;
      next;
    }
    # Count finished jobs
    if ("done" eq $task_status_ptr->{$benchmark}->{status}) {
      $num_jobs_finish++;
      next;
    }
  }
  if ($num_jobs == ($num_jobs_running + $num_jobs_finish + $num_jobs_to_run)) {
    print "Jobs Progress: (Finish rate = ".sprintf("%.2f",100*$num_jobs_finish/$num_jobs) ."%)\n";
    print "Thead utilization rate = ".sprintf("%.2f",100*$num_jobs_running/$opt_ptr->{multi_thread_val})."%\n";
    print "Total No. of Jobs: $num_jobs.\n";
    print "No. of Running Jobs: $num_jobs_running.\n";
    print "No. of Finished Jobs: $num_jobs_finish.\n";
    print "No. of To Run Jobs: $num_jobs_to_run.\n";
  } else {
    print "Internal problem: num_jobs($num_jobs) != num_jobs_running($num_jobs_running)\n";
    print "                                        +num_jobs_finish($num_jobs_finish)\n";
      die "                                        +num_jobs_to_run($num_jobs_to_run)\n";
  }
  return; 
}

sub format_dir_path($) {
  my ($dir_path) = @_;
  my ($formatted_dir_path) = ($dir_path);

  if (!($formatted_dir_path =~ m/\/$/)) {
    $formatted_dir_path = $formatted_dir_path."/";
  }
 
  return $formatted_dir_path; 
}

sub gen_fpga_spice_netlists_path($ $) {
  my ($spice_dir, $sp_prefix) = @_;
  my ($formatted_spice_dir) = &format_dir_path($spice_dir);
  my ($shell_script_path, $top_sp, $muxtb_sp);

  $shell_script_path = $formatted_spice_dir.$conf_ptr->{dir_path}->{shell_script_name}->{val};
  $top_sp = $formatted_spice_dir.$sp_prefix.$conf_ptr->{dir_path}->{top_netlist_postfix}->{val};
  $muxtb_sp = $formatted_spice_dir.$sp_prefix.$conf_ptr->{dir_path}->{mux_testbench_postfix}->{val};
  
  return ($shell_script_path, $top_sp, $muxtb_sp);
}

sub gen_fpga_spice_measure_results_path($ $) {
  my ($spice_dir, $sp_prefix) = @_;
  my ($formatted_spice_dir) = &format_dir_path($spice_dir);
  my ($formatted_result_dir) = &format_dir_path($conf_ptr->{dir_path}->{result_dir}->{val});
  my ($shell_script_path, $top_mt, $muxtb_mt);

  $shell_script_path = $formatted_spice_dir.$conf_ptr->{dir_path}->{shell_script_name}->{val};
  $top_mt = $formatted_spice_dir.$formatted_result_dir.$sp_prefix.$conf_ptr->{dir_path}->{top_netlist_postfix}->{val};
  $muxtb_mt = $formatted_spice_dir.$formatted_result_dir.$sp_prefix.$conf_ptr->{dir_path}->{mux_testbench_postfix}->{val};

  $top_mt =~ s/\.sp/.mt0/;
  $muxtb_mt =~ s/\.sp/.mt0/;
  
  return ($shell_script_path, $top_mt, $muxtb_mt);
}

sub check_one_spice_lis_error($) {
  my ($lis_path) = @_;
  my ($line, $line_no, $warn_no, $err_no) = (undef,0,0,0);
  my ($LISFH) = FileHandle->new;

  print "INFO: Checking LIS file($lis_path)...\n";

  if (!(-e $lis_path)) {
    die "ERROR: Fail to find SPICE lis file($lis_path)!\n";
  }
  if ($LISFH->open("< $lis_path")) {
    while(defined($line = <$LISFH>)) {
      chomp $line;
      $line_no++;
      if ($line =~ m/error/i) {
        $err_no++;
        print "ERROR($err_no),LIS FILE[LINE$line_no]: ".$line."\n"; 
      }
      if ($line =~ m/warning/i) {
        $warn_no++;
        print "WARNING($warn_no),LIS FILE[LINE$line_no]: ".$line."\n"; 
      }
    } 
  } else {
    die "ERROR: fail to open $lis_path!\n";
  }
  # Close the lis file 
  close($LISFH);
  # Print HSPICE ERROR and Warning Stats 
  print "HSPICE Sim. reports $err_no errors and $warn_no warnings.\n";
  if ($err_no > 0) {
    die "ERROR: Terminate due to errors in HSPICE Sim. reports.\n";
  }

  return;
}

sub check_one_fpga_spice_task_lis($ $ $) {
  my ($benchmark, $spice_netlist_prefix, $spice_dir) = @_; 
  my ($shell_script_path,$top_mt_path,$muxtb_mt_path) = &gen_fpga_spice_measure_results_path($spice_dir, $spice_netlist_prefix);
  my ($top_lis_path, $muxtb_lis_path) = ($top_mt_path, $muxtb_mt_path);

  $top_lis_path =~ s/\.mt0/.lis/;
  $muxtb_lis_path =~ s/\.mt0/.lis/;

  &check_one_spice_lis_error($top_lis_path);

  if ("on" eq $opt_ptr->{parse_mux_testbench}) {
    &check_one_spice_lis_error($muxtb_lis_path);
  }

  return;
}

sub parse_one_fpga_spice_task_muxtb_results($ $ $) {
  my ($benchmark,$muxtb_lispath, $muxtb_mtpath) = @_;
  my (@leakage_tags) = split('\|', $conf_ptr->{csv_tags}->{mux_testbench_leakage_power_prefix_tags}->{val});
  my (@dynamic_tags) = split('\|', $conf_ptr->{csv_tags}->{mux_testbench_dynamic_power_prefix_tags}->{val});
  my ($line, $found_tran_analysis);
  my ($LISFH) = FileHandle->new;

  if (!(-e $muxtb_lispath)) {
    die "ERROR: Fail to find SPICE lis file($muxtb_lispath)!\n";
  }
  if ($LISFH->open("< $muxtb_lispath")) {
    $found_tran_analysis = 0;
    while(defined($line = <$LISFH>)) {
      chomp $line;
      if ((0 == $found_tran_analysis)&&($line =~ m/transient\s+analysis/)) {
        $found_tran_analysis = 1;
      }
      if (0 == $found_tran_analysis) {
        next;
      }
      # Special: get peak memory used and total elapsed time
      if ($line =~ m/peak\s+memory\s+used\s+([\d.]+)\s+megabytes/i) {
        $rpt_ptr->{$benchmark}->{muxtb}->{peak_mem_used} = $1;
      }
      if ($line =~ m/total\s+elapsed\s+time\s+([\d.]+)\s+seconds/i) {
        $rpt_ptr->{$benchmark}->{muxtb}->{total_elapsed_time} = $1;
      }
      foreach my $tag(@leakage_tags) {
        if ($line =~ m/$tag(\w+)\s*=\s*([\d.\w\-\+]+)/i) {
          $rpt_ptr->{$benchmark}->{muxtb}->{"$tag$1"} = $2;
        }
      }
      if ("off" eq $opt_ptr->{sim_leakage_power_only}) {
        foreach my $tag(@dynamic_tags) {
          if ($line =~ m/$tag(\w+)\s*=\s*([\d.\w\-\+]+)/i) {
            $rpt_ptr->{$benchmark}->{muxtb}->{"$tag$1"} = $2;
          }
        }
      } 
    }
  } else {
    die "ERROR: fail to open $muxtb_lispath!\n";
  }
  # Close file
  close($LISFH);

  return;
}

sub parse_one_fpga_spice_task_top_results($ $ $) {
  my ($benchmark,$top_lispath, $top_mtpath) = @_;
  my (@leakage_tags) = split('\|', $conf_ptr->{csv_tags}->{top_netlist_leakage_power_tags}->{val});
  my (@dynamic_tags) = split('\|', $conf_ptr->{csv_tags}->{top_netlist_dynamic_power_tags}->{val});
  my ($line, $found_tran_analysis);
  my ($LISFH) = FileHandle->new;

  # Check if there is any conflict to reserved words
  foreach my $tag(@leakage_tags) {
    if (("peak_mem_used" eq $tag)||("total_elapsed_time" eq $tag)) {
      die "ERROR: top_netlist_leakage_power_tags has a conflict word($tag)!\n"; 
    }
  }
  if ("off" eq $opt_ptr->{sim_leakage_power_only}) {
    foreach my $tag(@dynamic_tags) {
      if (("peak_mem_used" eq $tag)||("total_elapsed_time" eq $tag)) {
        die "ERROR: top_netlist_leakage_power_tags has a conflict word($tag)!\n"; 
      }
    }
  }

  if (!(-e $top_lispath)) {
    die "ERROR: Fail to find SPICE lis file($top_lispath)!\n";
  }
  if ($LISFH->open("< $top_lispath")) {
    $found_tran_analysis = 0;
    while(defined($line = <$LISFH>)) {
      chomp $line;
      if ((0 == $found_tran_analysis)&&($line =~ m/transient\s+analysis/)) {
        $found_tran_analysis = 1;
      }
      if (0 == $found_tran_analysis) {
        next;
      }
      # Special: get peak memory used and total elapsed time
      if ($line =~ m/peak\s+memory\s+used\s+([\d.]+)\s+megabytes/i) {
        $rpt_ptr->{$benchmark}->{top}->{peak_mem_used} = $1;
      }
      if ($line =~ m/total\s+elapsed\s+time\s+([\d.]+)\s+seconds/i) {
        $rpt_ptr->{$benchmark}->{top}->{total_elapsed_time} = $1;
      }
      foreach my $tag(@leakage_tags) {
        if ($line =~ m/$tag\s*=\s*([\d.\w\-\+]+)/i) {
          $rpt_ptr->{$benchmark}->{top}->{$tag} = $1;
        }
      }
      if ("off" eq $opt_ptr->{sim_leakage_power_only}) {
        foreach my $tag(@dynamic_tags) {
          if ($line =~ m/$tag\s*=\s*([\d.\w\-\+]+)/i) {
            $rpt_ptr->{$benchmark}->{top}->{$tag} = $1;
          }
        }
      } 
    }
  } else {
    die "ERROR: fail to open $top_lispath!\n";
  }
  # Close file
  close($LISFH);

  return;
}

sub parse_one_fpga_spice_task_results($ $ $) {
  my ($benchmark, $spice_netlist_prefix, $spice_dir) = @_; 
  my ($shell_script_path,$top_mtpath,$muxtb_mtpath) = &gen_fpga_spice_measure_results_path($spice_dir, $spice_netlist_prefix);
  my ($top_lispath, $muxtb_lispath) = ($top_mtpath, $muxtb_mtpath);

  $top_lispath =~ s/\.mt0/.lis/;
  $muxtb_lispath =~ s/\.mt0/.lis/;

  &parse_one_fpga_spice_task_top_results($benchmark,$top_lispath, $top_mtpath);

  if ("on" eq $opt_ptr->{parse_mux_testbench}) {
    &parse_one_fpga_spice_task_muxtb_results($benchmark,$muxtb_lispath, $muxtb_mtpath);
  }
  
  return;
}

# Run a fpga_spice_task
sub run_one_fpga_spice_task($ $ $) {
  my ($benchmark, $spice_netlist_prefix, $spice_dir) = @_; 
  my ($shell_script_path,$top_sp_path,$muxtb_sp_path) 
    = &gen_fpga_spice_netlists_path($spice_dir, $spice_netlist_prefix);

  # change to the spice_dir
  chdir $spice_dir;
  # Check all the SPICE netlists and shell scripts exist
  if (!(-e $top_sp_path)) {
    die "ERROR: File($top_sp_path) does not exist!";
  } 
  if (("on" eq $opt_ptr->{parse_mux_testbench})&&(!(-e $muxtb_sp_path))) {
    die "ERROR: File($muxtb_sp_path) does not exist!";
  }
  if (!(-e $shell_script_path)) {
    die "ERROR: File($shell_script_path) does not exist!";
  }

  # Call the shell script 
  print "INFO: Running shell script ($shell_script_path)...\n";
  `csh -c 'source $shell_script_path'`
    or die "ERROR: fail in executing $shell_script_path!\n";

  # return to current dir
  chdir $cwd;  

  &check_one_fpga_spice_task_lis($benchmark, $spice_netlist_prefix, $spice_dir); 

  return; 
}

sub parse_all_tasks_results() {
  foreach my $benchmark(@benchmark_names) {
    if ("done" eq $task_status_ptr->{$benchmark}->{status}) {
      &parse_one_fpga_spice_task_results($benchmark,$benchmarks_ptr->{$benchmark}->{spice_netlist_prefix},$benchmarks_ptr->{$benchmark}->{spice_dir});
    } else {
      die "ERROR: found unfinished tasks when try to parse results!\n";
    }
  } 
}

# Multi-thread Running FPGA SPICE tasks 
sub multi_thread_run_fpga_spice_tasks($) {
  my ($num_threads) = @_;

  # Evaluate include threads ok
  if (!(eval 'use threads; 1')) {
    die "ERROR: cannot use threads package in Perl! Please check the installation of package...\n";
  }
  # Lauch threads up to the limited number of threads number
  if ($num_threads < 2) {
    $num_threads = 2;
  }
  my ($num_thread_running) = (0);
  
  # Iterate until all the tasks has been assigned, finished
  while (1 != &check_all_fpga_spice_tasks_done()) {
    foreach my $benchmark(@benchmark_names) {
      # Bypass finished job
      if ("done" eq $task_status_ptr->{$benchmark}->{status}) {
        next;
      }
      # Check running job
      if ("running" eq $task_status_ptr->{$benchmark}->{status}) {
        my ($thr_id) = ($task_status_ptr->{$benchmark}->{thread_id});
        if (!($thr_id)) { 
          die "INTERNAL ERROR: invalid thread_id for task: $benchmark!\n"; 
        }
        # Check if there is any error
        if ($thr_id->error()) {
          die "ERROR: Task: $benchmark, Thread(ID:$thr_id) exit abnormally!\n";
        }
        if ($thr_id->is_running()) {
          $task_status_ptr->{$benchmark}->{status} = "running";
        }
        if ($thr_id->is_joinable()) {
          $num_thread_running--;
          $thr_id->join(); # Join the thread results
          # Update task status
          $task_status_ptr->{$benchmark}->{status} = "done";
          print "INFO: task: $benchmark finished!\n";
          &print_tasks_status(); 
        }
        next;
      }
      # If we reach the thread number limit, we have to wait...
      if (($num_thread_running == $num_threads)
        ||($num_thread_running > $num_threads)) {
        next;
      }
      # Start a new thread for a waiting task
      if ("wait" eq $task_status_ptr->{$benchmark}->{status}) {
        # We try to start a new thread since there are still threads available
        my ($thr_new) = (threads->create(\&run_one_fpga_spice_task, $benchmark,$benchmarks_ptr->{$benchmark}->{spice_netlist_prefix},$benchmarks_ptr->{$benchmark}->{spice_dir}));
        if ($thr_new) {
          print "INFO: a new thread for task (benchmark: $benchmark) is lauched!\n";
          # Update status
          $task_status_ptr->{$benchmark}->{status} = "running";
          $task_status_ptr->{$benchmark}->{thread_id} = $thr_new;
          $num_thread_running++;
          &print_tasks_status(); 
        } else {
          # Fail to create a new thread, wait...
          print "INFO: fail to lauch a new thread for task (benchmark: $benchmark)!\n";
        }
      }
    }
  }
  &print_tasks_status(); 
  
  # Parse_results
  &parse_all_tasks_results();
}

# Single-thread mode Running FPGA SPICE tasks
sub single_thread_run_fpga_spice_tasks() {
  # Iterate until all the tasks has been assigned, finished
  while (1 != &check_all_fpga_spice_tasks_done()) {
    foreach my $benchmark(@benchmark_names) {
      # Bypass finished job
      if ("done" eq $task_status_ptr->{$benchmark}->{status}) {
        next;
      }
      &run_one_fpga_spice_task($benchmark,$benchmarks_ptr->{$benchmark}->{spice_netlist_prefix},$benchmarks_ptr->{$benchmark}->{spice_dir});
      $task_status_ptr->{$benchmark}->{status} = "done";
    }
  }
  # Parse_results
  &parse_all_tasks_results();
  return;
}

# Plan to run tasks
sub plan_run_tasks() {
  &init_tasks_status();

  if (("on" eq $opt_ptr->{multi_thread})
     &&(1 < $opt_ptr->{multi_thread_val})
     &&(1 < $#benchmark_names)) {
    &multi_thread_run_fpga_spice_tasks($opt_ptr->{multi_thread_val});
  } else { 
    if ("on" eq $opt_ptr->{multi_thread}) {
      print "INFO: multi_thread is selected but only 1 processor can be used or 1 benchmark to run...\n";
      print "INFO: switch to single thread mode.\n";
    }
    &single_thread_run_fpga_spice_tasks();
  }
}

sub gen_csv_rpt($) {
  my ($rpt_file) = @_;
  my ($RPTFH) = FileHandle->new;
  my (@leakage_tags) = split('\|', $conf_ptr->{csv_tags}->{top_netlist_leakage_power_tags}->{val});
  my (@dynamic_tags) = split('\|', $conf_ptr->{csv_tags}->{top_netlist_dynamic_power_tags}->{val});

  if ($RPTFH->open("> $rpt_file")) {
    print "INFO: print CVS report($rpt_file)...\n";
  } else {
    die "ERROR: fail to create $rpt_file!\n";
  }
  # Print Title line
  print $RPTFH "Benchmark,SimElapseTime,SimPeakMemUsed,";
  foreach my $tag(@leakage_tags) {
    print $RPTFH "$tag,";
  }
  if ("off" eq $opt_ptr->{sim_leakage_power_only}) {
    foreach my $tag(@dynamic_tags) {
      print $RPTFH "$tag,";
    }
  }
  print $RPTFH "\n";
  # NOW, I only output the results of TOP netlist 
  foreach my $benchmark(@benchmark_names) {
    print $RPTFH "$benchmark,";
    foreach my $tag(@leakage_tags) {
      print $RPTFH "$rpt_ptr->{$benchmark}->{top}->{$tag},";
    }
    if ("off" eq $opt_ptr->{sim_leakage_power_only}) {
      foreach my $tag(@dynamic_tags) {
        print $RPTFH "$rpt_ptr->{$benchmark}->{top}->{$tag},";
      }
    }
    print $RPTFH "\n";
  }

  close($RPTFH);
  return;
}

# Main Program
sub main() {
  &opts_read();
  &read_conf();
  &read_benchmarks();
  &check_fpga_spice();
  if ("on" eq $opt_ptr->{parse_results_only}) {
    &mark_all_fpga_spice_tasks_done();
    &parse_all_tasks_results();
  } else {
    &plan_run_tasks();
  }
  &gen_csv_rpt($opt_ptr->{rpt_val});
}
&main();
exit(0);
