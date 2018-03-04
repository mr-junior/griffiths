#!/bin/bash

input=
output=
width=0.5
x_label=
y_label=
title=
column=1
start=0

function usage()
{
	cat << EOF
Usage: $0 [OPTIONS]
OPTIONS:
	-i [file]  Input file containing values.
	-o [file]  Output image file.
	-w [num]   Bin width.
	-x [str]   X axis label.
	-y [str]   Y axis label.
	-t [str]   Title.
	-c [int]   Column number to use.
	-s [int]   Start line.
EOF
}

while getopts "i:o:w:x:y:t:c:s:" opt
do
	case "$opt" in
		i) input=$OPTARG;;
		o) output=$OPTARG;;
		w) width=$OPTARG;;
		x) x_label=$OPTARG;;
		y) y_label=$OPTARG;;
		t) title=$OPTARG;;
		c) column=$OPTARG;;
		s) start=$OPTARG;;
		\:) usage; exit;;
		\?) usage; exit;;
	esac
done

if [ -z "$output" ]; then
	output=${input%.*}.png
fi

if [ -z "$title" ]; then
	title="Bin width = $width"
fi

> histogram_plotter.gnu
echo "set terminal png enhanced size 4096, 2160 font \"Tahoma, 15\"" >> histogram_plotter.gnu
echo "set output \"$output\"" >> histogram_plotter.gnu
echo "set xtics" >> histogram_plotter.gnu
echo "set mxtics" >> histogram_plotter.gnu
echo "set ytics" >> histogram_plotter.gnu
echo "set mytics" >> histogram_plotter.gnu
echo "set grid" >> histogram_plotter.gnu
echo "stats \"$input\" using $column every ::$start" >> histogram_plotter.gnu
if [[ ! -z $x_label ]]; then
	echo "set xlabel \"$x_label\"" >> histogram_plotter.gnu
fi
if [[ ! -z $y_label ]]; then
	echo "set ylabel \"$y_label\"" >> histogram_plotter.gnu
fi
echo "set title \"$title\"" >> histogram_plotter.gnu
echo "set xrange [STATS_min-(STATS_max-STATS_min)/20:STATS_max+(STATS_max-STATS_min)/20]" >> histogram_plotter.gnu
echo "binwidth=$width" >> histogram_plotter.gnu
echo "bin(x, width)=width*floor(x/width)" >> histogram_plotter.gnu
echo "set style fill solid 0.5" >> histogram_plotter.gnu
echo "set boxwidth binwidth" >> histogram_plotter.gnu
echo "plot \"$input\" every ::$start using (bin(\$$column,binwidth)):(1.0) smooth freq with boxes notitle" >> histogram_plotter.gnu

gnuplot histogram_plotter.gnu

rm -f histogram_plotter.gnu