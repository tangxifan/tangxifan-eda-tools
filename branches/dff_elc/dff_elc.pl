#!usr/bin/perl -w
# Perl Script for DFF (D Flip-flop) ELC (Encounter Library Characterizer) 
# use the strict mode
use strict;
# Use the Shell enviornment
use Shell;
# Use the time
use Time::gmtime;
# Use switch module
use Switch;
use File::Path;
use Cwd;
# Use FileHandle to open&close file
use FileHandle;

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

# Matrix informations
my %mclusters;
my ($mclusters_ptr) = (\%mclusters);

# Configuration file keywords list
# Category for conf file.
# main category : 1st class
my @mctgy;
# sub category : 2nd class
my @sctgy;
# Initialize these categories
@mctgy = ("general_settings",
          "circuit_definition",
          "library_settings",
         );
# refer to the keywords of arch_model
@{$sctgy[0]} = (#"hspice_path",
                "spice_dir",
                "process_tech",
                "dff_subckt",
                "verilogA_hspice_sim",
                "include_lib",
               );
# refer to the keywords of arch_device
# Support uni-directional routing architecture and
# single type segment only
@{$sctgy[1]} = ("dff_subckt_name",
                "VDD_port_name",
                "GND_port_name",
                "D_port_name",
                "CLK_port_name",
                "RST_port_name",
                "SET_port_name",
                "Q_port_name",
                "area",
                "clk_freq",
                );
# refer to the keywords of arch_complexblocks
@{$sctgy[2]} = (# Alias
                "design_name",
                "delay_template",
                "energy_template",
                "hold_template",
                "passive_energy_template",
                "recovery_template",
                "removal_template",
                "setup_template",
                "width_template",
                "time_unit",
                "voltage_unit",
                "leakage_power_unit",
                "capacitive_load_unit",
                "rail_type",
                "slew_upper_threshold_pct_rise",
                "slew_lower_threshold_pct_rise",
                "slew_upper_threshold_pct_fall",
                "slew_lower_threshold_pct_fall",
                "input_threshold_pct_rise",
                "input_threshold_pct_fall",
                "output_threshold_pct_rise",
                "output_threshold_pct_fall",
                "supply_voltage",
                "temperature",
                "table_size",
                "transient",
                "bisec",
                "transition",
                "cload",
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
  print "  perl dff_elc.pl [-options <value>]\n";
  print "    Mandatory options: \n";
  print "      -conf : specify the basic configuration files for dff_elc\n";
  print "      -lib : file name of output library\n";
  print "    Other Options:\n";
  print "      -hspice64 : use 64-bit hspice, by default it is 32bit.\n";
  print "      -leakage_only: measuring leakage power only.\n";
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
  if (-1 != $argfd) {
    print "Help desk:\n";
    &print_usage();
  }  
  # Then Check the debug with highest priority
  $argfd = &spot_option($cur_arg,"-debug");
  if (-1 != $argfd) {
    $opt_ptr->{"debug"} = "on";
  }
  else {
    $opt_ptr->{"debug"} = "off";
  }

  # Read Opt into Hash(opt_ptr) : "opt_name","with_val","mandatory"

  # Check mandatory options
  &read_opt_into_hash("conf","on","on"); # Check -conf
  &read_opt_into_hash("lib","on","on");  # Check -lib
  
  # Check Optional
  &read_opt_into_hash("hspice64","off","off");  # Check -hspice32
  &read_opt_into_hash("leakage_only","off","off");  # Check -leakage_only

  &opts_echo(); 

  return 1;
}
  
# List the options
sub opts_echo()
{
  print "Echo your options:\n"; 
 
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
  print "Reading $opt_ptr->{conf_val}...";
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
  print "complete!\n";
  print "Checking these keywords...";
  &check_keywords_conf(); 
  print "Successfully\n";
  close(CONF);
  return 1;
}

# Input program path is like "~/program_dir/program_name"
# We split it from the scalar
sub split_prog_path($)
{ 
  my ($prog_path) = @_;
  my @path_elements = split /\//,$prog_path;
  my ($prog_dir,$prog_name);
 
  $prog_name = $path_elements[$#path_elements]; 
  $prog_dir = $prog_path;
  $prog_dir =~ s/$prog_name$//g;
    
  return ($prog_dir,$prog_name);
}

# Detect and convert unit, NO Case Insentive.
sub process_unit($ $)
{
  my ($unit,$type) = @_;
  my ($ret,$coeff) = (0,0);

  # Check type, can be <time|power|voltage|capacitance>
  if ("time" eq $type) {
    $unit =~ s/s$//i;
  } 
  elsif ("power" eq $type) {
    $unit =~ s/W$//i;
  }
  elsif ("voltage" eq $type) {
    $unit =~ s/V$//i;
  }
  elsif ("capacitance" eq $type) {
    $unit =~ s/f$//i;
  }
  elsif ("empty" ne $type) {
    die "Error: (process_unit)Unknown type!Should be <time|power|voltage|capacitance|empty>\n";
  }
  
  # Accepte unit: m = 1e-3, u = 1e-6, n = 1e-9, p = 1e-12, f = 1e-15, a = 1e-18
  if ($unit =~ m/a$/i) { 
    $unit =~ s/a$//i;
    $coeff = 1e-18;
  }
  elsif ($unit =~ m/f$/i) {
    $unit =~ s/f$//i;
    $coeff = 1e-15;
  }
  elsif ($unit =~ m/p$/i) {
    $unit =~ s/p$//i;
    $coeff = 1e-12;
  }
  elsif ($unit =~ m/n$/i) {
    $unit =~ s/n$//i;
    $coeff = 1e-9;
  }
  elsif ($unit =~ m/u$/i) {
    $unit =~ s/u$//i;
    $coeff = 1e-6;
  }
  elsif ($unit =~ m/m$/i) {
    $unit =~ s/m$//i;
    $coeff = 1e-3;
  }
  elsif ($unit =~ m/\d$/i) {
    $coeff = 1;
  }

  # Quick check, there should be only numbers in remaining
  if (!($unit =~ m/\d$/)) {
    die "Error: (process_unit) Invalid number($unit)!\n";
  }  

  return $ret = $unit*$coeff;
}

# Check library settings
# Tasks:
# 1. Check table_size
# 2. Check size of transient
# 3. Check size of bisec
# 4. Check size of transition
# 5. Check size of cload
sub check_lib_settings()
{
  # Check table_size
  if (1 > $conf_ptr->{library_settings}->{table_size}->{val}) {
    die "Error: (check_lib_settings)Invalid table_size in configuration file($opt_ptr->{conf_val})!\n";
  }
  # Check slew_upper_threshold_pct_rise > slew_lower_threshold_pct_rise
  if ($conf_ptr->{library_settings}->{slew_upper_threshold_pct_rise}->{val} 
     < $conf_ptr->{library_settings}->{slew_lower_threshold_pct_rise}->{val}) {
    die "Error: (check_lib_settings)Invalid slew_[upper|lower]_threshold_pct_rise! Upper should >= Lower.\n";
  }
  # Check slew_upper_threshold_pct_fall > slew_lower_threshold_pct_fall
  if ($conf_ptr->{library_settings}->{slew_upper_threshold_pct_fall}->{val} 
     > $conf_ptr->{library_settings}->{slew_lower_threshold_pct_fall}->{val}) {
    die "Error: (check_lib_settings)Invalid slew_[upper|lower]_threshold_pct_fall! Upper should <= Lower.\n";
  }
  # Check transient, there should be 3 components
  $conf_ptr->{library_settings}->{transient}->{val} =~ s/,$//;
  my @trans = split /,/,$conf_ptr->{library_settings}->{transient}->{val};
  if (2 != $#trans) {
    die "Error: (check_lib_settings)Invalid transient(@trans)!Should be [step],[stop_time],[start_time]\n";
  }
  $conf_ptr->{library_settings}->{transient}->{step} = $trans[0];
  $conf_ptr->{library_settings}->{transient}->{stop_time} = $trans[1];
  $conf_ptr->{library_settings}->{transient}->{start_time} = $trans[2];
  
  # Check bisec, there should be 3 components
  $conf_ptr->{library_settings}->{bisec}->{val} =~ s/,$//;
  my @bisec = split /,/,$conf_ptr->{library_settings}->{bisec}->{val};
  if (2 != $#bisec) {
    die "Error: (check_lib_settings)Invalid bisec(@bisec)!Should be [initial_pass],[initial_fail],[step]\n";
  }
  $conf_ptr->{library_settings}->{bisec}->{initial_pass} = &process_unit($bisec[0],"time");
  $conf_ptr->{library_settings}->{bisec}->{initial_fail} = &process_unit($bisec[1],"time");
  $conf_ptr->{library_settings}->{bisec}->{step} = &process_unit($bisec[2],"time");

  # Check transition 
  $conf_ptr->{library_settings}->{transition}->{val} =~ s/,$//;
  my @transi = split /,/,$conf_ptr->{library_settings}->{transition}->{val};
  if ($conf_ptr->{library_settings}->{table_size}->{val} != ($#transi + 1)) {
    die "Error: (check_lib_settings)Invalid transition!Should be match table_size(=$conf_ptr->{library_settings}->{table_size}->{val})\n";
  }
  for (my $i=0; $i<$conf_ptr->{library_settings}->{table_size}->{val}; $i++) {
    $conf_ptr->{library_settings}->{transition}->{$i} = $transi[$i];
  }

  # Check cload 
  $conf_ptr->{library_settings}->{cload}->{val} =~ s/,$//;
  my @cload = split /,/,$conf_ptr->{library_settings}->{cload}->{val};
  if ($conf_ptr->{library_settings}->{table_size}->{val} != ($#cload + 1)) {
    die "Error: (check_lib_settings)Invalid cload!Should be match table_size(=$conf_ptr->{library_settings}->{table_size}->{val})\n";
  }
  for (my $i=0; $i<$conf_ptr->{library_settings}->{table_size}->{val}; $i++) {
    $conf_ptr->{library_settings}->{cload}->{$i} = $cload[$i];
  }

}

# Run Hspice
sub run_hspice($ $ $)
{
  my ($mypath,$fname,$verilogA_sim) = @_;
  my ($err_no,$warn_no) = (0,0);

  $fname =~ s/\.(\w+)$//;
  $mypath = $mypath.$fname."/";

  if ("on" eq $verilogA_sim) {
    my ($process_dir) = ("/home/xitang/tangxifan-eda-tools/branches/dff_elc/process/");
    chdir $process_dir;
    #`csh -cx 'source /softs/synopsys/hspice/2010.12/hspice/bin/cshrc.meta'`;
  }
  if (!(-e "$mypath"))
  {
    print "$mypath does not exist. Create it...\n";
    mkpath "$mypath";
  }
  my ($fspice) = $mypath.$fname.".sp";
  my ($flis) = $mypath.$fname.".lis";
  my ($hspice_path) = ("hspice");

  my ($mt_def);
  if ($opt_ptr->{"hspice64"} eq "on")
  {$hspice_path = $hspice_path."64";}
  #if ("on" eq $opt_ptr->{"mt"})
  #{$mt_def = "-mt $opt_ptr->{mt_val} ";}
  #my ($server_mode);
  #if ("on" eq $opt_ptr->{"server_mode"})
  #{$server_mode = "-C";}

  #`csh -cx 'cd ./process'`;
  # Use Cshell to run hspice
  if ("on" eq $verilogA_sim) {
    `csh -cx '$hspice_path -i $fspice -o $flis -hdlpath /softs/synopsys/hspice/2010.12/hspice/include'`;
    chdir $cwd;
  }
  elsif ("off" eq $verilogA_sim) {
    `csh -cx '$hspice_path -i $fspice -o $flis'`;
  }
  else {
    die "Error: (run_hspice)Invalid verilogA_sim!\n";
  }
  #`csh -cx 'cd  ..'`;

  #Check warnings&errors in the simulation results
  open (FLIS, "< $flis") or die "Fail to open $flis!\n";
  my ($line,$line_num);
  $line_num = 0;
  print "*****HSPICE simulation notes*****\n";
  while(defined($line = <FLIS>)) {
    chomp $line;
    if ($line =~ /error/i) { 
      print "Error:lis file[LINE $line_num]$line\n";
      $err_no++;
    }
    elsif ($line =~ /warn/i)
    {
      print "Warning:lis file[LINE $line_num]$line\n";
      $warn_no++;
    }
    $line_num++;
  }
  # Exit program if we have errors.
  if ($err_no > 0) {
    die "Error: Hspice simulation fails! Please fix errors.\n";
  }
  # Report warnings 
  print "Info: Hspice simulation success with $err_no errors, $warn_no warnings.\n";
}

# Generate Spice netlist -- Common Part
sub gen_sp_common($ $ $ $ $ $ $ $ $)
{
  my ($spfh,$usage,$cload,$clk_pwl,$clk_pwh,$clk_slew,$d_slew,$clk2d,$thold) = @_;

  # Print Header
  &tab_print($spfh,"DFF ELC HSPICE Bench\n",0);
  &tab_print($spfh,"**********************\n",0);
  &tab_print($spfh,"*   HSPICE Netlist   *\n",0);
  &tab_print($spfh,"* Author: Xifan TANG *\n",0);
  &tab_print($spfh,"*         EPFL, LSI  *\n",0);
  &tab_print($spfh,"* Date: $mydate    *\n",0);
  &tab_print($spfh,"**********************\n",0);
  &tab_print($spfh,"* Test Bench Usage: $usage\n",0);
  &tab_print($spfh,"* Include Technology Library\n",0);
  if ("on" eq $conf_ptr->{general_settings}->{include_lib}->{val}) {
    &tab_print($spfh,".lib ",0);
  }
  else {
    &tab_print($spfh,".include ",0);
  }
  &tab_print($spfh,"'$conf_ptr->{general_settings}->{process_tech}->{val}'\n",0);
  &tab_print($spfh,"* Working Temperature\n",0);
  &tab_print($spfh,".temp $conf_ptr->{library_settings}->{temperature}->{val}\n",0);
  &tab_print($spfh,"* Global Nodes\n",0);
  &tab_print($spfh,".global $conf_ptr->{circuit_definition}->{VDD_port_name}->{val} $conf_ptr->{circuit_definition}->{GND_port_name}->{val}\n",0);
  &tab_print($spfh,"* Print Node Capacitance\n",0);
  &tab_print($spfh,".option captab\n",0);
  &tab_print($spfh,"* Print Waveforms\n",0);
  &tab_print($spfh,".option POST\n",0);
  &tab_print($spfh,"* Parameters for measurements\n",0);
  &tab_print($spfh,".param clk2d=$clk2d\n",0);
  &tab_print($spfh,".param clk_pwl=$clk_pwl\n",0);
  &tab_print($spfh,".param clk_pwh=$clk_pwh\n",0);
  &tab_print($spfh,".param clk_slew=$clk_slew\n",0);
  &tab_print($spfh,".param d_slew=$d_slew\n",0);
  &tab_print($spfh,".param thold=$thold\n",0);
  &tab_print($spfh,".param vsp=$conf_ptr->{library_settings}->{supply_voltage}->{val}\n",0);
  &tab_print($spfh,"* Parameters for Measuring Slew\n",0);
  &tab_print($spfh,".param slew_upper_threshold_pct_rise=$conf_ptr->{library_settings}->{slew_upper_threshold_pct_rise}->{val}\n",0);
  &tab_print($spfh,".param slew_lower_threshold_pct_rise=$conf_ptr->{library_settings}->{slew_lower_threshold_pct_rise}->{val}\n",0);
  &tab_print($spfh,".param slew_upper_threshold_pct_fall=$conf_ptr->{library_settings}->{slew_upper_threshold_pct_fall}->{val}\n",0);
  &tab_print($spfh,".param slew_lower_threshold_pct_fall=$conf_ptr->{library_settings}->{slew_lower_threshold_pct_fall}->{val}\n",0);
  &tab_print($spfh,"* Parameters for Measuring Delay\n",0);
  &tab_print($spfh,".param input_threshold_pct_rise=$conf_ptr->{library_settings}->{input_threshold_pct_rise}->{val}\n",0);
  &tab_print($spfh,".param input_threshold_pct_fall=$conf_ptr->{library_settings}->{input_threshold_pct_fall}->{val}\n",0);
  &tab_print($spfh,".param output_threshold_pct_rise=$conf_ptr->{library_settings}->{output_threshold_pct_rise}->{val}\n",0);
  &tab_print($spfh,".param output_threshold_pct_fall=$conf_ptr->{library_settings}->{output_threshold_pct_fall}->{val}\n",0);
  &tab_print($spfh,"* Include Circuits Library\n",0);
  &tab_print($spfh,".include $cwd/$conf_ptr->{general_settings}->{dff_subckt}->{val}\n",0);

  # Now Call the sub circuit
  &tab_print($spfh,"Xdff_ckt $conf_ptr->{circuit_definition}->{D_port_name}->{val} $conf_ptr->{circuit_definition}->{CLK_port_name}->{val} $conf_ptr->{circuit_definition}->{Q_port_name}->{val} $conf_ptr->{circuit_definition}->{SET_port_name}->{val} $conf_ptr->{circuit_definition}->{RST_port_name}->{val}  $conf_ptr->{circuit_definition}->{dff_subckt_name}->{val}\n",0);
  # Add Load Cap
  &tab_print($spfh,"Cload $conf_ptr->{circuit_definition}->{Q_port_name}->{val} $conf_ptr->{circuit_definition}->{GND_port_name}->{val} $cload\n",0);

  # Add Stimulates
  &tab_print($spfh,"Vsupply $conf_ptr->{circuit_definition}->{VDD_port_name}->{val} $conf_ptr->{circuit_definition}->{GND_port_name}->{val} vsp\n",0);
  &tab_print($spfh,"Vgnd $conf_ptr->{circuit_definition}->{GND_port_name}->{val} gnd 0\n",0);
  
  &tab_print($spfh,"* Common Part Over.\n",0);
  
}

sub gen_sp_setup($ $)
{
  my ($spfh,$type) = @_;
  my ($vh,$vl) = ("vsp",0);
  if ($type eq "fall") { 
    ($vh,$vl) = (0,"vsp");
  }
  # Generate Set and Reset
  &tab_print($spfh,"Vreset $conf_ptr->{circuit_definition}->{RST_port_name}->{val} $conf_ptr->{circuit_definition}->{GND_port_name}->{val} 0\n",0);
  &tab_print($spfh,"Vset $conf_ptr->{circuit_definition}->{SET_port_name}->{val} $conf_ptr->{circuit_definition}->{GND_port_name}->{val} 0\n",0);
  # Generate CLK
  &tab_print($spfh,"Vclk $conf_ptr->{circuit_definition}->{CLK_port_name}->{val} $conf_ptr->{circuit_definition}->{GND_port_name}->{val} pwl(0 0 \'clk_pwl\' 0 \'clk_pwl+clk_slew\' vsp \'clk_pwl+clk_slew+clk_pwh\' vsp \'clk_pwl+clk_slew+clk_pwh+clk_slew\' 0 \'clk_pwl+clk_slew+clk_pwh+clk_slew+clk_pwl\' 0 \'clk_pwl+clk_slew+clk_pwh+clk_slew+clk_pwl+clk_slew\' vsp)\n",0);
  # Generate D
  &tab_print($spfh,"Vd $conf_ptr->{circuit_definition}->{D_port_name}->{val} $conf_ptr->{circuit_definition}->{GND_port_name}->{val} pwl(0 $vl \'2*clk_pwl+2*d_slew+clk_pwh-clk2d\' $vl \'2*clk_pwl-clk2d+3*d_slew+clk_pwh\' $vh)\n",0);
  
}

sub gen_sp_hold($ $ $)
{
  my ($spfh,$type,$thold) = @_;
  my ($vh,$vl) = ("vsp",0);
  if ($type eq "fall") { 
    ($vh,$vl) = (0,"vsp");
  }
  # Generate Set and Reset
  &tab_print($spfh,"Vreset $conf_ptr->{circuit_definition}->{RST_port_name}->{val} $conf_ptr->{circuit_definition}->{GND_port_name}->{val} 0\n",0);
  &tab_print($spfh,"Vset $conf_ptr->{circuit_definition}->{SET_port_name}->{val} $conf_ptr->{circuit_definition}->{GND_port_name}->{val} 0\n",0);
  # Generate CLK
  &tab_print($spfh,"Vclk $conf_ptr->{circuit_definition}->{CLK_port_name}->{val} $conf_ptr->{circuit_definition}->{GND_port_name}->{val} pwl(0 0 \'clk_pwl\' 0 \'clk_pwl+clk_slew\' vsp \'clk_pwl+clk_slew+clk_pwh\' vsp \'clk_pwl+clk_slew+clk_pwh+clk_slew\' 0 \'clk_pwl+clk_slew+clk_pwh+clk_slew+clk_pwl\' 0 \'clk_pwl+clk_slew+clk_pwh+clk_slew+clk_pwl+clk_slew\' vsp)\n",0);
  # Generate D
  &tab_print($spfh,"Vd $conf_ptr->{circuit_definition}->{D_port_name}->{val} $conf_ptr->{circuit_definition}->{GND_port_name}->{val} pwl(0 $vl \'1*clk_pwl+1*d_slew+clk_pwh\' $vl ",0);
  if ($thold > 0) {
    &tab_print($spfh,"\'1*clk_pwl+2*d_slew+clk_pwh\' $vh \'2*clk_pwl+clk_pwh+2*d_slew+thold\' $vh \'2*clk_pwl+clk_pwh+3*d_slew+thold\' $vl)\n",0);
  } else {
    &tab_print($spfh,"\'1*clk_pwl+clk_pwh+2*d_slew\' $vh \'2*clk_pwl+clk_pwh+2*d_slew+thold\' $vh \'2*clk_pwl+thold+3*d_slew+clk_pwh\' $vl)\n",0);
  }
}

sub gen_sp_leakage($ $)
{
  my ($spfh,$input_vectors) = @_;
  # Generate Set and Reset
  &tab_print($spfh,"Vreset $conf_ptr->{circuit_definition}->{RST_port_name}->{val} $conf_ptr->{circuit_definition}->{GND_port_name}->{val} $$input_vectors[0]\n",0);
  &tab_print($spfh,"Vset $conf_ptr->{circuit_definition}->{SET_port_name}->{val} $conf_ptr->{circuit_definition}->{GND_port_name}->{val} $$input_vectors[1]\n",0);
  # Generate CLK
  &tab_print($spfh,"Vclk $conf_ptr->{circuit_definition}->{CLK_port_name}->{val} $conf_ptr->{circuit_definition}->{GND_port_name}->{val} pwl(0 0 \'clk_pwl\' 0 \'clk_pwl+clk_slew\' vsp \'clk_pwl+clk_slew+clk_pwh\' vsp \'clk_pwl+2*clk_slew+clk_pwh\' $$input_vectors[2])\n",0);
  # Generate D
  &tab_print($spfh,"Vd $conf_ptr->{circuit_definition}->{D_port_name}->{val} $conf_ptr->{circuit_definition}->{GND_port_name}->{val} pwl(0 $$input_vectors[4] \'clk_pwl\' $$input_vectors[4] \'clk_pwl+clk_slew\' $$input_vectors[4] \'clk_pwl+clk_slew+clk_pwh\' $$input_vectors[4] \'clk_pwl+2*clk_slew+clk_pwh\' $$input_vectors[3])\n",0);
    
}

sub gen_measure($ $ $ $ $)
{
  my ($spfh,$type,$tran_step,$tran_stop,$clk2q_max) = @_;

  # Check $type
  if (($type ne "rise")&&($type ne "fall")) {
    die "Invalid type($type) for gen_measure!\n";
  }

  my ($vh,$vl) = ("vsp",0);
  if ($type eq "fall") { 
    ($vh,$vl) = (0,"vsp");
  }

  # Generate Tran time
  &tab_print($spfh,".tran $tran_step $tran_stop\n",0); 
  # Generate Measure
  # Measure slew
  &tab_print($spfh,".meas tran slew_q trig v($conf_ptr->{circuit_definition}->{Q_port_name}->{val}) val=\'slew_lower_threshold_pct_$type*vsp\' $type=1 td=\'2*clk_pwl+clk_pwh+2*clk_slew\'\n",0);
  &tab_print($spfh,"+                 targ v($conf_ptr->{circuit_definition}->{Q_port_name}->{val}) val=\'slew_upper_threshold_pct_$type*vsp\' $type=1 td=\'2*clk_pwl+clk_pwh+2*clk_slew\'\n",0);
  # Measure Clk To Q 
  &tab_print($spfh,".meas tran clk2q trig v($conf_ptr->{circuit_definition}->{CLK_port_name}->{val}) val=\'input_threshold_pct_$type*vsp\' rise=2\n",0);
  &tab_print($spfh,"+                targ v($conf_ptr->{circuit_definition}->{Q_port_name}->{val}) val=\'output_threshold_pct_$type*vsp\' $type=1 td=\'2*clk_pwl+clk_pwh+2*clk_slew\'\n",0);
  &tab_print($spfh,".meas tran power_vdd avg p(vsupply) from=\'2*clk_pwl+clk_pwh+2*clk_slew\' to=\'2*clk_pwl+clk_pwh+3*clk_slew+$clk2q_max\'\n",0);
  #&tab_print($spfh,".meas tran avg_vq avg V($conf_ptr->{circuit_definition}->{Q_port_name}->{val}) from=\'2*clk_pwl+clk_pwh+2*slew\' to=\'2*clk_pwl+clk_pwh+3*slew+$clk2q_max\'\n",0);
  &tab_print($spfh,".meas tran avg_vq avg V($conf_ptr->{circuit_definition}->{Q_port_name}->{val}) from=\'$tran_stop-$tran_step\' to=$tran_stop\n",0);

  # End of SP
  &tab_print($spfh,".end DFF ELC HSPICE Bench\n",0);
  close($spfh);
}

sub gen_leakage_measure($ $ $)
{
  my ($spfh,$tran_step,$tran_stop) = @_;

  &tab_print($spfh,".tran $tran_step $tran_stop\n",0); 
  &tab_print($spfh,".meas tran leakage_power avg p(vsupply) from=\'0.8*$tran_stop\' to=$tran_stop\n",0);

  # End of SP
  &tab_print($spfh,".end DFF ELC HSPICE Bench\n",0);
  close($spfh);
}

sub get_sim_results($ $ $)
{
  my ($flis,$keywords,$results) = @_;
  my ($line,$lineno) = ("",0);
  my ($lisfh) = FileHandle->new;
  if ($lisfh->open("< $flis")) {
    print "Open $flis to get results...\n";
    while(defined($line = <$lisfh>)) {
      chomp $line;
      $lineno++;
      foreach my $tmp(@$keywords) {
        if ($line =~ m/$tmp\s*=\s*([\d\.\w\-\+]+)/i) {
          $results->{$tmp} = $1;
        }
      }
    }
  }
  else {
    die "Fail to open $flis!\n";
  }
  # Check data
  foreach my $tmp(@$keywords) {
    if (!(defined($results->{$tmp}))) {
      $results->{$tmp} = "fail";
    }
    print "Sim Results: $tmp = $results->{$tmp}\n";
  }
  close($lisfh);
}

# Run once Simulation - Setup Time, Fall|Rise
sub run_setup_sim($ $ $ $ $ $ $ $ $ $ $)
{
  my ($type,$clk_slew,$d_slew,$clk_pwl,$clk_pwh,$clk2d,$cload,$tran_step,$tran_stop,$results,$clk2q_max) = @_;

  # create folder
  my ($setup_folder) = ("$cwd/"."$conf_ptr->{general_settings}->{spice_dir}->{val}"."setup_time/");
  &generate_path($setup_folder);
  my ($setup_file) = ($setup_folder."setup_time.sp");
  my ($lis_file) = ($setup_file);
  $lis_file =~ s/sp$/lis/;
  my @sim_keywds = ("slew_q","clk2q","power_vdd","avg_vq");

  print "Setup($type) Time : $clk2d\n";
  # Create a SPICE file
  my $spfh = FileHandle->new;
  if ($spfh->open("> $setup_file")) {
    &gen_sp_common($spfh,"setup_$type",$cload,$clk_pwl,$clk_pwh,$clk_slew,$d_slew,$clk2d,0);
    &gen_sp_setup($spfh,$type); 
    &gen_measure($spfh,$type,$tran_step,$tran_stop,$clk2q_max);
    &run_hspice("$cwd/"."$conf_ptr->{general_settings}->{spice_dir}->{val}","setup_time.sp",$conf_ptr->{general_settings}->{verilogA_hspice_sim}->{val});
    &get_sim_results($lis_file,\@sim_keywds,$results);
    if (("fall" eq $type)&&($results->{avg_vq} > $conf_ptr->{library_settings}->{supply_voltage}->{val}*0.1)) {
      return 0;
    }
    if (("rise" eq $type)&&($results->{avg_vq} < $conf_ptr->{library_settings}->{supply_voltage}->{val}*0.9)) {
      return 0;
    }
    $results->{power_vdd} = abs($results->{power_vdd});
    $results->{avg_vq} = abs($results->{avg_vq});
    foreach my $tmp(@sim_keywds) {
      if (($results->{$tmp} eq "failed")||($results->{$tmp} < 0)) {
        return 0;
      }
    } 
    return 1;
  }
  else {
    die "Fail to create $setup_file!\n";
  }

}

# Run once Simulation - Hold Time, Fall|Rise
sub run_hold_sim($ $ $ $ $ $ $ $ $ $ $ $)
{
  my ($type,$clk_slew,$d_slew,$clk_pwl,$clk_pwh,$clk2d,$thold,$cload,$tran_step,$tran_stop,$results,$clk2q_max) = @_;

  # create folder
  my ($hold_folder) = ("$cwd/"."$conf_ptr->{general_settings}->{spice_dir}->{val}"."hold_time/");
  &generate_path($hold_folder);
  my ($hold_file) = ($hold_folder."hold_time.sp");
  my ($lis_file) = ($hold_file);
  $lis_file =~ s/sp$/lis/;
  my @sim_keywds = ("slew_q","clk2q","power_vdd","avg_vq");

  print "Hold($type) Time : $thold\n";
  # Create a SPICE file
  my $spfh = FileHandle->new;
  if ($spfh->open("> $hold_file")) {
    &gen_sp_common($spfh,"hold_$type",$cload,$clk_pwl,$clk_pwh,$clk_slew,$d_slew,$clk2d,$thold);
    &gen_sp_hold($spfh,$type,$thold); 
    &gen_measure($spfh,$type,$tran_step,$tran_stop,$clk2q_max);
    &run_hspice("$cwd/"."$conf_ptr->{general_settings}->{spice_dir}->{val}","hold_time.sp",$conf_ptr->{general_settings}->{verilogA_hspice_sim}->{val});
    &get_sim_results($lis_file,\@sim_keywds,$results);
    if (("fall" eq $type)&&($results->{avg_vq} > $conf_ptr->{library_settings}->{supply_voltage}->{val}*0.1)) {
      return 0;
    }
    if (("rise" eq $type)&&($results->{avg_vq} < $conf_ptr->{library_settings}->{supply_voltage}->{val}*0.9)) {
      return 0;
    }
    $results->{power_vdd} = abs($results->{power_vdd});
    $results->{avg_vq} = abs($results->{avg_vq});
    foreach my $tmp(@sim_keywds) {
      if (($results->{$tmp} eq "failed")||($results->{$tmp} < 0)) {
        return 0;
      }
    } 
    return 1;
  }
  else {
    die "Fail to create $hold_file!\n";
  }

}

sub binary_search_setup_time($ $ $ $ $ $ $ $ $)
{
  my ($tag,$last_success,$last_fail,$clk_slew,$input_slew,$type,$cload,$tran_step,$stop_period) = @_;
  my ($tran_stop);
  my (%sim_results);
  my ($sim_results_ref) = \%sim_results;
  my ($success_temp,$fail_temp) = ($last_success,$last_fail);
  my ($clk_ratio) = (1);
  my ($clk2q,$slew_q,$power,$total_power,$power_num);
  my ($max_slew);
  
  if ($clk_slew > $input_slew) {
    $max_slew = $clk_slew;
  }
  else {
    $max_slew = $input_slew;
  }
  
  # Got default clk to Q delay and Q slew rate.  
  $tran_stop = 2*($last_success + $max_slew + $clk_ratio*$last_success)+$max_slew;
  if (0 == &run_setup_sim($type,$clk_slew,$input_slew,$last_success,$clk_ratio*$last_success,$last_success,$cload,$tran_step,$tran_stop,\%sim_results,0)) {
    die "Error: (clk_slew: $clk_slew,d_slew: $input_slew)Initial success is a failure!\n";
  }
  my ($slew_q_std,$clk2q_std) = ($sim_results_ref->{slew_q},$sim_results_ref->{clk2q});
  my ($setup_time) = ($last_success);
  $total_power = 0;
  $power = 0;
  $power_num = 0; 
  $clk2q = $sim_results_ref->{clk2q};
  $slew_q = $sim_results_ref->{slew_q};
  $power = $sim_results_ref->{power_vdd};
  $total_power += $power;
  $power_num++;

  # Get fail
  $tran_stop = 2*($last_success + $max_slew + $clk_ratio*$last_success)+$max_slew;
  if (1 == &run_setup_sim($type,$clk_slew,$input_slew,$last_success,$clk_ratio*$last_success,$last_fail,$cload,$tran_step,$tran_stop,\%sim_results,0)) {
    die "Error: (clk_slew: $clk_slew,d_slew: $input_slew)Initial Fail is a success!\n"; 
  }

  while($stop_period < abs($success_temp-$fail_temp)){
    $tran_stop = 2*($last_success + $max_slew + $clk_ratio*$last_success)+$max_slew;
    my ($temp_try) = ($success_temp+$fail_temp)/2;
    undef %sim_results;
    #if ((1 == &run_setup_sim($type,$clk_slew,$input_slew,$last_success,$clk_ratio*$last_success,$temp_try,$cload,$tran_step,$tran_stop,\%sim_results))&&($sim_results_ref->{slew_q} ne "failed")&&($sim_results_ref->{slew_q} < ($slew_q_std*1.1))&&($sim_results_ref->{clk2q} < ($clk2q_std*1.1))) {
    if ((1 == &run_setup_sim($type,$clk_slew,$input_slew,$last_success,$clk_ratio*$last_success,$temp_try,$cload,$tran_step,$tran_stop,\%sim_results,$clk2q_std*1.1))&&($sim_results_ref->{slew_q} ne "failed")&&($sim_results_ref->{clk2q} < ($clk2q_std*1.1))) {
      $success_temp = $temp_try;
      $setup_time = $success_temp;
      $clk2q = $sim_results_ref->{clk2q};
      $slew_q = $sim_results_ref->{slew_q};
      $power = $sim_results_ref->{power_vdd};
      $total_power += $power;
      $power_num++;
    }
    else {
      $fail_temp = $temp_try;
    }
    print "(clk_slew: $clk_slew,d_slew: $input_slew)Last success: $success_temp, Last fail: $fail_temp\n";
  }
  print "Setup_time($type) is $setup_time\n";
  print "Clk2Q std is $clk2q_std, Clk2q is $clk2q\n";
  print "Q slew std is $slew_q_std, Q slew is $slew_q\n";
  print "Power : $power\n";

  # Record in report hash
  print "Input Slew: $input_slew\n";
  $rpt_ptr->{$tag}->{$type}->{setup_time}->{setup_time} = $setup_time;
  $rpt_ptr->{$tag}->{$type}->{setup_time}->{clk2q} = $clk2q;
  $rpt_ptr->{$tag}->{$type}->{setup_time}->{clk2q_std} = $clk2q_std;
  $rpt_ptr->{$tag}->{$type}->{setup_time}->{slew_q} = $slew_q;
  $rpt_ptr->{$tag}->{$type}->{setup_time}->{slew_q_std} = $slew_q_std;
  $rpt_ptr->{$tag}->{$type}->{setup_time}->{power} = $power;
  $rpt_ptr->{$tag}->{$type}->{setup_time}->{avg_power} = $total_power/$power_num;

}

sub binary_search_hold_time($ $ $ $ $ $ $ $ $ $)
{
  my ($tag,$last_success,$last_fail,$clk_slew,$input_slew,$type,$setup_time,$cload,$tran_step,$stop_period) = @_;
  my ($tran_stop);
  my (%sim_results);
  my ($sim_results_ref) = \%sim_results;
  my ($success_temp,$fail_temp) = ($last_success,$last_fail);
  my ($clk_ratio) = (1);
  my ($clk2q,$slew_q,$power,$total_power,$power_num);
  
  my ($max_slew);
  
  if ($clk_slew > $input_slew) {
    $max_slew = $clk_slew;
  }
  else {
    $max_slew = $input_slew;
  }
  
  # Check Last Fail
  #if ($setup_time < 0) {
  #  $last_fail = abs($setup_time) +$tran_step*1;
  #}
  #if (($last_fail + $last_success) == 0) { 

  # Got default clk to Q delay and Q slew rate.  
  $tran_stop = 2*($last_success + $max_slew + $clk_ratio*$last_success)+$max_slew;
  if (0 == &run_hold_sim($type,$clk_slew,$input_slew,$last_success,$clk_ratio*$last_success,$setup_time,$last_success,$cload,$tran_step,$tran_stop,\%sim_results,0)) {
    die "Error: (clk_slew: $clk_slew,d_slew: $input_slew)Initial success is a failure!\n";
  }
  my ($slew_q_std,$clk2q_std) = ($sim_results_ref->{slew_q},$sim_results_ref->{clk2q});
  my ($hold_time) = ($last_success);

  # Get fail
  $tran_stop = 2*($last_success + $max_slew + $clk_ratio*$last_success)+$max_slew;
  if (1 == &run_hold_sim($type,$clk_slew,$input_slew,$last_success,$clk_ratio*$last_success,$setup_time,$last_fail,$cload,$tran_step,$tran_stop,\%sim_results,0)) {
    die "Error: (clk_slew: $clk_slew,d_slew: $input_slew)Initial Fail is a success!\n"; 
  }

  $total_power = 0;
  $power = 0;
  $power_num = 0; 

  while($stop_period < abs($success_temp-$fail_temp)){
    $tran_stop = 2*($last_success + $max_slew + $clk_ratio*$last_success)+$max_slew;
    my ($temp_try) = ($success_temp+$fail_temp)/2;
    undef %sim_results;
    #if ((1 == &run_hold_sim($type,$input_slew,$last_success,$clk_ratio*$last_success,$setup_time,$temp_try,$cload,$tran_step,$tran_stop,\%sim_results))&&($sim_results_ref->{slew_q} ne "failed")&&($sim_results_ref->{slew_q} < ($slew_q_std*1.1))&&($sim_results_ref->{clk2q} < ($clk2q_std*1.1))) {
    if ((1 == &run_hold_sim($type,$clk_slew,$input_slew,$last_success,$clk_ratio*$last_success,$setup_time,$temp_try,$cload,$tran_step,$tran_stop,\%sim_results,$clk2q_std*1.1))&&($sim_results_ref->{slew_q} ne "failed")&&($sim_results_ref->{clk2q} < ($clk2q_std*1.1))) {
      $success_temp = $temp_try;
      $hold_time = $success_temp;
      $clk2q = $sim_results_ref->{clk2q};
      $slew_q = $sim_results_ref->{slew_q};
      $power = $sim_results_ref->{power_vdd};
      $total_power += $power;
      $power_num++;
    }
    else {
      $fail_temp = $temp_try;
    }
    print "(clk_slew: $clk_slew,d_slew: $input_slew)Last success: $success_temp, Last fail: $fail_temp\n";
  }
  print "Hold_time($type) is $hold_time\n";
  print "Clk2Q std is $clk2q_std, Clk2q is $clk2q\n";
  print "Q slew std is $slew_q_std, Q slew is $slew_q\n";
  print "Power : $power\n";

  # Record in report hash
  $rpt_ptr->{$tag}->{$type}->{hold_time}->{hold_time} = $hold_time;
  $rpt_ptr->{$tag}->{$type}->{hold_time}->{clk2q} = $clk2q;
  $rpt_ptr->{$tag}->{$type}->{hold_time}->{clk2q_std} = $clk2q_std;
  $rpt_ptr->{$tag}->{$type}->{hold_time}->{slew_q} = $slew_q;
  $rpt_ptr->{$tag}->{$type}->{hold_time}->{slew_q_std} = $slew_q_std;
  $rpt_ptr->{$tag}->{$type}->{hold_time}->{power} = $power;
  $rpt_ptr->{$tag}->{$type}->{hold_time}->{avg_power} = $total_power/$power_num;
}

sub leakage_power($ $ $ $)
{
  my ($tag,$cload,$tran_step,$tran_stop) = @_;

  # create folder
  my ($leakage_folder) = ("$cwd/"."$conf_ptr->{general_settings}->{spice_dir}->{val}"."leakage_power/");
  &generate_path($leakage_folder);
  my ($leakage_file) = ($leakage_folder."leakage_power.sp");
  my ($lis_file) = ($leakage_file);
  $lis_file =~ s/sp$/lis/;
  my (%hash_results);
  my ($results) = (\%hash_results);
  my @sim_keywds = ("leakage_power");
  my ($num_bit) = 5;
  my @input_vectors;
  my ($leakage_counter) = (0);

  print "Measuring Leakage Power\n";
  for (my $i=0; $i<2**$num_bit; $i++) {
    # generate input vectors
    my ($tmp) = ($i);
    for (my $j=0; $j<$num_bit; $j++) {
      if ((2**($num_bit-$j-1) < $tmp)||(2**($num_bit-$j-1) == $tmp)) { 
        $input_vectors[$j] = "vsp";
        $tmp = $tmp - 2**($num_bit-$j-1);
      }
      else { 
        $input_vectors[$j] = 0;
      }
    } 
    if (($input_vectors[0] eq "vsp")||($input_vectors[1] eq "vsp")) {
      print "Testing leakage: skip conditions when RESET or SET is enabled...\n"; 
      next;
    }
    print "Testing leakage: tmp = $i, vectors = ";
    for (my $j=0; $j < $num_bit; $j++) {
      print "$input_vectors[$j](index:$j), ";
    }
    print "\n";
    # Create a SPICE file
    my $spfh = FileHandle->new;
    if ($spfh->open("> $leakage_file")) {
      &gen_sp_common($spfh,"leakage_power",$cload,3e-9,3e-9,1e-10,0,0,0);
      &gen_sp_leakage($spfh,\@input_vectors); 
      &gen_leakage_measure($spfh,$tran_step,$tran_stop);
      &run_hspice("$cwd/"."$conf_ptr->{general_settings}->{spice_dir}->{val}","leakage_power.sp",$conf_ptr->{general_settings}->{verilogA_hspice_sim}->{val});
      &get_sim_results($lis_file,\@sim_keywds,$results);
      $results->{leakage_power} = abs($results->{leakage_power});
      foreach my $tmp(@sim_keywds) {
        if ($results->{$tmp} eq "fail") {
          die "Error: failure in simulating $tag!\n";
        }
      } 
    }
    else {
      die "Fail to create $leakage_file!\n";
    }
    $rpt_ptr->{$tag}->{avg_leakage} += $results->{leakage_power}; 
    $rpt_ptr->{$tag}->{"leakage$leakage_counter"}->{input_vectors} = join(',', @input_vectors);
    $rpt_ptr->{$tag}->{"leakage$leakage_counter"}->{leakage_power} = $results->{leakage_power};
    $leakage_counter++;
  }
  $rpt_ptr->{$tag}->{counter} = $leakage_counter;
  $rpt_ptr->{$tag}->{avg_leakage} = $rpt_ptr->{$tag}->{avg_leakage}/$leakage_counter;
  print "Info: number of Leakage test conditions = $leakage_counter, Average Leakage Power=$rpt_ptr->{$tag}->{avg_leakage}\n";
}

sub run_once($ $ $ $ $ $ $ $)
{
  my ($tag,$type,$clk_slew,$d_slew,$clk_pwl,$clk2d,$cload,$tran_step) = @_;
  my ($max_slew);
  my (%sim_results);
  my ($sim_results_ref) = \%sim_results;
  my ($clk_ratio) = (1);
  
  if ($clk_slew > $d_slew) {
    $max_slew = $clk_slew;
  }
  else {
    $max_slew = $d_slew;
  }
  my ($tran_stop) = 2*($clk_pwl + $max_slew + $clk_ratio*$clk_pwl)+$max_slew;
  if (0 == &run_setup_sim($type,$clk_slew,$d_slew,$clk_pwl,$clk_ratio*$clk_pwl,$clk2d,$cload,$tran_step,$tran_stop,\%sim_results,0)) {
    die "Error: Failure in run once!\n";
  }
  my ($slew_q_std,$clk2q_std) = ($sim_results_ref->{slew_q},$sim_results_ref->{clk2q});
  $rpt_ptr->{$tag}->{$type}->{clk2q_std} = $clk2q_std;
  $rpt_ptr->{$tag}->{$type}->{slew_q_std} = $slew_q_std;
}

sub print_constraint($ $ $)
{
  my ($splib,$type,$time) = @_;
  print $splib "$type\_constraint($time\_template) {\n";
  print $splib "  values(\\\n";
  my ($clk_slew,$d_slew);
  my ($tag); 
  for (my $i=0; $i<$conf_ptr->{library_settings}->{table_size}->{val}; $i++) {
    $clk_slew = $conf_ptr->{library_settings}->{transition}->{$i}*1e-9;
    print $splib "  \"";
    for (my $j=0; $j<$conf_ptr->{library_settings}->{table_size}->{val}; $j++) {
      $d_slew = $conf_ptr->{library_settings}->{transition}->{$j}*1e-9;
      $tag = "clk_slew$clk_slew\_d_slew$d_slew";
      print $splib "$rpt_ptr->{$tag}->{$type}->{\"$time\_time\"}->{\"$time\_time\"}";
      if ($j < ($conf_ptr->{library_settings}->{table_size}->{val}-1)) {
        print $splib ",";
      }
    }
    print $splib "  \"\\\n";
  }
  print $splib "  );\n";
}

sub print_delay($ $ $)
{
  my ($splib,$type,$time) = @_;
  if ($time eq "slew_q_std") {
    print $splib "$type\_transition(delay\_template) {\n";
  }
  else {
    print $splib "cell\_$type(delay\_template) {\n";
  }
  print $splib "  values(\\\n";
  my ($clk_slew,$cload);
  my ($tag); 
  for (my $i=0; $i<$conf_ptr->{library_settings}->{table_size}->{val}; $i++) {
    $clk_slew = $conf_ptr->{library_settings}->{transition}->{$i}*1e-9;
    print $splib "  \"";
    for (my $j=0; $j<$conf_ptr->{library_settings}->{table_size}->{val}; $j++) {
      $cload = $conf_ptr->{library_settings}->{cload}->{$j}*1e-12;
      $tag = ("clk_slew$clk_slew\_cload$cload");
      print $splib "$rpt_ptr->{$tag}->{$type}->{$time}";
      if ($j < ($conf_ptr->{library_settings}->{table_size}->{val}-1)) {
        print $splib ",";
      }
    }
    print $splib "  \"\\\n";
  }
  print $splib "  );\n";
}

sub print_leakage($)
{
  my ($splib) = @_;

  print $splib "/*\n";
  print $splib "Number of Leakage conditions: $rpt_ptr->{leakage_power}->{counter}\n";   
  print $splib "Input Vector: <RESET>,<SET>,<CLK>,<D>,<InitialVal>\n"; 
  for (my $i=0; $i < $rpt_ptr->{leakage_power}->{counter}; $i++) {
    print $splib "Input Vector: ".$rpt_ptr->{leakage_power}->{"leakage$i"}->{input_vectors}.", Leakage: ".$rpt_ptr->{leakage_power}->{"leakage$i"}->{leakage_power}."\n"; 
  }  
  print $splib "*/\n";

  print $splib "cell_leakage_power: $rpt_ptr->{leakage_power}->{avg_leakage}\n"; 
}

sub print_results()
{
  my ($lib_file) = $opt_ptr->{lib_val};
  my $splib = FileHandle->new;
  if ($splib->open("> $lib_file")) {
    &print_leakage($splib);
    if ("on" ne $opt_ptr->{leakage_only}) { 
      &print_constraint($splib,"rise","hold");
      &print_constraint($splib,"fall","hold");
      &print_constraint($splib,"rise","setup");
      &print_constraint($splib,"fall","setup");
      &print_delay($splib,"rise","slew_q_std");
      &print_delay($splib,"fall","slew_q_std");
      &print_delay($splib,"rise","clk2q_std");
      &print_delay($splib,"fall","clk2q_std");
    }
  }
  else {
    die "Fail to create $lib_file!\n";
  }
  close($splib);
}

sub main() 
{
  # Read Options
  &opts_read();
 
  # Read basic configuration file. All confs stored in conf_ptr
  &read_conf();

  # Check library settings in configuration files
  &check_lib_settings();

  # Run simulations(ELC)
  &leakage_power("leakage_power",0,1e-15,10e-9);
  if ("on" ne $opt_ptr->{leakage_only}) { 
    my ($clk_slew,$d_slew);
    my ($tag); 
    for (my $i=0; $i<$conf_ptr->{library_settings}->{table_size}->{val}; $i++) {
      $clk_slew = $conf_ptr->{library_settings}->{transition}->{$i}*1e-9;
      for (my $j=0; $j<$conf_ptr->{library_settings}->{table_size}->{val}; $j++) {
        $d_slew = $conf_ptr->{library_settings}->{transition}->{$j}*1e-9;
        $tag = ("clk_slew$clk_slew\_d_slew$d_slew");
        my ($binary_start,$binary_stop,$stop_period) 
         = ($conf_ptr->{library_settings}->{bisec}->{initial_pass},
            $conf_ptr->{library_settings}->{bisec}->{initial_fail},
            $conf_ptr->{library_settings}->{bisec}->{step});
        my ($cload) = $conf_ptr->{library_settings}->{cload}->{0}*1e-12;
        &binary_search_setup_time($tag,$binary_stop,$binary_start,$clk_slew,$d_slew,"rise",$cload,1e-15,$stop_period);
        &binary_search_setup_time($tag,$binary_stop,$binary_start,$clk_slew,$d_slew,"fall",$cload,1e-15,$stop_period);
        &binary_search_hold_time($tag,$binary_stop,$binary_start,$clk_slew,$d_slew,"rise",$rpt_ptr->{$tag}->{rise}->{setup_time}->{setup_time},$cload,1e-15,$stop_period);
        &binary_search_hold_time($tag,$binary_stop,$binary_start,$clk_slew,$d_slew,"fall",$rpt_ptr->{$tag}->{fall}->{setup_time}->{setup_time},$cload,1e-15,$stop_period);
      }
      $d_slew = $conf_ptr->{library_settings}->{transition}->{$i}*1e-9;
      for (my $j=0; $j<$conf_ptr->{library_settings}->{table_size}->{val}; $j++) {
        my ($cload) = $conf_ptr->{library_settings}->{cload}->{$j}*1e-12;
        $tag = ("clk_slew$clk_slew\_cload$cload");
        my ($binary_start,$binary_stop,$stop_period) 
         = ($conf_ptr->{library_settings}->{bisec}->{initial_pass},
            $conf_ptr->{library_settings}->{bisec}->{initial_fail},
            $conf_ptr->{library_settings}->{bisec}->{step});
        &run_once($tag,"rise",$clk_slew,$d_slew,$binary_stop,$binary_stop,$cload,1e-15);
        &run_once($tag,"fall",$clk_slew,$d_slew,$binary_stop,$binary_stop,$cload,1e-15);
      }
    }
  }

  &print_results();

  #&leakage_power("leakage_power",0,1e-15,1e-9);
  #&binary_search_setup_time($tag,3e-9,-3e-9,100e-12,100e-12,"rise",0,1e-15,1e-13);
  #&binary_search_setup_time($tag,3e-9,-3e-9,100e-12,100e-12,"fall",0,1e-15,1e-13);
  #&binary_search_hold_time($tag,3e-9,-2e-9,100e-12,100e-12,"rise",-26e-12,0,1e-15,1e-13);
  #&binary_search_hold_time($tag,3e-9,-2e-9,100e-12,100e-12,"fall",42e-12,0,1e-15,1e-13);

  # Print results
  #print "Slew: 100e-12\n";
  #print "Leakage power: ".$rpt_ptr->{leakage_power}."\n";
  #print "Setup_time(rise): ".$rpt_ptr->{$tag}->{rise}->{setup_time}->{setup_time}."\n";
  #print "Clk2Q(rise): ".$rpt_ptr->{$tag}->{rise}->{setup_time}->{clk2q}."\n";
  #print "Clk2Q_std(rise): ".$rpt_ptr->{$tag}->{rise}->{setup_time}->{clk2q_std}."\n";
  #print "Q_Slew(rise): ".$rpt_ptr->{$tag}->{rise}->{setup_time}->{slew_q}."\n";
  #print "Power(rise): ".$rpt_ptr->{$tag}->{rise}->{setup_time}->{power}."\n";
  #print "AVG Power(rise): ".$rpt_ptr->{$tag}->{rise}->{setup_time}->{avg_power}."\n";
  #print "Setup_time(fall): ".$rpt_ptr->{$tag}->{fall}->{setup_time}->{setup_time}."\n";
  #print "Clk2Q(fall): ".$rpt_ptr->{$tag}->{fall}->{setup_time}->{clk2q}."\n";
  #print "Clk2Q_std(fall): ".$rpt_ptr->{$tag}->{fall}->{setup_time}->{clk2q_std}."\n";
  #print "Q_Slew(fall): ".$rpt_ptr->{$tag}->{fall}->{setup_time}->{slew_q}."\n";
  #print "Power(fall): ".$rpt_ptr->{$tag}->{fall}->{setup_time}->{power}."\n";
  #print "AVG Power(fall): ".$rpt_ptr->{$tag}->{fall}->{setup_time}->{avg_power}."\n";
  #print "Hold_time(rise): ".$rpt_ptr->{$tag}->{rise}->{hold_time}->{hold_time}."\n";
  #print "Clk2Q(rise): ".$rpt_ptr->{$tag}->{rise}->{hold_time}->{clk2q}."\n";
  #print "Clk2Q_std(rise): ".$rpt_ptr->{$tag}->{rise}->{hold_time}->{clk2q_std}."\n";
  #print "Q_Slew(rise): ".$rpt_ptr->{$tag}->{rise}->{hold_time}->{slew_q}."\n";
  #print "Power(rise): ".$rpt_ptr->{$tag}->{rise}->{hold_time}->{power}."\n";
  #print "AVG Power(rise): ".$rpt_ptr->{$tag}->{rise}->{hold_time}->{avg_power}."\n";
  #print "Hold_time(fall): ".$rpt_ptr->{$tag}->{fall}->{hold_time}->{hold_time}."\n";
  #print "Clk2Q(fall): ".$rpt_ptr->{$tag}->{fall}->{hold_time}->{clk2q}."\n";
  #print "Clk2Q_std(fall): ".$rpt_ptr->{$tag}->{fall}->{hold_time}->{clk2q_std}."\n";
  #print "Q_Slew(fall): ".$rpt_ptr->{$tag}->{fall}->{hold_time}->{slew_q}."\n";
  #print "Power(fall): ".$rpt_ptr->{$tag}->{fall}->{hold_time}->{power}."\n";
  #print "AVG Power(fall): ".$rpt_ptr->{$tag}->{fall}->{hold_time}->{avg_power}."\n";

  #&run_setup_sim("rise",1e-10,"3e-9","2e-9","0","1e-11","35e-9");
}

&main();
exit(0);
