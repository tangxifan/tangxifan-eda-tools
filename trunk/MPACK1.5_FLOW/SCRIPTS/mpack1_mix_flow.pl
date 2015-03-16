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
use Cwd;

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
my @benchmarks;

# Configuration file keywords list
# Category for conf file.
# main category : 1st class
my @mctgy;
# sub category : 2nd class
my @sctgy;
# Initialize these categories
@mctgy = ("dir_path",
          "flow_conf",
          "csv_tags",
         );
# refer to the keywords of dir_path
@{$sctgy[0]} = ("benchmark_dir",
                "abc_path",
                "mpack_path",
                "vpr_path",
                "rpt_dir",
                "m2net_path",
                "ace_path",
               );
# refer to the keywords of flow_type
@{$sctgy[1]} = ("flow_type",
                "std_vpr_arch",
                "mpack_stdlib",
                "m2net_conf",
                "power_tech_xml",
                );
# refer to the keywords of csv_tags
@{$sctgy[2]} = ("mpack_tags",
                "vpr_tags",
                "vpr_power_tags"
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
  print "      mpack1_mix_flow [-options <value>]\n";
  print "      Mandatory options: \n";
  print "      -conf : specify the basic configuration files for fpga_flow\n";
  print "      -benchmark : the configuration file contains benchmark file names\n"; 
  print "      -rpt : CSV file consists of data\n";
  print "      -N : N-LUT/Matrix\n";
  print "      -I : Number of inputs of a CLB\n";
  print "      Other Options:\n";
  print "      -K : K-LUT, mandatory when standard flow is chosen\n";
  print "      -M : M-Matrix, mandatory when mpack flow is chosen\n";
  print "      -power : run power estimation oriented flow\n";
  print "      -remove_designs : clear the results folder before start a new job\n";
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
  &read_opt_into_hash("N","on","on");
  &read_opt_into_hash("I","on","on");
  &read_opt_into_hash("K","on","off");
  &read_opt_into_hash("M","on","off");
  &read_opt_into_hash("power","off","off");
  &read_opt_into_hash("remove_designs","off","off");

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
      $benchmarks[$cur] = $post_line;       
      $cur++;
    } 
  }  
  print "Benchmarks(total $cur):\n";
  foreach my $temp(@benchmarks)
  {print "$temp\n";}
  close(FCONF);
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

# Run ABC with standard library mapping
sub run_abc_libmap($ $ $)
{
  my ($bm,$blif_out,$log) = @_;
  # Get ABC path
  my ($abc_dir,$abc_name) = &split_prog_path($conf_ptr->{dir_path}->{abc_path}->{val});
  
  chdir $abc_dir;
  my ($mpack_stdlib) = ($conf_ptr->{flow_conf}->{mpack_stdlib}->{val});
  # Run MPACK ABC
  `csh -cx './$abc_name -c \"read_blif $bm; resyn2; read_library $mpack_stdlib; map -v; write_blif $blif_out; quit\" > $log'`;
  chdir $cwd;
}

# Run ABC by FPGA-oriented synthesis
sub run_abc_fpgamap($ $ $)
{
  my ($bm,$blif_out,$log) = @_;
  # Get ABC path
  my ($abc_dir,$abc_name) = &split_prog_path($conf_ptr->{dir_path}->{abc_path}->{val});
  chdir $abc_dir;
  my ($lut_num) = $opt_ptr->{K_val};
  # Run FPGA ABC
  `csh -cx './$abc_name -c \"read_blif $bm; resyn2; fpga -K $lut_num; write_blif $blif_out; quit\" > $log'`;
  chdir $cwd;
}

sub run_mpack1p5($ $ $ $ $)
{
  my ($blif_in,$prefix,$matrix_size,$cell_size,$log) = @_;
  # Get MPACK path
  my ($mpack_dir,$mpack_name) = &split_prog_path($conf_ptr->{dir_path}->{mpack_path}->{val});
  $prefix =~ s/_$//;

  chdir $mpack_dir;
  # Run MPACK
  `csh -cx './$mpack_name $blif_in $prefix -matrix_depth $matrix_size -matrix_width $matrix_size > $log'`;
  chdir $cwd;
  
}

# Extract Mpack stats
sub extract_mpack_stats($ $)
{
  my ($bm,$mstats) = @_;
  my ($line);
  my @keywords = split /\|/,$conf_ptr->{csv_tags}->{mpack_tags}->{val};
  open (MSTATS, "< $mstats") or die "Fail to open $mstats!\n";
  while(defined($line = <MSTATS>)) {
    chomp $line; 
    $line =~ s/\s//g;
    foreach my $tmp(@keywords) {
      $tmp =~ s/\s//g;
      if ($line =~ m/$tmp\s*([0-9E\-\+.\/]+)/i) {
        $rpt_ptr->{$bm}->{$opt_ptr->{N_val}}->{$opt_ptr->{M_val}}->{$tmp} = $1;
      }
    }
  }
  close(MSTATS);
}

# Extract VPR Power Esti
sub extract_vpr_power_esti($ $ $)
{
  my ($mpack_vpr_blif,$bm,$type) = @_;
  my ($line,$tmp,$line_num);
  my @keywords = split /\|/,$conf_ptr->{csv_tags}->{vpr_power_tags}->{val};
  my ($vpr_power_stats) = $mpack_vpr_blif;
  
  $line_num = 0;
  $vpr_power_stats =~ s/blif$/power/;
  open (VSTATS, "< $vpr_power_stats") or die "Fail to open $vpr_power_stats!\n";
  while(defined($line = <VSTATS>)) {
    chomp $line; 
    $line_num++;
    # Special for Total, we want leakage
    if ($line =~ m/^Total/i) {
      my @power_info = split /\s+/,$line;
      if ($#power_info < 3) {
        print "Error: (vpr_power_stats:$vpr_power_stats)ilegal definition at LINE[$line_num]!\n";
        die "Format should be [tag] [Power] [Proposition] [Dynamic Proposition] [Method](Optional)\n";
      }
      if ($power_info[3] > 1) {
        die "Error: (vpr_power_stats:$vpr_power_stats)Dynamic Power Proposition should not be greater than 1 at LINE[$line_num]!\n";
      }
      $rpt_ptr->{$bm}->{$opt_ptr->{N_val}}->{$type}->{power}->{total} = $power_info[1];
      $rpt_ptr->{$bm}->{$opt_ptr->{N_val}}->{$type}->{power}->{dynamic} = $power_info[3]*$power_info[1];
      $rpt_ptr->{$bm}->{$opt_ptr->{N_val}}->{$type}->{power}->{leakage} = (1-$power_info[3])*$power_info[1];
      next;
    }
    $line =~ s/\s//g;
    foreach my $tmpkw(@keywords) {
      $tmp = $tmpkw;
      $tmp =~ s/\s//g;
      $tmp =~ s/\(/\\\(/g;
      $tmp =~ s/\)/\\\)/g;
      #print "$tmp\n";
      if ($line =~ m/$tmp\s*([0-9E\-+.]+)/i) {
        $rpt_ptr->{$bm}->{$opt_ptr->{N_val}}->{$type}->{power}->{$tmpkw} = $1;
        my @tempdata = split /\./,$rpt_ptr->{$bm}->{$opt_ptr->{N_val}}->{$type}->{power}->{$tmpkw};
        $rpt_ptr->{$bm}->{$opt_ptr->{N_val}}->{$type}->{power}->{$tmpkw} = join('.',$tempdata[0],$tempdata[1]);
        $rpt_ptr->{$bm}->{$opt_ptr->{N_val}}->{$type}->{power}->{$tmpkw} =~ s/0$//;
      }
    }
  }
  close(VSTATS);
}

# Extract VPR stats
sub extract_vpr_stats($ $ $)
{
  my ($bm,$vstats,$type) = @_;
  my ($line,$tmp);
  my @keywords = split /\|/,$conf_ptr->{csv_tags}->{vpr_tags}->{val};
  open (VSTATS, "< $vstats") or die "Fail to open $vstats!\n";
  while(defined($line = <VSTATS>)) {
    chomp $line; 
    $line =~ s/\s//g;
    foreach my $tmpkw(@keywords) {
      $tmp = $tmpkw;
      $tmp =~ s/\s//g;
      $tmp =~ s/\(/\\\(/g;
      $tmp =~ s/\)/\\\)/g;
      #print "$tmp\n";
      if ($line =~ m/$tmp\s*([0-9E\-+.]+)/i) {
        $rpt_ptr->{$bm}->{$opt_ptr->{N_val}}->{$type}->{$tmpkw} = $1;
        $rpt_ptr->{$bm}->{$opt_ptr->{N_val}}->{$type}->{$tmpkw} =~ s/\.$//;
      }
    }
  }
  close(VSTATS);
}

# Extract AAPack stats
sub extract_aapack_stats($ $ $ $)
{
  my ($bm,$vstats,$type,$keywords) = @_;
  my ($line,$tmp);
  open (VSTATS, "< $vstats") or die "Fail to open $vstats!\n";
  while(defined($line = <VSTATS>)) {
    chomp $line; 
    #$line =~ s/\s//g;
    foreach my $tmpkw(@{$keywords}) {
      $tmp = $tmpkw;
      $tmp =~ s/\(/\\\(/g;
      $tmp =~ s/\)/\\\)/g;
      if ($line =~ m/\s*([0-9E\-+.]+)\s+of\s+type\s+$tmpkw/i) {
        $rpt_ptr->{$bm}->{$opt_ptr->{N_val}}->{$type}->{$tmpkw} = $1;
        $rpt_ptr->{$bm}->{$opt_ptr->{N_val}}->{$type}->{$tmpkw} =~ s/\.$//;
      }
    }
  }
  close(VSTATS);
}

sub run_std_vpr($ $ $ $ $ $)
{
  my ($bm,$blif,$arch,$net,$place,$route,$log) = @_;
  my ($vpr_dir,$vpr_name) = &split_prog_path($conf_ptr->{dir_path}->{vpr_path}->{val});
  chdir $vpr_dir;
  `csh -cx './$vpr_name $arch $bm --net_file $net --place_file $place --route_file $route --full_stats --nodisp > $log'`;
  chdir $cwd;
}

sub run_ace($ $ $) 
{
  my ($mpack_vpr_blif,$act_file,$ace_new_blif,$log) = @_;
  my ($ace_dir,$ace_name) = &split_prog_path($conf_ptr->{dir_path}->{ace_path}->{val});
  
  chdir $ace_dir;
  `csh -cx './$ace_name -b $mpack_vpr_blif -o $act_file -n $ace_new_blif > $log'`;

   chdir $cwd;
} 

sub run_mpack_vpr($ $ $ $ $ $ $)
{
  my ($arch,$blif,$net,$place,$route,$log,$act_file) = @_;
  my ($vpr_dir,$vpr_name) = &split_prog_path($conf_ptr->{dir_path}->{vpr_path}->{val});
  my ($power_opts) = ("");
  if ("on" eq $opt_ptr->{power}) {
    $power_opts = "--power --activity_file $act_file --tech_properties $conf_ptr->{flow_conf}->{power_tech_xml}->{val}";
  }
  chdir $vpr_dir;
  `csh -cx './$vpr_name $arch $blif --net_file $net --place_file $place --route_file $route --place --route --full_stats --nodisp $power_opts > $log'`;
  chdir $cwd;
}

sub run_aapack($ $ $ $)
{
  my ($blif,$arch,$net,$aapack_log) = @_; 
  my ($vpr_dir,$vpr_name) = &split_prog_path($conf_ptr->{dir_path}->{vpr_path}->{val});
  
  chdir $vpr_dir;

  `csh -cx './$vpr_name $arch $blif --net_file $net --pack --timing_analysis off --nodisp > $aapack_log'`;

  chdir $cwd; 
}

sub run_m2net_pack_arch($ $ $ $ $)
{
  my ($m2net_conf,$mpack1_rpt,$pack_arch,$N,$I,$m2net_pack_arch_log) = @_;
  my ($m2net_dir,$m2net_name) = &split_prog_path($conf_ptr->{dir_path}->{m2net_path}->{val});

  chdir $m2net_dir;

  `csh -cx 'perl $m2net_name -conf $m2net_conf -mpack1_rpt $mpack1_rpt -mode pack_arch -N $N -I $I -arch_file_pack $pack_arch > $m2net_pack_arch_log'`;

  chdir $cwd;
} 

sub run_m2net_m2net($ $ $ $ $)
{
  my ($m2net_conf,$mpack1_rpt,$aapack_net,$vpr_net,$vpr_arch,$N,$I,$m2net_m2net_log) = @_;
  my ($m2net_dir,$m2net_name) = &split_prog_path($conf_ptr->{dir_path}->{m2net_path}->{val});

  chdir $m2net_dir;

  my ($power_opt) = ("");

  if ("on" eq $opt_ptr->{power}) {
    $power_opt = "-power";
  }
 
  `csh -cx 'perl $m2net_name -conf $m2net_conf -mpack1_rpt $mpack1_rpt -mode m2net -N $N -I $I -net_file_in $aapack_net -net_file_out $vpr_net -arch_file_vpr $vpr_arch $power_opt > $m2net_m2net_log'`;

  chdir $cwd;
} 

# Run EDA flow
sub run_flow()
{
  my ($tmp,$rpt_dir,$prefix);
  my ($abc_bm,$abc_blif_out,$abc_log);
  my ($mpack_blif_out,$mpack_stats,$mpack_log);
  my ($vpr_net,$vpr_place,$vpr_route,$vpr_log);

  # Run Benchmark one by one
  foreach $tmp(@benchmarks) {
    # Run Standard flow
    $tmp =~ s/\.blif$//g;     
    if (("both" eq $conf_ptr->{flow_conf}->{flow_type}->{val})||("standard" eq $conf_ptr->{flow_conf}->{flow_type}->{val})) {
      $rpt_dir = "$conf_ptr->{dir_path}->{rpt_dir}->{val}"."/$tmp/standard";
      &generate_path($rpt_dir);
      $abc_bm = "$conf_ptr->{dir_path}->{benchmark_dir}->{val}"."/$tmp".".blif";
      $prefix = "$rpt_dir/$tmp\_"."K$opt_ptr->{K_val}\_"."N$opt_ptr->{N_val}\_";
      $abc_blif_out = "$prefix"."abc.blif";
      $abc_log = "$prefix"."abc.log";

      &run_abc_fpgamap($abc_bm,"$abc_blif_out".".bak",$abc_log);
      `perl pro_blif.pl -i "$abc_blif_out\.bak" -o $abc_blif_out`;

      $vpr_net = "$prefix"."vpr.net";
      $vpr_place = "$prefix"."vpr.place";
      $vpr_route = "$prefix"."vpr.route";
      $vpr_log = "$prefix"."vpr.log";

      &run_std_vpr($abc_blif_out,$tmp,$conf_ptr->{flow_conf}->{std_vpr_arch}->{val},$vpr_net,$vpr_place,$vpr_route,$vpr_log);
      # Extract data from VPR stats
      &extract_vpr_stats($tmp,$vpr_log,$opt_ptr->{K_val});
    }
    # Run MPACK-oriented flow
    if (("both" eq $conf_ptr->{flow_conf}->{flow_type}->{val})||("mpack" eq $conf_ptr->{flow_conf}->{flow_type}->{val})) {
      $rpt_dir = "$conf_ptr->{dir_path}->{rpt_dir}->{val}"."/$tmp/mpack";
      &generate_path($rpt_dir);
      $abc_bm = "$conf_ptr->{dir_path}->{benchmark_dir}->{val}"."/$tmp".".blif";
      $prefix = "$rpt_dir/$tmp\_"."M$opt_ptr->{M_val}\_"."N$opt_ptr->{N_val}\_";
      $abc_blif_out = "$prefix"."abc.blif";
      $abc_log = "$prefix"."abc.log";

      &run_abc_libmap($abc_bm,"$abc_blif_out\.bak",$abc_log);
      `perl pro_blif.pl -i "$abc_blif_out\.bak" -o $abc_blif_out`;

      my ($mpack_pack_blif_out) = ("$prefix"."matrix.blif");
      my ($mpack_vpr_blif_out) = ("$prefix"."formatted.blif");
      my ($mpack_rpt) = ("$prefix"."mapped.rpt");
      my ($mpack_log) = ("$prefix"."mpack1p5.log");
      &run_mpack1p5("$abc_blif_out","$prefix",$opt_ptr->{M_val},2,$mpack_log);
  
      # Extract data from MPACK stats
      &extract_mpack_stats($tmp,$mpack_log);
  
      # Generate Architecture XML
      my ($aapack_arch) = ("$prefix"."aapack_arch.xml");
      my ($m2net_pack_arch_log) = ("$prefix"."m2net_pack_arch.log");
      &run_m2net_pack_arch($conf_ptr->{flow_conf}->{m2net_conf}->{val},$mpack_rpt,$aapack_arch,$opt_ptr->{N_val},$opt_ptr->{I_val},$m2net_pack_arch_log);

      # Run AAPACK
      my ($aapack_log) = ("$prefix"."aapack.log");
      my ($aapack_net) = ("$prefix"."aapack.net");
      &run_aapack($mpack_pack_blif_out,$aapack_arch,$aapack_net,$aapack_log);
      my @aapack_stats = ("MATRIX");
      &extract_aapack_stats($tmp,$aapack_log,$opt_ptr->{M_val},\@aapack_stats);
  
      $vpr_net = "$prefix"."mpack.net";
      $vpr_place = "$prefix"."vpr.place";
      $vpr_route = "$prefix"."vpr.route";
      $vpr_log = "$prefix"."vpr.log";

      # Run m2net.pl 
      my ($vpr_arch) = ("$prefix"."vpr_arch.xml");
      my ($m2net_m2net_log) = ("$prefix"."m2net_m2net.log");
      &run_m2net_m2net($conf_ptr->{flow_conf}->{m2net_conf}->{val},$mpack_rpt,$aapack_net,$vpr_net,$vpr_arch,$opt_ptr->{N_val},$opt_ptr->{I_val},$m2net_m2net_log);

      my ($act_file,$ace_new_blif,$ace_log) = ("$prefix"."ace.act","$prefix"."ace_new.blif","$prefix"."ace.log");
      # Turn on Power Estimation and Run ace
      if ("on" eq $opt_ptr->{power}) {
        &run_ace($mpack_vpr_blif_out,$act_file,$ace_new_blif,$ace_log);
      }

      &run_mpack_vpr($vpr_arch,$mpack_vpr_blif_out,$vpr_net,$vpr_place,$vpr_route,$vpr_log,$act_file);
      
      # Extract data from VPR stats
      &extract_vpr_stats($tmp,$vpr_log,$opt_ptr->{M_val});

      if ("on" eq $opt_ptr->{power}) {
        &extract_vpr_power_esti($mpack_vpr_blif_out,$tmp,$opt_ptr->{M_val});
      }
    }
  }
}

# Generate CSV report
sub gen_csv_rpt()
{
  my ($tmp,$ikw,$tmpkw);
  my @keywords;
  # Open CSV report
  open (CSV, "> $opt_ptr->{rpt_val}") or die "Fail to open $opt_ptr->{rpt_val}!\n";
  if (("both" eq $conf_ptr->{flow_conf}->{flow_type}->{val})||("standard" eq $conf_ptr->{flow_conf}->{flow_type}->{val})) {
    # Print out Standard Stats First
    print CSV "Standard"; 
    @keywords = split /\|/,$conf_ptr->{csv_tags}->{vpr_tags}->{val};
    #foreach $tmpkw(@keywords) {
    for($ikw=0; $ikw < ($#keywords+1); $ikw++) {
      print CSV ",$keywords[$ikw]";
    }
    print CSV "\n";
    # Check log/stats one by one
    foreach $tmp(@benchmarks) {
      $tmp =~ s/\.blif$//g;     
      print CSV "$tmp";
      #foreach $tmpkw(@keywords) {
      for($ikw=0; $ikw < ($#keywords+1); $ikw++) {
        $tmpkw = $keywords[$ikw];
        $tmpkw =~ s/\s//g;  
        print CSV ",$rpt_ptr->{$tmp}->{$opt_ptr->{N_val}}->{$opt_ptr->{K_val}}->{$keywords[$ikw]}";
      }
      print CSV "\n";
    }
  }  
  if (("both" eq $conf_ptr->{flow_conf}->{flow_type}->{val})||("mpack" eq $conf_ptr->{flow_conf}->{flow_type}->{val})) {
    # Print out Mpack stats Second
    print CSV "Mpack"; 
    print CSV ",MATRIX";
    @keywords = split /\|/,$conf_ptr->{csv_tags}->{mpack_tags}->{val};
    for($ikw=0; $ikw < ($#keywords+1); $ikw++) {
      print CSV ",$keywords[$ikw]";
    }
    @keywords = split /\|/,$conf_ptr->{csv_tags}->{vpr_tags}->{val};
    #foreach $tmpkw(@keywords) {
    for($ikw=0; $ikw < ($#keywords+1); $ikw++) {
      print CSV ",$keywords[$ikw]";
    }
    # Print Power Tags
    @keywords = split /\|/,$conf_ptr->{csv_tags}->{vpr_power_tags}->{val};
    #foreach $tmpkw(@keywords) {
    for($ikw=0; $ikw < ($#keywords+1); $ikw++) {
      print CSV ",$keywords[$ikw]";
    }
    print CSV ",Total Power,Total Dynamic Power, Total Leakage Power";
    print CSV "\n";
    # Check log/stats one by one
    foreach $tmp(@benchmarks) {
      $tmp =~ s/\.blif$//g;     
      print CSV "$tmp";
      #foreach $tmpkw(@keywords) {
      print CSV ",$rpt_ptr->{$tmp}->{$opt_ptr->{N_val}}->{$opt_ptr->{M_val}}->{MATRIX}";
      @keywords = split /\|/,$conf_ptr->{csv_tags}->{mpack_tags}->{val};
      for($ikw=0; $ikw < ($#keywords+1); $ikw++) {
        $tmpkw = $keywords[$ikw];
        $tmpkw =~ s/\s//g;  
        print CSV ",$rpt_ptr->{$tmp}->{$opt_ptr->{N_val}}->{$opt_ptr->{M_val}}->{$keywords[$ikw]}";
      }
      @keywords = split /\|/,$conf_ptr->{csv_tags}->{vpr_tags}->{val};
      for($ikw=0; $ikw < ($#keywords+1); $ikw++) {
        $tmpkw = $keywords[$ikw];
        $tmpkw =~ s/\s//g;  
        print CSV ",$rpt_ptr->{$tmp}->{$opt_ptr->{N_val}}->{$opt_ptr->{M_val}}->{$keywords[$ikw]}";
      }
      # Print Power Results
      @keywords = split /\|/,$conf_ptr->{csv_tags}->{vpr_power_tags}->{val};
      for($ikw=0; $ikw < ($#keywords+1); $ikw++) {
        $tmpkw = $keywords[$ikw];
        $tmpkw =~ s/\s//g;  
        print CSV ",$rpt_ptr->{$tmp}->{$opt_ptr->{N_val}}->{$opt_ptr->{M_val}}->{power}->{$keywords[$ikw]}";
      }
      print CSV ",$rpt_ptr->{$tmp}->{$opt_ptr->{N_val}}->{$opt_ptr->{M_val}}->{power}->{total}";
      print CSV ",$rpt_ptr->{$tmp}->{$opt_ptr->{N_val}}->{$opt_ptr->{M_val}}->{power}->{dynamic}";
      print CSV ",$rpt_ptr->{$tmp}->{$opt_ptr->{N_val}}->{$opt_ptr->{M_val}}->{power}->{leakage}";
      print CSV "\n";
    }
  }  
  close(CSV);
}

sub remove_designs() {

  # Remove designs
  if ("on" eq $opt_ptr->{remove_designs}) {
    `rm -rf $conf_ptr->{dir_path}->{rpt_dir}->{val}`;
  }
 
}

# Main Program
sub main()
{
  &opts_read();
  &read_conf();
  &read_benchmarks();
  &remove_designs(); 
  &run_flow();
  &gen_csv_rpt();
}

&main();
exit(0);
