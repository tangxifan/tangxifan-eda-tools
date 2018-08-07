#!/usr/bin/perl

use strict;
#use Shell;
use POSIX;
use File::Basename;
use File::Spec;
use Cwd 'abs_path';

sub get_capacitances;
sub get_pn_ratio;
sub get_riset_fallt_diff;
sub get_Vth;
sub add_subckts;
sub get_leakage;
sub get_gate_leakage;
sub get_buffer_sc;
sub transistors;
sub components;
sub muxes;
sub nmos_leakages;


my $hspice      = "hspice";
my $script_path = ( fileparse( abs_path($0) ) )[1];
my $spice_path  = File::Spec->join( $script_path, "spice" );

my $quick_test = 0;

# Simulation Time
my $simt = "5n";

# P/N Ratio
my $pn_interval           = 0.1;
#my $pn_search_lower_bound = 0.5;
my $pn_search_lower_bound = 1; # For TSMC 40nm
my $pn_search_upper_bound = 4.0;

# Max width of a transistor
my $max_w_per_trans = 5; # Forced by TSMC 40nm, can be adpated to other technology

# Transistor Capacitances
my $max_size      = 500;
#my $max_size      = 5; # To Fit TSMC 40nm
my $size_interval = 1.05;

# NMOS Pass transistor sizes
my @nmos_pass_sizes;
my $nmos_pass_interval = 1.25;
#my $nmos_pass_max_size = 25;
my $nmos_pass_max_size = 5; # To Fit TSMC40nm

# Multiplexer Voltages
my $max_mux_size  = 30;
my $vin_intervals = 10;

# Vds Leakage
my $vds_intervals = 25;

if ($quick_test) {
	$pn_interval           = 1.0;
	$pn_search_lower_bound = 1.0;
	$pn_search_upper_bound = 3.0;
	$max_size              = 10;
	#$max_size              = 5; # To fit TSMC 40nm
	$size_interval         = 1.5;
	$max_mux_size          = 4;
	$vin_intervals         = 3;
	$vds_intervals         = 5;
	$nmos_pass_interval    = 2.0;
}

#my $max_buffer_size = 100;
#my $buffer_interval = 1.5;
#my $mux_interval    = 4;

my $number_arguments = @ARGV;
if ( $number_arguments < 4 ) {
	print(
		"usage: generate_cmos_tech_data.pl <tech_file> <tech_size> <Vdd> <temp> <lib_type>\n"
	);
	exit(-1);
}

my $tech_file      = abs_path( shift(@ARGV) );
#my $tech_file      = shift(@ARGV);
my $tech_size      = shift(@ARGV);
my $Vdd            = shift(@ARGV);
my $temp           = shift(@ARGV);
my $tech_file_name = basename($tech_file);
my $lib_type = shift(@ARGV);
#my $tech_file_name = $tech_file;
my $cwd = getcwd();

#-r $tech_file or die "Cannot open tech file ($tech_file)";

my $C_g;
my $C_s;
my $C_d;

my @sizes;
my @long_size        = ( 1,      2 );
my @transistor_types = ( "nmos", "pmos" );

#my $optimal_p_to_n = get_pn_ratio();
#print  "Optimal P/N ratio determined as $optimal_p_to_n.\n";
my $optimal_p_to_n = 2;


my $size = 1.0;
while ( $size < $max_size ) {
	my $size_rounded = ( sprintf "%.2f", $size );
	push( @sizes, [ $size_rounded, 1 ] );
	$size = $size * $size_interval;
}

$size = 1.0;
while ( $size < $nmos_pass_max_size ) {
	my $size_rounded = ( sprintf "%.2f", $size );
	push( @nmos_pass_sizes, $size_rounded );
	$size = $size * $nmos_pass_interval;
}

#print join(", ", @nmos_pass_sizes);

print "<technology file=\"$tech_file_name\" size=\"$tech_size\">\n";

print "\t<operating_point temperature=\"$temp\" Vdd=\"$Vdd\"/>\n";
print "\t<p_to_n ratio=\"" . $optimal_p_to_n . "\"/>\n";

#print "Running Paramter Extraction of Transistors...\n";
transistors();
#print "Running Output Voltage of Multiplexers...\n";
muxes();
#print "Running Leakage...\n";
nmos_leakages();
#print "Running Components...\n";
components();

print "</technology>\n";

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
  } elsif ($unit =~ m/\d$/i) {
    $coeff = 1;
  }
  # Quick check, there should be only numbers in remaining
  if (!($unit =~ m/\d$/)) {
    die "Error: (process_unit) Invalid number($unit)!\n";
  }  

  return $ret = $unit*$coeff;
}

sub transistors() {
	foreach my $type (@transistor_types) {

		#my $Vth = get_Vth();

		print "\t<transistor type=\"$type\">\n";

		( $C_g, $C_s, $C_d ) =
		  get_capacitances( $type, $long_size[0], $long_size[1] );
		print "\t\t<long_size W=\""
		  . $long_size[0]
		  . "\" L=\""
		  . $long_size[1] . "\">\n";
		print "\t\t\t<leakage_current subthreshold=\""
		  . get_leakage_long( $type, $long_size[0], $long_size[1] )
		  . "\"/>\n";
		print "\t\t\t<capacitance C_g=\"$C_g\" C_s=\"$C_s\" C_d=\"$C_d\"/>\n";
		print "\t\t</long_size>\n";

		foreach my $size_ref (@sizes) {
			my @size = @$size_ref;
			( $C_g, $C_s, $C_d ) =
			  get_capacitances( $type, $size[0], $size[1] );
			print "\t\t<size W=\"" . $size[0] . "\" L=\"" . $size[1] . "\">\n";
			print "\t\t\t<leakage_current subthreshold=\""
			  . get_leakage( $type, $size[0], $optimal_p_to_n )
			  . "\" gate=\""
			  . get_gate_leakage( $type, $size[0], $optimal_p_to_n )
			  . "\"/>\n";
			print
			  "\t\t\t<capacitance C_g=\"$C_g\" C_s=\"$C_s\" C_d=\"$C_d\"/>\n";
			print "\t\t</size>\n";
		}
		print "\t</transistor>\n";
	}
}

sub muxes() {
	print "\t<multiplexers>\n";
	foreach my $size (@nmos_pass_sizes) {
		print "\t\t<nmos size=\"" . $size . "\">\n";
		for ( my $i = 1 ; $i <= $max_mux_size ; $i++ ) {
			print "\t\t\t<multiplexer size=\"" . $i . "\">\n";
			for ( my $j = 0 ; $j <= $vin_intervals ; $j++ ) {
				my $Vin = $Vdd * ( 0.5 + 0.5 * ( $j / $vin_intervals ) );
				my ( $min, $max ) = get_mux_out_voltage( $size, $i, $Vin, $optimal_p_to_n );
				print "\t\t\t\t<voltages in=\"" . $Vin
				  . "\" out_min=\""
				  . $min
				  . "\" out_max=\""
				  . $max
				  . "\"/>\n";
			}
			print "\t\t\t</multiplexer>\n";
		}
		print "\t\t</nmos>\n";
	}
	print "\t</multiplexers>\n";
}

sub nmos_leakages() {
	print "\t<nmos_leakages>\n";
	foreach my $size (@nmos_pass_sizes) {
		print "\t\t<nmos size=\"" . $size . "\">\n";
		for ( my $i = 0 ; $i <= $vds_intervals ; $i++ ) {
			my $Vds = $Vdd * ( 0.5 + 0.5 * ( $i / $vds_intervals ) );
			my $leakage_current = get_nmos_leakage_for_Vds( $size, $Vds );
			print "\t\t\t<nmos_leakage Vds=\"" . $Vds
			  . "\" Ids=\""
			  . $leakage_current
			  . "\"/>\n";
		}
		print "\t\t</nmos>\n";
	}
	print "\t </nmos_leakages>\n";
}

sub components() {
	my $spice_script =
	  File::Spec->join( $script_path, "spice", "run_spice.py" );
	my $temp_file =
	  File::Spec->join( $script_path, "spice", "temp", "temp.txt" );

	my $tech_size_nm = $tech_size * 1e9;

	# 	Component_name,	@Inputs, @Sizes, type
	my @components = (
		[ "buf",      [1], [ 1, 4, 16, 64 ], 0 ],
		[ "buf_levr", [1], [ 1, 4, 16, 64 ], 0 ],
		[ "mux", [ 4, 9, 16, 25 ], \@nmos_pass_sizes, 1 ],
		[ "lut", [ 2, 4, 6 ], \@nmos_pass_sizes, 1 ],
		[ "dff", [1], [ 1, 2, 4, 8 ], 0 ]
		#[ "dff", [1], [ 1 ], 0 ]
	);

	print "\t<components>\n";

	foreach my $component_ref (@components) {
		my @component = @$component_ref;

		my $component_name = @component[0];
		my $inputs         = @component[1];
		my $sizes          = @component[2];
		my $type           = @component[3];

		print "\t\t<$component_name>\n";

		foreach my $num_inputs (@$inputs) {
			print "\t\t\t<inputs num_inputs=\"" . $num_inputs . "\">\n";
			foreach my $size (@$sizes) {
				my $cmd;
				if ( $type == 0 ) {
					$cmd =
					  "python $spice_script $tech_file $tech_size_nm $lib_type $Vdd $optimal_p_to_n $temp h $component_name $size ";
				}
				else {
					$cmd =
					  "python $spice_script $tech_file $tech_size_nm $lib_type $Vdd $optimal_p_to_n $temp h $component_name $num_inputs $size";
				}

				#				print $cmd;
				my $result = `$cmd`;
				chomp($result);
                $result = &process_unit($result, "power");
				print "\t\t\t\t<size transistor_size=\"" . $size
				  . "\" power=\""
				  . $result
				  . "\"/>\n";
			}
			print "\t\t\t</inputs>\n";
		}
		print "\t\t</$component_name>\n";
	}

	print "\t</components>\n";
}

sub get_pn_ratio {
	my @sizes;
	my $opt_size;
	my $opt_percent;

	for (
		my $size = $pn_search_lower_bound ;
		$size < $pn_search_upper_bound ;
		$size += $pn_interval
	  )
	{
		push( @sizes, $size );
	}

	$opt_percent = 10000;    # large number
	foreach my $size (@sizes) {
		my $diff = get_riset_fallt_diff($size);

		#print "$size $diff\n";
		if ( $diff < $opt_percent ) {
			$opt_size    = $size;
			$opt_percent = $diff;
		}
	}

	return $opt_size;
}

sub get_nmos_leakage_for_Vds {
	my $size = shift(@_);
	my $Vds  = shift(@_);

	my $s = spice_header();
	$s = $s . "Vin in 0 " . $Vds . "\n";
    $s = $s . spice_gen_one_fet_line("X0", "in 0 0 0", "nfet", $size);
	$s = $s . spice_sim(10);
	$s = $s . ".measure tran leakage avg I(Vin)\n";
	$s = $s . spice_end();

	my @results = spice_run( $s, ["leakage"] );
    $results[0] = &process_unit($results[0], "current");
	return $results[0];
}

sub get_mux_out_voltage_min {
	my $s = spice_header();

	$s = $s . "X0 Vdd Vdd out 0 nfet size='1'\n";

	for ( my $i = 1 ; $i < $max_mux_size ; $i++ ) {
		$s = $s . "X" . $i . " 0 0 out 0 nfet size='1'\n";
	}

	$s = $s . spice_sim(10);

	$s = $s . ".measure tran vout avg V(out)\n";

	$s = $s . spice_end();

	my @results = spice_run( $s, ["vout"] );

    for (my $i=0; $i < ($#results + 1); $i++) {
      $results[$i] = &process_unit($results[$i], "capacitance");
    }

	return $results[0];
}

sub get_mux_out_voltage {
	my $size     = shift(@_);
	my $mux_size = shift(@_);
	my $Vin      = shift(@_);
	my $pn      = shift(@_);

	my $s = spice_header();

	$s = $s . "Vin in 0 " . $Vin . "\n";
    # Xifan: Support transmission gates 
    # NMOS 
    $s = $s . spice_gen_one_fet_line("X0a", "in Vdd outa 0", "nfet", $size);
    # PMOS 
    $s = $s . spice_gen_one_fet_line("X0a_cpt", "in 0 outa Vdd", "pfet", $size*$pn);


	for ( my $i = 1 ; $i < $mux_size ; $i++ ) {
      # NMOS 
      $s = $s . spice_gen_one_fet_line("X$i"."a", "in 0 outa 0", "nfet", $size);
      # PMOS 
      $s = $s . spice_gen_one_fet_line("X$i"."a_cpt", "in Vdd outa Vdd", "pfet", $size);
	}

    # NMOS 
    $s = $s . spice_gen_one_fet_line("X0b", "in Vdd outb 0", "nfet", $size);
    # PMOS 
    $s = $s . spice_gen_one_fet_line("X0b_cpt", "in 0 outb Vdd", "nfet", $size);

	for ( my $i = 1 ; $i < $mux_size ; $i++ ) {
      # NMOS 
      $s = $s. spice_gen_one_fet_line("X$i"."b", "0 0 outb 0", "nfet", $size);
      # PMOS 
      $s = $s. spice_gen_one_fet_line("X$i"."b_cpt", "0 Vdd outb Vdd", "nfet", $size);
	}


	$s = $s . spice_sim(10);
	$s = $s . ".measure tran vout_min avg V(outb)\n";
	$s = $s . ".measure tran vout_max avg V(outa)\n";
	$s = $s . spice_end();
	my @results = spice_run( $s, [ "vout_min", "vout_max" ] );

    for (my $i=0; $i < ($#results + 1); $i++) {
      $results[$i] = &process_unit($results[$i], "voltage");
    }

	return @results;
}

sub calc_buffer_stage_effort {
	my $N = shift(@_);
	my $S = shift(@_);

	if ( $N > 1 ) {
		return $S**( 1.0 / ( $N - 1 ) );
	}
}

sub calc_buffer_num_stages {
	my $S = shift(@_);

	if ( $S <= 1.0 ) {
		return 1;
	}
	elsif ( $S <= 4.0 ) {
		return 2;
	}
	else {
		my $N = int( log($S) / log(4.0) + 1 );
		if (
			abs( calc_buffer_stage_effort( $N + 1, $S ) - 4 ) <
			abs( calc_buffer_stage_effort( $N, $S ) - 4 ) )
		{
			$N = $N + 1;
		}
		return $N;
	}
}

#sub print_buffer_sc {
#	my $pn_ratio = shift(@_);
#
#	print "\t<buffer_sc>\n";
#
#	for ( my $N = 1.0 ; $N <= 5 ; $N = $N + 1 ) {
#
#		print "\t\t<stages num_stages='" . $N . "'>\n";
#
#		for ( my $g = 1 ; $g <= 6 ; $g = $g + 1 ) {
#
#			my $sc_nolevr = get_buffer_sc( $N, $g, 0, 0, $pn_ratio );
#
#			print "\t\t\t<strength gain='" . $g
#			  . "' sc_nolevr='"
#			  . $sc_nolevr . "'>\n";
#
#			for (
#				my $i = 1 ;
#				$i <= $max_mux_size + $mux_interval ;
#				$i = $i + $mux_interval
#			  )
#			{
#				my $sc_levr = get_buffer_sc( $N, $g, 1, $i, $pn_ratio );
#				print "\t\t\t\t<input_cap mux_size='" . $i
#				  . "' sc_levr='"
#				  . $sc_levr . "'/>\n";
#			}
#
#			print "\t\t\t</strength>\n";
#
#			if ( $N == 1 ) {
#				last;
#			}
#		}
#
#		print "\t\t</stages>\n";
#	}
#
#	print "\t</buffer_sc>\n";
#}

#sub get_buffer_sc {
#	my $N              = shift(@_);
#	my $g              = shift(@_);
#	my $level_restorer = shift(@_);
#	my $mux_in_size    = shift(@_);
#	my $pn_ratio       = shift(@_);
#
#	my $s = "";
#
#	$s = spice_header();
#
#	# Voltage Sources per stage
#	for ( my $i = 0 ; $i < $N ; $i = $i + 1 ) {
#		$s = $s . "Vup" . $i . " Vdd VupL" . $i . " 0\n";
#		$s = $s . "Vdown" . $i . " VdownH" . $i . " 0 0\n";
#		$s = $s . "Vgate" . $i . " VgateH" . $i . " out" . ( $i + 1 ) . " 0\n";
#		$s =
#		    $s . "VgP"
#		  . ( $i + 1 ) . " out"
#		  . ( $i + 1 ) . " VgLP"
#		  . ( $i + 1 ) . " 0\n";
#		$s =
#		    $s . "VgN"
#		  . ( $i + 1 ) . " out"
#		  . ( $i + 1 ) . " VgLN"
#		  . ( $i + 1 ) . " 0\n";
#	}
#
#	# Input Pulse
#	$s = $s
#	  . "Vin in 0 PWL (0 0 'simt/4' 0 'simt/4+rise' Vol '3*simt/4' Vol '3*simt/4+rise' 0)\n";
#
#	if ($level_restorer) {
#		$s = $s . "Xmux0pre in Vdd inA 0 nfet size='1'\n";
#		$s = $s . "Xmux0 inA Vdd out0 0 nfet size='1'\n";
#		for ( my $i = 1 ; $i < $mux_in_size ; $i = $i + 1 ) {
#			if ( $i % 2 == 0 ) {
#				$s = $s . "Xmux" . $i . " 0 0 out0 0 nfet size='1'\n";
#			}
#			else {
#				$s = $s . "Xmux" . $i . " Vdd 0 out0 0 nfet size='1'\n";
#			}
#
#		}
#		$s = $s . "X0 out0 VgateH0 VupL0 VdownH0 levr\n";
#	}
#	else {
#		$s = $s . "XinA in inA Vdd 0 inv nsize='1' psize='" . $pn_ratio . "'\n";
#		$s =
#		  $s . "XinB inA out0 Vdd 0 inv nsize='1' psize='" . $pn_ratio . "'\n";
#		$s = $s
#		  . "X0 out0 VgateH0 VupL0 VdownH0 inv nsize='1' psize='"
#		  . $pn_ratio . "'\n";
#	}
#
#	my $stage_size = $g;
#	for ( my $i = 1 ; $i < $N ; $i = $i + 1 ) {
#
#		$s =
#		    $s . "X" . $i . " VgLP" . $i . " VgLN" . $i
#		  . " VgateH"
#		  . $i . " VupL"
#		  . $i
#		  . " VdownH"
#		  . $i
#		  . " invd nsize='"
#		  . $stage_size
#		  . "' psize='"
#		  . ( $stage_size * $pn_ratio ) . "'\n";
#		$stage_size = $stage_size * $g;
#	}
#
#	$s = $s . spice_sim(10000);
#
#	my $up = 1;
#	for ( my $i = 0 ; $i < $N ; $i = $i + 1 ) {
#		if ($up) {
#			$s = $s
#			  . ".measure rs"
#			  . $i
#			  . " when V(out"
#			  . $i
#			  . ")='0.1*Vol' CROSS=1\n";
#			$s = $s
#			  . ".measure re"
#			  . $i
#			  . " when V(out"
#			  . ( $i + 1 )
#			  . ")='0.1*Vol' CROSS=1\n";
#			$s = $s
#			  . ".measure fs"
#			  . $i
#			  . " when V(out"
#			  . $i
#			  . ")='0.9*Vol' CROSS=2\n";
#			$s = $s
#			  . ".measure fe"
#			  . $i
#			  . " when V(out"
#			  . ( $i + 1 )
#			  . ")='0.9*Vol' CROSS=2\n";
#		}
#		else {
#			$s = $s
#			  . ".measure fs"
#			  . $i
#			  . " when V(out"
#			  . $i
#			  . ")='0.9*Vol' CROSS=1\n";
#			$s = $s
#			  . ".measure fe"
#			  . $i
#			  . " when V(out"
#			  . ( $i + 1 )
#			  . ")='0.9*Vol' CROSS=1\n";
#			$s = $s
#			  . ".measure rs"
#			  . $i
#			  . " when V(out"
#			  . $i
#			  . ")='0.1*Vol' CROSS=2\n";
#			$s = $s
#			  . ".measure re"
#			  . $i
#			  . " when V(out"
#			  . ( $i + 1 )
#			  . ")='0.1*Vol' CROSS=2\n";
#		}
#		$s = $s . ".measure rd" . $i . " param=('re" . $i . "-rs" . $i . "')\n";
#		$s = $s . ".measure fd" . $i . " param=('fe" . $i . "-fs" . $i . "')\n";
#		$up = !$up;
#
#		$s = $s
#		  . ".measure tran ITr"
#		  . $i
#		  . " integ Par('0.5*(I(Vdown"
#		  . $i
#		  . ") + abs(I(Vdown"
#		  . $i
#		  . ")))') FROM '(rs"
#		  . $i
#		  . " - 0.25 * rd"
#		  . $i
#		  . ")' TO '(re"
#		  . $i
#		  . " + 0.25 * rd"
#		  . $i . ")'\n";
#		$s = $s
#		  . ".measure tran ISCr"
#		  . $i
#		  . " integ Par('0.5*(I(Vup"
#		  . $i
#		  . ") + abs(I(Vup"
#		  . $i
#		  . ")))') FROM '(rs"
#		  . $i
#		  . " - 0.25 * rd"
#		  . $i
#		  . ")' TO '(re"
#		  . $i
#		  . " + 0.25 * rd"
#		  . $i . ")'\n";
#
#		#$s = $s . ".measure tran IGr" . $i . " integ I(Vgate" . $i . ") FROM 'rs" . $i . "' TO 're" . $i . "'\n";
#		#$s = $s . ".measure tran IPr" . $i . " integ I(VgP" . ($i+1) . ") FROM 'rs" . $i . "' TO 're" . $i . "'\n";
#		#$s = $s . ".measure tran INr" . $i . " integ I(VgN" . ($i+1) . ") FROM 'rs" . $i . "' TO 're" . $i . "'\n";
#
#		$s = $s
#		  . ".measure tran ITf"
#		  . $i
#		  . " integ Par('0.5*(I(Vup"
#		  . $i
#		  . ") + abs(I(Vup"
#		  . $i
#		  . ")))') FROM '(fs"
#		  . $i
#		  . " - 0.25 * fd"
#		  . $i
#		  . ")' TO '(fe"
#		  . $i
#		  . " + 0.25 * fd"
#		  . $i . ")'\n";
#		$s = $s
#		  . ".measure tran ISCf"
#		  . $i
#		  . " integ Par('0.5*(I(Vdown"
#		  . $i
#		  . ") + abs(I(Vdown"
#		  . $i
#		  . ")))') FROM '(fs"
#		  . $i
#		  . " - 0.25 * fd"
#		  . $i
#		  . ")' TO '(fe"
#		  . $i
#		  . " + 0.25 * fd"
#		  . $i . ")'\n";
#
#		#$s = $s . ".measure tran IGf" . $i . " integ I(Vgate" . $i . ") FROM 'fs" . $i . "' TO 'fe" . $i . "'\n";
#		#$s = $s . ".measure tran IPf" . $i . " integ I(VgP" . ($i+1) . ") FROM 'fs" . $i . "' TO 'fe" . $i . "'\n";
#		#$s = $s . ".measure tran INf" . $i . " integ I(VgN" . ($i+1) . ") FROM 'fs" . $i . "' TO 'fe" . $i . "'\n";
#
#		$s = $s
#		  . ".measure tran fSC"
#		  . $i
#		  . " Param=('(ISCr"
#		  . $i . "+ISCf"
#		  . $i
#		  . ")/(ITr"
#		  . $i
#		  . " + ITf"
#		  . $i
#		  . " - ISCr"
#		  . $i
#		  . " - ISCf"
#		  . $i . ")')\n";
#	}
#
#	my $SCp;
#	my $SCm = "";
#	my $T;
#	for ( my $i = 0 ; $i < $N ; $i = $i + 1 ) {
#
#		if ( $i == 0 ) {
#			$SCp = "ISCr0 + ISCf0";
#			$T   = "ITr0 + ITf0";
#		}
#		else {
#			$SCp = $SCp . " + ISCr" . $i . " + ISCf" . $i;
#			$T   = $T . " + ITr" . $i . " + ITf" . $i;
#		}
#		$SCm = $SCm . " - ISCr" . $i . " - ISCf" . $i;
#	}
#	$s = $s
#	  . ".measure tran fSC Param=('("
#	  . $SCp
#	  . ")/(0.5*("
#	  . $T
#	  . $SCm
#	  . "))')\n";
#
#	$s = $s . spice_end();
#
#	my @results = spice_run( $s, [ "fsc", "fsc0" ] );
#
#	return $results[0];
#}

sub get_riset_fallt_diff {
	my $size = shift(@_);

	my $s = "";
	my $rise;
	my $fall;

	$s = spice_header();

	# Input Voltage
	$s = $s . "Vin in 0 PULSE(0 Vol 'simt/4' 'rise' 'fall' 'simt/2' 'simt')\n";

	# Inverter
	$s = $s . "X0 in out Vdd 0 inv nsize='2.0' psize='2.0*" . $size . "'\n";

	$s = $s . spice_sim(1000);

	$s = $s
	  . ".measure tran fallt TRIG V(in) VAL = '0.5*Vol' TD = 0 RISE = 1 TARG V(out) VAL = '0.5*Vol' FALL = 1\n";
	$s = $s
	  . ".measure tran riset TRIG V(in) VAL = '0.5*Vol' TD = 'simt/2' FALL = 1 TARG V(out) VAL = '0.5*Vol' RISE = 1\n";

	$s = $s . spice_end();

	my @results = spice_run( $s, [ "fallt", "riset" ] );

    for (my $i=0; $i < ($#results + 1); $i++) {
      $results[$i] = &process_unit($results[$i], "time");
    }

	return abs( $results[0] - $results[1] ) / $results[1];
}

sub spice_header {
	my $s = "";

	$s = $s . "Automated spice simuation: " . localtime() . "\n";
    $s = $s . ".global Vdd gnd\n";
    if (($tech_file =~ m/\.lib$/) or ($tech_file =~ m/\.l$/)) {
	  $s = $s . ".lib \'$tech_file\' $lib_type\n";
    } else {
	  $s = $s . ".include \'$tech_file\'\n";
    }
	$s = $s . ".param tech = $tech_size\n";
	$s = $s . ".param Vol = $Vdd\n";

	$s = $s . ".param simt = " . $simt . "\n";
	$s = $s . ".param rise = 'simt/500'\n";
	$s = $s . ".param fall = 'simt/500'\n";

	$s = $s . ".include \'${spice_path}/subckt/nmos_pmos.sp\'\n";
	$s = $s . ".include \'${spice_path}/subckt/inv.sp\'\n";
	$s = $s . ".include \'${spice_path}/subckt/level_restorer.sp\'\n";

	$s = $s . "Vdd Vdd 0 'Vol'\n";

	return $s;
}

# Xifan Tang: a sub function to format all the lines that dumping a FET 
sub spice_gen_one_fet_line($ $ $ $ $) {
  my ($fet_prefix, $fet_port_map, $fet_name, $fet_width, $fet_chanl) = @_;   
  my ($cur_fet_w);

  my $s = "";

  if ($fet_width <= $max_w_per_trans) {
    my ($j, $res_fet_w) = (int($fet_width / $max_w_per_trans) + 1, $fet_width % $max_w_per_trans);
    $s = $s . "$fet_prefix\_0 $fet_port_map $fet_name size='$fet_width'\n";
  } else {
    for (my $i = 0; $i < ceil($fet_width / $max_w_per_trans); $i++) { 
      $cur_fet_w = $fet_width / (ceil($fet_width / $max_w_per_trans));
      $s = $s . "$fet_prefix\_$i $fet_port_map $fet_name size='$cur_fet_w'\n";
    }
  }

  return $s;
}

# Xifan Tang: a sub function to format all the lines that dumping a FET 
sub spice_gen_one_fetz_line($ $ $ $ $) {
  my ($fet_prefix, $fet_port_map, $fet_name, $fet_width, $fet_chanl) = @_;   
  my ($cur_fet_w);

  my $s = "";

  if ( $fet_width <= $max_w_per_trans) {
    $s = $s . "$fet_prefix\_0 $fet_port_map $fet_name wsize='$fet_width' lsize='$fet_chanl'\n";
  } else {
    for (my $i = 0; $i < ceil($fet_width / $max_w_per_trans); $i++) { 
      $cur_fet_w = $fet_width / (ceil($fet_width / $max_w_per_trans));
      $s = $s . "$fet_prefix\_$i $fet_port_map $fet_name wsize='$cur_fet_w' lsize='$fet_chanl'\n";
    }
  }

  return $s;
}

# Xifan Tang: a sub function to format all the lines that dumping a FET 
sub spice_gen_one_inv_line($ $ $ $ $) {
  my ($inv_prefix, $inv_port_map, $inv_name, $nfet_w, $pn) = @_;   
  my ($cur_nfet_w, $cur_pfet_w);

  my $s = "";

  if ($nfet_w * $pn <= $max_w_per_trans) {
    $cur_nfet_w = $nfet_w;
    $cur_pfet_w = $nfet_w * $pn;
    $s = $s . "$inv_prefix\_0 $inv_port_map $inv_name nsize='$cur_nfet_w' psize='$cur_pfet_w'\n";
  } else {
    for (my $i = 0; $i < ceil($nfet_w * $pn / $max_w_per_trans); $i++) { 
      $cur_nfet_w = $nfet_w / (ceil($nfet_w * $pn / $max_w_per_trans));
      $cur_pfet_w = $cur_nfet_w * $pn;
      $s = $s . "$inv_prefix\_$i $inv_port_map $inv_name nsize='$cur_nfet_w' psize='$cur_pfet_w'\n";
    }
  }

  return $s;
}

sub spice_sim {
	my $accuracy = shift(@_);
	my $s        = "";

	$s = $s . ".TEMP $temp\n";
	$s = $s . ".OP\n";
	$s = $s . ".OPTIONS LIST NODE POST CAPTAB\n";

	$s = $s . ".tran 'simt/" . $accuracy . "' simt\n";
	return $s;
}

sub spice_end {
	my $s = "";

	$s = $s . ".end\n\n";

	return $s;
}

sub spice_run {
	my $cmd          = shift(@_);
	my $result_names = shift(@_);

	my @results;

	my $run_dir    = File::Spec->join( $script_path, "spice", "temp" );
	my $spice_file = File::Spec->join( $run_dir,     "vtr_auto_spice.sp" );
	my $spice_out  = File::Spec->join( $run_dir,     "vtr_auto_spice.lis" );
	#my $spice_out  = File::Spec->join( $run_dir,     "vtr_auto_spice.lis" );
	#my $spice_file = ("./vtr_auto_spice.sp");
	#my $spice_out  = ("./vtr_auto_spice.lis");

    chdir "$run_dir";

	open( SP_FILE, "> $spice_file" );
	print SP_FILE $cmd;
	close(SP_FILE);

    my ($hdlpath) = "-hdlpath /softs/synopsys/hspice/2010.12/hspice/include";
	#system("cd $run_dir; $hspice $spice_file 1> $spice_out 2 $hdlpath > /dev/null");
	#system("$hspice $spice_file 1> $spice_out 2 > /dev/null");
	#system("cd $run_dir");
    #`source /softs/synopsys/hspice/2010.12/hspice/bin/cshrc.meta`;
    #`csh -cx "hspice -i $spice_file -o $spice_out $hdlpath"`;
    `/bin/csh -cx '$hspice -i $spice_file -o $spice_out'`;

	my $spice_data;
	{
		local $/ = undef;
		open( SP_FILE, "$spice_out" );
		$spice_data = <SP_FILE>;
        #print $spice_data;
		close SP_FILE;
	}

	foreach my $result_name ( @{$result_names} ) {
		if ( $spice_data =~
			/transient analysis.*?$result_name\s*=\s*[+-]*(\S+)/s )
		{
			push( @results, $1 );
            #print "DEBUG: Xifan TANG, $result_name = $1\n";
		}
		else {
			die "Could not find $result_name in spice output($spice_out).\n";
		}
	}

    chdir $cwd;
	return @results;
}

sub get_leakage_long {
	my $type = shift(@_);
	my $w    = shift(@_);
	my $l    = shift(@_);

	my $s = spice_header();

	$s = $s . "Vleak Vleak 0 Vol\n";
	if ( $type eq "nmos" ) {
        $s = $s. spice_gen_one_fetz_line("X0", "Vleak 0 0 0", "nfetz", $w, $l);
	}
	else {
        $s = $s. spice_gen_one_fetz_line("X0", "0 Vdd Vleak Vdd", "pfetz", $w, $l);
	}

	$s = $s . spice_sim(100);
	$s = $s . ".measure tran leakage avg I(Vleak)\n";
	$s = $s . spice_end();
	my @results = spice_run( $s, ["leakage"] );

	return &process_unit($results[0], "power");
}

sub get_gate_leakage {
	my $type = shift(@_);
	my $size = shift(@_);
	my $pn   = shift(@_);

	my $s = spice_header();

	if ( $type eq "nmos" ) {
		$s = $s . "Vleak Vdd VleakL 0\n";
        $s = $s . spice_gen_one_inv_line("X0", "VleakL out Vdd 0", "inv", $size, $pn);
	}
	else {
		$s = $s . "Vleak VleakH 0 0\n";
        $s = $s . spice_gen_one_inv_line("X0", "VleakH out Vdd 0", "inv", $size, $pn);
	}

	$s = $s . spice_sim(100);
	$s = $s . ".measure tran leakage avg I(Vleak)\n";
	$s = $s . spice_end();
	my @results = spice_run( $s, ["leakage"] );

    $results[0] = &process_unit($results[0], "power");
	if ( $type eq "nmos" ) {
    } else {
      $results[0] = $results[0] / $pn;
    }

	return $results[0];

}

sub get_leakage {
	my $type = shift(@_);
	my $size = shift(@_);
	my $pn   = shift(@_);

	my $s = spice_header();

	if ( $type eq "nmos" ) {
		$s = $s . "Vleak Vleak 0 0\n";
        # Xifan : adapt to TSMC 40nm lib
        $s =  $s. spice_gen_one_inv_line("X0", "0 out Vdd Vleak", "inv", $size, $pn);
	}
	else {
		$s = $s . "Vleak Vdd VleakL 0\n";
        # Xifan : adapt to TSMC 40nm lib
        $s =  $s. spice_gen_one_inv_line("X0", "Vdd out VleakL 0", "inv", $size, $pn);
	}

	$s = $s . spice_sim(100);
	$s = $s . ".measure tran leakage avg I(Vleak)\n";
	$s = $s . spice_end();

	my @results = spice_run( $s, ["leakage"] );

    $results[0] = &process_unit($results[0], "power");
	if ( $type eq "nmos" ) {
	} else {
      $results[0] = $results[0] / $pn;
    }

	return $results[0];

}

sub get_capacitances {
	my $type   = shift(@_);
	my $width  = shift(@_);
	my $length = shift(@_);

	my $s = "";
	my $C_g;
	my $C_s;
	my $C_d;

	$s = spice_header();
	$s = $s . ".param tick = 'simt/6'\n";

	# Gate, Drain, Source Inputs
	# Time	NMOS			PMOS
	# 		G 	D 	S		G 	D 	S
	# 0		0 	0 	0		0 	0 	0
	# 1		0 	0 	1     	0 	1 	1
	# 2		0 	1 	0     	1 	0 	0
	# 3		0 	1 	1     	1 	0 	1
	# 4		1 	0 	0     	1 	1 	0
	# 5		1 	1 	1	    1 	1 	1

	$s = $s . ".param t0s = '0*tick+2*rise'\n";
	$s = $s . ".param t0e = '1*tick-rise'\n";
	$s = $s . ".param t1s = '1*tick+2*rise'\n";
	$s = $s . ".param t1e = '2*tick-rise'\n";
	$s = $s . ".param t2s = '2*tick+2*rise'\n";
	$s = $s . ".param t2e = '3*tick-rise'\n";
	$s = $s . ".param t3s = '3*tick+2*rise'\n";
	$s = $s . ".param t3e = '4*tick-rise'\n";
	$s = $s . ".param t4s = '4*tick+2*rise'\n";
	$s = $s . ".param t4e = '5*tick-rise'\n";
	$s = $s . ".param t5s = '5*tick+2*rise'\n";
	$s = $s . ".param t5e = '6*tick-rise'\n";

	if ( $type eq "nmos" ) {
		$s = $s . "Vgate gate 0 PWL(	0 0 	'4*tick' 0 '4*tick+rise' Vol)\n";
		$s = $s
		  . "Vdrain drain 0 PWL(	0 0 	'2*tick' 0 '2*tick+rise' Vol 	'4*tick' Vol '4*tick+rise' 0 	'5*tick' 0 '5*tick+rise' Vol)\n";
		$s = $s
		  . "Vsource source 0 PWL(0 0 	'1*tick' 0 '1*tick+rise' Vol 	'2*tick' Vol '2*tick+rise' 0 	'3*tick' 0 '3*tick+rise' Vol 	'4*tick' Vol '4*tick+rise' 0 	'5*tick' 0 '5*tick+rise' Vol)\n";

        # Split the transistors if their size exceed the limit per bin 
        $s = $s. spice_gen_one_fetz_line("X1", "drain gate source 0", "nfetz", $size, $length);
	}
	else {
		$s = $s . "Vgate gate 0 PWL(	0 0 	'2*tick' 0 '2*tick+rise' Vol)\n";
		$s = $s
		  . "Vdrain drain 0 PWL(	0 0 	'1*tick' 0 '1*tick+rise' Vol 	'2*tick' Vol '2*tick+rise' 0 	'4*tick' 0 '4*tick+rise' Vol)\n";
		$s = $s
		  . "Vsource source 0 PWL(0 0 	'1*tick' 0 '1*tick+rise' Vol 	'2*tick' Vol '2*tick+rise' 0 	'3*tick' 0 '3*tick+rise' Vol 	'4*tick' Vol '4*tick+rise' 0 	'5*tick' 0 '5*tick+rise' Vol)\n";

        # Split the transistors if their size exceed the limit per bin 
        $s = $s. spice_gen_one_fetz_line("X1", "drain gate source 0", "pfetz", $size, $length);
	}

	$s = $s . spice_sim(100);

	#$s = $s . ".print tran cap(gate)\n";
	#$s = $s . ".print tran cap(drain)\n";
	#$s = $s . ".print tran cap(source)\n";

	if ( $type eq "nmos" ) {
		$s = $s . ".measure tran c_g avg cap(gate) FROM = t4s TO = t5e\n";
		$s = $s . ".measure tran c_d1 avg cap(drain) FROM = t2s TO = t3e\n";
		$s = $s . ".measure tran c_d2 avg cap(drain) FROM = t5s TO = t5e\n";
		$s = $s . ".measure tran c_d Param=('(2*c_d1 + c_d2)/3')\n";
		$s = $s . ".measure tran c_s1 avg cap(source) FROM = t1s TO = t1e\n";
		$s = $s . ".measure tran c_s2 avg cap(source) FROM = t3s TO = t3e\n";
		$s = $s . ".measure tran c_s3 avg cap(source) FROM = t5s TO = t5e\n";
		$s = $s . ".measure tran c_s Param=('(c_s1 + c_s2 + c_s3)/3')\n";
	}
	else {
		$s = $s . ".measure tran c_g avg cap(gate) FROM = t2s TO = t5e\n";
		$s = $s . ".measure tran c_d1 avg cap(drain) FROM = t1s TO = t1e\n";
		$s = $s . ".measure tran c_d2 avg cap(drain) FROM = t4s TO = t5e\n";
		$s = $s . ".measure tran c_d Param=('(c_d1 + 2*c_d2)/3')\n";
		$s = $s . ".measure tran c_s1 avg cap(source) FROM = t1s TO = t1e\n";
		$s = $s . ".measure tran c_s2 avg cap(source) FROM = t3s TO = t3e\n";
		$s = $s . ".measure tran c_s3 avg cap(source) FROM = t5s TO = t5e\n";
		$s = $s . ".measure tran c_s Param=('(c_s1 + c_s2 + c_s3)/3')\n";
	}

	$s = $s . spice_end();
	my @results = spice_run( $s, [ "c_g", "c_s", "c_d" ] );

    for (my $i=0; $i < ($#results + 1); $i++) {
      $results[$i] = &process_unit($results[$i], "capacitance");
    }

	return @results;
}
