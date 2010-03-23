#!/usr/bin/perl 

# gps_parse.pl

# Copyright (c) 2010, Jeremiah LaRocco jeremiah.larocco@gmail.com

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


use warnings;
use strict;
use XML::Parser;
use Image::Magick;
use POSIX qw(strftime);
use Time::Local 'timegm_nocheck';

use Geo::Coordinates::UTM;

use Data::Dumper;

if (@ARGV!=2) {
    print "This command takes two command line arguments: an input file and an output file:\n\t $ARGV[0] infile outfile\n";
    exit 1;
}

my $curLat;
my $curLon;
my $curEle;
my $curTime;

my $oldLat;
my $oldLon;
my $oldEle;
my $oldTime;

my $totDist = 0;
my $totTime = 0;

my $minLat = 361;
my $maxLat = -1;
my $minLong = 361;
my $maxLong = -361;

$curLat = $curLon = $curEle = $oldEle = -1;

my @points;

my $maxVel = -1;


my $p = XML::Parser->new(Style => 'Subs', Pkg => 'GpsSubs');
$p->parsefile($ARGV[0]);

print "Went a total of $totDist in $totTime seconds\n";
my $avVel = $totDist / $totTime;
print "Average speed: $avVel m/s\n";


sub mapPoint {
    my ($x, $y, $w, $h, $maxX, $minX, $maxY, $minY) = @_;
    my $yp = ($x-$minX)/($maxX-$minX) * ($w-20) + 10;
    my $xp = (1.0-($y-$minY)/($maxY-$minY)) * ($h-20) + 10;
    return ($xp, $yp);
    
}

sub mapColor {
    my ($vel, $min, $max) = @_;
#     print "Vel: ($vel, $min, $max)\n";
    my $cv = $vel/($max-$min);
    return sprintf("#%02x%02x%02x\n", $cv*255,0,0);
}

print "MinLat: $minLat\nMaxLat: $maxLat\nMinLon: $minLong\nMaxLon: $maxLong\n";
print "Max velocity: $maxVel\n";
my $image = Image::Magick->new or die "Could not create image: $!\n";
$image->Set(size=>'1000x1000');
$image->ReadImage('xc:white');

shift @points;
my $prev = $points[0];

for my $var (@points) {
    my @ln = @{$var};
    my $ln = @ln;
    if ($ln == 5) {

        my ($lat, $lon, $elevation, $time, $vel) = @{$var};
        my ($xp, $yp) = mapPoint($lat, $lon, 1000, 1000, $minLat, $maxLat, $minLong, $maxLong);
        
        my $BOX_SIZE = 3;
        my ($x1,$y1, $x2,$y2) = ($xp-$BOX_SIZE,$yp-$BOX_SIZE,$xp+$BOX_SIZE,$yp+$BOX_SIZE);

#     print "Location: ($xp, $yp), El: $elevation, Speed: $vel\n";
        
        my $color = mapColor($vel, 0.0, $maxVel);
#     print "Color: $color\n";
        print 
            my $err = $image->Draw(stroke=>$color, fill=>$color,  primitive=>'rectangle', points=>"$x1,$y1 $x2,$y2");
        if ($err) {
            warn $err;
            print "($lat, $lon, $elevation, $time, $vel)\n";
        }

        if ($vel == $maxVel) {
            my ($x1,$y1, $x2,$y2) = ($xp-4*$BOX_SIZE,$yp-4*$BOX_SIZE,$xp+4*$BOX_SIZE,$yp+4*$BOX_SIZE);
            my $err = $image->Draw(stroke=>'green', fill=>'green',  primitive=>'rectangle', points=>"$x1,$y1 $x2,$y2");
            warn $err if $err;
        }

        ($x2, $y2) = mapPoint(${$prev}[0], ${$prev}[1], 1000, 1000, $minLat, $maxLat, $minLong, $maxLong);
        $err = $image->Draw(stroke=>$color, fill=>$color,  primitive=>'line', points=>"$xp,$yp $x2,$y2");
        warn $err if $err;

        $prev = $var;
    }
}


print "Writing $ARGV[1]\n";
my $err = $image->Write($ARGV[1]);
warn $err if $err;



{
    package GpsSubs;

    sub bounds {
        my ($p, $el, %atts) = @_;
# 	$minLat = $atts{minlat};
# 	$minLong = $atts{minlon};
# 	$maxLat = $atts{maxlat};
# 	$maxLong = $atts{maxlon};
    }
    sub bounds_ {
        my ($p, $el) = @_;
    }
    sub trkpt {
        my ($p, $el, %atts) = @_;
        $oldLat = $curLat;
        $oldLon = $curLon;
        
        $curLat = $atts{lat};
        $curLon = $atts{lon};

        if ($curLat < $minLat) { $minLat = $curLat; }
        if ($curLat > $maxLat) { $maxLat = $curLat; }

        if ($curLon < $minLong) { $minLong = $curLon; }
        if ($curLon > $maxLong) { $maxLong = $curLon; }

        push @points, [$curLat, $curLon];
    }

    sub trkpt_ {
        my ($p, $el) = @_;
    }
    sub ele {
        my ($p, $el, %atts) = @_;
        $p->setHandlers(Char    => \&parseEle)
    }
    sub ele_ {
        my ($p, $el) = @_;
        $p->setHandlers(Char    => \&ignore);

    }
    sub time {
        my ($p, $el, %atts) = @_;
        if ($#points>0) {
            $p->setHandlers(Char    => \&parseTime);
        }
    }
    sub time_ {
        my ($p, $el) = @_;
        $p->setHandlers(Char    => \&ignore);
        
        if ($#points>0) {
            if ($oldEle >= 0) {
                my $distance = GpsSubs::compDist($oldLat, $oldLon, $oldEle,
                                                 $curLat, $curLon, $curEle);
                $totDist += $distance;
                my $td = $curTime-$oldTime;
                if ($td>0) {
                    $totTime += $td;
                    my $vel = $distance / $td;
                    push @{$points[$#points]}, $vel;
                    if ($vel > $maxVel) {
                        $maxVel = $vel;
                    }
                    print "Went $distance in $td seconds: $vel m/s\n";
                }
            }
        }

    }

    sub parseEle {
        my ($p, $str) = @_;

        $oldEle = $curEle;
        $curEle = $str + 0.0;
        push @{$points[$#points]}, $curEle;
    }

    sub parseTime {
        my ($p, $str) = @_;
        $oldTime = $curTime;
        $str =~ /(\d{4})-(\d{2})-(\d{2})T(\d{2}):(\d{2}):(\d{2})Z/;
        my ($year, $month, $day, $hour, $min, $sec) = ($1,$2,$2,$4,$5,$6);
        $curTime = Time::Local::timegm_nocheck($sec, $min,$hour,$day,$month,$year);
        push @{$points[$#points]}, $curTime;
    }

    sub ignore {
    }
    sub compDist {
        my ($lat1, $lon1, $el1, $lat2, $lon2, $el2) = @_;
        my ($z1,$e1,$n1)=Geo::Coordinates::UTM::latlon_to_utm(24, $lat1, $lon1);
        my ($z2,$e2,$n2)=Geo::Coordinates::UTM::latlon_to_utm(24, $lat2, $lon2);
        my $de = ($e2-$e1);
        my $dn = ($n2-$n1);
        my $del = ($el2-$el1);
        my $retVal = sqrt($de*$de + $dn*$dn + $del*$del);
        return $retVal;
    }
}
