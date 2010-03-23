#!/usr/bin/perl 

# gps_speed.pl

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

use POSIX qw(strftime);
use Time::Local 'timegm_nocheck';

use Geo::Coordinates::UTM;

my @pts;
my $minLat;
my $maxLat;
my $minLong;
my $maxLong;

if (@ARGV!=1) {
    print "This command takes one command line argument: an input file:\n\t $ARGV[0] infile\n";
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

my $curTrackNum = 0;

my $totDist = 0;
my $totTime = 0;
$curLat = $curLon = $curEle = $oldEle = -1;

my $ptsRead = 0;

my $grandTotalDist = 0;
my $grandTotalTime = 0;

my $maxSpeed = -1.0;

my $p = XML::Parser->new(Style => 'Subs', Pkg => 'GpsSubs');
$p->parsefile($ARGV[0]);
print "\n";
print "Overall went a total of $grandTotalDist in $grandTotalTime seconds\n";
my $avVel = $grandTotalDist / $grandTotalTime;
print "Average speed: $avVel m/s\n";
print "Max speed: $maxSpeed\n";
{
    package GpsSubs;
    my $curBuffer;
    sub trk {
        $curTrackNum += 1;
        $totDist = 0;
        $totTime = 0;
        $curLat = $curLon = $curEle = $oldEle = -1;
        $curBuffer = '';
    }
    sub trk_ {
        if ($ptsRead) {
            $grandTotalDist += $totDist;
            $grandTotalTime += $totTime;
            print "Track $curTrackNum went a total of $totDist in $totTime seconds\n";
            my $avVel = $totDist / $totTime;
            print "Average speed: $avVel m/s\n";
        } else {
            print "Track $curTrackNum was empty!\n";
        }
        $ptsRead = 0;
    }
    sub trkpt {
        my ($p, $el, %atts) = @_;
        $oldLat = $curLat;
        $oldLon = $curLon;
        
        $curLat = $atts{lat};
        $curLon = $atts{lon};
        $ptsRead += 1;
    }

    sub trkpt_ {
        my ($p, $el) = @_;
        if ($ptsRead > 1) {
            my $distance = compDist($oldLat, $oldLon, $oldEle,
                                    $curLat, $curLon, $curEle);
            $totDist += $distance;
            my $td = $curTime-$oldTime;
            if ($distance/$td > $maxSpeed) {
                $maxSpeed = $distance/$td;
            }
            if ($td>0) {
                $totTime += $td;
            }
        }
    }

    sub ele {
        my ($p, $el, %atts) = @_;
        $curBuffer = '';
        $p->setHandlers(Char    => \&collectText)
    }
    sub ele_ {
        my ($p, $el) = @_;
        $p->setHandlers(Char    => \&ignore);
        $oldEle = $curEle;
        $curEle = $curBuffer + 0.0;
    }
    sub time {
        my ($p, $el, %atts) = @_;
        $curBuffer = '';
        $p->setHandlers(Char    => \&collectText)
    }

    sub time_ {
        my ($p, $el) = @_;
        $p->setHandlers(Char    => \&ignore);
        $oldTime = $curTime;
        $curBuffer =~ /(\d{4})-(\d{2})-(\d{2})T(\d{2}):(\d{2}):(\d{2})Z/;
        my ($year, $month, $day, $hour, $min, $sec) = ($1,$2,$2,$4,$5,$6);
        $curTime = Time::Local::timegm_nocheck($sec, $min,$hour,$day,$month,$year);
    }
    sub collectText {
        my ($p, $str) = @_;
        $curBuffer .= $str;
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
        
        return sqrt($de*$de + $dn*$dn + $del*$del);
    }

}

