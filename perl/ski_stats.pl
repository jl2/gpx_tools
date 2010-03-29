#!/usr/bin/perl

# ski_stats.pl

# Copyright (c) 2010, Robert Tadlock rtadlock@gmail.com and Jeremiah LaRocco jeremiah.larocco@gmail.com

# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.

# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
# ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
# ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
# OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

use XML::Simple;
use Date::Manip;
use IO::Handle;
use Data::Dumper;
use Math::Trig qw(great_circle_distance deg2rad);

# redirecting STDOUT to hide errors from Data::Manip on Linux (it's a hack, I know)
open ERROR, '>', "/dev/null";
STDERR->fdopen( \*ERROR, 'w');

# Constants
# =========================================
my $miles_per_km = 0.621371192;
my $feet_per_meter = 3.2808399;
my $pi = 3.14159265;
my $earth_radius = 6378; # in km

# Some ideas for stats I like to have
# =========================================

# Vertical feet skied
# total distance skied vs. travelled
# time skiing
# time on lift
# number of runs

if ($#ARGV < 0) 
{
    print "No file name given.\n";
    exit 1;
}

my $xml = new XML::Simple;

# Get the resort points
my $resort_data = $xml->XMLin( "north_america.gpx", ForceArray=>1 );
my @north_am_resorts = @{$resort_data->{wpt}};

# Parse out the GPX file
my $data = $xml->XMLin($ARGV[0], ForceArray=>1);

my @trks = @{$data->{trk}};
my ($max_elevation, $min_elevation, $dist, $temp_distance, $temp_speed, $num_runs, $max_speed, $average_speed) = 0;
my (@speed_vals, @points, @start, @finish, @segs);
my $resort_skied = "Unknown";

foreach my $trk (@trks)
{
	push( @segs,@{$trk->{trkseg}});
}

foreach my $seg (@segs)
{
	push( @points, @{$seg->{trkpt}} );	

	$min_elevation = $points[0]->{ele}->[0];
	for (my $i = 0; $i < $#points; $i++)
	{
		if( $resort_skied eq "Unknown" )
		{
			my $last_dist = -1;
			foreach my $res ( @north_am_resorts )
			{
				@start = earth_point( $res->{lon}, $res->{lat} );
				@finish = earth_point( $points[$i]->{lon}, $points[$i]->{lat} );

				$temp_distance = great_circle_distance( @start, @finish, $earth_radius );
				if( $last_dist == -1 || $temp_distance < $last_dist )
				{
					$resort_skied = $res;
					$last_dist = $temp_distance;
				}
			}
		}

		if( $i+1 <= $#points )
		{
			@start = earth_point( $points[$i]->{lon}, $points[$i]->{lat} );
			@finish = earth_point( $points[$i+1]->{lon}, $points[$i+1]->{lat} );

			$temp_distance = great_circle_distance( @start, @finish, $earth_radius );

			# Calculate the speed between points
			$temp_speed = &speed( $points[$i]->{'time'}->[0], $points[$i+1]->{'time'}->[0], $temp_distance );
			push( @speed_vals, $temp_speed );
			$dist += $temp_distance;
		}

		if( $max_elevation < $points[$i]->{ele}->[0] )
		{
			$max_elevation = $points[$i]->{ele}->[0];
		}

		if( $min_elevation > $points[$i]->{ele}->[0] )
		{
			$min_elevation = $points[$i]->{ele}->[0];
		}
	}
	@points = ();
}

# Find the max speed & average speed
foreach my $speed ( @speed_vals )
{
	# Drop speeds >100 mph because they are probably wrong
	# world record ski speed is ~150 mph anyway 
	if( $speed <= 100 )
	{
		$average_speed += $speed;
		if( $max_speed < $speed )
		{
			$max_speed = $speed;
		}
	}
}

$average_speed = $average_speed / $#speed_vals;

# Print out skiing data
print("=============================================\n");
printf( "Resort: %s\n", $resort_skied->{name}->[0] );
printf( "Elevation (base): %s\n", $resort_skied->{ele}->[0] );
printf( "Number of runs: %s\n", $resort_skied->{numruns}->[0] );
printf( "Number of lifts: %s\n", $resort_skied->{numlifts}->[0] );
printf( "Website: %s\n", $resort_skied->{url}->[0] );
print("=============================================\n\n");
printf( "Total distance travelled: %.1f miles\n", $dist * $miles_per_km);
printf( "Max speed: %.2f mph\n", $max_speed );
printf( "Average speed: %.2f mph\n", $average_speed );
printf( "Max elevation: %.1f feet\n", $max_elevation * $feet_per_meter );
printf( "Min elevation: %.1f feet\n", $min_elevation * $feet_per_meter );


# Calc/misc functions
# ==============================================================

# Returns speed in miles/hour
sub speed
{
	my( $start_time, $finish_time, $distance ) = @_;

	# To use Date::Manip, you have to set this on NT
	# machines.  Doesn't really matter what I set it to
	# here because I'm just calculating speed
	Date_Init("TZ=EST5EDT"); 
	my $startDate = ParseDate( $start_time );
	my $endDate = ParseDate( $finish_time );
	my $delta = DateCalc( $startDate, $endDate );

	$delta = Delta_Format( $delta, "exact", "%hd" );
	my $speed = 0;
	if( $delta > 0 )
	{
		$speed = ($distance * $miles_per_km)/$delta;
	}

	return $speed;
}

sub earth_point 
{ 
	return deg2rad($_[0]), deg2rad(90 - $_[1]); 
}
