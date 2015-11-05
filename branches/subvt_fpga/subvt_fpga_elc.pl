#!usr/bin/perl -w
# Perl Script for sub-Vt FPGA HSPICE simulations 
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
# For ceil, floor function
use POSIX;

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

# Reserved word list for checking...
my @reserved_words;

# Matrix informations
my %mclusters;
my ($mclusters_ptr) = (\%mclusters);

my ($elc_nmos_pmos_sp) = ("elc_nmos_pmos.sp");
my ($elc_nmos_subckt_name, $elc_pmos_subckt_name) = ("elc_nmos", "elc_pmos");
my ($elc_prog_nmos_subckt_name, $elc_prog_pmos_subckt_name) = ("elc_prog_nmos", "elc_prog_pmos");

# Configuration file keywords list
# Category for conf file.
# main category : 1st class
my @mctgy;
# sub category : 2nd class
my @sctgy;
# Initialize these categories
@mctgy = ("general_settings",
          "dff_settings",
          "lut_settings",
          "mux_settings",
          "inv_settings",
          "rram_settings",
         );
# refer to the keywords of arch_model
@{$sctgy[0]} = (#"hspice_path",
                "spice_dir",
                "process_tech",
                "process_type",
                "verilogA_hspice_sim",
                "time_unit",
                "voltage_unit",
                "leakage_power_unit",
                "capacitive_load_unit",
                "slew_upper_threshold_pct_rise",
                "slew_lower_threshold_pct_rise",
                "slew_upper_threshold_pct_fall",
                "slew_lower_threshold_pct_fall",
                "input_threshold_pct_rise",
                "input_threshold_pct_fall",
                "output_threshold_pct_rise",
                "output_threshold_pct_fall",
                "temperature",
                "VDD_port_name",
                "GND_port_name",
                "LOAD_VDD_port_name",
                "vsp_sweep",
                "clk_slew",
                "input_slew",
                "nmos_name",
                "pmos_name",
                "nl",
                "pl",
                "min_wn",
                "min_wp",
                "pn_ratio",
                "std_vdd",
                "vthn",
                "vthp",
               );
# refer to the keywords of dff_settings
@{$sctgy[1]} = ( # Alias
                #"dff_sp",
                "dff_subckt_name",
                "D_port_name",
                "CLK_port_name",
                "RST_port_name",
                "SET_port_name",
                "Q_port_name",
                "bisec",
                "delay_tolerance",
                "load_type",
                "load_cap",
                "load_inv_size",
                "load_inv_num",
                );
# refer to the keywords of lut_settings 
@{$sctgy[2]} = (# Alias
                #"lut_sp",
                "lut_subckt_name",
                #"lut_size",
                "IN_port_prefix",
                "OUT_port_name",
                "SRAM_port_prefix",
                "SRAM_bits",
                "input_vectors",
                "load_type",
                "load_cap",
                "load_inv_size",
                "load_inv_num",
               );
# refer to the keywords of mux_settings
@{$sctgy[3]} = (# Alias
                "mux_subckt_name",
                "IN_port_prefix",
                "OUT_port_name",
                "SRAM_port_prefix",
                "invert_SRAM_port_prefix",
                #"SRAM_bits",
                #"input_vectors",
                "inv_size_in",
                "inv_size_out",
                "load_type",
                "load_cap",
                "load_inv_size",
                "load_inv_num",
                "worst_delay",
               );

# refer to the keywords of inv_settings
@{$sctgy[4]} = (# Alias
                "inv_subckt_name",
                "IN_port_name",
                "OUT_port_name",
                "local_vdd_name",
                "local_gnd_name",
               );

# refer to the keywords of rram_settings
@{$sctgy[5]} = (# Alias
                "Wprog",
                "Vprog",
                "Ids",
                "Wprog_step",
                "Vprog_step",
                "Vprog_slack",
                "Rmin",
                "Coff_nmos",
                "Cs_pmos",
                "Cs_nmos",
                "Cg_pmos",
                "Cg_nmos",
                "Vdd_std",
                "pn_ratio",
                "ron_upbound",
                "ron_lowbound",
                "roff",
                "fix_vprog",
                "rram_verilogA_model_path",
                "rram_subckt_name",
                "rram_initial_params",
                "Vdd_prog",
                "Vdd_break",
                "Tprog",
                "prog_nmos_name",
                "prog_pmos_name",
                "prog_nl",
                "prog_pl",
                "prog_min_wn",
                "prog_min_wp",
                "prog_pn_ratio",
                "rram_initial_on_gap",
                "rram_initial_off_gap",
                "prog_process_tech",
                "prog_process_type",
                "prog_trans_model_ref",
                "gap_keyword",
               );

# ----------Subrountines------------#

# Min(min,cand)
sub my_min($ $)
{
  my ($min,$cand) = @_;
  if ((-1 == $$min)||($$min > $cand)) {
    $$min = $cand;
    return 1;
  }
  return 0;
}

# Print TABs and strings
sub tab_print($ $ $)
{
  my ($FILE,$str,$num_tab) = @_;
  my ($my_tab) = ("    ");
  if (!(defined($num_tab))) {
    $num_tab = 0;
  }
  
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
  print "  perl subvt_fpga.pl [-options <value>]\n";
  print "    Mandatory options: \n";
  print "      -conf : specify the basic configuration files for dff_elc\n";
  print "      -rpt : file name of report\n";
  print "    Other Options:\n";
  print "      -dff <dff_spice> : run HSPICE simulations for DFF\n";
  print "      -lut <lut_spice> : run HSPICE simulations for Look-Up Table\n";
  print "      -lut_size <int> : mandatory when option lut is enabled\n";
  print "      Options for RRAM 2T1R Structure:\n";
  print "      -rram2t1r <RRAM_verilogA> : run RRAM 2T1R analysis.\n";
  print "      -vprog_sweep <max_vprog>: sweep the vprog for RRAM 2T1R\n";
  print "      -rram2n1r: use two n-type transistors in rram2t1r\n";
  print "      -transmission_gate: use transmission gates in rram2n1r\n";
  print "      -driver_inv_size: define the size of inverters that drive rram2t1r structure\n";
  print "      Options for Multiplexers:\n";
  print "      -mux <mux2_spice> : run HSPICE simulations for Multiplexer. 2-input multiplexer spice netlist should be provided\n";
  print "      -mux_size <int> : number of inputs of multiplexer, mandatory when option mux is enabled.\n";
  print "      -rram_enhance : turn on enhancements for RRAM (Valid for MUX only)\n";
  print "      -one_level_mux:<spice>: build one-level MUX(Applicable for both SRAM and RRAM), the SPICE netlist of SRAM/RRAM-based switch should be provided!\n";
  print "      -two_level_mux <spice>: build two-level MUX(Applicable for both SRAM and RRAM), the SPICE netlist of SRAM/RRAM-based switch should be provided!\n";
  print "      -wprog_sweep <max_wprog>: sweep the wprog when turn on enhancements for RRAM (Valid for MUX only)\n";
  print "      -enum_mux_leakage: test all cases for multiplexer leakages\n";
  print "      -auto_out_tapered_buffer <level>: automatically add a tapered buffer at output port for high-fan-out nets.\n";
  print "      -mux_unbuffered: turn off adding buffers to inputs and outputs of MUXes\n";
  print "      -hspice64 : use 64-bit hspice, by default it is 32bit.\n";
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
    print "Help desk:\n";
    &print_usage();
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

  # Read Opt into Hash(opt_ptr) : "opt_name","with_val","mandatory"

  # Check mandatory options
  &read_opt_into_hash("conf","on","on"); # Check -conf
  &read_opt_into_hash("rpt","on","on");  # Check -rpt
  
  # Check Optional
  &read_opt_into_hash("dff","on","off");  # Check -dff
  &read_opt_into_hash("lut","on","off");  # Check -lut
  &read_opt_into_hash("mux","on","off");  # Check -mux
  &read_opt_into_hash("rram2t1r","on","off");  # Check -rram2t1r
  &read_opt_into_hash("auto_out_tapered_buffer","on","off");  # Check -auto_out_tapered_buffer

  if ("on" eq $opt_ptr->{lut}) {
    &read_opt_into_hash("lut_size","on","on");  # Check -lut_size
  }

  if ("on" eq $opt_ptr->{mux}) {
    &read_opt_into_hash("mux_size","on","on");  # Check -mux_size
    &read_opt_into_hash("rram_enhance","off","off");  # Check -rram_enhance
    &read_opt_into_hash("wprog_sweep","on","off");  # Check -wprog_sweep
    &read_opt_into_hash("enum_mux_leakage","on","off");  # Check -enum_mux_leakage
    &read_opt_into_hash("one_level_mux","on","off");  # Check -one_level_mux
    &read_opt_into_hash("two_level_mux","on","off");  # Check -one_level_mux
    &read_opt_into_hash("mux_unbuffered","off","off");  # Check -mux_unbuffered
  }

  if ("on" eq $opt_ptr->{rram2t1r}) {
    &read_opt_into_hash("vprog_sweep","on","off");  # Check -vprog_sweep
    &read_opt_into_hash("wprog_sweep","on","off");  # Check -wprog_sweep
    &read_opt_into_hash("rram2n1r","off","off");  # Check -rram2n1r
    if ("on" eq $opt_ptr->{rram2n1r}) {
      &read_opt_into_hash("transmission_gate","off","off");  # Check -driver_inv_size
      &read_opt_into_hash("driver_inv_size","on","on");  # Check -driver_inv_size
    }
  }

  &read_opt_into_hash("hspice64","off","off");  # Check -hspice32

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

sub determine_rram_vprog($ $ $ $)
{
  my ($vprog,$vdd,$slack,$fix_prog) = @_;
  my ($vprog_low_bound) = sprintf("%.2f",(1+$slack)*$vdd);
  if (("on" eq $fix_prog)&&($vprog > $vprog_low_bound)) {
    $vprog_low_bound = $vprog;
  }
  return $vprog_low_bound;
}

sub lut_output_vector($ $ $)
{
  my ($lut_size,$SRAM_bits,$input_vectors) = @_;

  my @sram_bits = split /,/,$SRAM_bits;
  my @input_vects = split /,/,$input_vectors;

  my @start_input_vectors;
  my @final_input_vectors;
  my ($start_output,$final_output);
  my ($start_sram_index,$final_sram_index);

  for (my $i=0; $i<$lut_size; $i++) {
    if ("1" eq $input_vects[$i]) {
      $start_input_vectors[$i] = 1;
      $final_input_vectors[$i] = 1;
    } elsif ("0" eq $input_vects[$i]) {
      $start_input_vectors[$i] = 0;
      $final_input_vectors[$i] = 0;
    } elsif ("r" eq $input_vects[$i]) {
      $start_input_vectors[$i] = 0;
      $final_input_vectors[$i] = 1;
    } elsif ("f" eq $input_vects[$i]) {
      $start_input_vectors[$i] = 1;
      $final_input_vectors[$i] = 0;
    }
  }

  for (my $i=0; $i<$lut_size; $i++) {
    $start_sram_index += (1 - $start_input_vectors[$i])*(2**$i);
    $final_sram_index += (1 - $final_input_vectors[$i])*(2**$i);
  }

  if ($sram_bits[$start_sram_index] == $sram_bits[$final_sram_index]) {
    if (1 == $sram_bits[$final_sram_index]) {
      return "1";
    } else {
      return "0";
    }
  } elsif ($sram_bits[$start_sram_index] > $sram_bits[$final_sram_index]) {
    return "f";
  } elsif ($sram_bits[$start_sram_index] < $sram_bits[$final_sram_index]) {
    return "r";
  }
}

sub mux_last_level_input_num($ $)
{
  my ($num_sram,$mux_size) = @_;
  # Determine the number of 2to1 MUX required at the last level(num_sram)
  my ($num_inputs_last) = (2**$num_sram - $mux_size);
  if (0 < $num_inputs_last) {
    $num_inputs_last = 2*($mux_size - 2**($num_sram-1));
  } elsif (0 > $num_inputs_last) {
    die "Error: wrong SRAM numbers($num_sram) determined!\n";
  } else {
    $num_inputs_last = $mux_size;
  }
  return $num_inputs_last;
}
 
sub mux_output_vector($ $ $)
{
  my ($mux_size,$SRAM_bits,$input_vectors) = @_;

  my @sram_bits = split /,/,$SRAM_bits;
  my @input_vects = split /,/,$input_vectors;

  my ($select_path_index) = (0);

  if ("on" eq $opt_ptr->{one_level_mux}) {
    my $path_checker = 0;
    for (my $i=0; $i<($#sram_bits+1); $i++) {
      if (("1" ne $sram_bits[$i])&&("0" ne $sram_bits[$i])) {
        die "Error: (mux_output_vector) SRAM bits should be either 0 or 1!\n";
      } elsif ("1" eq $sram_bits[$i]) {
        $select_path_index = $i;
        $path_checker++;
      }
    }
    if (1 != $path_checker) {
      die "Error: Invalid path($path_checker) for one-level multiplexer!\n";
    }
    return ($select_path_index,$input_vects[$select_path_index]);
  }

  if ("on" eq $opt_ptr->{two_level_mux}) {
    my ($basis) = (($#sram_bits+1)/2);
    my ($offset1, $offset2) = (0,0);
    # For the 2nd level SRAMs 
    my ($check) = 0;
    for (my $i=($#sram_bits+1)/2; $i<($#sram_bits+1); $i++) {
      if (("1" ne $sram_bits[$i])&&("0" ne $sram_bits[$i])) {
        die "Error: (mux_output_vector) SRAM bits should be either 0 or 1!\n";
      } elsif ("1" eq $sram_bits[$i]) {
        $select_path_index = $basis*($i-$basis);
        $offset2 = $i-$basis;
        $check++;
      }
    }
    if (1 != $check) {
      die "Error: SRAMs of the 2nd level of a two-level MUX has 0 or more than 1 \'1\'!\n";
    }
    # For the 1st level SRAMs 
    $check = 0;
    for (my $i=0; $i<($#sram_bits+1)/2; $i++) {
      if (("1" ne $sram_bits[$i])&&("0" ne $sram_bits[$i])) {
        die "Error: (mux_output_vector) SRAM bits should be either 0 or 1!\n";
      } elsif ("1" eq $sram_bits[$i]) {
        $select_path_index += $i;
        $offset1 = $i;
        $check++;
      }
    }
    if (1 != $check) {
      die "Error: SRAMs of the 1st level of a two-level MUX has 0 or more than 1 \'1\'!\n";
    }
    # Check if the input to be measured is on the second level...
    if (!($select_path_index < $mux_size)) {
      $select_path_index = ($offset2-1)*$basis+$offset1;
    }
    return ($select_path_index,$input_vects[$select_path_index]);
  }

  my ($num_input_last) = (&mux_last_level_input_num($#sram_bits+1,$mux_size));

  for (my $i=0; $i<($#sram_bits+1); $i++) {
    if (("1" ne $sram_bits[$i])&&("0" ne $sram_bits[$i])) {
      die "Error: (mux_output_vector) SRAM bits should be either 0 or 1!\n";
    }
    $select_path_index += (1 - $sram_bits[$i])*(2**$i);
  }
  #print "Num_input_last = $num_input_last\n";  
  #print "Select_path_index = $select_path_index\n"; 

  # For non 2**sram_bits inputs, some sram configuration is overlapped.
  if ($select_path_index > ($num_input_last-1)) {
    $select_path_index = 0;
    for (my $i=1; $i<($#sram_bits+1); $i++) {
      if (("1" ne $sram_bits[$i])&&("0" ne $sram_bits[$i])) {
        die "Error: (mux_output_vector) SRAM bits should be either 0 or 1!\n";
      }
      $select_path_index += (1 - $sram_bits[$i])*(2**($i-1));
    }
    $select_path_index += $num_input_last/2;
  }  
  #print "Select_path_index = $select_path_index\n"; 

  # Check select path index
  if ((0 > $select_path_index)||(($mux_size-1) < $select_path_index)) {
    die "Error: (mux_output_vector) Fatal error in determining select_path_index($select_path_index)!\n";
  }

  return ($select_path_index,$input_vects[$select_path_index]);
}


sub determine_lut_delay_measure_input($ $)
{
  my ($lut_size,$input_vectors) = @_;

  my @input_vects = split /,/,$input_vectors;
  for (my $i=0; $i<$lut_size; $i++) {
    if (("r" eq $input_vects[$i])||("f" eq $input_vects[$i])) {
      return $i;
    }
  }
}

sub determine_lut_delay_measure($ $ $)
{
  my ($delay_measure_input,$input_vector,$output_vector) = @_;
  my ($delay_measure) = (1);
  my ($input_vector_type,$output_vector_type);

  my @input_vects = split /,/,$input_vector;
  # input[$delay_measure_input] should be either rise or fall
  if ("r" eq $input_vects[$delay_measure_input]) {
    $input_vector_type = "rise";
  } elsif ("f" eq $input_vects[$delay_measure_input]) {
    $input_vector_type = "fall";
  } else {
    print "Warning: LUT delay,slew and dynamic power won't be measured when input_vectors don't change!\n";
    $delay_measure = 0;
  }
  if ("r" eq $output_vector) {
    $output_vector_type = "rise";
  } elsif ("f" eq $output_vector) {
    $output_vector_type = "fall";
  } else {
    print "Warning: LUT delay,slew and dynamic power won't be measured when output_vectors don't change!\n";
    $delay_measure = 0;
  }
  return ($delay_measure,$input_vector_type,$output_vector_type); 
}

sub determine_mux_level($) 
{
  my ($mux_size) = @_;
  my ($mux_level) = (0);

  if ("on" eq $opt_ptr->{one_level_mux}) {
    return 1;
  }

  # Determine the level of multiplexer
  $mux_level = int(log($mux_size)/log(2));
  while ($mux_size > 2**$mux_level) {
    $mux_level++;
  }
  return $mux_level;
}

sub determine_2level_mux_basis($) {
  my ($mux_size) = @_;
  my ($basis) = sqrt($mux_size); 

  $basis = ceil($basis);
 
  return $basis;
}
 
sub determine_mux_num_sram($) {
  my ($mux_size) = @_;
  if ("on" eq $opt_ptr->{one_level_mux}) {
    return $mux_size;
  } elsif ("on" eq $opt_ptr->{two_level_mux}) {
    return 2*&determine_2level_mux_basis($mux_size);
  } else {
    return &determine_mux_level($mux_size);
  }
}

# Detect and convert unit, NO Case Insentive.
sub process_unit($ $)
{
  my ($unit,$type) = @_;
  my ($ret,$coeff) = (0,0);

  # Check type, can be <time|power|voltage|capacitance>
  if ("time" eq $type) {
    $unit =~ s/s$//i;
  } elsif ("current" eq $type) {
    $unit =~ s/A$//; # Special should not mix with "a" = 1e-18
  } elsif ("power" eq $type) {
    $unit =~ s/W$//;
  } elsif ("voltage" eq $type) {
    $unit =~ s/V$//;
  } elsif ("capacitance" eq $type) {
    $unit =~ s/F$//; # Special should not mix with "f" = 1e-15
  } elsif ("empty" ne $type) {
    die "Error: (process_unit)Unknown type!Should be <time|power|voltage|capacitance|empty>\n";
  }
  
  # Accepte unit: m = 1e-3, u = 1e-6, n = 1e-9, p = 1e-12, f = 1e-15, a = 1e-18
  if ($unit =~ m/a$/) { 
    $unit =~ s/a$//;
    $coeff = 1e-18;
  } elsif ($unit =~ m/f$/) {
    $unit =~ s/f$//;
    $coeff = 1e-15;
  } elsif ($unit =~ m/p$/) {
    $unit =~ s/p$//;
    $coeff = 1e-12;
  } elsif ($unit =~ m/n$/) {
    $unit =~ s/n$//;
    $coeff = 1e-9;
  } elsif ($unit =~ m/u$/) {
    $unit =~ s/u$//;
    $coeff = 1e-6;
  } elsif ($unit =~ m/m$/) {
    $unit =~ s/m$//;
    $coeff = 1e-3;
  } elsif ($unit =~ m/k$/) {
    $unit =~ s/k$//;
    $coeff = 1e3;
  } elsif ($unit =~ m/Meg$/) {
    $unit =~ s/Meg$//;
    $coeff = 1e6;
  } elsif ($unit =~ m/\d$/i) {
    $coeff = 1;
  }
  # Chomp the possible point at the end 
  $unit =~ s/\.$//;

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
  
  #$conf_ptr->{general_settings}->{spice_dir}->{val} =~ s/^\.\///;
  #$conf_ptr->{general_settings}->{process_tech}->{val} =~ s/^\.\///;

  if ("on" eq $opt_ptr->{dff}) {
    $opt_ptr->{dff_val} =~ s/^(\.)*\///;
  }

  if ("on" eq $opt_ptr->{lut}) {
    $opt_ptr->{lut_val} =~ s/^(\.)*\///;
    # Check if LUT size is an integer
    if ($opt_ptr->{lut_size_val} =~ m/[A-Za-z\.]/) {
      die "Error: LUT size should be an integer!\n";
    } 
  }

  # Check slew_upper_threshold_pct_rise > slew_lower_threshold_pct_rise
  if ($conf_ptr->{general_settings}->{slew_upper_threshold_pct_rise}->{val} 
     < $conf_ptr->{general_settings}->{slew_lower_threshold_pct_rise}->{val}) {
    die "Error: (check_general_settings)Invalid slew_[upper|lower]_threshold_pct_rise! Upper should >= Lower.\n";
  }
  # Check slew_upper_threshold_pct_fall > slew_lower_threshold_pct_fall
  if ($conf_ptr->{general_settings}->{slew_upper_threshold_pct_fall}->{val} 
     > $conf_ptr->{general_settings}->{slew_lower_threshold_pct_fall}->{val}) {
    die "Error: (check_general_settings)Invalid slew_[upper|lower]_threshold_pct_fall! Upper should <= Lower.\n";
  }

  # Check vsp_sweep, there should be 3 components
  $conf_ptr->{general_settings}->{vsp_sweep}->{val} =~ s/,$//;
  my @vsp_sweep = split /,/,$conf_ptr->{general_settings}->{vsp_sweep}->{val};
  if (2 != $#vsp_sweep) {
    die "Error: (check_general_settings)Invalid vsp_sweep(@vsp_sweep)!Should be [low_bound],[high_bound],[step]\n";
  }
  $conf_ptr->{general_settings}->{vsp_sweep}->{low_bound} = &process_unit($vsp_sweep[0],"voltage");
  $conf_ptr->{general_settings}->{vsp_sweep}->{high_bound} = &process_unit($vsp_sweep[1],"voltage");
  $conf_ptr->{general_settings}->{vsp_sweep}->{step} = &process_unit($vsp_sweep[2],"voltage");
  
  # Check bisec, there should be 3 components
  if ("on" eq $opt_ptr->{dff}) {
    $conf_ptr->{dff_settings}->{bisec}->{val} =~ s/,$//;
    my @bisec = split /,/,$conf_ptr->{dff_settings}->{bisec}->{val};
    if (2 != $#bisec) {
      die "Error: (check_dff_settings)Invalid bisec(@bisec)!Should be [initial_pass],[initial_fail],[step]\n";
    }
    $conf_ptr->{dff_settings}->{bisec}->{initial_pass} = &process_unit($bisec[0],"time");
    $conf_ptr->{dff_settings}->{bisec}->{initial_fail} = &process_unit($bisec[1],"time");
    $conf_ptr->{dff_settings}->{bisec}->{step} = &process_unit($bisec[2],"time");
  }

  # Check SRAM bits for LUT 
  if ("on" eq $opt_ptr->{lut}) {
    my @sram_bits = split /,/,$conf_ptr->{lut_settings}->{SRAM_bits}->{val};
    if ((2**$opt_ptr->{lut_size_val}) != ($#sram_bits + 1)) {
      die "Error:(check_lut_settings)Invaild SRAM_bits! Length should match 2^$opt_ptr->{lut_size_val}!\n";
    }
    for (my $i=0; $i < 2**$opt_ptr->{lut_size_val}; $i++) {
      if (($sram_bits[$i] != 0)&&($sram_bits[$i] != 1)) {
        die "Error:(check_lut_settings)Invaild SRAM_bits! Each bit should be either 0 or 1!\n";
      }
      $conf_ptr->{lut_settings}->{SRAM_bits}->{"sram$i"} = $sram_bits[$i]; 
    }

    my @input_vectors = split /,/,$conf_ptr->{lut_settings}->{input_vectors}->{val};
    if ($opt_ptr->{lut_size_val} != ($#input_vectors + 1)) {
      die "Error:(check_lut_settings)Invaild input_vectors! Length should match $opt_ptr->{lut_size_val}!\n";
    }
    for (my $i=0; $i < $opt_ptr->{lut_size_val}; $i++) {
      if (($input_vectors[$i] ne "1")&&($input_vectors[$i] ne "0")&&($input_vectors[$i] ne "r")&&($input_vectors[$i] ne "f")) {
      die "Error:(check_lut_settings)Invaild input_vectors! Each should be among 0,1,r,f.\n";
      }
      $conf_ptr->{lut_settings}->{input_vectors}->{"input$i"} = $input_vectors[$i]; 
    }
  }

  # Check SRAM bits for MUX 
  if ("on" eq $opt_ptr->{mux}) {
    #my @sram_bits = split /,/,$conf_ptr->{mux_settings}->{SRAM_bits}->{val};
    #if (($opt_ptr->{mux_size_val} > 2**($#sram_bits + 1))||($opt_ptr->{mux_size_val} < 2**($#sram_bits))) {
    #  die "Error:(check_mux_settings)Invaild SRAM_bits! Length should match log2($opt_ptr->{mux_size_val})!\n";
    #}
    # Check MUX SRAM bits
    #for (my $i=0; $i < (log($opt_ptr->{mux_size_val})/log(2)); $i++) {
    #  if (($sram_bits[$i] != 0)&&($sram_bits[$i] != 1)) {
    #    die "Error:(check_mux_settings)Invaild SRAM_bits! Each bit should be either 0 or 1!\n";
    #  }
    #  $conf_ptr->{mux_settings}->{SRAM_bits}->{"sram$i"} = $sram_bits[$i]; 
    #}

    # Check MUX input vectors
    #my @input_vectors = split /,/,$conf_ptr->{mux_settings}->{input_vectors}->{val};
    #if ($opt_ptr->{mux_size_val} != ($#input_vectors + 1)) {
    #  die "Error:(check_mux_settings)Invaild input_vectors! Length should match $opt_ptr->{mux_size_val}!\n";
    #}
    #for (my $i=0; $i < $opt_ptr->{mux_size_val}; $i++) {
    #  if (($input_vectors[$i] ne "1")&&($input_vectors[$i] ne "0")&&($input_vectors[$i] ne "r")&&($input_vectors[$i] ne "f")) {
    #  die "Error:(check_mux_settings)Invaild input_vectors! Each should be among 0,1,r,f.\n";
    #  }
    #  $conf_ptr->{mux_settings}->{input_vectors}->{"input$i"} = $input_vectors[$i]; 
    #}
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
    my ($process_dir,$process_file) = &split_prog_path("$conf_ptr->{general_settings}->{process_tech}->{val}");
    chdir $process_dir;
    print "Enter directory($process_dir)...\n";
    `csh -x 'source /softs/synopsys/hspice/2013.12/hspice/bin/cshrc.meta'`;
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
    `csh -cx '$hspice_path -i $fspice -o $flis -hdlpath /softs/synopsys/hspice/2013.12/hspice/include'`;
    chdir $cwd;
    print "Return directory($cwd)...\n";
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
    elsif ($line =~ /warning/i)
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

sub gen_common_sp_technology($)
{
  my ($spfh) = @_;

  &tab_print($spfh,"* Include Technology Library\n",0);
  if (($conf_ptr->{general_settings}->{process_tech}->{val} =~ m/\.lib$/)
    ||($conf_ptr->{general_settings}->{process_tech}->{val} =~ m/\.l$/)) {
    &tab_print($spfh,".lib ",0);
  } else {
    &tab_print($spfh,".include ",0);
  }

  &tab_print($spfh,"\'$conf_ptr->{general_settings}->{process_tech}->{val}\' ",0);

  if (($conf_ptr->{general_settings}->{process_tech}->{val} =~ m/\.lib$/)
    ||($conf_ptr->{general_settings}->{process_tech}->{val} =~ m/\.l$/)) {
    &tab_print($spfh,"$conf_ptr->{general_settings}->{process_type}->{val} ",0);
  }
  &tab_print($spfh,"\n",0);

  &tab_print($spfh,"*Design Parameters\n",0);
  &tab_print($spfh,".param beta = $conf_ptr->{general_settings}->{pn_ratio}->{val}\n",0);
  &tab_print($spfh,".param nl = $conf_ptr->{general_settings}->{nl}->{val}\n",0);
  &tab_print($spfh,".param pl = $conf_ptr->{general_settings}->{pl}->{val}\n",0);
  &tab_print($spfh,".param wn = $conf_ptr->{general_settings}->{min_wn}->{val}\n",0);
  &tab_print($spfh,".param wp = $conf_ptr->{general_settings}->{min_wp}->{val}\n",0);

  # FOR RRAM MUX:
  #   1. Define parameters for programming transistors: nl,pl, nmos, pmos
  #   2. Include the technology library for programming transistors 
  if ("on" eq $opt_ptr->{rram_enhance}) {
    if (($conf_ptr->{rram_settings}->{prog_process_tech}->{val} eq $conf_ptr->{general_settings}->{process_tech}->{val})
     &&($conf_ptr->{rram_settings}->{prog_process_type}->{val} eq $conf_ptr->{general_settings}->{process_type}->{val})) {
      &tab_print($spfh,"* Technology Library for RRAM Programming Transistors is same with standard transistors.\n",0);
      &tab_print($spfh,"* Additional tech. library is included.\n",0);
    } else {
      &tab_print($spfh,"* Include Technology Library for RRAM Programming Transistors\n",0);
      if (($conf_ptr->{rram_settings}->{prog_process_tech}->{val} =~ m/\.lib$/)
        ||($conf_ptr->{rram_settings}->{prog_process_tech}->{val} =~ m/\.l$/)) {
        &tab_print($spfh,".lib ",0);
      } else {
        &tab_print($spfh,".include ",0);
      }

      &tab_print($spfh,"\'$conf_ptr->{rram_settings}->{prog_process_tech}->{val}\' ",0);
 
      if (($conf_ptr->{rram_settings}->{prog_process_tech}->{val} =~ m/\.lib$/)
        ||($conf_ptr->{rram_settings}->{prog_process_tech}->{val} =~ m/\.l$/)) {
        &tab_print($spfh,"$conf_ptr->{rram_settings}->{prog_process_type}->{val} ",0);
      }
    }
    &tab_print($spfh,"\n",0);

    &tab_print($spfh,"*Design Parameters for RRAM programming transistors\n",0);
    &tab_print($spfh,".param prog_beta = $conf_ptr->{rram_settings}->{prog_pn_ratio}->{val}\n",0);
    &tab_print($spfh,".param prog_nl = $conf_ptr->{rram_settings}->{prog_nl}->{val}\n",0);
    &tab_print($spfh,".param prog_pl = $conf_ptr->{rram_settings}->{prog_pl}->{val}\n",0);
    &tab_print($spfh,".param prog_wn = $conf_ptr->{rram_settings}->{prog_min_wn}->{val}\n",0);
    &tab_print($spfh,".param prog_wp = $conf_ptr->{rram_settings}->{prog_min_wp}->{val}\n",0);
  }

  &tab_print($spfh,"*Include  ELC NMOS and PMOS Package\n",0);
  &tab_print($spfh,".include \'$cwd/$conf_ptr->{general_settings}->{spice_dir}->{val}$elc_nmos_pmos_sp'\n",0);
  #&tab_print($spfh,"\n",0);
  
}

sub gen_common_sp_parameters($ $)
{
  my ($spfh,$vsp) = @_;

  &tab_print($spfh,"* Working Temperature\n",0);
  &tab_print($spfh,".temp $conf_ptr->{general_settings}->{temperature}->{val}\n",0);
  &tab_print($spfh,"* Global Nodes\n",0);
  &tab_print($spfh,".global $conf_ptr->{general_settings}->{VDD_port_name}->{val} $conf_ptr->{general_settings}->{LOAD_VDD_port_name}->{val}\n",0);
  &tab_print($spfh,"* Print Node Capacitance\n",0);
  &tab_print($spfh,".option captab\n",0);
  &tab_print($spfh,"* Print Waveforms\n",0);
  &tab_print($spfh,".option POST\n",0);
  &tab_print($spfh,".param vsp=$vsp\n",0);
  &tab_print($spfh,"* Parameters for Measuring Slew\n",0);
  &tab_print($spfh,".param slew_upper_threshold_pct_rise=$conf_ptr->{general_settings}->{slew_upper_threshold_pct_rise}->{val}\n",0);
  &tab_print($spfh,".param slew_lower_threshold_pct_rise=$conf_ptr->{general_settings}->{slew_lower_threshold_pct_rise}->{val}\n",0);
  &tab_print($spfh,".param slew_upper_threshold_pct_fall=$conf_ptr->{general_settings}->{slew_upper_threshold_pct_fall}->{val}\n",0);
  &tab_print($spfh,".param slew_lower_threshold_pct_fall=$conf_ptr->{general_settings}->{slew_lower_threshold_pct_fall}->{val}\n",0);
  &tab_print($spfh,"* Parameters for Measuring Delay\n",0);
  &tab_print($spfh,".param input_threshold_pct_rise=$conf_ptr->{general_settings}->{input_threshold_pct_rise}->{val}\n",0);
  &tab_print($spfh,".param input_threshold_pct_fall=$conf_ptr->{general_settings}->{input_threshold_pct_fall}->{val}\n",0);
  &tab_print($spfh,".param output_threshold_pct_rise=$conf_ptr->{general_settings}->{output_threshold_pct_rise}->{val}\n",0);
  &tab_print($spfh,".param output_threshold_pct_fall=$conf_ptr->{general_settings}->{output_threshold_pct_fall}->{val}\n",0);
  
}

# Generate a subckt of a inverter tree driving a number of loads
sub gen_sp_subckt_fanout_inv_tree($ $ $) {
  my ($spfh, $fan_out, $max_fan_out_per_level) = @_;
  my ($num_inv_tree_level) = (int(0.5 + (log($fan_out)/log($max_fan_out_per_level))));
  my ($i);
 
  if (!((!($max_fan_out_per_level**$num_inv_tree_level < $fan_out))
    &&(!($max_fan_out_per_level**($num_inv_tree_level-1) > $fan_out)))) {
    print "Internal Error: \nmax_fan_out_per_level=$max_fan_out_per_level\n";
    die "num_inv_tree_level=$num_inv_tree_level\nfan_out=$fan_out\n";
  }
  # Print the definition
  &tab_print($spfh, ".subckt fanout_inv_tree_size$fan_out in ", 0); 
  for ($i = 0; $i < $fan_out; $i++) {
    &tab_print($spfh, "out[$i] ", 0);
  }
  &tab_print($spfh, "svdd sgnd\n",0);

  # Print the tree
  for ($i = 0; $i < $num_inv_tree_level; $i++) {
  }
  
  # End of subckt
  &tab_print($spfh, ".eom\n", 0);

  return;
}

sub gen_auto_out_tapered_buffer($ $ $) {
  my ($spfh, $fan_out, $f_inv) = @_;
  my ($num_level_tapered_buf) = int(0.5 + log($fan_out)/log($f_inv));
  my ($cur_level_f, $i, $nexti);
  
  # Print the definition
  &tab_print($spfh, ".subckt tapbuf_size$fan_out in out svdd sgnd\n", 0); 
  for ($i = 0; $f_inv**$i < $fan_out; $i++) {
    $cur_level_f = $f_inv**$i;
    $nexti = $i + 1;
    for (my $j = 0; $j < $cur_level_f; $j++) {
      &tab_print($spfh, "Xinvlvl$i\_no$j\_tapbuf in_lvl$i in_lvl$nexti svdd sgnd inv size=1\n", 0);
    }
  }
  if (1 == $i%2) {
    # Short connected inputs
    &tab_print($spfh, "Rin in in_lvl0 0\n", 0);
  } else {
    # Add one more inverter 
    &tab_print($spfh, "Xinv_in in in_lvl0 svdd sgnd inv size=1\n", 0);
  }
  # Short connected output
  &tab_print($spfh, "Rout in_lvl$i out 0\n", 0);
  # End of subckt
  &tab_print($spfh, ".eom\n", 0);

  print "Tapered buffer: size=$fan_out, f=$f_inv, level=$i\n";

  return;
}

# Depending on the number of inverters in loads,
# We use a a tree structure to shape inverters driving the loads
sub gen_sp_inv_loads($ $ $ $) {
  my ($spfh, $outport_name, $inv_load_size, $inv_load_num) = @_;
  my ($load_port_name) = ("out");

  for (my $i = 0; $i < $inv_load_num; $i++) {
    &tab_print($spfh, "Xinv_load$i $load_port_name inv_load$i\_out $conf_ptr->{general_settings}->{LOAD_VDD_port_name}->{val} $conf_ptr->{general_settings}->{GND_port_name}->{val} inv size=$inv_load_size\n", 0);
  }

  return;
} 

sub gen_lut_sp_common($ $ $ $ $ $ $) 
{
  my ($spfh,$usage,$cload,$input_pwl,$input_pwh,$input_slew,$vsp) = @_;

  # Print Header
  &tab_print($spfh,"Sub-Vt LUT HSPICE Bench\n",0);
  &tab_print($spfh,"**********************\n",0);
  &tab_print($spfh,"*   HSPICE Netlist   *\n",0);
  &tab_print($spfh,"* Author: Xifan TANG *\n",0);
  &tab_print($spfh,"*         EPFL, LSI  *\n",0);
  &tab_print($spfh,"* Date: $mydate    *\n",0);
  &tab_print($spfh,"**********************\n",0);
  &tab_print($spfh,"* Test Bench Usage: $usage\n",0);

  &gen_common_sp_technology($spfh); 

  &gen_common_sp_parameters($spfh,$vsp);

  &tab_print($spfh,".param input_pwl=$input_pwl\n",0);
  &tab_print($spfh,".param input_pwh=$input_pwh\n",0);
  &tab_print($spfh,".param input_slew=$input_slew\n",0);
 
  &tab_print($spfh,"* Include Circuits Library\n",0);

  &tab_print($spfh,".include $cwd/$opt_ptr->{lut_val}\n",0);

  # Now Call the sub circuit
  &tab_print($spfh,"Xlut_ckt ",0);
  for (my $i=0; $i<2**$opt_ptr->{lut_size_val}; $i++) {
    &tab_print($spfh,"$conf_ptr->{lut_settings}->{SRAM_port_prefix}->{val}$i ",0);
  }
  for (my $i=0; $i<$opt_ptr->{lut_size_val}; $i++) {
    &tab_print($spfh,"$conf_ptr->{lut_settings}->{IN_port_prefix}->{val}$i ",0);
  }
  &tab_print($spfh,"$conf_ptr->{lut_settings}->{OUT_port_name}->{val} ",0);
  &tab_print($spfh,"$conf_ptr->{lut_settings}->{lut_subckt_name}->{val}\n",0);

  if (($conf_ptr->{lut_settings}->{load_type}->{val} =~ m/cap/)
    # Call the subckt 
     &&(0 < $cload)) {
    &tab_print($spfh,"Cload $conf_ptr->{lut_settings}->{OUT_port_name}->{val} $conf_ptr->{general_settings}->{GND_port_name}->{val} $cload\n",0);
  }
  if ($conf_ptr->{lut_settings}->{load_type}->{val} =~ m/inv/) {
    &gen_sp_inv_loads($spfh, $conf_ptr->{lut_settings}->{OUT_port_name}->{val},
                      $conf_ptr->{lut_settings}->{load_inv_size}->{val},
                      $conf_ptr->{lut_settings}->{load_inv_num}->{val}); 
  }

  # Add Stimulates
  &tab_print($spfh,"Vsupply $conf_ptr->{general_settings}->{VDD_port_name}->{val} $conf_ptr->{general_settings}->{GND_port_name}->{val} vsp\n",0);

  if ("0" ne $conf_ptr->{general_settings}->{GND_port_name}->{val}) {
    &tab_print($spfh,"Vgnd $conf_ptr->{general_settings}->{GND_port_name}->{val} 0 0\n",0);
  }
  
  &tab_print($spfh,"* Common Part Over.\n",0);

}

sub gen_lut_sp_stimulates($)
{
  my ($spfh,$SRAM_bits,$input_vectors) = @_;
  
  my @sram_bits = split /,/,$SRAM_bits;
  my @input_vects = split /,/,$input_vectors;

  # Add Stimulates, SRAM bits
  for (my $i=0; $i<2**$opt_ptr->{lut_size_val}; $i++) {
    &tab_print($spfh,"Vsram$i $conf_ptr->{lut_settings}->{SRAM_port_prefix}->{val}$i $conf_ptr->{general_settings}->{GND_port_name}->{val} ",0);
    if ("1" eq $sram_bits[$i]) {
      &tab_print($spfh,"vsp",0);
    } elsif ("0" eq $sram_bits[$i]) {
      &tab_print($spfh,"0",0);
    } else {
      die "Error: Invalid value for SRAM bits(should be 1 or 0)!\n";
    } 
    &tab_print($spfh,"\n",0);
  }
  # Add Stimulates, input_ports
  for (my $i=0; $i<$opt_ptr->{lut_size_val}; $i++) {
    &tab_print($spfh,"Vin$i $conf_ptr->{lut_settings}->{IN_port_prefix}->{val}$i $conf_ptr->{general_settings}->{GND_port_name}->{val} ",0);
    if ("1" eq $input_vects[$i]) {
      &tab_print($spfh,"vsp",0);
    } elsif ("0" eq $input_vects[$i]) {
      &tab_print($spfh,"gnd",0);
    } elsif ("r" eq $input_vects[$i]) {
      &tab_print($spfh,"pwl(0 0 input_pwl 0 \'input_pwl+input_slew\' vsp \'input_pwl+input_slew+input_pwh\' vsp)",0);
    } elsif ("f" eq $input_vects[$i]) {
      &tab_print($spfh,"pwl(0 vsp input_pwl vsp \'input_pwl+input_slew\' 0 \'input_pwl+input_slew+input_pwh\' 0)",0);
    } else {
      die "Error: Invalid value for SRAM bits(should be 1 or 0)!\n";
    } 
    &tab_print($spfh,"\n",0);
  }
  &tab_print($spfh,"\n",0);
   
}

sub gen_lut_sp_measure($ $ $ $ $ $ $) 
{
  my ($spfh,$tran_step,$tran_time,$delay_measure,$delay_measure_input,$input_vector_type,$output_vector_type) = @_;
 
  # .Tran
  &tab_print($spfh,".tran $tran_step $tran_time\n",0);
  &tab_print($spfh,"\n",0);
  # Measure

  &tab_print($spfh,".measure tran pleak avg p(vsupply) from=0 to=\'input_pwl\'\n",0);

  if (1 == $delay_measure) {
    &tab_print($spfh,".measure tran dly_lut trig v($conf_ptr->{lut_settings}->{IN_port_prefix}->{val}$delay_measure_input) val=\'input_threshold_pct_$input_vector_type*vsp\' $input_vector_type=1\n",0);
    &tab_print($spfh,"+                     targ v($conf_ptr->{lut_settings}->{OUT_port_name}->{val}) val=\'output_threshold_pct_$output_vector_type*vsp\' $output_vector_type=1 td=\'input_pwl\'\n",0);
    &tab_print($spfh,".measure tran slew_lut trig v($conf_ptr->{lut_settings}->{OUT_port_name}->{val}) val=\'slew_lower_threshold_pct_$output_vector_type*vsp\' $output_vector_type=1 td=\'input_pwl\'\n",0);
    &tab_print($spfh,"+                      targ v($conf_ptr->{lut_settings}->{OUT_port_name}->{val}) val=\'slew_upper_threshold_pct_$output_vector_type*vsp\' $output_vector_type=1 td=\'input_pwl\'\n",0);
    &tab_print($spfh,".measure tran pdynamic avg p(vsupply) from=\'input_pwl\' to=\'input_pwl+input_slew+slew_lut\'\n",0);
    &tab_print($spfh,".measure tran energy_per_toggle param=\'pdynamic*(input_slew+slew_lut)\'\n",0);
    &tab_print($spfh,".measure tran avg_vlut avg V($conf_ptr->{lut_settings}->{OUT_port_name}->{val}) from=\'input_slew+slew_lut\' to=\'$tran_time\'\n",0);
  }
  &tab_print($spfh,".end Sub-Vt LUT HSPICE Bench\n",0);

}

sub gen_mux_sp_common($ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $) 
{
  my ($spfh,$usage,$cload,$input_pwl,$input_pwh,$input_slew,$vsp,$mux_size,$inv_size_in,$inv_size_out,$rram_enhance,$ron,$wprog, $roff,$gap_on, $gap_off) = @_;

  # Print Header
  &tab_print($spfh,"Sub-Vt MUX HSPICE Bench\n",0);
  &tab_print($spfh,"**********************\n",0);
  &tab_print($spfh,"*   HSPICE Netlist   *\n",0);
  &tab_print($spfh,"* Author: Xifan TANG *\n",0);
  &tab_print($spfh,"*         EPFL, LSI  *\n",0);
  &tab_print($spfh,"* Date: $mydate    *\n",0);
  &tab_print($spfh,"**********************\n",0);
  &tab_print($spfh,"* Test Bench Usage: $usage\n",0);

  &gen_common_sp_technology($spfh); 

  &gen_common_sp_parameters($spfh,$vsp);

  &tab_print($spfh,".param input_pwl=$input_pwl\n",0);
  &tab_print($spfh,".param input_pwh=$input_pwh\n",0);
  &tab_print($spfh,".param input_slew=$input_slew\n",0);
  &tab_print($spfh,".param inv_size_in=$inv_size_in\n",0);
  &tab_print($spfh,".param inv_size_out=$inv_size_out\n",0);
  
  if ($rram_enhance) {
    &tab_print($spfh,".param N=$mux_size\n",0);
    #&tab_print($spfh,".param ron=$ron\n",0);
    #&tab_print($spfh,".param roff=$roff\n",0);
    &tab_print($spfh,".param gap_on=$gap_on\n",0);
    &tab_print($spfh,".param gap_off=$gap_off\n",0);
    &tab_print($spfh,".param wprog=$wprog\n",0);
    &tab_print($spfh,".param tprog=$conf_ptr->{rram_settings}->{Tprog}->{val}\n",0);
    &tab_print($spfh,".param vprog=$conf_ptr->{rram_settings}->{Vdd_break}->{val}\n",0);
    &tab_print($spfh,"* Global port for programming vdd \n",0);
    &tab_print($spfh,".global prog_vdd prog_gnd\n",0);
    &tab_print($spfh,"* Include RRAM verilogA model \n",0);
    # Include RRAM verilogA model
    if ($conf_ptr->{rram_settings}->{rram_verilogA_model_path}->{val} =~ m/\.va$/) {
      &tab_print($spfh,".hdl \'$conf_ptr->{rram_settings}->{rram_verilogA_model_path}->{val}\'\n",0);
    } else {
      &tab_print($spfh,".include \'$conf_ptr->{rram_settings}->{rram_verilogA_model_path}->{val}\'\n",0);
    }
  }
 
  &tab_print($spfh,"* Include Circuits Library\n",0);

  &tab_print($spfh,".include $cwd/$opt_ptr->{mux_val}\n",0);

  my ($buf_val) = ("buffer");
  if ($opt_ptr->{mux_unbuffered}) {
    $buf_val = "unbuffered";
  }

  my ($inv_load_num) = ($conf_ptr->{mux_settings}->{load_inv_num}->{val});
  if ("on" eq $opt_ptr->{auto_out_tapered_buffer}) {
    # Add a tapered buffer at output
    my ($eq_inv_load_num) = (0);
    my ($cap_eq_inv_num) = (&process_unit($conf_ptr->{mux_settings}->{load_cap}->{val},"capacitance")/(&process_unit($conf_ptr->{rram_settings}->{Cs_pmos}->{val},"capacitance")+&process_unit($conf_ptr->{rram_settings}->{Cs_nmos}->{val},"capacitance")));
    if ($conf_ptr->{mux_settings}->{load_type}->{val} =~ m/cap/) {
      $eq_inv_load_num += $cap_eq_inv_num;
    }
    #if ($conf_ptr->{mux_settings}->{load_type}->{val} =~ m/inv/) {
      $eq_inv_load_num += $conf_ptr->{mux_settings}->{load_inv_num}->{val};
    #}
    &gen_auto_out_tapered_buffer($spfh, 4**$opt_ptr->{auto_out_tapered_buffer_val}, 4);
    #&gen_auto_out_tapered_buffer($spfh, $eq_inv_load_num, 4);
  }
 
  if ("on" eq $opt_ptr->{one_level_mux}) {
    # Generate the sub circuit of 1-level N-input MUX
    &gen_1level_mux_subckt($spfh,$mux_size,"mux2_size$mux_size",$opt_ptr->{one_level_mux_val},"buffered",$rram_enhance);
  } elsif ("on" eq $opt_ptr->{two_level_mux}) {
    &gen_2level_mux_subckt($spfh,$mux_size,"mux2_size$mux_size",$opt_ptr->{two_level_mux_val},"buffered",$rram_enhance);
  } else {
    # Generate the sub circuit of N-input MUX with a given 2-input MUX subckt
    &gen_multilevel_mux_subckt($spfh,$mux_size,"mux2_size$mux_size",$conf_ptr->{mux_settings}->{mux_subckt_name}->{val},"buffered",$rram_enhance);
  } 

  # Use the sub circuit
  &tab_print($spfh,"Xmux2_size$mux_size ",0);
  for (my $i=0; $i < &determine_mux_num_sram($mux_size); $i++) {
    &tab_print($spfh,"$conf_ptr->{mux_settings}->{SRAM_port_prefix}->{val}$i ",0);
    &tab_print($spfh,"$conf_ptr->{mux_settings}->{invert_SRAM_port_prefix}->{val}$i ",0);
  } 
  for (my $i=0; $i < $mux_size; $i++) {
    &tab_print($spfh,"$conf_ptr->{mux_settings}->{IN_port_prefix}->{val}$i ",0);
  }
  &tab_print($spfh,"$conf_ptr->{mux_settings}->{OUT_port_name}->{val} ",0);
  &tab_print($spfh,"$conf_ptr->{general_settings}->{VDD_port_name}->{val} ",0);
  &tab_print($spfh,"$conf_ptr->{general_settings}->{GND_port_name}->{val} ",0);
  &tab_print($spfh,"mux2_size$mux_size\n",0);
  
  # Add Load Cap
  if (($conf_ptr->{mux_settings}->{load_type}->{val} =~ m/cap/)
     &&(0 < $cload)) {
    &tab_print($spfh,"Cload $conf_ptr->{mux_settings}->{OUT_port_name}->{val} $conf_ptr->{general_settings}->{GND_port_name}->{val} $cload\n",0);
  }
  if ($conf_ptr->{mux_settings}->{load_type}->{val} =~ m/inv/) {
    &gen_sp_inv_loads($spfh, $conf_ptr->{mux_settings}->{OUT_port_name}->{val},
                      $conf_ptr->{mux_settings}->{load_inv_size}->{val},
                      $conf_ptr->{mux_settings}->{load_inv_num}->{val}); 
  }
 
  &tab_print($spfh,"* Common Part Over.\n",0);

}

sub gen_rram_mux_sp_stimulates($) {
  my ($spfh,$SRAM_bits,$input_vectors) = @_;

  # Add Stimulates: Standard VDD
  &tab_print($spfh,"* Standard Vdd\n",0);
  &tab_print($spfh,"* PUSLE(V1 V2 TDELAY TRISE TFALL PW PERIOD)\n",0);
  &tab_print($spfh,"Vsupply $conf_ptr->{general_settings}->{VDD_port_name}->{val} 0 pulse(vsp vsp \'(2*N+1)*tprog\' \'input_slew\' \'input_slew\' \'(2*N+1)*tprog\' \'(4*N+2)*tprog\')\n",0);
  &tab_print($spfh, "Vload_supply $conf_ptr->{general_settings}->{LOAD_VDD_port_name}->{val} 0 pulse(vsp vsp \'(2*N+1)*tprog\' \'input_slew\' \'input_slew\' \'(2*N+1)*tprog\' \'(4*N+2)*tprog\')\n",0);

  if ("0" ne $conf_ptr->{general_settings}->{GND_port_name}->{val}) {
    &tab_print($spfh,"Vgnd $conf_ptr->{general_settings}->{GND_port_name}->{val} 0 0\n",0);
  }
  # Add Stimulates: Programming Vdd
  &tab_print($spfh,"* Programming Vdd\n",0);
  &tab_print($spfh,"Vprog_vdd prog_vdd 0 pulse(\'vprog/2\' \'vsp\' \'(2*N+1)*tprog\' input_slew input_slew \'(2*N+1)*tprog\' \'(4*N+2)*tprog\')\n",0);
  &tab_print($spfh,"Vprog_gnd prog_gnd 0 pulse(\'-vprog/2\' 0 \'(2*N+1)*tprog\' input_slew input_slew \'(2*N+1)*tprog\' \'(4*N+2)*tprog\')\n",0);
 
  # Add Stimulates: Controlling signals for programming RRAM.
  &tab_print($spfh,"* Control signals for Programming Vdd\n",0);
  &tab_print($spfh,"Vbl[0]_b bl[0]_b 0 ",0);
  &tab_print($spfh,"pwl(0 \'vprog/2\' \n",0); 
  &tab_print($spfh,"+   \'tprog-input_slew\' \'vprog/2\' \n",0);
  &tab_print($spfh,"+   \'tprog\' \'-vprog/2\' \n",0);
  &tab_print($spfh,"+   \'2*tprog-input_slew\' \'-vprog/2\' \n",0);
  &tab_print($spfh,"+   \'2*tprog\' \'+vprog/2\' \n",0);
  &tab_print($spfh,"+   \'(2*N+1)*tprog-input_slew\' \'+vprog/2\' \n",0);
  &tab_print($spfh,"+   \'(2*N+1)*tprog\' \'vsp\')\n",0);
  #&tab_print($spfh,"pulse(\'+vprog/2+vsp/2\' \'-vprog/2+vsp/2\' tprog input_slew input_slew tprog \'(4*N+2)*tprog\')\n",0);
  # WL[0] should be always 0
  &tab_print($spfh,"Vwl[0] wl[0] 0 ",0);
  &tab_print($spfh,"pulse(\'-vprog/2\' \'vsp\' \'(2*N+2)*tprog\' input_slew input_slew \'(2*N+1)*tprog\' \'(4*N+1)*tprog\')\n",0);
  for (my $i=1; $i < $opt_ptr->{mux_size_val}; $i++) {
    # BL[i]_b 1 <= i < mux_size should be always 0
    &tab_print($spfh,"Vbl[$i]_b bl[$i]_b 0 ",0);
    &tab_print($spfh,"pulse(\'vprog/2\' \'vsp\' \'(2*N+2)*tprog\' input_slew input_slew \'(2*N+1)*tprog\' \'(4*N+2)*tprog\')\n",0);
    #&tab_print($spfh,"pulse(\'+vprog/2+vsp/2\' vsp\' \'(1+2*$i)*tprog\' input_slew input_slew tprog \'(4*N+2)*tprog\')\n",0);
    &tab_print($spfh,"Vwl[$i] wl[$i] 0 ",0);
    &tab_print($spfh,"pwl(0 \'-vprog/2\' \n",0); 
    &tab_print($spfh,"+   \'(1+2*$i)*tprog-input_slew\' \'-vprog/2\'\n",0);  
    &tab_print($spfh,"+   \'(1+2*$i)*tprog\' \'vprog/2\'\n",0);  
    &tab_print($spfh,"+   \'(2+2*$i)*tprog-input_slew\' \'vprog/2\'\n",0);   
    &tab_print($spfh,"+   \'(2+2*$i)*tprog\' \'-vprog/2\'\n",0); 
    &tab_print($spfh,"+   \'(2*N+1)*tprog-input_slew\' \'-vprog/2\'\n",0); 
    &tab_print($spfh,"+   \'(2*N+1)*tprog\' 0)\n",0);
    #&tab_print($spfh,"pulse(\'-vprog/2+vsp/2\' \'+vprog/2+vsp/2\' \'(1+2*$i)*tprog\' input_slew input_slew tprog \'(4*N+2)*tprog\')\n",0);
  }
  my ($mux_size) = ($opt_ptr->{mux_size_val});
  &tab_print($spfh,"Vbl[$mux_size]_b bl[$mux_size]_b 0 ",0);
  &tab_print($spfh,"pwl(0 \'vprog/2\' \n",0);
  for (my $i = 1; $i < $mux_size; $i++) {
    &tab_print($spfh,"+   \'(1+2*$i)*tprog-input_slew\' \'vprog/2\'  \n",0);
    &tab_print($spfh,"+   \'(1+2*$i)*tprog\' \'-vprog/2\' \n",0); 
    &tab_print($spfh,"+   \'(2+2*$i)*tprog-input_slew\' \'-vprog/2\' \n",0);
    &tab_print($spfh,"+   \'(2+2*$i)*tprog\' \'vprog/2\'  \n",0);
  }
  &tab_print($spfh,"+   \'(2*N+1)*tprog-input_slew\' \'vprog/2\'  \n",0);
  &tab_print($spfh,"+   \'(2*N+1)*tprog\' \'vsp\'  \n",0);
  # WL[mux_size] should be always 0 except the first cycle
  &tab_print($spfh,"Vwl[$mux_size] wl[$mux_size] 0 ",0);
  &tab_print($spfh,"pwl(0 \'-vprog/2\' \n",0);
  &tab_print($spfh,"+   \'tprog-input_slew\' \'-vprog/2\'  \n",0);
  &tab_print($spfh,"+   \'tprog\' \'vprog/2\'  \n",0);
  &tab_print($spfh,"+   \'2*tprog-input_slew\' \'vprog/2\'  \n",0);
  &tab_print($spfh,"+   \'2*tprog\' \'-vprog/2\'  \n",0);
  &tab_print($spfh,"+   \'(2*N+1)*tprog-input_slew\' \'-vprog/2\'  \n",0);
  &tab_print($spfh,"+   \'(2*N+1)*tprog\' \'0\'  \n",0);

  # Add Stimulates, input_ports
  &tab_print($spfh,"* MUX inputs signals\n",0);
  for (my $i=0; $i<$opt_ptr->{mux_size_val}; $i++) {
    &tab_print($spfh,"Vin$i $conf_ptr->{mux_settings}->{IN_port_prefix}->{val}$i 0 ",0);
    &tab_print($spfh,"pulse(0 vsp \'(2*N+2+$i*2)*tprog\' input_slew input_slew tprog \'(4*N+2)*tprog\')\n",0);
  }
  &tab_print($spfh,"\n",0);
  
}

sub gen_mux_sp_stimulates($)
{
  my ($spfh,$SRAM_bits,$input_vectors) = @_;
  
  my @sram_bits = split /,/,$SRAM_bits;
  my @input_vects = split /,/,$input_vectors;
  my ($num_mux_sram) = (&determine_mux_num_sram($opt_ptr->{mux_size_val}));

  # Add Stimulates
  &tab_print($spfh,"Vsupply $conf_ptr->{general_settings}->{VDD_port_name}->{val} $conf_ptr->{general_settings}->{GND_port_name}->{val} vsp\n",0);
  &tab_print($spfh, "Vload_supply $conf_ptr->{general_settings}->{LOAD_VDD_port_name}->{val} 0 vsp\n", 0);

  if ("0" ne $conf_ptr->{general_settings}->{GND_port_name}->{val}) {
    &tab_print($spfh,"Vgnd $conf_ptr->{general_settings}->{GND_port_name}->{val} 0 0\n",0);
  }

  # Add Stimulates, SRAM bits
  for (my $i=0; $i< $num_mux_sram; $i++) {
    &tab_print($spfh,"Vsram$i $conf_ptr->{mux_settings}->{SRAM_port_prefix}->{val}$i $conf_ptr->{general_settings}->{GND_port_name}->{val} ",0);
    if ("1" eq $sram_bits[$i]) {
      &tab_print($spfh,"vsp",0);
    } elsif ("0" eq $sram_bits[$i]) {
      &tab_print($spfh,"0",0);
    } else {
      die "Error: Invalid value for SRAM bits(should be 1 or 0)!\n";
    } 
    &tab_print($spfh,"\n",0);
    # Invert SRAM bits
    &tab_print($spfh,"Vinv_sram$i $conf_ptr->{mux_settings}->{invert_SRAM_port_prefix}->{val}$i $conf_ptr->{general_settings}->{GND_port_name}->{val} ",0);
    if ("1" eq $sram_bits[$i]) {
      &tab_print($spfh,"0",0);
    } elsif ("0" eq $sram_bits[$i]) {
      &tab_print($spfh,"vsp",0);
    } else {
      die "Error: Invalid value for SRAM bits(should be 1 or 0)!\n";
    } 
    &tab_print($spfh,"\n",0);

  }
  # Add Stimulates, input_ports
  for (my $i=0; $i<$opt_ptr->{mux_size_val}; $i++) {
    &tab_print($spfh,"Vin$i $conf_ptr->{mux_settings}->{IN_port_prefix}->{val}$i $conf_ptr->{general_settings}->{GND_port_name}->{val} ",0);
    if ("1" eq $input_vects[$i]) {
      &tab_print($spfh,"vsp",0);
    } elsif ("0" eq $input_vects[$i]) {
      &tab_print($spfh,"0",0);
    } elsif ("r" eq $input_vects[$i]) {
      &tab_print($spfh,"pwl(0 0 input_pwl 0 \'input_pwl+input_slew\' vsp \'input_pwl+input_slew+input_pwh\' vsp)",0);
    } elsif ("f" eq $input_vects[$i]) {
      &tab_print($spfh,"pwl(0 vsp input_pwl vsp \'input_pwl+input_slew\' 0 \'input_pwl+input_slew+input_pwh\' 0)",0);
    } else {
      die "Error: Invalid value for SRAM bits(should be 1 or 0)!\n";
    } 
    &tab_print($spfh,"\n",0);
  }
  &tab_print($spfh,"\n",0);
   
}

sub gen_rram_mux_sp_measure($ $ $ $ $ $ $) {
  my ($spfh,$tran_step,$tran_time,$delay_measure,$delay_measure_input,$input_vector_type,$output_vector_type) = @_;

  # .Tran
  &tab_print($spfh,".tran $tran_step \'(4*N+2)*tprog\'\n",0);
  &tab_print($spfh,"\n",0);

  # Measure
  &tab_print($spfh,".measure tran pleak avg p(vsupply) from=\'(2*N+1)*tprog\' to=\'(2*N+2)*tprog'\n",0);

  if (1 == $delay_measure) {
    &tab_print($spfh,"* Measure delay: rising edge \n",0);
    $input_vector_type = "rise";
    $output_vector_type = "rise";
    &tab_print($spfh,".measure tran dly_mux_rise trig v($conf_ptr->{mux_settings}->{IN_port_prefix}->{val}$delay_measure_input) val=\'input_threshold_pct_$input_vector_type*vsp\' $input_vector_type=1 td=\'(2*N+2)*tprog\'\n",0);
    &tab_print($spfh,"+                     targ v($conf_ptr->{mux_settings}->{OUT_port_name}->{val}) val=\'output_threshold_pct_$output_vector_type*vsp\' $output_vector_type=1 td=\'(2*N+2)*tprog\'\n",0);
    &tab_print($spfh,".measure tran slew_mux_rise trig v($conf_ptr->{mux_settings}->{OUT_port_name}->{val}) val=\'slew_lower_threshold_pct_$output_vector_type*vsp\' $output_vector_type=1 td=\'(2*N+2)*tprog\'\n",0);
    &tab_print($spfh,"+                      targ v($conf_ptr->{mux_settings}->{OUT_port_name}->{val}) val=\'slew_upper_threshold_pct_$output_vector_type*vsp\' $output_vector_type=1 td=\'(2*N+2)*tprog\'\n",0);
    &tab_print($spfh,".measure tran pdynamic_rise avg p(vsupply) from=\'(2*N+2)*tprog\' to=\'(2*N+2)*tprog+input_slew+slew_mux_rise\'\n",0);
    &tab_print($spfh,".measure tran energy_per_toggle_rise param=\'pdynamic_rise*(input_slew+slew_mux_rise)\'\n",0);
    &tab_print($spfh,"* Measure delay: falling edge \n",0);
    $input_vector_type = "fall";
    $output_vector_type = "fall";
    &tab_print($spfh,".measure tran dly_mux_fall trig v($conf_ptr->{mux_settings}->{IN_port_prefix}->{val}$delay_measure_input) val=\'input_threshold_pct_$input_vector_type*vsp\' $input_vector_type=1 td=\'(2*N+2)*tprog\'\n",0);
    &tab_print($spfh,"+                     targ v($conf_ptr->{mux_settings}->{OUT_port_name}->{val}) val=\'output_threshold_pct_$output_vector_type*vsp\' $output_vector_type=1 td=\'(2*N+2)*tprog\'\n",0);
    &tab_print($spfh,".measure tran slew_mux_fall trig v($conf_ptr->{mux_settings}->{OUT_port_name}->{val}) val=\'slew_lower_threshold_pct_$output_vector_type*vsp\' $output_vector_type=1 td=\'(2*N+2)*tprog\'\n",0);
    &tab_print($spfh,"+                      targ v($conf_ptr->{mux_settings}->{OUT_port_name}->{val}) val=\'slew_upper_threshold_pct_$output_vector_type*vsp\' $output_vector_type=1 td=\'(2*N+2)*tprog\'\n",0);
    &tab_print($spfh,".measure tran pdynamic_fall avg p(vsupply) from=\'(2*N+3)*tprog\' to=\'(2*N+3)*tprog+input_slew+slew_mux_fall\'\n",0);
    &tab_print($spfh,".measure tran energy_per_toggle_fall param=\'pdynamic_fall*(input_slew+slew_mux_fall)\'\n",0);

    &tab_print($spfh,".measure tran avg_vmux_high avg V($conf_ptr->{mux_settings}->{OUT_port_name}->{val}) from=\'(2*N+2)*tprog+input_slew+slew_mux_rise\' to=\'(2*N+3)*tprog\'\n",0);
    &tab_print($spfh,".measure tran avg_vmux_low avg V($conf_ptr->{mux_settings}->{OUT_port_name}->{val}) from=\'(2*N+3)*tprog+input_slew+slew_mux_fall\' to=\'(4*N+2)*tprog\'\n",0);
  }
  &tab_print($spfh,".end Sub-Vt MUX HSPICE Bench\n",0);

} 

sub gen_mux_sp_measure($ $ $ $ $ $ $) 
{
  my ($spfh,$tran_step,$tran_time,$delay_measure,$delay_measure_input,$input_vector_type,$output_vector_type) = @_;
 
  # .Tran
  &tab_print($spfh,".tran $tran_step $tran_time\n",0);
  &tab_print($spfh,"\n",0);
  # Measure

  &tab_print($spfh,".measure tran pleak avg p(vsupply) from=0 to=\'input_pwl\'\n",0);

  if (1 == $delay_measure) {
    &tab_print($spfh,".measure tran dly_mux trig v($conf_ptr->{mux_settings}->{IN_port_prefix}->{val}$delay_measure_input) val=\'input_threshold_pct_$input_vector_type*vsp\' $input_vector_type=1\n",0);
    &tab_print($spfh,"+                     targ v($conf_ptr->{mux_settings}->{OUT_port_name}->{val}) val=\'output_threshold_pct_$output_vector_type*vsp\' $output_vector_type=1 td=\'input_pwl\'\n",0);
    &tab_print($spfh,".measure tran slew_mux trig v($conf_ptr->{mux_settings}->{OUT_port_name}->{val}) val=\'slew_lower_threshold_pct_$output_vector_type*vsp\' $output_vector_type=1 td=\'input_pwl\'\n",0);
    &tab_print($spfh,"+                      targ v($conf_ptr->{mux_settings}->{OUT_port_name}->{val}) val=\'slew_upper_threshold_pct_$output_vector_type*vsp\' $output_vector_type=1 td=\'input_pwl\'\n",0);
    &tab_print($spfh,".measure tran pdynamic avg p(vsupply) from=\'input_pwl\' to=\'input_pwl+input_slew+slew_mux\'\n",0);
    &tab_print($spfh,".measure tran energy_per_toggle param=\'pdynamic*(input_slew+slew_mux)\'\n",0);
    &tab_print($spfh,".measure tran avg_vmux avg V($conf_ptr->{mux_settings}->{OUT_port_name}->{val}) from=\'input_slew+slew_mux\' to=\'$tran_time\'\n",0);
  }
  &tab_print($spfh,".end Sub-Vt MUX HSPICE Bench\n",0);

}

sub gen_1level_mux_subckt($ $ $ $ $ $) {
  my ($spfh,$mux_size,$subckt_name,$mux1level_subckt,$buffered,$rram_enhance) = @_;
  my ($num_sram) = ($mux_size);
  my ($ongap_kw, $ongap_val, $offgap_kw, $offgap_val);

  if ("on" eq $rram_enhance) {
    my ($rram_init_on_gap, $rram_init_off_gap);
    # Find initial parameters for RRAMs that are initialized to ON/OFF states.
    $rram_init_on_gap = $conf_ptr->{rram_settings}->{rram_initial_on_gap}->{val};
    $rram_init_off_gap = $conf_ptr->{rram_settings}->{rram_initial_off_gap}->{val};
    ($ongap_kw, $ongap_val) = split /:/,$rram_init_on_gap;
    ($offgap_kw, $offgap_val) = split /:/,$rram_init_off_gap;
    &tab_print($spfh,".global ",0);
    for (my $i = 0; $i < ($mux_size+1); $i++) {
      &tab_print($spfh,"bl[$i]_b wl[$i] ",0);
    }
    &tab_print($spfh,"\n",0);
  }

  # Print definitions 
  &tab_print($spfh,".subckt $subckt_name ",0);
  for (my $i=0; $i<$num_sram; $i++) {
    &tab_print($spfh,"$conf_ptr->{mux_settings}->{SRAM_port_prefix}->{val}$i ",0);
    &tab_print($spfh,"$conf_ptr->{mux_settings}->{invert_SRAM_port_prefix}->{val}$i ",0);
  }
  for (my $i=0; $i<$mux_size; $i++) {
    &tab_print($spfh,"$conf_ptr->{mux_settings}->{IN_port_prefix}->{val}$i ",0);
  }
  &tab_print($spfh,"$conf_ptr->{mux_settings}->{OUT_port_name}->{val} ",0);
  &tab_print($spfh,"svdd sgnd \n", 0);

  # Print array of transistors/RRAMs
  if ("on" eq $rram_enhance) {
    for (my $i = 0; $i < $mux_size; $i++) {
      #  Program Pair 
      &tab_print($spfh, "Xprog_pmos$i mux1level_in$i bl[$i]_b prog_vdd prog_vdd $elc_prog_pmos_subckt_name L=\'prog_pl\' W=\'wprog*prog_wp*prog_beta\'\n",0);
      &tab_print($spfh, "Xprog_nmos$i mux1level_in$i wl[$i] prog_gnd prog_gnd $elc_prog_nmos_subckt_name L=\'prog_nl\' W=\'wprog*prog_wn\'\n",0);
      # Initilization: RRAM0 (in0) is off, rest of RRAMs are on. Programming will switch RRAM0 to on and the rest to off.
      if (0 == $i) {
        &tab_print($spfh, "Xrram$i mux1level_in$i mux1level_out $conf_ptr->{rram_settings}->{rram_subckt_name}->{val} $offgap_kw=gap_off\n");
        #&tab_print($spfh,"Rmux1level_$i mux1level_in$i mux1level_out \'ron\'\n",0);
      } else { 
        &tab_print($spfh, "Xrram$i mux1level_in$i mux1level_out $conf_ptr->{rram_settings}->{rram_subckt_name}->{val} $ongap_kw=gap_on\n");
        #&tab_print($spfh,"Rmux1level_$i mux1level_in$i mux1level_out \'roff\'\n",0);
      }
    }
    # Add output program pair 
    &tab_print($spfh, "Xprog_pmos$mux_size mux1level_out bl[$mux_size]_b prog_vdd prog_vdd $elc_prog_pmos_subckt_name L=\'prog_pl\' W=\'wprog*prog_wp*prog_beta\'\n",0);
    &tab_print($spfh, "Xprog_nmos$mux_size mux1level_out wl[$mux_size] prog_gnd prog_gnd $elc_prog_nmos_subckt_name L=\'prog_nl\' W=\'wprog*prog_wn\'\n",0);
  } else {
    for (my $i = 0; $i < $mux_size; $i++) {
      # Call defined 1level Subckt 
      &tab_print($spfh, "Xmux1level_$i mux1level_in$i mux1level_out $conf_ptr->{mux_settings}->{SRAM_port_prefix}->{val}$i $conf_ptr->{mux_settings}->{invert_SRAM_port_prefix}->{val}$i svdd sgnd $mux1level_subckt\n",0);
    }
  }

  # Add buffers
  for (my $i=0; $i < $mux_size; $i++) {
    if ("buffered" eq $buffered) {
      &tab_print($spfh,"Xinv$i $conf_ptr->{mux_settings}->{IN_port_prefix}->{val}$i mux1level_in$i ",0); 
      &tab_print($spfh,"svdd sgnd $conf_ptr->{inv_settings}->{inv_subckt_name}->{val} size=\'inv_size_in\'\n",0);
      #&tab_print($spfh,"$conf_ptr->{general_settings}->{VDD_port_name}->{val} $conf_ptr->{general_settings}->{GND_port_name}->{val} $conf_ptr->{inv_settings}->{inv_subckt_name}->{val} size=\'inv_size_in\'\n",0);
    } else {
      &tab_print($spfh,"V$i $conf_ptr->{mux_settings}->{IN_port_prefix}->{val}$i muxlevel_in$i 0\n",0); 
    }
  }

  if ("on" eq $opt_ptr->{auto_out_tapered_buffer}) {
    my ($tapbuf_size) = (4**$opt_ptr->{auto_out_tapered_buffer_val});
    # Call the subckt 
    &tab_print($spfh, "Xtapbuf mux1level_out $conf_ptr->{mux_settings}->{OUT_port_name}->{val} svdd sgnd tapbuf_size$tapbuf_size\n", 0);
  } elsif ("buffered" eq $buffered) {
    &tab_print($spfh,"Xinv_out mux1level_out ",0); 
    &tab_print($spfh,"$conf_ptr->{mux_settings}->{OUT_port_name}->{val} svdd sgnd $conf_ptr->{inv_settings}->{inv_subckt_name}->{val} size=\'inv_size_out\'\n",0);
  } else {
    &tab_print($spfh,"Vout mux1level_out ",0); 
    &tab_print($spfh,"$conf_ptr->{mux_settings}->{OUT_port_name}->{val} 0\n",0);
  } 

  # Print end of subckt
  &tab_print($spfh,".eom $subckt_name\n",0);
}

sub gen_2level_mux_subckt($ $ $ $ $ $) {
  my ($spfh,$mux_size,$subckt_name,$mux2level_subckt,$buffered,$rram_enhance) = @_;
  my ($basis) = (&determine_2level_mux_basis($mux_size));
  my ($num_sram) = (2*$basis);

  print "Two-level MUX: number of a basis is auto-set to $basis.\n";
  print "Two-level MUX: number of configurable bits is auto-set to $num_sram.\n";

  # Print definitions 
  &tab_print($spfh,".subckt $subckt_name ",0);
  for (my $i=0; $i<$num_sram; $i++) {
    &tab_print($spfh,"$conf_ptr->{mux_settings}->{SRAM_port_prefix}->{val}$i ",0);
    &tab_print($spfh,"$conf_ptr->{mux_settings}->{invert_SRAM_port_prefix}->{val}$i ",0);
  }
  for (my $i=0; $i<$mux_size; $i++) {
    &tab_print($spfh,"$conf_ptr->{mux_settings}->{IN_port_prefix}->{val}$i ",0);
  }
  &tab_print($spfh,"$conf_ptr->{mux_settings}->{OUT_port_name}->{val} ",0);
  &tab_print($spfh,"svdd sgnd \n", 0);

  # Print array of transistors/RRAMs
  if ($rram_enhance) {
    # First level 
    for (my $i = 0; $i < $mux_size; $i++) {
      my ($in2lvl) = int($i/$basis);
      my ($offset) = $i % $basis;
      # Call defined 1level Subckt - Program Pair 
      #&tab_print($spfh, "Xmux2_l2_$i mux2_l2_in$i $conf_ptr->{mux_settings}->{SRAM_port_prefix}->{val}$offset $conf_ptr->{mux_settings}->{invert_SRAM_port_prefix}->{val}$offset svdd sgnd $mux2level_subckt wprog=\'wprog\'\n",0);
      #if (0 == $i) {
      #  &tab_print($spfh,"Rmux2level_$i mux2_l2_in$i mux2_l1_in$in2lvl \'ron\'\n",0);
      #} else { 
      #  &tab_print($spfh,"Rmux2level_$i mux2_l2_in$i mux2_l1_in$in2lvl \'roff\'\n",0);
      #}
    }
    # Add output program pair 
    for (my $i = 0; $i < $basis; $i++) {
      #&tab_print($spfh, "Xmux2_l2_progpairout mux2_l1_in$i $conf_ptr->{mux_settings}->{SRAM_port_prefix}->{val}$i $conf_ptr->{mux_settings}->{invert_SRAM_port_prefix}->{val}$i svdd sgnd $mux2level_subckt wprog=\'wprog\'\n",0);
    }
    # Add intermedia inverter 
    # Second level 
    for (my $i = 0; $i < $basis; $i++) {
      my ($offset) = ($basis + $i);
    }
    # Add output program pair 
   ;
    # Add intermedia inverter 
  } else {
    # CMOS 2-level MUX
    # First level 
    for (my $i = 0; $i < $mux_size; $i++) {
      my ($in2lvl) = int($i/$basis);
      my ($offset) = ($i % $basis);
      # Call defined 2level Subckt 
      &tab_print($spfh, "Xmux2_l1_in$i mux2_l1_in$i mux2_l0_in$in2lvl $conf_ptr->{mux_settings}->{SRAM_port_prefix}->{val}$offset $conf_ptr->{mux_settings}->{invert_SRAM_port_prefix}->{val}$offset svdd sgnd $mux2level_subckt\n",0);
    }
    # Second level 
    for (my $i = 0; $i < $basis; $i++) {
      my ($offset) = ($basis + $i);
      &tab_print($spfh, "Xmux2_l0_in$i mux2_l0_in$i mux2_l0_in0 $conf_ptr->{mux_settings}->{SRAM_port_prefix}->{val}$offset $conf_ptr->{mux_settings}->{invert_SRAM_port_prefix}->{val}$offset svdd sgnd $mux2level_subckt\n",0);
    }
  }

  # Add buffers
  for (my $i=0; $i < $mux_size; $i++) {
    if ("buffered" eq $buffered) {
      &tab_print($spfh,"Xinv$i $conf_ptr->{mux_settings}->{IN_port_prefix}->{val}$i mux2_l1_in$i ",0); 
      &tab_print($spfh,"svdd sgnd $conf_ptr->{inv_settings}->{inv_subckt_name}->{val} size=\'inv_size_in\'\n",0);
      #&tab_print($spfh,"$conf_ptr->{general_settings}->{VDD_port_name}->{val} $conf_ptr->{general_settings}->{GND_port_name}->{val} $conf_ptr->{inv_settings}->{inv_subckt_name}->{val} size=\'inv_size_in\'\n",0);
    } else {
      &tab_print($spfh,"R$i $conf_ptr->{mux_settings}->{IN_port_prefix}->{val}$i muxlevel_in$i 0\n",0); 
    }
  }

  if ("on" eq $opt_ptr->{auto_out_tapered_buffer}) {
    my ($tapbuf_size) = (4**$opt_ptr->{auto_out_tapered_buffer_val});
    # Call the subckt 
    &tab_print($spfh, "Xtapbuf mux2_l0_in0 $conf_ptr->{mux_settings}->{OUT_port_name}->{val} svdd sgnd tapbuf_size$tapbuf_size\n", 0);
  } elsif ("buffered" eq $buffered) {
    &tab_print($spfh,"Xinv_out mux2_l0_in0 ",0); 
    &tab_print($spfh,"$conf_ptr->{mux_settings}->{OUT_port_name}->{val} svdd sgnd $conf_ptr->{inv_settings}->{inv_subckt_name}->{val} size=\'inv_size_out\'\n",0);
  } else {
    &tab_print($spfh,"Rout mux2_l0_in0 ",0); 
    &tab_print($spfh,"$conf_ptr->{mux_settings}->{OUT_port_name}->{val} 0\n",0);
  } 

  # Print end of subckt
  &tab_print($spfh,".eom $subckt_name\n",0);
}

sub gen_multilevel_mux_subckt($ $ $ $ $ $)
{
  my ($spfh,$mux_size,$subckt_name,$mux2_subckt,$buffered,$rram_enhance) = @_;
  my ($num_sram) = (&determine_mux_level($mux_size));
  
  #if ($num_sram != $mux_level_esti) {
  #  die "Error: estimated MUX level is $mux_level_esti. But given SRAM bits consist of $num_sram bits!\n";
  #}

  # Check input_vectors - will be reused latter elsewhere
  #my @input_vects = split /,/,$input_vectors;
  #if ($mux_size != ($#input_vects + 1)) {
  #  die "Error: MUX size is $mux_size, but given input vectors consist of ".$#input_vects+1."bits!\n";
  #}
 
  # Print definitions 
  &tab_print($spfh,".subckt $subckt_name ",0);
  for (my $i=0; $i<$num_sram; $i++) {
    &tab_print($spfh,"$conf_ptr->{mux_settings}->{SRAM_port_prefix}->{val}$i ",0);
    &tab_print($spfh,"$conf_ptr->{mux_settings}->{invert_SRAM_port_prefix}->{val}$i ",0);
  }
  for (my $i=0; $i<$mux_size; $i++) {
    &tab_print($spfh,"$conf_ptr->{mux_settings}->{IN_port_prefix}->{val}$i ",0);
  }
  &tab_print($spfh,"$conf_ptr->{mux_settings}->{OUT_port_name}->{val} ",0);
  &tab_print($spfh,"svdd sgnd\n");

  my ($num_inputs_last) = (&mux_last_level_input_num($num_sram,$mux_size));

  # Determine the level and index of per MUX inputs
  my %mux_inputs_ports;
  my ($mux_inputs_ports_ref) = (\%mux_inputs_ports);
  for (my $i=0; $i < $num_inputs_last; $i++) {
    # Priority : fill the last level
    $mux_inputs_ports_ref->{$i}->{level} = $num_sram;
    $mux_inputs_ports_ref->{$i}->{offset} = $i;
  }
  if ($mux_size > $num_inputs_last) {
    my ($cur) = ($num_inputs_last/2);
    for (my $i=$num_inputs_last; $i < $mux_size; $i++) {
      $mux_inputs_ports_ref->{$i}->{level} = $num_sram - 1;
      $mux_inputs_ports_ref->{$i}->{offset} = $cur;
      $cur++;
    } 
    if ($cur > 2**($num_sram-1)) {
      die "Error: fatal error in determining level($num_sram) and index($cur) of per MUX inputs!\n";
    }
  }

  my ($mux2_cnt) = (0);
  # Print internal structure
  for (my $i=0; $i < $num_sram; $i++) {
    my ($level,$nextlevel) = ($num_sram-$i,$num_sram-$i-1); 
    my ($mux2_in,$mux2_out) = ("mux2_l$level\_in","mux2_l$nextlevel\_in");
    my ($num_input_cur_level) = (2**$level);
    if (($level == $num_sram)&&($num_inputs_last < $num_input_cur_level)) {
      $num_input_cur_level = $num_inputs_last;
    }

    for (my $j=0; $j < $num_input_cur_level; $j++) {
      my ($nextj) = ($j + 1);
      my ($out_idx) = int($j/2);
      &tab_print($spfh,"Xmux2_no$mux2_cnt $mux2_in$j $mux2_in$nextj $conf_ptr->{mux_settings}->{SRAM_port_prefix}->{val}$nextlevel $conf_ptr->{mux_settings}->{invert_SRAM_port_prefix}->{val}$nextlevel $mux2_out$out_idx svdd sgnd $mux2_subckt ",0); 
      if ($rram_enhance) {
        &tab_print($spfh,"ron=\'ron\' wprog=\'wprog\'",0);
      }
      &tab_print($spfh,"\n",0);
      if ($rram_enhance) {
        my ($nextnextj) = ($nextj + 1);
        if (0 == $j) { 
          &tab_print($spfh,"Xprog_extral$i $mux2_in$j sgnd sgnd sgnd elc_nmos L=$conf_ptr->{general_settings}->{nl}->{val} W=\'wprog*$conf_ptr->{general_settings}->{min_wn}->{val}\'\n",0);
        }
        if ($nextnextj < $num_input_cur_level) {
          &tab_print($spfh,"Xprog_no$mux2_cnt $mux2_in$nextj gnd $mux2_in$nextnextj sgnd elc_nmos L=$conf_ptr->{general_settings}->{nl}->{val} W=\'wprog*$conf_ptr->{general_settings}->{min_wn}->{val}\'\n",0);
        } else {
          &tab_print($spfh,"Xprog_no$mux2_cnt $mux2_in$nextj sgnd sgnd sgnd elc_nmos L=$conf_ptr->{general_settings}->{nl}->{val} W=\'wprog*$conf_ptr->{general_settings}->{min_wn}->{val}\'\n",0);
        }
      }
      $j = $nextj;
      $mux2_cnt++;
    } 
  }

  for (my $i=0; $i < $mux_size; $i++) {
    if ("buffered" eq $buffered) {
      &tab_print($spfh,"Xinv$i $conf_ptr->{mux_settings}->{IN_port_prefix}->{val}$i mux2_l$mux_inputs_ports_ref->{$i}->{level}\_in$mux_inputs_ports_ref->{$i}->{offset} ",0); 
      &tab_print($spfh,"svdd sgnd $conf_ptr->{inv_settings}->{inv_subckt_name}->{val} size=\'inv_size_in\'\n",0);
    } else {
      &tab_print($spfh,"V$i $conf_ptr->{mux_settings}->{IN_port_prefix}->{val}$i mux2_l$mux_inputs_ports_ref->{$i}->{level}\_in$mux_inputs_ports_ref->{$i}->{offset} 0\n",0); 
    }
  }

  if ("on" eq $opt_ptr->{auto_out_tapered_buffer}) {
    my ($tapbuf_size) = (4**$opt_ptr->{auto_out_tapered_buffer_val});
    # Call the subckt 
    &tab_print($spfh, "Xtapbuf mux2_l0_in0 $conf_ptr->{mux_settings}->{OUT_port_name}->{val} svdd sgnd tapbuf_size$tapbuf_size\n", 0);
  } elsif ("buffered" eq $buffered) {
    &tab_print($spfh,"Xinv_out mux2_l0_in0 ",0); 
    &tab_print($spfh,"$conf_ptr->{mux_settings}->{OUT_port_name}->{val} svdd sgnd $conf_ptr->{inv_settings}->{inv_subckt_name}->{val} size=\'inv_size_out\'\n",0);
  } else {
    &tab_print($spfh,"Vout mux2_l0_in0 ",0); 
    &tab_print($spfh,"$conf_ptr->{mux_settings}->{OUT_port_name}->{val} 0\n",0);
  } 

  # Add programming transistor at the output
  if ($rram_enhance) {
    &tab_print($spfh,"Xprog_out mux2_l0_in0 sgnd sgnd sgnd elc_nmos L=$conf_ptr->{general_settings}->{nl}->{val} W=\'wprog*$conf_ptr->{general_settings}->{min_wn}->{val}\'\n",0);
  }

  # Print end of subckt
  &tab_print($spfh,".eom $subckt_name\n",0);

}


# Generate Spice netlist -- Common Part
sub gen_dff_sp_common($ $ $ $ $ $ $ $ $ $)
{
  my ($spfh,$usage,$cload,$clk_pwl,$clk_pwh,$clk_slew,$d_slew,$clk2d,$thold,$vsp) = @_;

  # Print Header
  &tab_print($spfh,"Sub-Vt DFF HSPICE Bench\n",0);
  &tab_print($spfh,"**********************\n",0);
  &tab_print($spfh,"*   HSPICE Netlist   *\n",0);
  &tab_print($spfh,"* Author: Xifan TANG *\n",0);
  &tab_print($spfh,"*         EPFL, LSI  *\n",0);
  &tab_print($spfh,"* Date: $mydate    *\n",0);
  &tab_print($spfh,"**********************\n",0);
  &tab_print($spfh,"* Test Bench Usage: $usage\n",0);

  &gen_common_sp_technology($spfh); 

  &gen_common_sp_parameters($spfh,$vsp);
    
  &tab_print($spfh,"* Parameters for measurements\n",0);
  &tab_print($spfh,".param clk2d=$clk2d\n",0);
  &tab_print($spfh,".param clk_pwl=$clk_pwl\n",0);
  &tab_print($spfh,".param clk_pwh=$clk_pwh\n",0);
  &tab_print($spfh,".param clk_slew=$clk_slew\n",0);
  &tab_print($spfh,".param d_slew=$d_slew\n",0);
  &tab_print($spfh,".param thold=$thold\n",0);
 
  &tab_print($spfh,"* Include Circuits Library\n",0);

  &tab_print($spfh,".include $cwd/$opt_ptr->{dff_val}\n",0);

  # Now Call the sub circuit
  &tab_print($spfh,"Xdff_ckt $conf_ptr->{dff_settings}->{D_port_name}->{val} $conf_ptr->{dff_settings}->{CLK_port_name}->{val} $conf_ptr->{dff_settings}->{Q_port_name}->{val} $conf_ptr->{dff_settings}->{SET_port_name}->{val} $conf_ptr->{dff_settings}->{RST_port_name}->{val}  $conf_ptr->{dff_settings}->{dff_subckt_name}->{val}\n",0);

  # Add Load Cap
  if (($conf_ptr->{dff_settings}->{load_type}->{val} =~ m/cap/)
     &&(0 < $cload)) {
    &tab_print($spfh,"Cload $conf_ptr->{dff_settings}->{Q_port_name}->{val} $conf_ptr->{general_settings}->{GND_port_name}->{val} $cload\n",0);
  }
  if ($conf_ptr->{dff_settings}->{load_type}->{val} =~ m/inv/) {
    &gen_sp_inv_loads($spfh, $conf_ptr->{dff_settings}->{Q_port_name}->{val},
                      $conf_ptr->{dff_settings}->{load_inv_size}->{val},
                      $conf_ptr->{dff_settings}->{load_inv_num}->{val}); 
  }

  # Add Stimulates
  &tab_print($spfh,"Vsupply $conf_ptr->{general_settings}->{VDD_port_name}->{val} $conf_ptr->{general_settings}->{GND_port_name}->{val} vsp\n",0);

  if ("0" ne $conf_ptr->{general_settings}->{GND_port_name}->{val}) {
    &tab_print($spfh,"Vgnd $conf_ptr->{general_settings}->{GND_port_name}->{val} 0 0\n",0);
  }
  
  &tab_print($spfh,"* Common Part Over.\n",0);
  
}

sub gen_dff_sp_setup($ $)
{
  my ($spfh,$type) = @_;
  my ($vh,$vl) = ("vsp",0);
  if ($type eq "fall") { 
    ($vh,$vl) = (0,"vsp");
  }
  # Generate Set and Reset
  &tab_print($spfh,"Vreset $conf_ptr->{dff_settings}->{RST_port_name}->{val} $conf_ptr->{general_settings}->{GND_port_name}->{val} 0\n",0);
  &tab_print($spfh,"Vset $conf_ptr->{dff_settings}->{SET_port_name}->{val} $conf_ptr->{general_settings}->{GND_port_name}->{val} 0\n",0);
  # Generate CLK
  &tab_print($spfh,"Vclk $conf_ptr->{dff_settings}->{CLK_port_name}->{val} $conf_ptr->{general_settings}->{GND_port_name}->{val} pwl(0 0 \'clk_pwl\' 0 \'clk_pwl+clk_slew\' vsp \'clk_pwl+clk_slew+clk_pwh\' vsp \'clk_pwl+clk_slew+clk_pwh+clk_slew\' 0 \'clk_pwl+clk_slew+clk_pwh+clk_slew+clk_pwl\' 0 \'clk_pwl+clk_slew+clk_pwh+clk_slew+clk_pwl+clk_slew\' vsp)\n",0);
  # Generate D
  &tab_print($spfh,"Vd $conf_ptr->{dff_settings}->{D_port_name}->{val} $conf_ptr->{general_settings}->{GND_port_name}->{val} pwl(0 $vl \'2*clk_pwl+2*d_slew+clk_pwh-clk2d\' $vl \'2*clk_pwl-clk2d+3*d_slew+clk_pwh\' $vh)\n",0);
  
}

sub gen_dff_sp_hold($ $ $ $ $ $)
{
  my ($spfh,$type,$thold) = @_;
  my ($vh,$vl) = ("vsp",0);
  if ($type eq "fall") { 
    ($vh,$vl) = (0,"vsp");
  }
  # Generate Set and Reset
  &tab_print($spfh,"Vreset $conf_ptr->{dff_settings}->{RST_port_name}->{val} $conf_ptr->{general_settings}->{GND_port_name}->{val} 0\n",0);
  &tab_print($spfh,"Vset $conf_ptr->{dff_settings}->{SET_port_name}->{val} $conf_ptr->{general_settings}->{GND_port_name}->{val} 0\n",0);
  # Generate CLK
  &tab_print($spfh,"Vclk $conf_ptr->{dff_settings}->{CLK_port_name}->{val} $conf_ptr->{general_settings}->{GND_port_name}->{val} pwl(0 0 \'clk_pwl\' 0 \'clk_pwl+clk_slew\' vsp \'clk_pwl+clk_slew+clk_pwh\' vsp \'clk_pwl+clk_slew+clk_pwh+clk_slew\' 0 \'clk_pwl+clk_slew+clk_pwh+clk_slew+clk_pwl\' 0 \'clk_pwl+clk_slew+clk_pwh+clk_slew+clk_pwl+clk_slew\' vsp)\n",0);
  # Generate D
  &tab_print($spfh,"Vd $conf_ptr->{dff_settings}->{D_port_name}->{val} $conf_ptr->{general_settings}->{GND_port_name}->{val} pwl(0 $vl \'1*clk_pwl+1*d_slew+clk_pwh\' $vl \'1*clk_pwl+2*d_slew+clk_pwh\' $vh ",0);
  #if ($thold > 0) {
  &tab_print($spfh,"  \'2*clk_pwl+clk_pwh+2*d_slew+thold\' $vh \'2*clk_pwl+clk_pwh+3*d_slew+thold\' $vl)\n",0);
  #}
  #else {
  #  &tab_print($spfh,")\n",0);
  #}
}

sub gen_dff_sp_leakage($ $)
{
  my ($spfh,$input_vectors) = @_;
  # Generate Set and Reset
  &tab_print($spfh,"Vreset $conf_ptr->{dff_settings}->{RST_port_name}->{val} $conf_ptr->{general_settings}->{GND_port_name}->{val} $$input_vectors[0]\n",0);
  &tab_print($spfh,"Vset $conf_ptr->{dff_settings}->{SET_port_name}->{val} $conf_ptr->{general_settings}->{GND_port_name}->{val} $$input_vectors[1]\n",0);
  # Generate CLK
  &tab_print($spfh,"Vclk $conf_ptr->{dff_settings}->{CLK_port_name}->{val} $conf_ptr->{general_settings}->{GND_port_name}->{val} pwl(0 0 \'clk_pwl\' 0 \'clk_pwl+clk_slew\' vsp \'clk_pwl+clk_slew+clk_pwh\' vsp \'clk_pwl+2*clk_slew+clk_pwh\' $$input_vectors[2])\n",0);
  # Generate D
  &tab_print($spfh,"Vd $conf_ptr->{dff_settings}->{D_port_name}->{val} $conf_ptr->{general_settings}->{GND_port_name}->{val} pwl(0 $$input_vectors[4] \'clk_pwl+clk_slew+clk_pwh\' $$input_vectors[4] \'clk_pwl+2*clk_slew+clk_pwh\' $$input_vectors[3])\n",0);
    
}

sub gen_dff_measure($ $ $ $ $)
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
  &tab_print($spfh,".meas tran slew_q trig v($conf_ptr->{dff_settings}->{Q_port_name}->{val}) val=\'slew_lower_threshold_pct_$type*vsp\' $type=1 td=\'2*clk_pwl+clk_pwh+2*clk_slew\'\n",0);
  &tab_print($spfh,"+                 targ v($conf_ptr->{dff_settings}->{Q_port_name}->{val}) val=\'slew_upper_threshold_pct_$type*vsp\' $type=1 td=\'2*clk_pwl+clk_pwh+2*clk_slew\'\n",0);
  # Measure Clk To Q 
  &tab_print($spfh,".meas tran clk2q trig v($conf_ptr->{dff_settings}->{CLK_port_name}->{val}) val=\'input_threshold_pct_$type*vsp\' rise=2\n",0);
  &tab_print($spfh,"+                targ v($conf_ptr->{dff_settings}->{Q_port_name}->{val}) val=\'output_threshold_pct_$type*vsp\' $type=1 td=\'2*clk_pwl+clk_pwh+2*clk_slew\'\n",0);
  &tab_print($spfh,".meas tran power_vdd avg p(vsupply) from=\'2*clk_pwl+clk_pwh+2*clk_slew\' to=\'2*clk_pwl+clk_pwh+3*clk_slew+clk2q\'\n",0);
  &tab_print($spfh,".meas tran energy_per_toggle param=\'power_vdd*(clk_slew+clk2q)\'\n",0);
  #&tab_print($spfh,".meas tran avg_vq avg V($conf_ptr->{circuit_definition}->{Q_port_name}->{val}) from=\'2*clk_pwl+clk_pwh+2*slew\' to=\'2*clk_pwl+clk_pwh+3*slew+$clk2q_max\'\n",0);
  &tab_print($spfh,".meas tran avg_vq avg V($conf_ptr->{dff_settings}->{Q_port_name}->{val}) from=\'$tran_stop-$tran_step\' to=$tran_stop\n",0);

  # End of SP
  &tab_print($spfh,".end DFF ELC HSPICE Bench\n",0);
  close($spfh);
}

sub gen_dff_leakage_measure($ $ $)
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
      $results->{$tmp} = "failed";
    } elsif ("failed" ne $results->{$tmp}) {
      $results->{$tmp} = &process_unit($results->{$tmp},"empty");
      $results->{$tmp} = sprintf("%.4g", $results->{$tmp});
    }
    print "Sim Results: $tmp = $results->{$tmp}\n";
  }
  close($lisfh);
}


sub run_lut_sim($ $ $ $ $ $ $ $)
{
  my ($lut_size,$SRAM_bits,$input_vectors,$vsp,$input_slew,$input_pwl,$input_pwh,$cload,$tran_step,$tran_stop,$results) = @_;

  my ($delay_measure,$delay_measure_input,$output_vector,$input_vector_type,$output_vector_type);

  # create folder
  my ($lut_folder) = ("$cwd/"."$conf_ptr->{general_settings}->{spice_dir}->{val}"."lut$lut_size/");
  &generate_path($lut_folder);
  my ($lut_file) = ($lut_folder."lut$lut_size.sp");
  my ($lis_file) = ($lut_file);
  $lis_file =~ s/sp$/lis/;
  my @sim_keywds = ("dly_lut","slew_lut","pleak","pdynamic","avg_vlut","energy_per_toggle");

  $output_vector = &lut_output_vector($lut_size,$SRAM_bits,$input_vectors); 
  $delay_measure_input = &determine_lut_delay_measure_input($lut_size,$input_vectors); 
  ($delay_measure,$input_vector_type,$output_vector_type) = &determine_lut_delay_measure($delay_measure_input,$input_vectors,$output_vector); 

  # Create a SPICE file
  my $spfh = FileHandle->new;
  if ($spfh->open("> $lut_file")) {
    &gen_lut_sp_common($spfh,"lut$lut_size\_delay_power",$cload,$input_pwl,$input_pwh,$input_slew,$vsp);
    &gen_lut_sp_stimulates($spfh,$SRAM_bits,$input_vectors); 
    &gen_lut_sp_measure($spfh,$tran_step,$tran_stop,$delay_measure,$delay_measure_input,$input_vector_type,$output_vector_type);
    close($spfh);
    &run_hspice("$cwd/"."$conf_ptr->{general_settings}->{spice_dir}->{val}","lut$lut_size.sp",$conf_ptr->{general_settings}->{verilogA_hspice_sim}->{val});
    &get_sim_results($lis_file,\@sim_keywds,$results);

    if ($results->{avg_vlut} eq "failed") {
      return 0;
    }

    if (0 == $delay_measure) {
      $results->{pleak} = abs($results->{pleak});      
      if (($results->{pleak} eq "failed")||($results->{pleak} < 0)) {
        return 0;
      }
      return 1; 
    } else {
      if (($output_vector_type eq "r")||($output_vector_type eq "1")) {
        if ($results->{avg_vlut} < $vsp*0.95) {
          return 0;
        }
      }
      if (($output_vector_type eq "f")||($output_vector_type eq "0")) {
        if ($results->{avg_vlut} > $vsp*0.05) {
          return 0;
        }
      }
      $results->{pdynamic} = abs($results->{pdynamic});      
      $results->{energy_per_toggle} = abs($results->{energy_per_toggle});      
      $results->{pleak} = abs($results->{pleak});      
      foreach my $tmp(@sim_keywds) {
        if (($results->{$tmp} eq "failed")||($results->{$tmp} < 0)) {
          return 0;
        }
      }
      return 1; 
    }
  } else {
    die "Fail to create $lut_file!\n";
  }

}

sub run_mux_sim($ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $ $)
{
  my ($mux_size,$SRAM_bits,$input_vectors,$vsp,$input_slew,$input_pwl,$input_pwh,$cload,$tran_step,$tran_stop,$results,$inv_size_in,$inv_size_out,$rram_enhance,$ron,$wprog,$roff,$on_gap, $off_gap) = @_;

  my ($delay_measure,$delay_measure_input,$output_vector,$input_vector_type,$output_vector_type);

  # create folder
  my ($mux_folder) = ("$cwd/"."$conf_ptr->{general_settings}->{spice_dir}->{val}"."mux$mux_size/");
  &generate_path($mux_folder);
  my ($mux_file) = ($mux_folder."mux$mux_size.sp");
  my ($lis_file) = ($mux_file);
  $lis_file =~ s/sp$/lis/;
  my @sim_keywds = ("dly_mux","slew_mux","pleak","pdynamic","avg_vmux","energy_per_toggle");
      
  print "SRAM_bits: $SRAM_bits\n";
  ($delay_measure_input,$output_vector) = &mux_output_vector($mux_size,$SRAM_bits,$input_vectors); 
  ($delay_measure,$input_vector_type,$output_vector_type) = &determine_lut_delay_measure($delay_measure_input,$input_vectors,$output_vector); 

  #print "MUX size=$mux_size.\nSRAM bits: $SRAM_bits\n";
  #print "MUX Input Vectors: $input_vectors\n";
  #print "MUX Output Vector: $output_vector\nDelay measure input: $delay_measure_input\n";

  # Create a SPICE file
  my $spfh = FileHandle->new;
  if ($spfh->open("> $mux_file")) {
    &gen_mux_sp_common($spfh,"mux$mux_size\_delay_power",$cload,$input_pwl,$input_pwh,$input_slew,$vsp,$mux_size,$inv_size_in,$inv_size_out,$rram_enhance,$ron,$wprog,$roff, $on_gap, $off_gap);
    # FOR RRAM MUX: we need special stimulates for programming phase!
    if ("on" eq $rram_enhance) {
      &gen_rram_mux_sp_stimulates($spfh,$SRAM_bits,$input_vectors); 
      &gen_rram_mux_sp_measure($spfh,$tran_step,$tran_stop,$delay_measure,$delay_measure_input,$input_vector_type,$output_vector_type);
    } else {
    # FOR SRAM MUX: go normally.
      &gen_mux_sp_stimulates($spfh,$SRAM_bits,$input_vectors); 
      &gen_mux_sp_measure($spfh,$tran_step,$tran_stop,$delay_measure,$delay_measure_input,$input_vector_type,$output_vector_type);
    }
    close($spfh);
  } else {
    die "Fail to create $mux_file!\n";
  }
    
  # FOR RRAM MUX, We need two simulations:
  #   1. In the 1st simulation, we find the gap to be programmed.
  #   2. In the 2nd simulation, we use the found gap. Get delay results and do verification on waveforms.
  if ("on" eq $rram_enhance) {
    # Run 1st simulation
    &run_hspice("$cwd/"."$conf_ptr->{general_settings}->{spice_dir}->{val}","mux$mux_size.sp",$conf_ptr->{general_settings}->{verilogA_hspice_sim}->{val});
    # Get Gap
    my @gap_keyword = ($conf_ptr->{rram_settings}->{gap_keyword}->{val});
    &get_sim_results($lis_file,\@gap_keyword,$results);
    $on_gap = $results->{$gap_keyword[0]};
    # Update the source file with found gap.
    if ($spfh->open("> $mux_file")) {
      print "INFO: updating gaps in $mux_file...\n";
      &gen_mux_sp_common($spfh,"mux$mux_size\_delay_power",$cload,$input_pwl,$input_pwh,$input_slew,$vsp,$mux_size,$inv_size_in,$inv_size_out,$rram_enhance,$ron,$wprog,$roff,$on_gap, $off_gap);
      # FOR RRAM MUX: we need special stimulates for programming phase!
      &gen_rram_mux_sp_stimulates($spfh,$SRAM_bits,$input_vectors); 
      &gen_rram_mux_sp_measure($spfh,$tran_step,$tran_stop,$delay_measure,$delay_measure_input,$input_vector_type,$output_vector_type);
      close($spfh);
    } else {
      die "Fail to create $mux_file!\n";
    }
    # Run 2nd simulation
    &run_hspice("$cwd/"."$conf_ptr->{general_settings}->{spice_dir}->{val}","mux$mux_size.sp",$conf_ptr->{general_settings}->{verilogA_hspice_sim}->{val});
    # Get results 
    @sim_keywds = ("dly_mux_rise","dly_mux_fall", "slew_mux_rise", "slew_mux_fall", "pleak","pdynamic_rise","pdynamic_fall", "avg_vmux_high", "avg_vmux_low", "energy_per_toggle_rise", "energy_per_toggle_fall");
    &get_sim_results($lis_file,\@sim_keywds,$results);
    if (($results->{avg_vmux_high} eq "failed")||($results->{avg_vmux_low} eq "failed")) {
      return 0;
    } elsif (($results->{avg_vmux_high} < $vsp*0.95)||($results->{avg_vmux_low} > $vsp*0.05)) {
      return 0;
    }
    print "Output_vector_type: $output_vector_type\n";
    if (($output_vector_type eq "rise")||($output_vector_type eq "1")) {
      $results->{dly_mux} = $results->{dly_mux_rise};      
      $results->{slew_mux} = $results->{slew_mux_rise};      
      $results->{pdynamic} = abs($results->{pdynamic_rise});      
      $results->{energy_per_toggle} = abs($results->{energy_per_toggle_rise});      
      $results->{avg_vmux} = $results->{avg_vmux_high};      
    }
    if (($output_vector_type eq "fall")||($output_vector_type eq "0")) {
      $results->{dly_mux} = $results->{dly_mux_fall};      
      $results->{slew_mux} = $results->{slew_mux_fall};      
      $results->{pdynamic} = abs($results->{pdynamic_fall});      
      $results->{energy_per_toggle} = abs($results->{energy_per_toggle_fall});      
      $results->{avg_vmux} = $results->{avg_vmux_high};      
    }
    $results->{pleak} = abs($results->{pleak});      
    # Revert to old keywords
    @sim_keywds = ("dly_mux","slew_mux","pleak","pdynamic","avg_vmux","energy_per_toggle");
    foreach my $tmp(@sim_keywds) {
      if (($results->{$tmp} eq "failed")||($results->{$tmp} < 0)) {
        return 0;
      }
    }
  } else {
  # FOR SRAM MUX, we need only one simulation.
    &run_hspice("$cwd/"."$conf_ptr->{general_settings}->{spice_dir}->{val}","mux$mux_size.sp",$conf_ptr->{general_settings}->{verilogA_hspice_sim}->{val});
    &get_sim_results($lis_file,\@sim_keywds,$results);

    if ($results->{avg_vmux} eq "failed") {
      return 0;
    }

    if (0 == $delay_measure) {
      $results->{pleak} = abs($results->{pleak});      
      if (($results->{pleak} eq "failed")||($results->{pleak} < 0)) {
        return 0;
      }
      return 1; 
    } else {
      if (($output_vector_type eq "rise")||($output_vector_type eq "1")) {
        if ($results->{avg_vmux} < $vsp*0.95) {
          return 0;
        }
      }
       if (($output_vector_type eq "fall")||($output_vector_type eq "0")) {
        if ($results->{avg_vmux} > $vsp*0.05) {
          return 0;
        }
      }
      $results->{pdynamic} = abs($results->{pdynamic});      
      $results->{pleak} = abs($results->{pleak});      
      $results->{energy_per_toggle} = abs($results->{energy_per_toggle});      
      foreach my $tmp(@sim_keywds) {
        if (($results->{$tmp} eq "failed")||($results->{$tmp} < 0)) {
          return 0;
        }
      }
      return 1; 
    }
  }
}


# Run once Simulation - Setup Time, Fall|Rise
sub run_setup_sim($ $ $ $ $ $ $ $ $ $ $ $)
{
  my ($type,$clk_slew,$d_slew,$clk_pwl,$clk_pwh,$clk2d,$cload,$tran_step,$tran_stop,$results,$clk2q_max,$vsp) = @_;

  # create folder
  my ($setup_folder) = ("$cwd/"."$conf_ptr->{general_settings}->{spice_dir}->{val}"."setup_time/");
  &generate_path($setup_folder);
  my ($setup_file) = ($setup_folder."setup_time.sp");
  my ($lis_file) = ($setup_file);
  $lis_file =~ s/sp$/lis/;
  my @sim_keywds = ("slew_q","clk2q","power_vdd","avg_vq","energy_per_toggle");

  print "Setup($type) Time : $clk2d\n";
  # Create a SPICE file
  my $spfh = FileHandle->new;
  if ($spfh->open("> $setup_file")) {
    &gen_dff_sp_common($spfh,"setup_$type",$cload,$clk_pwl,$clk_pwh,$clk_slew,$d_slew,$clk2d,0,$vsp);
    &gen_dff_sp_setup($spfh,$type); 
    &gen_dff_measure($spfh,$type,$tran_step,$tran_stop,$clk2q_max);
    &run_hspice("$cwd/"."$conf_ptr->{general_settings}->{spice_dir}->{val}","setup_time.sp",$conf_ptr->{general_settings}->{verilogA_hspice_sim}->{val});
    &get_sim_results($lis_file,\@sim_keywds,$results);
    if (("fall" eq $type)&&($results->{avg_vq} > $vsp/2)) {
      return 0;
    }
    if (("rise" eq $type)&&($results->{avg_vq} < $vsp/2)) {
      return 0;
    }
    $results->{power_vdd} = abs($results->{power_vdd});
    $results->{avg_vq} = abs($results->{avg_vq});
    $results->{energy_per_toggle} = abs($results->{energy_per_toggle});
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
sub run_hold_sim($ $ $ $ $ $ $ $ $ $ $ $ $)
{
  my ($type,$clk_slew,$d_slew,$clk_pwl,$clk_pwh,$clk2d,$thold,$cload,$tran_step,$tran_stop,$results,$clk2q_max,$vsp) = @_;

  # create folder
  my ($hold_folder) = ("$cwd/"."$conf_ptr->{general_settings}->{spice_dir}->{val}"."hold_time/");
  &generate_path($hold_folder);
  my ($hold_file) = ($hold_folder."hold_time.sp");
  my ($lis_file) = ($hold_file);
  $lis_file =~ s/sp$/lis/;
  my @sim_keywds = ("slew_q","clk2q","power_vdd","avg_vq", "energy_per_toggle");

  print "Hold($type) Time : $thold\n";
  # Create a SPICE file
  my $spfh = FileHandle->new;
  if ($spfh->open("> $hold_file")) {
    &gen_dff_sp_common($spfh,"hold_$type",$cload,$clk_pwl,$clk_pwh,$clk_slew,$d_slew,$clk2d,$thold,$vsp);
    &gen_dff_sp_hold($spfh,$type,$thold); 
    &gen_dff_measure($spfh,$type,$tran_step,$tran_stop,$clk2q_max);
    &run_hspice("$cwd/"."$conf_ptr->{general_settings}->{spice_dir}->{val}","hold_time.sp",$conf_ptr->{general_settings}->{verilogA_hspice_sim}->{val});
    &get_sim_results($lis_file,\@sim_keywds,$results);
    if (("fall" eq $type)&&($results->{avg_vq} > $conf_ptr->{library_settings}->{supply_voltage}->{val}*0.01)) {
      return 0;
    }
    if (("rise" eq $type)&&($results->{avg_vq} < $conf_ptr->{library_settings}->{supply_voltage}->{val}*0.99)) {
      return 0;
    }
    $results->{power_vdd} = abs($results->{power_vdd});
    $results->{energy_per_toggle} = abs($results->{energy_per_toggle});
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

sub binary_search_setup_time($ $ $ $ $ $ $ $ $ $)
{
  my ($tag,$last_success,$last_fail,$clk_slew,$input_slew,$type,$cload,$tran_step,$stop_period,$vsp) = @_;
  my ($tran_stop);
  my (%sim_results);
  my ($sim_results_ref) = \%sim_results;
  my ($success_temp,$fail_temp) = ($last_success,$last_fail);
  my ($clk_ratio) = (1);
  my ($clk2q,$slew_q,$power,$total_power,$power_num,$energy_per_toggle);
  my ($max_slew);
  my ($delay_tolerance) = ($conf_ptr->{dff_settings}->{delay_tolerance}->{val});
  my ($clk_pwl,$clk_pwh) = (abs($last_success),abs($clk_ratio*$last_success));
  
  if ($clk_slew > $input_slew) {
    $max_slew = $clk_slew;
  }
  else {
    $max_slew = $input_slew;
  }
  
  # Got default clk to Q delay and Q slew rate.  
  $tran_stop = 2*(abs($last_success) + $max_slew + $clk_ratio*abs($last_success))+$max_slew;
  if (0 == &run_setup_sim($type,$clk_slew,$input_slew,$clk_pwl,$clk_pwh,$last_success,$cload,$tran_step,$tran_stop,\%sim_results,0,$vsp)) {
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
  $energy_per_toggle = $sim_results_ref->{energy_per_toggle};
  $total_power += $power;
  $power_num++;

  # Get fail
  $tran_stop = 2*(abs($last_success) + $max_slew + $clk_ratio*abs($last_success))+$max_slew;
  if (1 == &run_setup_sim($type,$clk_slew,$input_slew,$clk_pwl,$clk_pwh,$last_fail,$cload,$tran_step,$tran_stop,\%sim_results,0,$vsp)) {
    die "Error: (clk_slew: $clk_slew,d_slew: $input_slew)Initial Fail is a success!\n"; 
  }

  while($stop_period < abs($success_temp-$fail_temp)){
    $tran_stop = 2*(abs($last_success) + $max_slew + $clk_ratio*abs($last_success))+$max_slew;
    my ($temp_try) = ($success_temp+$fail_temp)/2;
    undef %sim_results;
    #if ((1 == &run_setup_sim($type,$clk_slew,$input_slew,$last_success,$clk_ratio*$last_success,$temp_try,$cload,$tran_step,$tran_stop,\%sim_results))&&($sim_results_ref->{slew_q} ne "failed")&&($sim_results_ref->{slew_q} < ($slew_q_std*1.1))&&($sim_results_ref->{clk2q} < ($clk2q_std*1.1))) {
    if ((1 == &run_setup_sim($type,$clk_slew,$input_slew,$clk_pwl,$clk_pwh,$temp_try,$cload,$tran_step,$tran_stop,\%sim_results,$clk2q_std*(1+$delay_tolerance),$vsp))&&($sim_results_ref->{slew_q} ne "failed")&&($sim_results_ref->{clk2q} < ($clk2q_std*(1+$delay_tolerance)))) {
      $success_temp = $temp_try;
      $setup_time = $success_temp;
      $clk2q = $sim_results_ref->{clk2q};
      $slew_q = $sim_results_ref->{slew_q};
      $power = $sim_results_ref->{power_vdd};
      $energy_per_toggle = $sim_results_ref->{energy_per_toggle};
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
  print "Power : $power, Energy per toggle: $energy_per_toggle\n";

  # Record in report hash
  print "Input Slew: $input_slew\n";
  $rpt_ptr->{$tag}->{$type}->{setup_time}->{setup_time} = $setup_time;
  $rpt_ptr->{$tag}->{$type}->{setup_time}->{clk2q} = $clk2q;
  $rpt_ptr->{$tag}->{$type}->{setup_time}->{clk2q_std} = $clk2q_std;
  $rpt_ptr->{$tag}->{$type}->{setup_time}->{slew_q} = $slew_q;
  $rpt_ptr->{$tag}->{$type}->{setup_time}->{slew_q_std} = $slew_q_std;
  $rpt_ptr->{$tag}->{$type}->{setup_time}->{power} = $power;
  $rpt_ptr->{$tag}->{$type}->{setup_time}->{energy_per_toggle} = $energy_per_toggle;
  $rpt_ptr->{$tag}->{$type}->{setup_time}->{avg_power} = $total_power/$power_num;

}

sub binary_search_hold_time($ $ $ $ $ $ $ $ $ $ $)
{
  my ($tag,$last_success,$last_fail,$clk_slew,$input_slew,$type,$setup_time,$cload,$tran_step,$stop_period,$vsp) = @_;
  my ($tran_stop);
  my (%sim_results);
  my ($sim_results_ref) = \%sim_results;
  my ($success_temp,$fail_temp) = ($last_success,$last_fail);
  my ($clk_ratio) = (1);
  my ($clk2q,$slew_q,$power,$total_power,$power_num,$energy_per_toggle);
  my ($delay_tolerance) = ($conf_ptr->{dff_settings}->{delay_tolerance}->{val});
  
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
  if (0 == &run_hold_sim($type,$clk_slew,$input_slew,$last_success,$clk_ratio*$last_success,$setup_time,$last_success,$cload,$tran_step,$tran_stop,\%sim_results,0,$vsp)) {
    die "Error: (clk_slew: $clk_slew,d_slew: $input_slew)Initial success is a failure!\n";
  }
  my ($slew_q_std,$clk2q_std) = ($sim_results_ref->{slew_q},$sim_results_ref->{clk2q});
  my ($hold_time) = ($last_success);

  # Get fail
  $tran_stop = 2*($last_success + $max_slew + $clk_ratio*$last_success)+$max_slew;
  if (1 == &run_hold_sim($type,$clk_slew,$input_slew,$last_success,$clk_ratio*$last_success,$setup_time,$last_fail,$cload,$tran_step,$tran_stop,\%sim_results,0,$vsp)) {
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
    if ((1 == &run_hold_sim($type,$clk_slew,$input_slew,$last_success,$clk_ratio*$last_success,$setup_time,$temp_try,$cload,$tran_step,$tran_stop,\%sim_results,$clk2q_std*(1+$delay_tolerance),$vsp))&&($sim_results_ref->{slew_q} ne "failed")&&($sim_results_ref->{clk2q} < ($clk2q_std*(1+$delay_tolerance)))) {
      $success_temp = $temp_try;
      $hold_time = $success_temp;
      $clk2q = $sim_results_ref->{clk2q};
      $slew_q = $sim_results_ref->{slew_q};
      $power = $sim_results_ref->{power_vdd};
      $energy_per_toggle = $sim_results_ref->{energy_per_toggle};
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
  print "Power : $power, Energy per toggle: $energy_per_toggle\n";

  # Record in report hash
  $rpt_ptr->{$tag}->{$type}->{hold_time}->{hold_time} = $hold_time;
  $rpt_ptr->{$tag}->{$type}->{hold_time}->{clk2q} = $clk2q;
  $rpt_ptr->{$tag}->{$type}->{hold_time}->{clk2q_std} = $clk2q_std;
  $rpt_ptr->{$tag}->{$type}->{hold_time}->{slew_q} = $slew_q;
  $rpt_ptr->{$tag}->{$type}->{hold_time}->{slew_q_std} = $slew_q_std;
  $rpt_ptr->{$tag}->{$type}->{hold_time}->{power} = $power;
  $rpt_ptr->{$tag}->{$type}->{hold_time}->{energy_per_toggle} = $energy_per_toggle;
  $rpt_ptr->{$tag}->{$type}->{hold_time}->{avg_power} = $total_power/$power_num;
}

sub dff_leakage_power($ $ $ $ $)
{
  my ($tag,$cload,$tran_step,$tran_stop,$vsp) = @_;

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
      &gen_dff_sp_common($spfh,"leakage_power",$cload,3e-9,3e-9,1e-10,0,0,0,$vsp);
      &gen_dff_sp_leakage($spfh,\@input_vectors); 
      &gen_dff_leakage_measure($spfh,$tran_step,$tran_stop);
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

sub run_dff_once($ $ $ $ $ $ $ $)
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

sub run_lut_once($ $ $ $ $ $ $ $)
{
  my ($tag,$vsp,$lut_size,$SRAM_bits,$input_vectors,$input_slew,$cload,$tran_step) = @_;

  my (%sim_results);
  my ($sim_results_ref) = \%sim_results;

  my ($slew_clk,$input_pwl,$input_pwh,$tran_stop) = (20,0,0,0);

  $input_pwl = $slew_clk*$input_slew;
  $input_pwh = 10*$slew_clk*$input_slew;
  $tran_stop = $input_pwl+$input_slew+$input_pwh;

  &run_lut_sim($lut_size,$SRAM_bits,$input_vectors,$vsp,$input_slew,$input_pwl,$input_pwh,$cload,$tran_step,$tran_stop,\%sim_results);

  $rpt_ptr->{$tag}->{delay} = $sim_results_ref->{dly_lut};
  $rpt_ptr->{$tag}->{slew} = $sim_results_ref->{slew_lut};
  $rpt_ptr->{$tag}->{leakage_power} = $sim_results_ref->{pleak};
  $rpt_ptr->{$tag}->{dynamic_power} = $sim_results_ref->{pdynamic};
  $rpt_ptr->{$tag}->{energy_per_toggle} = $sim_results_ref->{energy_per_toggle};

}

sub measure_mux_leakage($ $ $ $ $ $ $ $ $ $ $ $)
{
  my ($tag,$vsp,$mux_size,$SRAM_bits,$input_vectors,$input_slew,$cload,$tran_step,$rram_enhance,$ron,$wprog,$roff) = @_;

  my (%sim_results);
  my ($sim_results_ref) = \%sim_results;

  my ($slew_clk,$input_pwl,$input_pwh,$tran_stop) = (20,0,0,0);

  $input_pwl = $slew_clk*$input_slew;
  $input_pwh = 10*$slew_clk*$input_slew;
  $tran_stop = $input_pwl+$input_slew+$input_pwh;

  &run_mux_sim($mux_size,$SRAM_bits,$input_vectors,$vsp,$input_slew,$input_pwl,$input_pwh,$cload,$tran_step,$tran_stop,\%sim_results,$conf_ptr->{mux_settings}->{inv_size_in}->{val},$conf_ptr->{mux_settings}->{inv_size_out}->{val},$rram_enhance,$ron,$wprog,$roff);

  $rpt_ptr->{$tag}->{leakage_power} = $sim_results_ref->{pleak};

}


sub run_mux_once($ $ $ $ $ $ $ $ $ $ $ $ $ $)
{
  my ($tag,$vsp,$mux_size,$SRAM_bits,$input_vectors,$input_slew,$cload,$tran_step,$rram_enhance,$ron,$wprog,$roff,$on_gap,$off_gap) = @_;

  my (%sim_results);
  my ($sim_results_ref) = \%sim_results;

  my ($slew_clk,$input_pwl,$input_pwh,$tran_stop) = (20,0,0,0);

  $input_pwl = $slew_clk*$input_slew;
  $input_pwh = 10*$slew_clk*$input_slew;
  $tran_stop = $input_pwl+$input_slew+$input_pwh;

  &run_mux_sim($mux_size,$SRAM_bits,$input_vectors,$vsp,$input_slew,$input_pwl,$input_pwh,$cload,$tran_step,$tran_stop,\%sim_results,$conf_ptr->{mux_settings}->{inv_size_in}->{val},$conf_ptr->{mux_settings}->{inv_size_out}->{val},$rram_enhance,$ron,$wprog,$roff,$on_gap,$off_gap);

  $rpt_ptr->{$tag}->{delay} = $sim_results_ref->{dly_mux};
  $rpt_ptr->{$tag}->{slew} = $sim_results_ref->{slew_mux};
  $rpt_ptr->{$tag}->{leakage_power} = $sim_results_ref->{pleak};
  $rpt_ptr->{$tag}->{dynamic_power} = $sim_results_ref->{pdynamic};
  $rpt_ptr->{$tag}->{energy_per_toggle} = $sim_results_ref->{energy_per_toggle};

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

sub print_dff_leakage($)
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

sub print_rpt_dff_leakage_description($) 
{
  my ($splib) = @_;

  print $splib "/*\n";
  print $splib "Number of Leakage conditions: $rpt_ptr->{leakage_power}->{counter}\n";   
  print $splib "Input Vector: <RESET>,<SET>,<CLK>,<D>,<InitialVal>\n"; 
  for (my $i=0; $i < $rpt_ptr->{leakage_power}->{counter}; $i++) {
    print $splib "Input Vector: ".$rpt_ptr->{leakage_power}->{"leakage$i"}->{input_vectors}.", Leakage: ".$rpt_ptr->{leakage_power}->{"leakage$i"}->{leakage_power}."\n"; 
  }  
  print $splib "*/\n";
}

sub enum_measure_mux_leakage($ $ $ $ $ $ $ $ $ $ $)
{
  my ($tag,$vsp,$mux_size,$d_slew,$ron,$wprog,$vprog,$rram_enhance,$avg_tag,$num_sram,$roff) = @_;
  my ($sram_bits,$input_vectors);
  # Measure the leakage enumeration
  print "Info: Measure Leakage Power...";
  $rpt_ptr->{$avg_tag}->{leakage_power} = 0; 
  my ($leakage_tag) = ($tag."_leakage_wprog$wprog\_vprog$vprog");
  my ($case_cnt) = (0);
  for (my $i=0; $i<2**$mux_size; $i++) {
    ($sram_bits,$input_vectors) = ("","");
    for (my $j=0; $j<$num_sram; $j++) {
      $sram_bits .= "1,";
    } 
    $sram_bits =~ s/,$//;
    # Generate input_vectors
    my ($temp) = ($i);
    for (my $j=0; $j<$mux_size; $j++) {
       my ($vector_bit) = $temp%2;
      $input_vectors .= "$vector_bit,";
      $temp = int($temp/2);
    }
    $input_vectors =~ s/,$//;
    &measure_mux_leakage($leakage_tag,$vsp,$mux_size,$sram_bits,$input_vectors,$d_slew,&process_unit($conf_ptr->{mux_settings}->{load_cap}->{val},"capacitance"),1e-13,$rram_enhance,$ron,$wprog,$roff);
    $rpt_ptr->{$avg_tag}->{leakage_power} += $rpt_ptr->{$leakage_tag}->{leakage_power}; 
    $case_cnt++;
    print "Case Tested: $case_cnt.\n";
  }
  $rpt_ptr->{$avg_tag}->{leakage_power} = $rpt_ptr->{$avg_tag}->{leakage_power}/$case_cnt; 
  print "\nLeakage_power: $rpt_ptr->{$avg_tag}->{leakage_power}\n"; 
}


sub print_results()
{
  my ($lib_file) = $opt_ptr->{lib_val};
  my $splib = FileHandle->new;
  if ($splib->open("> $lib_file")) {
    &print_dff_leakage($splib);
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

sub create_rpt_head($) 
{
  my ($rpt_file) = @_;
  my $rptfh = FileHandle->new;
  if ($rptfh->open("> $rpt_file")) {
    &tab_print($rptfh,"* Sub Vt FPGA Sim Report *\n",0);
    &tab_print($rptfh,"* Author: Xifan TANG *\n",0);
    &tab_print($rptfh,"* Date: $mydate *\n",0);
  } else {
    die  "Error: Fail to create $rpt_file!\n";
  }
  return $rptfh;
}

sub close_rpt($)
{
  my ($rptfh) = @_;
  &tab_print($rptfh,"* END *\n",0);
  close($rptfh);
}

sub print_rpt_lut_data($ $)
{
  my ($rptfh,$tag) = @_; 
  
  &tab_print($rptfh,"$rpt_ptr->{$tag}->{delay},",0);
  &tab_print($rptfh,"$rpt_ptr->{$tag}->{leakage_power},",0);
  &tab_print($rptfh,"$rpt_ptr->{$tag}->{dynamic_power},",0);
  &tab_print($rptfh,"$rpt_ptr->{$tag}->{energy_per_toggle},",0);
  &tab_print($rptfh,"\n",0);

}

sub print_rpt_mux_data($ $)
{
  my ($rptfh,$tag) = @_; 
  
  &tab_print($rptfh,"$rpt_ptr->{$tag}->{delay},",0);
  &tab_print($rptfh,"$rpt_ptr->{$tag}->{leakage_power},",0);
  &tab_print($rptfh,"$rpt_ptr->{$tag}->{dynamic_power},",0);
  &tab_print($rptfh,"$rpt_ptr->{$tag}->{energy_per_toggle},",0);
  &tab_print($rptfh,"$rpt_ptr->{$tag}->{elmore_delay},",0);
  &tab_print($rptfh,"\n",0);

}

sub print_rpt_dff_data($ $) 
{
  my ($rptfh,$tag) = @_; 
  
  &tab_print($rptfh,"$rpt_ptr->{$tag}->{rise}->{setup_time}->{setup_time},",0);
  &tab_print($rptfh,"$rpt_ptr->{$tag}->{fall}->{setup_time}->{setup_time},",0);
  &tab_print($rptfh,"$rpt_ptr->{$tag}->{rise}->{setup_time}->{clk2q_std},",0);
  &tab_print($rptfh,"$rpt_ptr->{$tag}->{fall}->{setup_time}->{clk2q_std},",0);
  &tab_print($rptfh,"$rpt_ptr->{$tag}->{rise}->{setup_time}->{avg_power},",0);
  &tab_print($rptfh,"$rpt_ptr->{$tag}->{fall}->{setup_time}->{avg_power},",0);
  &tab_print($rptfh,"$rpt_ptr->{$tag}->{fall}->{setup_time}->{energy_per_toggle},",0);
  &tab_print($rptfh,"$rpt_ptr->{$tag}->{fall}->{setup_time}->{energy_per_toggle},",0);
  &tab_print($rptfh,"\n",0);
}

sub run_dff_elc_sim($ $ $ $ $)
{
   my ($rptfh,$ivsp,$clk_slew,$d_slew,$cload) = @_;  
   my ($initial_pass,$initial_fail,$resolution,$tag,$leakage_tag);

   $leakage_tag = "dff_leakage_power_vdd$ivsp";
   $tag = ("dff_clk_slew$clk_slew\_d_slew$d_slew\_vdd$ivsp");
   $initial_pass = $conf_ptr->{dff_settings}->{bisec}->{initial_pass};
   $initial_fail = $conf_ptr->{dff_settings}->{bisec}->{initial_fail};
   $resolution = $conf_ptr->{dff_settings}->{bisec}->{step};
 
   &dff_leakage_power($leakage_tag,$cload,1e-15,10e-9,$ivsp);
    
   &binary_search_setup_time($tag,$initial_pass,$initial_fail,$clk_slew,$d_slew,"rise",$cload,1e-15,$resolution,$ivsp);
   &binary_search_setup_time($tag,$initial_pass,$initial_fail,$clk_slew,$d_slew,"fall",$cload,1e-15,$resolution,$ivsp);
   #&binary_search_hold_time($tag,$initial_pass,$initial_fail,$clk_slew,$d_slew,"rise",$initial_pass,0,1e-15,$resolution,$ivsp);
   #&binary_search_hold_time($tag,$initial_pass,$initial_fail,$clk_slew,$d_slew,"fall",$initial_pass,0,1e-15,$resolution,$ivsp);

  &tab_print($rptfh,"$ivsp,",0);
  &tab_print($rptfh,"$rpt_ptr->{$leakage_tag}->{avg_leakage},",0);
  &print_rpt_dff_data($rptfh,$tag);

}

sub average_results($ $ $)
{
  my ($avg_tag,$rise_tag,$fall_tag) = @_;

  $rpt_ptr->{$avg_tag}->{delay} = ($rpt_ptr->{$rise_tag}->{delay} + $rpt_ptr->{$fall_tag}->{delay})/2; 
  $rpt_ptr->{$avg_tag}->{slew} = ($rpt_ptr->{$rise_tag}->{slew} + $rpt_ptr->{$fall_tag}->{slew})/2; 
  $rpt_ptr->{$avg_tag}->{leakage_power} = ($rpt_ptr->{$rise_tag}->{leakage_power} + $rpt_ptr->{$fall_tag}->{leakage_power})/2; 
  $rpt_ptr->{$avg_tag}->{dynamic_power} = ($rpt_ptr->{$rise_tag}->{dynamic_power} + $rpt_ptr->{$fall_tag}->{dynamic_power})/2; 
  $rpt_ptr->{$avg_tag}->{energy_per_toggle} = ($rpt_ptr->{$rise_tag}->{energy_per_toggle} + $rpt_ptr->{$fall_tag}->{energy_per_toggle})/2; 

  print "Rise delay : $rpt_ptr->{$rise_tag}->{delay}\n";
  print "Fall delay : $rpt_ptr->{$fall_tag}->{delay}\n";
  print "Average delay : $rpt_ptr->{$avg_tag}->{delay}\n";
  print "Rise leakage : $rpt_ptr->{$rise_tag}->{leakage_power}\n";
  print "Fall leakage : $rpt_ptr->{$fall_tag}->{leakage_power}\n";
  print "Average leakage : $rpt_ptr->{$avg_tag}->{leakage_power}\n";

}

# Determine the elmore delay of a given mux
sub elmore_delay_mux($ $ $ $ $ $ $ $ $ $ $ $ $)
{
  my ($rmin,$cs_nmos,$cs_pmos,$coff_nmos,$cload,$winv_in,$winv_out,$vdd,$vdd_std,$mux_length,$wprog,$ron) = @_;
  my ($elmore_delay) = (0);


  $elmore_delay = $rmin*($cs_nmos+$cs_pmos) + ($rmin/$winv_in)*$winv_out*$cload + (2*$mux_length + 1)*($rmin/$winv_in)*($wprog*$coff_nmos) + $mux_length*$ron*$winv_out*$cload + $mux_length*$mux_length*$ron*$wprog*$coff_nmos;

  print "rmin=$rmin,cs_nmos=$cs_nmos,cs_pmos=$cs_pmos,cload=$cload,winv_in=$winv_in,winv_out=$winv_out,mux_length=$mux_length,Coff=$coff_nmos,ron=$ron,wprog=$wprog,elmore_delay=$elmore_delay\n";  

  return sprintf("%.3g",0.69*$elmore_delay);
}

# Determine the optimal width of programming transistor(NMOS)
sub optimal_wprog($ $ $ $ $ $ $ $ $ $)
{
  my ($vdd,$vdd_std,$cload,$winv_in,$winv_out,$mux_length,$lamda,$coff_nmos,$rmin,$id) = @_;
  my ($optimal_wprog) = (0);
  
  print "Vdd=$vdd,Vdd_std=$vdd_std,cload=$cload,winv_in=$winv_in,winv_out=$winv_out,mux_length=$mux_length,lamda=$lamda,Coff=$coff_nmos\n";  

  #$optimal_wprog = sqrt((3*$cload*$winv*$vdd)/(4*$lamda*$vdd_std*$coff_nmos*(1+2/$mux_length))); 
  $optimal_wprog = sqrt(($cload*$winv_out*$winv_in*$vdd*$lamda*$vdd)/($vdd_std*$id*$rmin*$coff_nmos*(2+1/$mux_length))); 

  return $optimal_wprog;
}

sub determine_rram_parameters($ $ $ $ $ $ $ $ $ $)
{
  my ($ivsp,$vprog,$vprog_slack,$wprog,$std_ids,$ron_upbound,$ron_lowbound,$fix_vprog,$std_vdd,$roff) = @_;
  my ($ron,$ids);

  $vprog = &determine_rram_vprog($vprog,$ivsp,$vprog_slack,$fix_vprog);
  # Ids varies with Vprog
  if ($std_vdd < $vprog) {
    $ids = $std_ids*(($vprog/$std_vdd)**2)# Ids,sat = u*W/L*(Vgs-Vth)^2
  } else {
    $ids = $std_ids;
  } 
  #$ids = $std_ids;
  $ron = sprintf("%.2f",$vprog/($wprog*$ids));
  if ($ron_upbound < $ron_lowbound) {
    die "Error: ron_upbound($ron_upbound) should be greater than ron_lowbound($ron_lowbound)!\n";
  }
  if ($ron > $ron_upbound) {
    $ron = $ron_upbound;
  } elsif ($ron < $ron_lowbound) {
    $ron = $ron_lowbound;
  }

  return ($ron,$vprog,$wprog,$ids,$roff);
}

sub run_lut_elc($ $ $ $ $ $ $ $) {
  my ($rptfh,$clk_slew,$d_slew,$lut_size,$load_cap,$vsp_lowbound,$vsp_upbound,$vsp_sweepstep) = @_;

  &tab_print($rptfh,"* LUT size = $lut_size,Load_cap = $load_cap\n",0);
  &tab_print($rptfh,"vsp,delay,leakage,dynamic_power,energy_per_toggle,\n",0);

  # Run simulations(ELC)
  for (my $ivsp = $vsp_lowbound; 
          $ivsp < $vsp_upbound; 
          $ivsp = $ivsp + $vsp_sweepstep) { 
    my ($tag) = ("lut$lut_size\_vsp$ivsp");
    my ($rise_tag,$fall_tag,$avg_tag);
    my ($sram_bits,$input_vectors) = ("","");

    for (my $i=0; $i<(2**$lut_size); $i++) {
      if (0 == $i%2) {
        $sram_bits .= "1,";
      } else {
        $sram_bits .= "0,";
      }
    } 
    $sram_bits =~ s/,$//;
    for (my $i=0; $i<$lut_size; $i++) {
      $input_vectors .= "r,";
    } 
    $input_vectors =~ s/,$//;

    # Test rise
    $rise_tag = $tag."_rise";
    &run_lut_once($rise_tag,$ivsp,$lut_size,$sram_bits,$input_vectors,$d_slew,&process_unit($load_cap,"capacitance"),1e-15);

    # Test fall
    $fall_tag = $tag."_fall";
    $input_vectors =~ s/r/f/g;
    &run_lut_once($fall_tag,$ivsp,$lut_size,$sram_bits,$input_vectors,$d_slew,&process_unit($load_cap,"capacitance"),1e-15);

    # Average rise and fall
    $avg_tag = $tag."_avg";
    &average_results($avg_tag,$rise_tag,$fall_tag);

    &tab_print($rptfh,"$ivsp,",0);
    &print_rpt_lut_data($rptfh,$avg_tag);
  }
}

sub run_dff_elc($ $ $ $ $ $ $) {
  my ($rptfh,$clk_slew,$d_slew,$load_cap,$vsp_lowbound,$vsp_upbound,$vsp_sweepstep) = @_;

  &tab_print($rptfh,"* DFF ,Load_cap = $load_cap\n",0);
  &tab_print($rptfh,"vsp,leakage,tsu(rise),tsu(fall),delay(rise),delay(fall),dynamic_power(rise),dynamic_power(fall),energy_per_toggle(rise),energy_per_toggle(fall)\n",0);

  for (my $ivsp = $vsp_lowbound; 
          $ivsp < $vsp_upbound; 
          $ivsp = $ivsp + $vsp_sweepstep) { 
    &run_dff_elc_sim($rptfh,$ivsp,$clk_slew,$d_slew,&process_unit($load_cap,"capacitance"));
  }
}

sub run_mux_elc($ $ $ $ $ $ $ $ $ $ $ $) {
  my ($rptfh,$clk_slew,$d_slew,$mux_size,$inv_size_in,$inv_size_out,$load_cap,
      $rram_enhance,$wprog_sweep,$vsp_lowbound,$vsp_upbound,$vsp_sweepstep) = @_;

  &tab_print($rptfh,"* MUX size = $mux_size, inv_size_in = $inv_size_in, inv_size_out = $inv_size_out,Load_cap = $load_cap",0);
  if ("on" eq $opt_ptr->{auto_out_tapered_buffer}) {
    &tab_print($rptfh, ",output_tapered_buffer_level=$opt_ptr->{auto_out_tapered_buffer}\n");
  } else {
    &tab_print($rptfh, "\n");
  }

  if ("off" eq $rram_enhance) {
    &tab_print($rptfh,"vsp,delay,leakage,dynamic_power,energy_per_toggle\n",0);
  } elsif ("off" eq $wprog_sweep) {
    &tab_print($rptfh,"Ron,Wprog,vsp,delay,leakage,dynamic_power,energy_per_toggle\n",0);
  }

  # Run simulations(ELC)
  for (my $ivsp = $vsp_lowbound; 
          $ivsp < $vsp_upbound; 
          $ivsp = $ivsp + $vsp_sweepstep) { 
    my ($tag) = ("mux$mux_size\_vsp$ivsp");
    my ($sram_bits,$input_vectors) = ("","");
    my ($num_sram) = (&determine_mux_num_sram($mux_size));
    my ($rram_enhance_on,$ron,$wprog,$ids,$vprog,$roff) = (0,0,0,0,0,0);
    my ($esti_optimal_wprog,$optimal_wprog,$optimal_delay) = (0,0,-1);
    my ($rise_tag,$fall_tag,$avg_tag,$leakage_tag);

    my ($on_gap,$off_gap) = ($conf_ptr->{rram_settings}->{rram_initial_on_gap}->{val},
                             $conf_ptr->{rram_settings}->{rram_initial_off_gap}->{val});
    my ($kw,$val) = split /:/,$on_gap;
    $on_gap = $val;
    ($kw, $val) = split /:/,$off_gap;
    $off_gap = $val;

    if ("on" eq $rram_enhance) {
      $rram_enhance_on = 1;
      ($ron,$vprog,$wprog,$ids,$roff) = 
        &determine_rram_parameters($ivsp,
                                   &process_unit($conf_ptr->{rram_settings}->{Vprog}->{val},"voltage"),
                                   $conf_ptr->{rram_settings}->{Vprog_slack}->{val},
                                   $conf_ptr->{rram_settings}->{Wprog}->{val},
                                   &process_unit($conf_ptr->{rram_settings}->{Ids}->{val},"current"),
                                   &process_unit($conf_ptr->{rram_settings}->{ron_upbound}->{val},"empty"),
                                   &process_unit($conf_ptr->{rram_settings}->{ron_lowbound}->{val},"empty"),
                                   $conf_ptr->{rram_settings}->{fix_vprog}->{val},
                                   $conf_ptr->{general_settings}->{std_vdd}->{val},
                                   &process_unit($conf_ptr->{rram_settings}->{roff}->{val},"empty"));
    }
   
    if (("on" eq $rram_enhance)&&("on" eq $wprog_sweep)) {
      &tab_print($rptfh,"* vsp = $ivsp, vprog = $vprog\n",0);
      &tab_print($rptfh,"ron,wprog,delay,leakage,dynamic_power,elmore_delay\n",0);
      for (my $iwprog = $conf_ptr->{rram_settings}->{Wprog}->{val};
           $iwprog < ($opt_ptr->{wprog_sweep_val} + $conf_ptr->{rram_settings}->{Wprog_step}->{val});
           $iwprog = $iwprog + $conf_ptr->{rram_settings}->{Wprog_step}->{val}) {
        $wprog = $iwprog;
        ($ron,$vprog,$wprog,$ids,$roff) =
          &determine_rram_parameters($ivsp,
                                     &process_unit($conf_ptr->{rram_settings}->{Vprog}->{val},"voltage"),
                                     $conf_ptr->{rram_settings}->{Vprog_slack}->{val},
                                     $wprog,
                                     &process_unit($conf_ptr->{rram_settings}->{Ids}->{val},"current"),
                                     &process_unit($conf_ptr->{rram_settings}->{ron_upbound}->{val},"empty"),
                                     &process_unit($conf_ptr->{rram_settings}->{ron_lowbound}->{val},"empty"),
                                     $conf_ptr->{rram_settings}->{fix_vprog}->{val},
                                     $conf_ptr->{general_settings}->{std_vdd}->{val},
                                     &process_unit($conf_ptr->{rram_settings}->{roff}->{val},"empty"));

        ($sram_bits,$input_vectors) = ("","");
        if ("on" eq $opt_ptr->{one_level_mux}) {
          for (my $i=0; $i<$num_sram; $i++) {
            if (0 == $i) {
              $sram_bits .= "1,";
            } else {
              $sram_bits .= "0,";
            }
          } 
        } else {
          for (my $i=0; $i<$num_sram; $i++) {
            if ((0 == $i)||($i == $num_sram/2)) {
              $sram_bits .= "1,";
            } else {
              $sram_bits .= "0,";
            }
          }
        } 
        $sram_bits =~ s/,$//;
        for (my $i=0; $i<$mux_size; $i++) {
          # Test the worst case of leakage!
          if (0 == $i%2) {
            $input_vectors .= "r,";
          } else { 
            $input_vectors .= "r,";
          } 
        }  
        $input_vectors =~ s/,$//;

        # Test rising delay
        $rise_tag = $tag."_rise_wprog$wprog\_vprog$vprog";
        &run_mux_once($rise_tag,$ivsp,$mux_size,$sram_bits,$input_vectors,$d_slew,&process_unit($load_cap,"capacitance"),1e-13,$rram_enhance,$ron,$wprog,$roff,$on_gap,$off_gap);

        # Test falling delay
        $fall_tag = $tag."_fall_wprog$wprog\_vprog$vprog";
        $input_vectors = "";
        for (my $i=0; $i<$mux_size; $i++) {
          # Test the worst case of leakage!
          if (0 == $i%2) {
            $input_vectors .= "f,";
          } else { 
            $input_vectors .= "f,";
          } 
        }  
        $input_vectors =~ s/,$//;
        &run_mux_once($fall_tag,$ivsp,$mux_size,$sram_bits,$input_vectors,$d_slew,&process_unit($load_cap,"capacitance"),1e-13,$rram_enhance,$ron,$wprog,$roff,$on_gap,$off_gap);
  
        # Average rise and fall
        $avg_tag = $tag."_avg_wprog$wprog\_vprog$vprog";
        &average_results($avg_tag,$rise_tag,$fall_tag);

        # Update_optimal_wprog regarding to delay
        if (&my_min(\$optimal_delay,$rpt_ptr->{$avg_tag}->{delay}*$rpt_ptr->{$avg_tag}->{leakage_power})) {
        #if (&my_min(\$optimal_delay,$rpt_ptr->{$avg_tag}->{delay})) {
          $optimal_wprog = $wprog;
        }
        
        # Elmore delay of MUX
        $rpt_ptr->{$avg_tag}->{elmore_delay} = 
        &elmore_delay_mux((&process_unit($conf_ptr->{rram_settings}->{Vdd_std}->{val},"voltage")*&process_unit($conf_ptr->{rram_settings}->{Rmin}->{val},"empty"))/$ivsp,
                          &process_unit($conf_ptr->{rram_settings}->{Cs_nmos}->{val},"capacitance"),
                          &process_unit($conf_ptr->{rram_settings}->{Cs_pmos}->{val},"capacitance"),
                          &process_unit($conf_ptr->{rram_settings}->{Coff_nmos}->{val},"capacitance"),
                          &process_unit($conf_ptr->{rram_settings}->{Cg_pmos}->{val},"capacitance") + &process_unit($conf_ptr->{rram_settings}->{Cg_nmos}->{val},"capacitance"),
                          $inv_size_in,
                          $inv_size_out,
                          $ivsp,
                          &process_unit($conf_ptr->{rram_settings}->{Vdd_std}->{val},"voltage"),
                          $num_sram,
                          $wprog,
                          $ron
                           );
        if ("on" eq $opt_ptr->{enum_mux_leakage}) {
          &enum_measure_mux_leakage($tag,$ivsp,$mux_size,$d_slew,$ron,$wprog,$vprog,$rram_enhance_on,$avg_tag,$num_sram,$roff);
        }        

        &tab_print($rptfh,"$ron,$iwprog,",0);
        &print_rpt_mux_data($rptfh,$avg_tag);
      }
         
      # determine the optimal wprog
      $esti_optimal_wprog =  
      &optimal_wprog($ivsp,
                     &process_unit($conf_ptr->{rram_settings}->{Vdd_std}->{val},"voltage"),
                     &process_unit($conf_ptr->{rram_settings}->{Cg_pmos}->{val},"capacitance") + &process_unit($conf_ptr->{rram_settings}->{Cg_nmos}->{val},"capacitance"),
                     $inv_size_in,
                     $inv_size_out,
                     $num_sram,
                     1+$conf_ptr->{rram_settings}->{Vprog_slack}->{val},
                     &process_unit($conf_ptr->{rram_settings}->{Coff_nmos}->{val},"capacitance"),
                     &process_unit($conf_ptr->{rram_settings}->{Rmin}->{val},"empty"),
                     &process_unit($conf_ptr->{rram_settings}->{Ids}->{val},"current")
                    );
      &tab_print($rptfh,"*estimate_optimal_wprog = $esti_optimal_wprog\n",0);
      &tab_print($rptfh,"*optimal_wprog = $optimal_wprog\n",0);
      &tab_print($rptfh,"\n",0); 

    } else {
      ($sram_bits,$input_vectors) = ("","");
      if ("on" eq $opt_ptr->{one_level_mux}) {
        for (my $i=0; $i<$num_sram; $i++) {
          if (0 == $i) {
            $sram_bits .= "1,";
          } else {
            $sram_bits .= "0,";
          }
        } 
      } else {
        for (my $i=0; $i<$num_sram; $i++) {
          if ((0 == $i)||($i == $num_sram/2)) {
            $sram_bits .= "1,";
          } else {
            $sram_bits .= "0,";
          }
        } 
      }
      $sram_bits =~ s/,$//;
      for (my $i=0; $i<$mux_size; $i++) {
        # Test the worst case of leakage!
        if (0 == $i%2) {
          $input_vectors .= "r,";
        } else { 
          $input_vectors .= "r,";
        } 
      }  
      $input_vectors =~ s/,$//;

      # Test rising delay
      $rise_tag = $tag."_rise_wprog$wprog\_vprog$vprog";
      &run_mux_once($rise_tag,$ivsp,$mux_size,$sram_bits,$input_vectors,$d_slew,&process_unit($load_cap,"capacitance"),1e-13,$rram_enhance_on,$ron,$wprog,$roff,$on_gap,$off_gap);

      # Test falling delay
      $fall_tag = $tag."_fall_wprog$wprog\_vprog$vprog";
      $input_vectors = "";
      for (my $i=0; $i<$mux_size; $i++) {
        # Test the worst case of leakage!
        if (0 == $i%2) {
          $input_vectors .= "f,";
        } else { 
          $input_vectors .= "f,";
        } 
      }  
      $input_vectors =~ s/,$//;
      &run_mux_once($fall_tag,$ivsp,$mux_size,$sram_bits,$input_vectors,$d_slew,&process_unit($load_cap,"capacitance"),1e-13,$rram_enhance_on,$ron,$wprog,$roff,$on_gap,$off_gap);
  
      # Average rise and fall
      $avg_tag = $tag."_avg_wprog$wprog\_vprog$vprog";
      &average_results($avg_tag,$rise_tag,$fall_tag);

      if ("on" eq $opt_ptr->{enum_mux_leakage}) {
        &enum_measure_mux_leakage($tag,$ivsp,$mux_size,$d_slew,$ron,$wprog,$vprog,$rram_enhance_on,$avg_tag,$num_sram);
      }        
      if ("on" eq $rram_enhance) {
        &tab_print($rptfh,"$ron,$wprog,",0);
      }
      &tab_print($rptfh,"$ivsp,",0);
      &print_rpt_mux_data($rptfh,$avg_tag);
    }
  }
}

# Generate initial parameters for RRAMs
# Currently, I only attach user-defined parameters
sub gen_rram_init_params($) {
  my ($user_defined_params) = @_;
  my ($ret) = ($user_defined_params);
  # Replace ':' with '=' 
  $ret =~ s/:/=/g;
  # Replace ',' with '\s' 
  $ret =~ s/,/ /g;

  return $ret;
}

sub gen_rram2t1r_sp_common($ $ $ $ $ $ $ $) {
  my ($spfh,$usage,$rram_verilogA_model,$rram_subckt_name,$rram_init_params,$vprog,$wprog,$tprog,$tprog_slew,$prog_vdd) = @_;

  # Print Header
  &tab_print($spfh,"Sub-Vt MUX HSPICE Bench\n",0);
  &tab_print($spfh,"**********************\n",0);
  &tab_print($spfh,"*   HSPICE Netlist   *\n",0);
  &tab_print($spfh,"* Author: Xifan TANG *\n",0);
  &tab_print($spfh,"*         EPFL, LSI  *\n",0);
  &tab_print($spfh,"* Date: $mydate    *\n",0);
  &tab_print($spfh,"**********************\n",0);
  &tab_print($spfh,"* Test Bench Usage: $usage\n",0);

  &gen_common_sp_technology($spfh); 

  &gen_common_sp_parameters($spfh,$prog_vdd);
  &tab_print($spfh,".OPTION RUNLVL=4\n",0);

  # Include RRAM verilogA model
  if ($rram_verilogA_model =~ m/\.va$/) {
    &tab_print($spfh,".hdl \'$rram_verilogA_model\'\n",0);
  } else {
    &tab_print($spfh,".include \'$rram_verilogA_model\'\n",0);
  }
  &tab_print($spfh,".param vprog=$vprog\n",0);
  &tab_print($spfh,".param wprog=$wprog\n",0);
  &tab_print($spfh,".param tprog=$tprog\n",0);
  &tab_print($spfh,".param tprog_slew=$tprog_slew\n",0);
  &tab_print($spfh,".param t_init=$tprog\n",0);
  &tab_print($spfh,".param prog_trans_vdd=$prog_vdd\n",0);
  &tab_print($spfh,".param vthn=$conf_ptr->{general_settings}->{vthn}->{val}\n",0);
  &tab_print($spfh,".param vthp=$conf_ptr->{general_settings}->{vthp}->{val}\n",0);
  
  # Build Prog. Trans. Pair
  &tab_print($spfh,".subckt rram_prog_pair prog_node prog_roffb prog_ron svdd sgnd wprog=1\n",0);  
  &tab_print($spfh,"Xp1 prog_node prog_roffb svdd svdd elc_pmos L=\'pl\' W=\'wprog*beta*wp\'\n",0);
  &tab_print($spfh,"Xn1 prog_node prog_ron sgnd sgnd elc_nmos L=\'nl\' W=\'wprog*wn\'\n",0);
  &tab_print($spfh,".eom rram_prog_pair\n",0);
  &tab_print($spfh,"\n",0);
  # Define subckt for inverters 
  &tab_print($spfh,".subckt inv in out svdd sgnd size=1\n",0);  
  &tab_print($spfh,"Xp1 out in svdd svdd elc_pmos L=\'pl\' W=\'size*wn*beta\'\n",0);
  &tab_print($spfh,"Xn1 out in sgnd sgnd elc_nmos L=\'nl\' W=\'size*wn\'\n",0);
  &tab_print($spfh,".eom inv\n",0);
  &tab_print($spfh,"\n",0);

  if (("on" eq $opt_ptr->{rram2n1r})&&("off" eq $opt_ptr->{transmission_gate})) {
    # Hack for 2 NMOS, just for comparison
    &tab_print($spfh,"***** 2T1R Structure *****\n",0);
    &tab_print($spfh,"Xn1 rram_te prog_ron prog_te prog_te elc_nmos L=\'nl\' W=\'wprog*wn\'\n",0);
    &tab_print($spfh,"Xn2 rram_be prog_ron prog_be prog_be elc_nmos L=\'nl\' W=\'wprog*wn\'\n",0);
    &tab_print($spfh,"Xinv1 0 prog_te vdd0 0 inv size=\'$opt_ptr->{driver_inv_size_val}\'\n",0);
    &tab_print($spfh,"Xinv2 vdd0 prog_be vdd0 0 inv size=\'$opt_ptr->{driver_inv_size_val}\'\n",0);
    # END Hack
  } elsif (("on" eq $opt_ptr->{rram2n1r})&&("on" eq $opt_ptr->{transmission_gate})) {
    &tab_print($spfh,"***** NMOS for transmission gate *****\n",0);
    &tab_print($spfh,"Xn1 rram_te prog_ron prog_te 0 elc_nmos L=\'nl\' W=\'wprog*wn\'\n",0);
    &tab_print($spfh,"Xn2 rram_be prog_ron prog_be 0 elc_nmos L=\'nl\' W=\'wprog*wn\'\n",0);
    &tab_print($spfh,"***** PMOS for transmission gate *****\n",0);
    &tab_print($spfh,"Xp1 rram_te prog_ronb prog_te vdd0 elc_pmos L=\'pl\' W=\'wprog*wn*beta\'\n",0);
    &tab_print($spfh,"Xp2 rram_be prog_ronb prog_be vdd0 elc_pmos L=\'pl\' W=\'wprog*wn*beta\'\n",0);
    &tab_print($spfh,"Xinv1 0 prog_te vdd0 0 inv size=\'$opt_ptr->{driver_inv_size_val}\'\n",0);
    &tab_print($spfh,"Xinv2 vdd0 prog_be vdd0 0 inv size=\'$opt_ptr->{driver_inv_size_val}\'\n",0);
    # END Hack
  } else {
    # Build 4T1R structure
    &tab_print($spfh,"***** 4T1R Structure *****\n",0);
    &tab_print($spfh,"Xprog_pair0 rram_te prog_ronb prog_roff vdd0 0 rram_prog_pair wprog=\'wprog\'\n",0);
    &tab_print($spfh,"Xprog_pair1 rram_be prog_roffb prog_ron vdd1 0 rram_prog_pair wprog=\'wprog\'\n",0);
  }
  if ($rram_verilogA_model =~ m/\.va$/) {
    &tab_print($spfh,"Xrram rram_te rram_be $rram_subckt_name $rram_init_params\n",0);
  } else {
    &tab_print($spfh,"Xrram rram_te rram_be $rram_subckt_name\n",0);
  }
  &tab_print($spfh,".nodeset V(rram_te)=vprog\n",0);
  &tab_print($spfh,".nodeset V(rram_be)=0\n",0);
  &tab_print($spfh,"\n",0);
  
}

sub gen_rram2t1r_sp_stimulates($ $) {
  my ($spfh,$tran_step) = @_;

  &tab_print($spfh,"** Stimulates\n",0);
  &tab_print($spfh,"Vsupply0 vdd0 0 vprog\n",0);
  &tab_print($spfh,"Vsupply1 vdd1 0 vprog\n",0);
  &tab_print($spfh,"Vprog_ron prog_ron 0 pulse(0 \'vprog\' \'t_init\' \'tprog_slew\' \'tprog_slew\' \'tprog\' \'2*tprog+2tprog_slew\')\n",0);
  &tab_print($spfh,"Vprog_ronb prog_ronb 0 pulse(\'vprog\' 0 \'t_init\' \'tprog_slew\' \'tprog_slew\' \'tprog\' \'2*tprog+2tprog_slew\')\n",0);
  # To match same Vgs as NMOS, but we can do this by using body effect
  #&tab_print($spfh,"Vprog_ronb prog_ronb 0 pulse(\'vprog\' \'vprog-prog_trans_vdd+(vthn-vthp)\' \'t_init\' \'tprog_slew\' \'tprog_slew\' \'tprog\' \'2*tprog+2tprog_slew\')\n",0);
  &tab_print($spfh,"Vprog_roff prog_roff 0 0\n",0);
  &tab_print($spfh,"Vprog_roffb prog_roffb 0 vprog\n",0);
  # Hack for 2 NMOS, just for comparison
  #&tab_print($spfh,"Vprog_ron prog_ron 0 pulse(0 \'prog_trans_vdd\' \'t_init\' \'tprog_slew\' \'tprog_slew\' \'tprog\' \'2*tprog+2tprog_slew\')\n",0);
  #&tab_print($spfh,"Vprog_ronb prog_ronb 0 pulse(\'vprog\' \'vprog-prog_trans_vdd\' \'t_init\' \'tprog_slew\' \'tprog_slew\' \'tprog\' \'2*tprog+2tprog_slew\')\n",0);
  #&tab_print($spfh,"Vprog_roff prog_roff 0 0\n",0);
  #&tab_print($spfh,"Vprog_roffb prog_roffb 0 vprog\n",0);
  # END Hack
  &tab_print($spfh,"\n",0);
  &tab_print($spfh,".tran '$tran_step' 't_init+2*tprog_slew+2*tprog'\n",0);
}

sub gen_rram2t1r_sp_measures($) {
  my ($spfh) = @_;

  &tab_print($spfh,"** Measurements\n",0);
  # Measure the Iprog
  &tab_print($spfh,".measure tran iprog find I(vsupply0) at=\'t_init+tprog_slew+tprog\'\n",0);
  
  # Measure the VTE
  &tab_print($spfh,".measure tran vte find v(rram_te) at=\'t_init+tprog_slew+tprog\'\n",0);

  # Measure the VBE
  &tab_print($spfh,".measure tran vbe find v(rram_be) at=\'t_init+tprog_slew+tprog\'\n",0);

  # Measure Ron
  &tab_print($spfh,".measure tran ron param=\'abs((vte-vbe)/iprog)\'\n",0);

  # Measure Iprog/Wprog
  &tab_print($spfh,".measure tran iprog_per_wprog param=\'iprog/wprog\'\n",0);
  # Check the device breakdown! Measure the peak voltage at Vbe and Vte
  &tab_print($spfh,"** Device breakdown voltage check \n",0);
  &tab_print($spfh,"** Max. voltage of vte \n",0); # check device P1 and N1
  &tab_print($spfh,".measure tran vte_max max V(rram_te) from=\'t_init+tprog_slew\'\n",0);
  &tab_print($spfh,"** Min. voltage of vbe \n",0); # check device P2 and N2
  &tab_print($spfh,".measure tran vbe_min min V(rram_be) from=\'t_init+tprog_slew\'\n",0);

  if ("on" eq $opt_ptr->{rram2n1r}) {
    &tab_print($spfh,".measure tran vprog_te find v(prog_te) at=\'t_init+tprog_slew+tprog\'\n",0);
    &tab_print($spfh,".measure tran vprog_be find v(prog_be) at=\'t_init+tprog_slew+tprog\'\n",0);
  }

  &tab_print($spfh,".end\n",0);
  &tab_print($spfh,"\n",0);
}

sub run_rram2t1r_sim($ $ $ $ $ $ $ $ $) {
  my ($results,$rram_verilogA_model,$rram_subckt_name,$rram_init_params,
      $vprog,$wprog,$t_prog,$t_prog_slew,$prog_vdd,$tran_step) = @_;

  # create folder
  my ($sp_folder) = ("$cwd/"."$conf_ptr->{general_settings}->{spice_dir}->{val}"."rram2t1r/");
  &generate_path($sp_folder);
  my ($sp_file) = ($sp_folder."rram2t1r.sp");
  my ($lis_file) = ($sp_file);
  $lis_file =~ s/sp$/lis/;
  my @sim_keywds = ("vte","vbe","iprog","ron","iprog_per_wprog","vte_max","vbe_min");

  if ("on" eq $opt_ptr->{rram2n1r}) {
    push(@sim_keywds,("vprog_te","vprog_be"));
  }
      
  my $spfh = FileHandle->new;
  if ($spfh->open("> $sp_file")) {
    # Build SPICE Netlist
    &gen_rram2t1r_sp_common($spfh,"RRAM2T1R_Vprog$vprog\_Wprog$wprog",
                            $rram_verilogA_model,$rram_subckt_name,$rram_init_params,
                            $vprog,$wprog,$t_prog,$t_prog_slew,$prog_vdd);
    &gen_rram2t1r_sp_stimulates($spfh,$tran_step);
    &gen_rram2t1r_sp_measures($spfh);
    close($spfh);
    # Run Simulation
    &run_hspice("$cwd/"."$conf_ptr->{general_settings}->{spice_dir}->{val}","rram2t1r.sp",$conf_ptr->{general_settings}->{verilogA_hspice_sim}->{val});
    # Parse Results
    &get_sim_results($lis_file,\@sim_keywds,$results);
    # Process results
  } else {
    die "Error: Fail to create $sp_file!\n";
  }
  
}

sub run_rram2t1r_once($ $ $ $ $ $ $ $ $ $) {
  my ($tag,$rram_verilogA_model,$rram_subckt_name,$rram_init_params,
      $vprog,$wprog,$t_prog,$t_prog_slew,$prog_vdd,$vdd_break,$tran_step) = @_; 

  my (%sim_results);
  my ($sim_results_ref) = \%sim_results;

  &run_rram2t1r_sim($sim_results_ref,$rram_verilogA_model,$rram_subckt_name,$rram_init_params,
                    $vprog,$wprog,$t_prog,$t_prog_slew,$prog_vdd,$tran_step);
  
  # Record simulation data
  $rpt_ptr->{$tag}->{Vte} = sprintf("%.4g",$sim_results_ref->{vte});
  $rpt_ptr->{$tag}->{Vbe} = sprintf("%.4g",$sim_results_ref->{vbe});
  $rpt_ptr->{$tag}->{Vrram} = sprintf("%.4g",$sim_results_ref->{vte} - $sim_results_ref->{vbe});
  $rpt_ptr->{$tag}->{Iprog} = sprintf("%.4g",$sim_results_ref->{iprog});
  $rpt_ptr->{$tag}->{Iprog_per_Wprog} = sprintf("%.4g",$sim_results_ref->{iprog_per_wprog});
  $rpt_ptr->{$tag}->{Ron} = sprintf("%.4g",$sim_results_ref->{ron});
  # Write Vds1, Vds2
  $rpt_ptr->{$tag}->{Vds1} = sprintf("%.4g",$vprog - $rpt_ptr->{$tag}->{Vte});
  $rpt_ptr->{$tag}->{Vds2} = sprintf("%.4g",$rpt_ptr->{$tag}->{Vbe});
  # Identify any device breaks down
  $rpt_ptr->{$tag}->{Vte_max} = sprintf("%.4g",$sim_results_ref->{vte_max});
  $rpt_ptr->{$tag}->{Vbe_min} = sprintf("%.4g",$sim_results_ref->{vbe_min});
  $rpt_ptr->{$tag}->{Breakdown} = "Fail"; 
  
  # For 2T1R structure, need to measure Vds3 & Vds4
  if ("on" eq $opt_ptr->{rram2n1r}) {
    $rpt_ptr->{$tag}->{Vds3} = sprintf("%.4g",$vprog - $sim_results_ref->{vprog_te});
    $rpt_ptr->{$tag}->{Vds4} = sprintf("%.4g",$sim_results_ref->{vprog_be});
  }

  if ((!($rpt_ptr->{$tag}->{Vte} > $vdd_break))
     &&(!(($vprog - $rpt_ptr->{$tag}->{Vbe}) > $vdd_break))
     &&(!($rpt_ptr->{$tag}->{Vds1} > $vdd_break))
     &&(!($rpt_ptr->{$tag}->{Vds2} > $vdd_break))
     &&(!($rpt_ptr->{$tag}->{Vte_max} > $vdd_break))
     &&(!(($vprog - $rpt_ptr->{$tag}->{Vbe_min}) > $vdd_break))) {
    $rpt_ptr->{$tag}->{Breakdown} = "Pass"; 
  }

}

sub print_rram2t1r_rpt($ $) {
  my ($rptfh,$tag) = @_;

  &tab_print($rptfh,"$rpt_ptr->{$tag}->{Vds1},",0);
  &tab_print($rptfh,"$rpt_ptr->{$tag}->{Vds2},",0);
  &tab_print($rptfh,"$rpt_ptr->{$tag}->{Vte},",0);
  &tab_print($rptfh,"$rpt_ptr->{$tag}->{Vbe},",0);
  &tab_print($rptfh,"$rpt_ptr->{$tag}->{Vrram},",0);
  &tab_print($rptfh,$rpt_ptr->{$tag}->{Iprog}/1e-6.",",0);
  &tab_print($rptfh,$rpt_ptr->{$tag}->{Iprog_per_Wprog}/1e-6.",",0);
  &tab_print($rptfh,$rpt_ptr->{$tag}->{Ron}/1e3.",",0);
  &tab_print($rptfh,$rpt_ptr->{$tag}->{Breakdown}.",",0); # Print if any device breaks down

  if ("on" eq $opt_ptr->{rram2n1r}) {
    &tab_print($rptfh,"$rpt_ptr->{$tag}->{Vds3},",0);
    &tab_print($rptfh,"$rpt_ptr->{$tag}->{Vds4},",0);
  }

  &tab_print($rptfh,"\n",0);
}

sub run_rram2t1r_elc($ $ $ $ $ $ $ $ $ $ $ $ $ $ $) {
  my ($rptfh,$rram_verilogA_model,$rram_subckt_name,$rram_vth,$rram_init_params,
      $sweep_wprog,$wprog_upbound,$sweep_wprog_step,$sweep_vprog,$vprog_upbound,$sweep_vprog_step,
      $t_prog_slew,$t_prog,$prog_vdd,$vdd_break) = @_;
  my ($tag);
  
  # Update Upbound and LowBound
  my ($wprog_lowbound,$vprog_lowbound) = (1,$rram_vth);
  #$vprog_lowbound = $rram_vth 
  #                + &process_unit($conf_ptr->{general_settings}->{vthp}->{val}, "voltage")
  #                + &process_unit($conf_ptr->{general_settings}->{vthn}->{val}, "voltage");
  $vprog_lowbound = $conf_ptr->{rram_settings}->{Vdd_prog}->{val};
  if ("on" ne $sweep_wprog) {
    $wprog_lowbound = $conf_ptr->{rram_settings}->{Wprog}->{val};
    $wprog_upbound = $wprog_lowbound + $sweep_wprog_step;
  } else {
    $wprog_upbound = $wprog_upbound + $sweep_wprog_step;
  }

  if ("on" ne $sweep_vprog) {
    $vprog_lowbound = $conf_ptr->{rram_settings}->{Vprog}->{val};
    $vprog_upbound = $vprog_lowbound + $sweep_vprog_step;
  } else {
    $vprog_upbound = $vprog_upbound + $sweep_vprog_step;
  }


  # Print a head for CSV report
  &tab_print($rptfh,"* RRAM2T1R: RRAM_model = $rram_subckt_name, Tprog=$t_prog, Prog.Trans.Vdd=$prog_vdd\n",0);
  if ("on" eq $opt_ptr->{rram2n1r}) {
    &tab_print($rptfh, "* RRAM2T1R: driver_inverter_size= $opt_ptr->{driver_inv_size_val}\n",0);
  }
  # Sweep the Vprog
  for (my $ivprog = $vprog_lowbound; $ivprog < $vprog_upbound; $ivprog = $ivprog + $sweep_vprog_step) {
    &tab_print($rptfh,"* Vprog =$ivprog\n",0);
    &tab_print($rptfh,"* Wprog(*min_size),Vds1(V),Vds2(V),Vte(V),Vbe(V),Vrram(V),Iprog(uA),Iprog/Wprog(uA),Ron(kOhm),Device Breakdown Check,",0);
    # Additional measurements in 2T1R
    if ("on" eq $opt_ptr->{rram2n1r}) {
      &tab_print($rptfh, "Vds3(V),Vds4(V),",0);
    }
    &tab_print($rptfh, "\n",0);
    # Sweep Wprog
    for (my $iwprog = $wprog_lowbound; $iwprog < $wprog_upbound; $iwprog = $iwprog + $sweep_wprog_step) {
      $tag = "rram2t1r_vprog$ivprog\_wprog$iwprog";
      &run_rram2t1r_once($tag,$rram_verilogA_model,$rram_subckt_name,$rram_init_params,
                         $ivprog,$iwprog,$t_prog,$t_prog_slew,$prog_vdd,$vdd_break,1e-13); 
      # Write CSV report
      &tab_print($rptfh,"$iwprog,",0);
      &print_rram2t1r_rpt($rptfh,$tag);
    } 
  } 

}
    

sub sweep_components_vsp()  {
  my ($rptfh) = (&create_rpt_head($opt_ptr->{rpt_val}));
  my ($clk_slew,$d_slew) = (0,0);

  $clk_slew = &process_unit($conf_ptr->{general_settings}->{clk_slew}->{val},"time");
  $d_slew = &process_unit($conf_ptr->{general_settings}->{input_slew}->{val},"time");

  if ("on" eq $opt_ptr->{lut}) { 
    &run_lut_elc($rptfh,$clk_slew,$d_slew,
                 $opt_ptr->{lut_size_val},$conf_ptr->{lut_settings}->{load_cap}->{val},
                 $conf_ptr->{general_settings}->{vsp_sweep}->{low_bound},
                 $conf_ptr->{general_settings}->{vsp_sweep}->{step}+$conf_ptr->{general_settings}->{vsp_sweep}->{high_bound}, 
                 $conf_ptr->{general_settings}->{vsp_sweep}->{step});
  }
  
  if ("on" eq $opt_ptr->{dff}) {
    &run_dff_elc($rptfh,$clk_slew,$d_slew,
                 $conf_ptr->{lut_settings}->{load_cap}->{val},
                 $conf_ptr->{general_settings}->{vsp_sweep}->{low_bound},
                 $conf_ptr->{general_settings}->{vsp_sweep}->{step}+$conf_ptr->{general_settings}->{vsp_sweep}->{high_bound}, 
                 $conf_ptr->{general_settings}->{vsp_sweep}->{step});
  }
  
  if ("on" eq $opt_ptr->{mux}) {
    &run_mux_elc($rptfh,$clk_slew,$d_slew,$opt_ptr->{mux_size_val},$conf_ptr->{mux_settings}->{inv_size_in}->{val},
                 $conf_ptr->{mux_settings}->{inv_size_out}->{val},
                 $conf_ptr->{mux_settings}->{load_cap}->{val},
                 $opt_ptr->{rram_enhance},$opt_ptr->{wprog_sweep},
                 $conf_ptr->{general_settings}->{vsp_sweep}->{low_bound},
                 $conf_ptr->{general_settings}->{vsp_sweep}->{step}+$conf_ptr->{general_settings}->{vsp_sweep}->{high_bound}, 
                 $conf_ptr->{general_settings}->{vsp_sweep}->{step});
  }

  if ("on" eq $opt_ptr->{rram2t1r}) {
    my ($rram_init_params) = &gen_rram_init_params($conf_ptr->{rram_settings}->{rram_initial_params}->{val});
    &run_rram2t1r_elc($rptfh,$opt_ptr->{rram2t1r_val},$conf_ptr->{rram_settings}->{rram_subckt_name}->{val},
                      $conf_ptr->{rram_settings}->{rram_vth}->{val},$rram_init_params,
                      $opt_ptr->{wprog_sweep},$opt_ptr->{wprog_sweep_val},$conf_ptr->{rram_settings}->{Wprog_step}->{val},
                      $opt_ptr->{vprog_sweep},$opt_ptr->{vprog_sweep_val},$conf_ptr->{rram_settings}->{Vprog_step}->{val},
                      $d_slew,$conf_ptr->{rram_settings}->{Tprog}->{val},$conf_ptr->{rram_settings}->{Vdd_prog}->{val},
                      $conf_ptr->{rram_settings}->{Vdd_break}->{val});
  }

  #if ("on" eq $opt_ptr->{dff}) {
    #&print_rpt_dff_leakage_description($rptfh);
  #}
  
  &close_rpt($rptfh);
}

sub gen_basic_sp() {
  my ($sp_folder) = ("$cwd"."/$conf_ptr->{general_settings}->{spice_dir}->{val}");
  my ($sp_file) = ($sp_folder.$elc_nmos_pmos_sp);

  &generate_path($sp_folder);
  
  #Generate basic spice
  my $spfh = FileHandle->new;
  if ($spfh->open("> $sp_file")) {
    &tab_print($spfh,"* NMOS and PMOS Package for ELC \n",0);
    &tab_print($spfh,"* Process Tech : $conf_ptr->{general_settings}->{process_tech}->{val}\n",0);
    &tab_print($spfh,"* Process Type : $conf_ptr->{general_settings}->{process_type}->{val}\n",0);
    &tab_print($spfh,"* NMOS \n",0);
    &tab_print($spfh,".subckt $elc_nmos_subckt_name drain gate source bulk L=nl W=wn\n",0);
    &tab_print($spfh,"$conf_ptr->{general_settings}->{trans_model_ref}->{val}N1 drain gate source bulk $conf_ptr->{general_settings}->{nmos_name}->{val} L=L W=W\n",0);
    &tab_print($spfh,".eom $elc_nmos_subckt_name\n",0);
    &tab_print($spfh,"\n",0);
   
    &tab_print($spfh,"* PMOS\n",0);
    &tab_print($spfh,".subckt $elc_pmos_subckt_name drain gate source bulk L=pl W=wp\n",0);
    &tab_print($spfh,"$conf_ptr->{general_settings}->{trans_model_ref}->{val}P1 drain gate source bulk $conf_ptr->{general_settings}->{pmos_name}->{val} L=L W=W\n",0);
    &tab_print($spfh,".eom $elc_pmos_subckt_name\n",0);
    &tab_print($spfh,"\n",0);

    if ("on" eq $opt_ptr->{rram_enhance}) {
      &tab_print($spfh,"* NMOS and PMOS Package for ELC RRAM MUX \n",0);
      &tab_print($spfh,"* Prog Process Tech : $conf_ptr->{rram_settings}->{prog_process_tech}->{val}\n",0);
      &tab_print($spfh,"* Prog Process Type : $conf_ptr->{rram_settings}->{prog_process_type}->{val}\n",0);
      &tab_print($spfh,"* NMOS \n",0);
      &tab_print($spfh,".subckt $elc_prog_nmos_subckt_name drain gate source bulk L=prog_nl W=prog_wn\n",0);
    &tab_print($spfh,"$conf_ptr->{rram_settings}->{prog_trans_model_ref}->{val}N1 drain gate source bulk $conf_ptr->{rram_settings}->{prog_nmos_name}->{val} L=L W=W\n",0);
      &tab_print($spfh,".eom $elc_prog_nmos_subckt_name\n",0);
      &tab_print($spfh,"\n",0);
     
      &tab_print($spfh,"* PMOS\n",0);
      &tab_print($spfh,".subckt $elc_prog_pmos_subckt_name drain gate source bulk L=prog_pl W=prog_wp\n",0);
    &tab_print($spfh,"$conf_ptr->{rram_settings}->{prog_trans_model_ref}->{val}P1 drain gate source bulk $conf_ptr->{rram_settings}->{prog_pmos_name}->{val} L=L W=W\n",0);
      &tab_print($spfh,".eom $elc_prog_pmos_subckt_name\n",0);
      &tab_print($spfh,"\n",0);
    }

    close($spfh);
    print "INFO: Auto-generate ELC NMOS and PMOS package($sp_file).\n";
  } else {
    die "Error: Fail to create $sp_file!\n";
  }  
  
}

sub main() 
{
  # Read Options
  &opts_read();
 
  # Read basic configuration file. All confs stored in conf_ptr
  &read_conf();

  # Check library settings in configuration files
  &check_lib_settings();

  # Generate basic spice file
  &gen_basic_sp();

  &sweep_components_vsp();
}

&main();
exit(0);
