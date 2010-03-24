#!/usr/bin/perl

# ski_stats.pl

# Copyright (c) 2010, Robert Tadlock rtadlock@gmail.com

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

# redirecting STDOUT to hide errors from Data::Manip on Linux (it's a hack, I know)
#open ERROR, '>', "/dev/null";
#STDERR->fdopen( \*ERROR, 'w');

# constants
my $meters_per_mile = 1609.344;
my $feet_per_meter = 3.2808399;
my $pi = 3.14159265;

if ($#ARGV < 0) {
    print "No file name given.\n";
    exit 1;
}

# Some ideas for stats I like to have
# =========================================

# Vertical feet skied
# total distance skied
# time skiing
# time on lift
# number of runs

# Parse out the GPX file
my $xml = new XML::Simple;
my $data = $xml->XMLin($ARGV[0], ForceArray=>1);

my @trks = @{$data->{trk}};
my @segs;
my @points;

foreach my $trk (@trks)
{
	@segs = @{$trk->{trkseg}};
	foreach my $seg (@segs)
	{
		push( @points, @{$seg->{trkpt}} );	
	}	
}

my ($max_elevation, $min_elevation, $dist, $temp_distance, $temp_speed, $num_runs) = 0;
my @speed_vals;
my $going_down = 0;

$min_elevation = $points[0]->{ele}->[0];
for (my $i = 0; $i < $#points; $i++)
{
	if( $points[$i]->{'time'}->[0] =~ m/1972/ )
	{
		next;
	}

	if( $i+1 <= $#points )
	{
		# calculate the distance
		$temp_distance = &great_circle_distance(&degrees_to_radians($points[$i]->{lat}),
							&degrees_to_radians($points[$i]->{lon}),
							&degrees_to_radians($points[$i+1]->{lat}),
							&degrees_to_radians($points[$i+1]->{lon}));

		# calculate the speed
		$temp_speed = &speed( $points[$i]->{'time'}->[0], $points[$i+1]->{'time'}->[0], $temp_distance );
		push( @speed_vals, $temp_speed );
		$dist += $temp_distance;

		# count runs somehow.  This won't work, but there is definetly an up/down pattern the ski runs
		if( $i > 0 )
		{
			if( $going_down == 0 && $points[$i]->{ele}->[0] < $points[$i-1]->{ele}->[0] )
			{		
				$num_runs++;
				$going_down = 1;
			}
			elsif( $going_down == 1 && $points[$i]->{ele}->[0] > $points[$i+1]->{ele}->[0] )
			{
				$going_down = 0;
			}
		}
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

# Find the max speed & average speed
my ($max_speed, $average_speed) = 0;

foreach my $speed ( @speed_vals )
{
	$average_speed += $speed;
	if( $max_speed < $speed )
	{
		$max_speed = $speed;
	}
}

$average_speed = $average_speed / $#speed_vals;

# Print out skiing data
printf( "Total distance travelled: %.1f miles\n", $dist / $meters_per_mile);
printf( "Number of runs: %d\n", $num_runs );
#printf( "Vertical feet skied: %.1f feet\n", $vert );
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
		$speed = ($distance/$meters_per_mile)/$delta;
	}
	return $speed;
}

# Got this calculation from here - http://www.indo.com/distance/dist.pl
# will probably update this to take elevation into account or use a more
# accurate function
sub great_circle_distance 
{
    my ($lat1,$long1,$lat2,$long2) = @_;

    # approx radius of Earth in meters.  True radius varies from
    # 6357km (polar) to 6378km (equatorial).
    my $earth_radius = 6371010;

    my $dlon = $long2 - $long1;
    my $dlat = $lat2 - $lat1;
    my $a = (sin($dlat / 2)) ** 2 
	    + cos($lat1) * cos($lat2) * (sin($dlon / 2)) ** 2;
    my $d = 2 * atan2(sqrt($a), sqrt(1 - $a));

    return $earth_radius * $d;
}

sub degrees_to_radians 
{
    return $_[0] * $pi / 180.0;
}
