#!usr/bin/perl -w
#declare all use relations
#use the shell because in the linux enviornment.
#use time package to specify when the sp file for HSPICE 
#Simulation is created.
use strict;
use Shell;
use Time::gmtime;
#GetDate Example
my $Date = gmctime();
#Presettings
#Sp(spice) file settings --defaults
my $sp_name = "temp.sp";
my $lis_name = "temp.lis";
#directory settings --defaults
my $sp_src_dir = "Benchmarks";
my $rpt_des_dir = "solutions";
my $spice_dir = "SpiceDir";
#configure file settings --defaults
my $dir_conf = "dir.conf";
my $spfile_conf = "spfile.conf";
#sp file list --defualts
my @sp_file_list = ();
#Report file settings --defaults
my $rpt_name = "temp.log";
#Options
my $opt_mode = "batch"; #default
my $input_sp = "temp.sp"; #default
my $output_lis = "temp.lis"; #default
my $output_rpt = "temp.log"; #default
my $mt = "off";
my $mt_val = 1;
my $server_mode = "off";
my $bit64_hspice = "off";
my $vias_report = "off";
 
#define an internal pipe storing the content of SP file.
my @sp_tmp = (
             "*****************************************",
             "*       Report for CIRCUIT_NAME         *",
             "*       Author : Xifan TANG             *",
             "*       Organization: EPFL              *",
             "*       Date :   DATE                   *",
             "*****************************************", 
             );
#define the hash containing all nodes in the netlist
my %node_hash;
#define the hash for layer mapping
my %layer_map;
#define the hash for vias mapping
my %vias_map;
my $vias_res = 0.05;
my %vias_stats;
$vias_stats{cur} = 0;
$vias_map{cur} = 0;
my %inode_list;


#debug mode
my $debug = 0;

my %unit_map;

$unit_map{"m"} = 1E-3;
$unit_map{"u"} = 1E-6;
$unit_map{"n"} = 1E-9;
$unit_map{"p"} = 1E-12;
$unit_map{"f"} = 1E-15;
$unit_map{"a"} = 1E-18;

sub print_usage()
{
    print "No arguments are detected!\n";
    print "Usage : \n";
    print "        PowerGrid -mode <value> -[options..] <value>\n";
    print "Arguments:\n";
    print "          -mode: there are two modes for chosen.\n";
    print "                One is batch, the other is single.\n";
    print "                Ex. -mode batch\n";
    print "          -input: For single mode only, specify the spice netlist.\n ";
    print "          -ouput: For single mode only, specify the output lis file\n";
    print "          -report: For single mode only, specify the report file\n";
    print "          -vias_report : turn on reporting the vias(Default : off).\n";
    print "          -dir_conf: For batch mode only, the configure file contains directory information.\n";
    print "          -spfile_conf For batch mode only, the configura file contains sp file information.\n";
    print "          -64_bit : turn on the 64-bit HSPICE.\n";
    print "          -server_mode : turn on the server mode of HSPICE.\n";
    print "          -mt <value> : turn on the multithread of HSPICE.\n";
    exit(0); 
}

sub opts_read()
{
  #when no arguments are input, print the usage
  if (0 == $#ARGV)
  {&print_usage();}
  else
  {
    #Debug mode: print all ARGVs
    if (1 == $debug)
    {
      print "Your input argvs are \n";
      for (my $iargv = 0; $iargv < $#ARGV+1 ; $iargv++)
      {print "$iargv : $ARGV[$iargv]\n";} 
    }    
    return 1;
  }
}

sub spot_option($ $)
{
  my ($start,$target) = @_;
  my ($arg_no,$flag) = (-1,"unfound");
  for (my $iarg = $start; $iarg<$#ARGV+1; $iarg++)
  {
    if ($ARGV[$iarg] eq $target)
    {
      if ("found" eq $flag) 
      {
        print "Repeated Arguments(IndexA: $arg_no, IndexB: $iarg) found!\n";
        &print_usage();    
      }
      else
      {
        $flag = "found";
        $arg_no = $iarg;
      }
    }
  }  
  #find the argument return the arg_no
  #or return -1 when nothing found.
  return $arg_no;
}

sub opts_check()
{
  my ($cur_arg,$arg_found);
  $cur_arg = 0;
  print "Checking your options...\n";
  #Check Options.
  #Check Mode: the first argument should be "-mode" 
  if ("-mode" eq $ARGV[$cur_arg])
  {
    $cur_arg++;
    if ("batch" eq $ARGV[$cur_arg])
    {$opt_mode = "batch";}
    elsif ("single" eq $ARGV[$cur_arg])
    {$opt_mode = "single";}
    else
    {
      print "Invaild Mode Name($ARGV[$cur_arg])!\n";
      &print_usage();
    }
  } 
  else
  {
    print "Error: Mode is not selected!\n";
    &print_usage();
  }
  
  #Check options with determined mode
  $cur_arg++;
  if ("batch" eq $opt_mode)
  {
    #Find directory configuration
    $arg_found = &spot_option($cur_arg,"-dir_conf");
    if ($arg_found != -1)
    {
      $dir_conf = $ARGV[$arg_found+1];
    }
    else
    {
      print "Warning: -dir_conf not found! Use default value($dir_conf)...\n";
    }
    #Find Sp file configuration
    $arg_found = &spot_option($cur_arg,"-spfile_conf");
    if ($arg_found != -1)
    {
      $spfile_conf = $ARGV[$arg_found+1];
    }
    else
    {
      print "Warning: -spfile_conf not found! Use default value($spfile_conf)...\n";
    }
  }
  elsif ("single" eq $opt_mode)
  {
    #Find input sp file
    $arg_found = &spot_option($cur_arg,"-input");
    if ($arg_found != -1)
    {
      $input_sp = $ARGV[$arg_found+1];
    }
    else
    {
      print "Warning: -input not found! Use default value($input_sp)...\n";
    }
    #Find ouput lis file
    $arg_found = &spot_option($cur_arg,"-output");
    if ($arg_found != -1)
    {
      $output_lis = $ARGV[$arg_found+1];
    }
    else
    {
      print "Warning: -output not found! Use default value($output_lis)...\n";
    }
    #Find output report file
    $arg_found = &spot_option($cur_arg,"-report");
    if ($arg_found != -1)
    {
      $output_rpt = $ARGV[$arg_found+1];
    }
    else
    {
      print "Warning: -report not found! Use default value($output_rpt)...\n";
    }

  }
  #Find 65-bit hspice mode
  $arg_found = &spot_option($cur_arg,"-64_bit");
  if ($arg_found != -1)
  {
    $bit64_hspice = "on";
  }
  else
  {
    $bit64_hspice = "off";
  }
  #Find vias report
  $arg_found = &spot_option($cur_arg,"-vias_report");
  if ($arg_found != -1)
  {
    $vias_report = "on";
  }
  else
  {
    $vias_report = "off";
  }
  #Find server mode
  $arg_found = &spot_option($cur_arg,"-server_mode");
  if ($arg_found != -1)
  {
    $server_mode = "on";
  }
  else
  {
    $server_mode = "off";
  }
  #Find multi-thread
  $arg_found = &spot_option($cur_arg,"-mt");
  if ($arg_found != -1)
  {
    $mt = "on";
    $mt_val = $ARGV[$arg_found+1];
  }
  else
  {
    $mt = "off";
  }
  #print the options 
  print "Your Options:\n";
  print "Mode : $opt_mode\n";
  print "Dir_conf : $dir_conf\n";
  print "Spfile_conf : $spfile_conf\n";
  print "Input : $input_sp\n";
  print "Output : $output_lis\n";
  print "Report : $output_rpt\n"; 
  print "Vias Report : $vias_report\n"; 
  print "Server mode : $server_mode\n";
  print "64-bit hspice mode : $bit64_hspice\n";
  print "Multi-thread : $mt, Thread No. : $mt_val\n";
  return 1;
}

#Read each line and ignore the comments which starts with "*"
#Return the valid information of line.
sub read_line($)
{
  my ($line) = @_;
  my ($rtline) = ("*");
  #Split the $line given into chars.  
  my @chars = split/\s*/,$line;
  #if (1 == $debug)
  #{print "Debug: chars : @chars\n";}
  for (my $ichar = 0; $ichar<($#chars+1); $ichar++)
  {
    #Spot where the comments start. 
    if ("*" ne $chars[$ichar])
    {$rtline = $rtline.$chars[$ichar];}
    else 
    {return $rtline;}
  }
  return $rtline;
}

sub read_dir_conf()
{
  my ($line,$post_line);
  my @equation;
  open (DIRCONF,"< $dir_conf") or die "Fail to open $dir_conf!\n";
  print "Reading $dir_conf...";
  while(defined($line = <DIRCONF>))
  {
    chomp $line;
    $post_line = &read_line($line);
    if ($post_line ne "*")
    {
      #Kick the space out.
      $post_line =~ s/[*|\s]//g;
      #split it
      @equation = split/=>/,$post_line;
      if (1 == $debug)
      {print "\nDebug: equation: (@equation)\n";}
      if (0 == $#equation) 
      {print "Warning: Invaild input lines!($post_line)\n";}
      else
      {
        if ("sp_src_dir" eq $equation[0])
        {$sp_src_dir = $equation[1];}
        elsif("rpt_des_dir" eq $equation[0])
        {$rpt_des_dir = $equation[1];}
        elsif ("spice_dir" eq $equation[0] )
        {$spice_dir = $equation[1];}         
        else
        {print "Warning: Unkown parameter($equation[0])!\n";}
      }
    }
  }
  print "Successfully!\n";
  if (1 == $debug)
  {
    print "Debug mode: Configuration read\n";
    print "sp_src_dir : $sp_src_dir\n";
    print "rpt_des_dir : $rpt_des_dir\n";
    print "SpiceDir : $spice_dir\n";
  }
  close(DIRCONF);
  return 1;
}

sub read_spfile_conf()
{
  my ($line,$post_line,$cur);
  $cur = 0;
  open (SPFCONF,"< $spfile_conf") or die "Fail to open $spfile_conf!\n";
  print "Reading $spfile_conf...";
  while(defined($line = <SPFCONF>))
  {
    chomp $line;
    $post_line = &read_line($line);
    if ($post_line ne "*")
    {
      #Kick the space out.
      $post_line =~ s/[\s|*]//g;
      if (1 == $debug)
      {print "\nDebug: postline($post_line)\n";}
      $sp_file_list[$cur] = $post_line;
      $cur++;
    }
  }
  print "Successfully!\n";
  if (1 == $debug)
  {
    print "Debug mode: Configuration read\n";
    for (my $i = 0; $i<$cur; $i++)
    {
      print "Index: $i Content: $sp_file_list[$i]\n";
    }
  }
  close(SPFCONF);
  #return the number of spice file read.
  return $cur;
}

sub anaylze_comment($ $)
{
  my ($line,$cur) = @_;
  if ($line =~ m/^\*/)
  {
    #Determine whether it is layer definition
    if ($line =~ m/layer(\s*):(\s*)([\d\w]+)/)
    {
      my ($layer_idx,$net_type,$net_idx);
      $layer_idx = $3;
      if ($line =~ m/vdd/i)
      {$net_type = "vdd";}
      elsif ($line =~ m/gnd/i)
      {$net_type = "gnd";}
      else
      {print "Unknown net type!\nLine($cur):$line\n";}
      $line =~ m/net(\s*):(\s*)(\d+)/;
      $net_idx = "n".$3;
      print "Comment on Layer info(Name=$layer_idx,Net idx=$net_idx,Net type=$net_type\n";
      if (1 == $debug)  
      {print "Inserting data into layer map...\n";}
      #Build hash for layer map
      $layer_map{$net_idx}->{layer} = $layer_idx;
      $layer_map{$net_idx}->{type} = $net_type;
    }
    #it is vias definition?
    elsif ($line =~ m/vias/)
    {
      $line =~ s/\s//g;
      $line =~ m/from:(\d+)to(\d+)/;
      my ($net_from,$net_to) = ($1,$2);
      print "Comment on Vias info(From net$net_from to net$net_to)\n";
      if (1== $debug)
      {print "Inserting data to vias map...\n";}
      #Build hash for vias map
      my $vias_cur = $vias_map{cur};
      $vias_map{$vias_cur}->{from} = $net_from;
      $vias_map{$vias_cur}->{to} = $net_to;
      $vias_map{cur}++;
    }
  }
  return 1;
}

sub reconstruct_spice($ $)
{
  my ($src_sp,$des_sp) =@_;
  my ($line,$cur) = ("",0);
  #Open the source file to analyze and the destination file for writing.
  print "Start analyzing spice source file...\n";
  open (SRCF," <$src_sp") or die "Fail to open the $src_sp!\n";
  open (DESF," >$des_sp") or die "Fail to create the $des_sp!\n";
  #First scan the source file and extract some information from comments
  #Such as vias from netA to netB
  #Such as layer: M5, VDD net: 2
  while(defined($line = <SRCF>))
  {
    chomp $line;
    &anaylze_comment($line,$cur);
    $cur++;
  }
  close (SRCF);
  
  open (SRCF," <$src_sp") or die "Fail to open the $src_sp!\n";
  $cur = 0;
  #Reread the source file and alter it. 
  while(defined($line = <SRCF>))
  {
    chomp $line;
    #Revise the vias with zero voltage source
    if ($line =~ m/^V/i)
    {
      #This may be a vias definition
      my @line_info = split /\s+/,$line;
      my @node_from;
      my @node_to;
      if (0 == $line_info[3])
      {
        #Zero voltage source->short,considering replacement.
        @node_from = split /_/, $line_info[1];
        @node_to = split /_/, $line_info[2]; 
        if (($node_from[0] ne "X")&&($node_to[0] ne "X"))
        {
          my $find_flag = 0;
          #Further, searching the vias map
          for (my $icur = 0; $icur<$vias_map{cur}; $icur++)
          {
            if (($node_from[0] eq "n".$vias_map{$icur}->{from})&&($node_to[0] eq "n".$vias_map{$icur}->{to})) 
            {$find_flag = 1;}
          }
          #If found, do replacement
          if (1 == $find_flag)
          {
            $line_info[0] = "Rvias_".$line_info[0];
            $line_info[3] = $vias_res;
            $line = join (" ", $line_info[0],$line_info[1],$line_info[2],$line_info[3]);
            #Record in the vias_stats to calculate the current
            my $cur = $vias_stats{cur};
            $vias_stats{$cur}->{from}->{node} = $line_info[1];
            $vias_stats{$cur}->{to}->{node} = $line_info[2];
            $vias_stats{$cur}->{res} = $line_info[3];
            $vias_stats{cur}++;
          }
        }
      }      
    }
    if ($line =~ m/^i/i)
    {
      #A current source
      #determine its type and record the node.
      my @line_info = split /\s+/,$line; 
      my @i_info = split /_/,$line_info[0];
      my ($i_node,$i_type);
      if ($i_info[2] =~ m/v/i)
      {
        $i_node = $line_info[1];
        $i_type = "vdd";
      }
      elsif ($i_info[2] =~ m/g/i)
      {
        $i_node = $line_info[2];
        $i_type = "gnd";
      }
      else
      {die "Undefined type of node: $line!\n";}
      #Record it into hash
      if (defined $i_node)
      {
        my @node_info = split /_/,$i_node;
        $inode_list{$i_node}->{x} = $node_info[1];
        $inode_list{$i_node}->{y} = $node_info[2];
        $inode_list{$i_node}->{type} = $i_type;
        $inode_list{$i_node}->{layer} = $layer_map{$node_info[0]}->{layer};
        $inode_list{$i_node}->{current} = $line_info[3];
        if ($i_type ne $layer_map{$node_info[0]}->{type})
        {print "Warning: Unmatch type!(layer_map:$layer_map{$node_info[0]}->{type},current_source: $i_type\n";}
        if (1 == $debug)
        {print "Node(x=$node_info[1],y=$node_info[2],type=$i_type,layer=$layer_map{$node_info[0]}->{layer}) connected to current source!\n";}
      }
    }
    #Record the nodes that connected to current source.
    #Put down the lines into destination file.
    print DESF "$line\n";
    #Increment in line counter
    $cur++;
  }
  print "Source file analysis($cur lines) completed...\n";
  close(SRCF);
  close(DESF);
  return 1;
}

sub run_hspice($ $)
{
  my ($sp_file,$lis_file) = @_;
  my ($hspice_path) = ("hspice");
  my ($multi_thread,$server_opt) = (" ", " ");
  if ($bit64_hspice eq "on")
  {$hspice_path .= "64";}
  if ($mt eq "on")
  {$multi_thread = "-mt $mt_val";}
  if ($server_mode eq "on")
  {$server_opt = "-C";}
  `csh -cx '$hspice_path $server_opt -i $sp_file -o $lis_file $multi_thread'`;
  #check whether there is fatal error in the lis file.
  open (LISF, "< $lis_file") or die "Fail to open the lis file($lis_file)!\n";
  my ($line,$flag_err);
  while(defined($line = <LISF>))
  {
    chomp $line;
    if ($line =~ /error/)
    {
      print "Error detected in the lis file!\n";
      print "Info:  $line\n";
      exit(1);
    }
  }
  print "Hspice has been run successfully!\n";
  close(LISF);
  return 1;
}


sub extract_position($)
{
  my ($node) = @_;
  $node =~ m/n(\d+)\_(\d+)\_(\d+)/;
  my ($type,$layer,$x,$y);
  ($layer,$x,$y) = ($1,$2,$3);
  $layer = "n".$layer;
  $type = $layer_map{$layer}->{type};
  $layer = $layer_map{$layer}->{layer};
  return ($type,$layer,$x,$y);
}

sub extract_volt($)
{
  my ($volt) = @_;
  my ($real) = (1);
  if (1 == $debug)
  {print "origin: $volt ";}
  if ($volt =~ m/([a-zA-Z])$/)
  {
    my $unit = $1;
    $real = $unit_map{$unit};
    $volt =~ s/(\w)$//;
    $real = $volt *$real;
    if (1 == $debug)
    {print "unit:$unit, converted to $real.\n";} 
  }
  else
  {$real = $volt;}
  if (1 == $debug)
  {print "Voltage after conversion : $real\n";}
  return $real;
}

sub extract_node_and_volt($ $)
{
  my ($node,$volt) = @_;
  my ($layer,$x,$y,$volt_real,$type);
  my ($flag);
  #Process the node's information
  if ($node =~ m/^\_x\_/) 
  {
    $node =~ s/^\_x\_//;
    #if match, the node is a package node
    ($type,$layer,$x,$y) = &extract_position($node);
    $type = "package_node";
    if (1 == $debug)
    {
      print "Type: Package node(_X_) ";
      print "Layer: $layer, (X,Y)=($x,$y).\n";
    }
  }
  else
  {
    ($type,$layer,$x,$y) = &extract_position($node);
    $flag = "internal_node";
    if (1 == $debug)
    {
      print "Type: internal node($type) ";
      print "Layer: $layer, (X,Y)=($x,$y).\n";
    }
  }
  if (1 == $debug)
  {
    print "Identify the voltage value...\n";
  }
  #Process the voltage conversion
  $volt_real = &extract_volt($volt);
  #Write the hash
  #"defined" function is useful to determine whether the variant is undef or not.
  #When it is undef return false.
  return ($type,$layer,$x,$y,$volt_real);
}

sub gen_summary($ $)
{
  my ($smy_file,$stats) = @_; 
  $smy_file =~ s/.log$/.summary/;
  my ($total_node) = (0);
  print "Generating Summary file($smy_file)...\n";
  open (SUMF, "> $smy_file") or die "Fail to create the $smy_file";
  #print the statistics 
  print "*******Breif Summary of Circuit********\n";
  print SUMF "*********Statistics*************\n";
  while (my ($layer,$vlayer) = each %{$stats})
  {
    print "*******Layer: $layer******\n";
    print SUMF "*******Layer: $layer******\n";
    while (my ($type,$vtype) = each %{$stats->{$layer}})
    {
      print "******Type: $type********,\n ";
      print "*\tMax(Voltage: $stats->{$layer}->{$type}->{max}->{v},";
      print "Node: $stats->{$layer}->{$type}->{max}->{node},";
      print "X=$stats->{$layer}->{$type}->{max}->{x},";
      print "Y=$stats->{$layer}->{$type}->{max}->{y},";
      print "is_blk=$stats->{$layer}->{$type}->{max}->{is_iblk}.\t*\n";    
      print SUMF "********Type: $type********,\n ";
      print SUMF "*\tMax(Voltage: $stats->{$layer}->{$type}->{max}->{v},";
      print SUMF "Node: $stats->{$layer}->{$type}->{max}->{node},";
      print SUMF "X=$stats->{$layer}->{$type}->{max}->{x},";
      print SUMF "Y=$stats->{$layer}->{$type}->{max}->{y},";
      print SUMF "is_blk=$stats->{$layer}->{$type}->{max}->{is_iblk}.\t*\n";    

      print "******Type: $type*******,\n ";
      print "*\tMin(Voltage: $stats->{$layer}->{$type}->{min}->{v},";
      print "Node: $stats->{$layer}->{$type}->{min}->{node},";
      print "X=$stats->{$layer}->{$type}->{min}->{x},";
      print "Y=$stats->{$layer}->{$type}->{min}->{y},";
      print "is_blk=$stats->{$layer}->{$type}->{min}->{is_iblk}.\t*\n";    
      print SUMF "*******Type: $type*******,\n ";
      print SUMF "*\tMin(Voltage: $stats->{$layer}->{$type}->{min}->{v},";
      print SUMF "Node: $stats->{$layer}->{$type}->{min}->{node},";
      print SUMF "X=$stats->{$layer}->{$type}->{min}->{x},";
      print SUMF "Y=$stats->{$layer}->{$type}->{min}->{y},";
      print SUMF "is_blk=$stats->{$layer}->{$type}->{min}->{is_iblk}.\t*\n"; 
    
      my $avg;
      if (defined $stats->{$layer}->{$type}->{cur})
      {
        $avg = $stats->{$layer}->{$type}->{total} / $stats->{$layer}->{$type}->{cur};
      }
      else 
      {
        $stats->{$layer}->{$type}->{cur} = 0;
        $avg = 0;
      }
      print "****Average: $avg****\n";
      print "*\tNumber of Nodes: $stats->{$layer}->{$type}->{cur}.\t*\n";
      print SUMF "****Average: $avg****\n";
      print SUMF "*\tNumber of Nodes: $stats->{$layer}->{$type}->{cur}.\t*\n";
        
      $total_node += $stats->{$layer}->{$type}->{cur};
    
      if ((($type eq "vdd")||($type eq "gnd"))&&(defined $stats->{$layer}->{$type}->{iblk}))
      {
        print "*\tAmong those nodes connected to current source:\t*\n";
        print SUMF "*\tAmong those nodes connected to current source:\t*\n";

        print "********Type: $type********,\n ";
        print "*\tMax(Voltage: $stats->{$layer}->{$type}->{iblk}->{max}->{v},";
        print "Node: $stats->{$layer}->{$type}->{iblk}->{max}->{node},";
        print "X=$stats->{$layer}->{$type}->{iblk}->{max}->{x},";
        print "Y=$stats->{$layer}->{$type}->{iblk}->{max}->{y},";
        print "is_blk=$stats->{$layer}->{$type}->{iblk}->{max}->{is_iblk}.\t*\n";    
        print SUMF "******Type: $type******,\n ";
        print SUMF "Max(Voltage: $stats->{$layer}->{$type}->{iblk}->{max}->{v},";
        print SUMF "*\tNode: $stats->{$layer}->{$type}->{iblk}->{max}->{node},";
        print SUMF "X=$stats->{$layer}->{$type}->{iblk}->{max}->{x},";
        print SUMF "Y=$stats->{$layer}->{$type}->{iblk}->{max}->{y},";
        print SUMF "is_blk=$stats->{$layer}->{$type}->{iblk}->{max}->{is_iblk}.\t*\n";    

        print "******Type: $type*********,\n ";
        print "*\tMin(Voltage: $stats->{$layer}->{$type}->{iblk}->{min}->{v},";
        print "Node: $stats->{$layer}->{$type}->{iblk}->{min}->{node},";
        print "X=$stats->{$layer}->{$type}->{iblk}->{min}->{x},";
        print "Y=$stats->{$layer}->{$type}->{iblk}->{min}->{y},";
        print "is_blk=$stats->{$layer}->{$type}->{iblk}->{min}->{is_iblk}.\t*\n";    
        print SUMF "*******Type: $type*****,\n ";
        print SUMF "*\tMin(Voltage: $stats->{$layer}->{$type}->{iblk}->{min}->{v},";
        print SUMF "Node: $stats->{$layer}->{$type}->{iblk}->{min}->{node},";
        print SUMF "X=$stats->{$layer}->{$type}->{iblk}->{min}->{x},";
        print SUMF "Y=$stats->{$layer}->{$type}->{iblk}->{min}->{y},";
        print SUMF "is_blk=$stats->{$layer}->{$type}->{iblk}->{min}->{is_iblk}.\t*\n";   

        my $avg;
        if (defined $stats->{$layer}->{$type}->{iblk}->{cur})
        {
          $avg = $stats->{$layer}->{$type}->{iblk}->{total} / $stats->{$layer}->{$type}->{iblk}->{cur};
        }
        else
        {
          $stats->{$layer}->{$type}->{iblk}->{cur} = 0;
          $avg = 0;
        }
  
        print "****Average: $avg****\n";
        print "*\tNumber of Nodes: $stats->{$layer}->{$type}->{iblk}->{cur}.\t*\n";
        print SUMF "****Average: $avg****\n";
        print SUMF "*\tNumber of Nodes: $stats->{$layer}->{$type}->{iblk}->{cur}.\t*\n";
       }
     }
  } 
  print "**************************************\n";
  print "Number of Node(Total): $total_node\n";
  print SUMF "**************************************\n";
  print SUMF "Number of Node(Total): $total_node\n";
  print "*****************END******************\n"; 
  print SUMF "**************END********************\n";
  close(SUMF); 
  print "Successfully!\n";
  return 1;
}


sub extract_rpt_and_gen_report($ $)
{ 
  my ($lis_file,$rpt_file) = @_;
  #Spot the ic file path to extract all operating points
  my ($ic_file) = ($lis_file);
  $ic_file =~ s/.lis$/.ic0/;
  print "Processing the $ic_file and start extracting data!\n"; 
  my ($line);
  #define the hash storing data
  my ($type,$layer,$x,$y,$volt_value,$is_iblk);
  #process the *.ic0 file 
  my $match_start = 0;
  #Statsitics
  my %h_stats;
  my $stats = \%h_stats;
  open (ICF, "< $ic_file") or die "Fail to open the $ic_file!\n";
  open (RPTF, "> $rpt_file") or die "Fail to create the $rpt_file";
  #Generate report;
  print "Generating Report......\n";
  #print description
  foreach my $tmp(@sp_tmp)
  {
    $Date = gmctime();
    $tmp =~ s/CIRCUIT_NAME/$rpt_file/;
    $tmp =~ s/DATE/$Date/;
    print RPTF "$tmp\n";
  }
  print RPTF "************Nodes List**************\n";
  print RPTF "*Node Type Layer (X,Y) Voltage IBLK*\n";
  while(defined($line = <ICF>)) 
  {
    if ($line =~ m/nodeset/)
    {$match_start = 1;}
    if (1 == $match_start)
    {
      #Basic processing the line
      #Kick off all the space
      $line =~ s/\s//g;
      #Kick off the "+" at the beginning of line
      $line =~ s/^\+//g;
      #Kick off the "." at the end of line if there is any
      $line =~ s/\.$//g;	
      #Extract the data we need if pattern matched
      my ($node,$volt);
      if ($line =~ /([\d\w\_]+)=([\d\w.]+)/)
      {
        $node = $1;
        $volt = $2;
       ($type,$layer,$x,$y,$volt_value) = &extract_node_and_volt($node,$volt);
      }
      else
      {next;}
      #Attribute the node and print
      if (defined $inode_list{$node})
      {$is_iblk = "Y";}
      else
      {$is_iblk = "N";}
      print RPTF "$node $type $layer $x $y $volt_value $is_iblk\n"; 
      if (1 == $debug)
      {print "Node Name: $node, Type: $type, Layer: $layer, (X,Y)=($x,$y), V=$volt_value, Is_IBLK?=$is_iblk\n";}
      if ($vias_report eq "on")
      {
        #Count in vias stats hash
        for (my $icur = 0; $icur<$vias_stats{cur};$icur++)
        {
          if ($vias_stats{$icur}->{from}->{node} eq $node)
          {$vias_stats{$icur}->{from}->{volt} = $volt_value;}
        
          if ($vias_stats{$icur}->{to}->{node} eq $node)
          {$vias_stats{$icur}->{to}->{volt} = $volt_value;}
        } 
      }
      #Count in stats hash
      #Count the number of nodes in this type
      if (defined $stats->{$layer}->{$type}->{cur}) 
      {
        $stats->{$layer}->{$type}->{cur}++;
        $stats->{$layer}->{$type}->{total} += $volt_value;
      } 
      else
      {
        $stats->{$layer}->{$type}->{cur} = 1;
        $stats->{$layer}->{$type}->{total} = $volt_value;
      } 
      if ($is_iblk eq "Y")
      {
        if (defined $stats->{$layer}->{$type}->{iblk}->{cur}) 
        {
          $stats->{$layer}->{$type}->{iblk}->{cur}++;
          $stats->{$layer}->{$type}->{iblk}->{total} += $volt_value;
        } 
        else
        {
          $stats->{$layer}->{$type}->{iblk}->{cur} = 1;
          $stats->{$layer}->{$type}->{iblk}->{total} = $volt_value;
        }
      }
      #determine the minimum
      if ($is_iblk eq  "Y")
      {
        if ((!(defined $stats->{$layer}->{$type}->{iblk}->{min}->{v}))||($stats->{$layer}->{$type}->{iblk}->{min}->{v} > $volt_value))
        {
          $stats->{$layer}->{$type}->{iblk}->{min}->{v} = $volt_value;
          $stats->{$layer}->{$type}->{iblk}->{min}->{node} = $node;
          $stats->{$layer}->{$type}->{iblk}->{min}->{x} = $x;
          $stats->{$layer}->{$type}->{iblk}->{min}->{y} = $y;
          $stats->{$layer}->{$type}->{iblk}->{min}->{is_iblk} = $is_iblk;
        }
        #determine the maximum
        if ((!(defined $stats->{$layer}->{$type}->{iblk}->{max}->{v}))||($stats->{$layer}->{$type}->{iblk}->{max}->{v} < $volt_value))
        {
          $stats->{$layer}->{$type}->{iblk}->{max}->{v} = $volt_value;
          $stats->{$layer}->{$type}->{iblk}->{max}->{node} = $node;
          $stats->{$layer}->{$type}->{iblk}->{max}->{x} = $x;
          $stats->{$layer}->{$type}->{iblk}->{max}->{y} = $y;
          $stats->{$layer}->{$type}->{iblk}->{max}->{is_iblk} = $is_iblk;
        }
      }
      #determine the minimum
      if ((!(defined $stats->{$layer}->{$type}->{min}->{v}))||($stats->{$layer}->{$type}->{min}->{v} > $volt_value))
      {
        $stats->{$layer}->{$type}->{min}->{v} = $volt_value;
        $stats->{$layer}->{$type}->{min}->{node} = $node;
        $stats->{$layer}->{$type}->{min}->{x} = $x;
        $stats->{$layer}->{$type}->{min}->{y} = $y;
        $stats->{$layer}->{$type}->{min}->{is_iblk} = $is_iblk;
      }
      #determine the maximum
      if ((!(defined $stats->{$layer}->{$type}->{max}->{v}))||($stats->{$layer}->{$type}->{max}->{v} < $volt_value))
      {
        $stats->{$layer}->{$type}->{max}->{v} = $volt_value;
        $stats->{$layer}->{$type}->{max}->{node} = $node;
        $stats->{$layer}->{$type}->{max}->{x} = $x;
        $stats->{$layer}->{$type}->{max}->{y} = $y;
        $stats->{$layer}->{$type}->{max}->{is_iblk} = $is_iblk;
      }
    } 
  }
  print RPTF "**************END********************\n";
  close(ICF);
  close(RPTF);
  print "Report $rpt_file generated successfully!\n";
  &gen_summary($rpt_file,$stats);
  return 1;
}

sub vias_current_report($)
{
  my ($o_rpt) = @_;
  my ($o_irpt) = ($o_rpt); 
  $o_irpt =~ s/.log$/_ivias.log/;
  open (IRPT, "> $o_irpt") or die "Fail to create $o_irpt!\n";
  print "Start calculating the vias current...\n";   
  #Count in vias stats hash
  for (my $icur = 0; $icur<$vias_stats{cur};$icur++)
  {
    if ((defined $vias_stats{$icur}->{from}->{volt})&&(defined $vias_stats{$icur}->{to}->{volt}))
    {
      $vias_stats{$icur}->{current} = ($vias_stats{$icur}->{from}->{volt}-$vias_stats{$icur}->{to}->{volt})/$vias_res;
    }
    else
    {die "Error: Missing vias voltage!(Vias,Index:$icur,From:$vias_stats{$icur}->{from}->{node},$vias_stats{$icur}->{from}->{volt},To:$vias_stats{$icur}->{to}->{node},$vias_stats{$icur}->{to}->{volt})\n";}
    print IRPT "Vias,Index:$icur,From:$vias_stats{$icur}->{from}->{node},$vias_stats{$icur}->{from}->{volt},To:$vias_stats{$icur}->{to}->{node},$vias_stats{$icur}->{to}->{volt},Resistance:$vias_res,Current:$vias_stats{$icur}->{current}.\n";
  }
  print "Report $o_irpt generated...successfully!\n";
  close(IRPT);
  return 1;
}


#Run the flow once
sub run_flow($ $ $)
{
  my ($i_sp,$o_lis,$o_rpt) = @_;
  my ($new_sp) = ($o_lis);
  $new_sp =~ s/.lis$/_revise.spice/i;
  if (1 == $debug)
  {print "SPICE file after revised: $new_sp\n";}
  &reconstruct_spice($i_sp,$new_sp);
  #Run the hspice
  &run_hspice($new_sp,$o_lis);
  #Extract the data and generate report
  &extract_rpt_and_gen_report($o_lis,$o_rpt);
  if ($vias_report eq "on")
  {&vias_current_report($o_rpt);}
  return 1;
}

#run the batch mode
sub run_batch()
{
  #from the directory configurations and spfile configurations
  #Run the flow one by one
  for (my $isp = 0; $isp<($#sp_file_list + 1); $isp++)
  {
    print "Prepare the spice,lis,rpt file path for simulation...\n";
    my $sp_path = $sp_src_dir."/".$sp_file_list[$isp];
    my $lis_path = $spice_dir."/".$sp_file_list[$isp];
    #Generate the lis file path
    $lis_path =~ s/.spice$/.lis/;
    my $rpt_path = $rpt_des_dir."/".$sp_file_list[$isp];
    $rpt_path =~ s/.spice$/.log/;
    print "Your spice file path: $sp_path\n";
    print "Your lis file path: $lis_path\n";
    print "Your report file path: $rpt_path\n";
    &run_flow($sp_path,$lis_path,$rpt_path);
    #Reinitialize the hash
    undef %layer_map;
    undef %vias_map;
    undef %vias_stats;
    $vias_stats{cur} = 0;
    $vias_map{cur} = 0;
    undef %inode_list;
  }
  return 1;
}

sub main()
{
  &opts_read();
  &opts_check();
  if ("batch" eq $opt_mode)
  {
    #read directory configuration file
    &read_dir_conf();
    #read spfile configuration file
    &read_spfile_conf();
    #run flow one by one
    &run_batch();
  }
  elsif ("single" eq $opt_mode)
  {
    &run_flow($input_sp,$output_lis,$output_rpt);
  } 
  return 1;
}


&main();
exit(0);
