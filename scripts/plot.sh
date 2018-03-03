#!/bin/bash

input=
start_step=0
with_inset=true

function usage()
{
  cat << EOF
Usage: $0 [OPTIONS]
OPTIONS:
  -i [file]   Input file containing trajectory.
  -s [int]    Start step for calculating statistics.
  -l [double] Lambda
  -m [double] Mu
  -N [int]    Network size
  -p [double] Probability
  -w [bool]   With inset
EOF
}

while getopts "i:s:l:m:N:p:w:" opt
do
  case "$opt" in
    i) input=$OPTARG;;
    s) start_step=$OPTARG;;
    l) lambda=$OPTARG;;
    m) mu=$OPTARG;;
    N) N=$OPTARG;;
    p) p=$OPTARG;;
    w) with_inset=$OPTARG;;
    \:) usage; exit;;
    \?) usage; exit;;
  esac
done

output=${input%.*}.png

function setup_plotter_with_inset()
{
  > plotter.gnu
  echo "set terminal pngcairo enhanced size 4096, 2160 font \"Tahoma, 15\"" >> plotter.gnu
  echo "set output \"$output\"" >> plotter.gnu
  echo "set multiplot" >> plotter.gnu
  echo "set key top right outside spacing 5" >> plotter.gnu
  echo "set origin 0,0" >> plotter.gnu
  echo "set size 1,1" >> plotter.gnu
  echo "set style fill solid" >> plotter.gnu
  echo "set grid xtics mxtics ytics mytics front lw 1.5, lw 0.5" >> plotter.gnu
  echo "set mytics 5" >> plotter.gnu
  echo "set mxtics 10" >> plotter.gnu
  echo "stats \"$input\" every ::$start_step using 2 name \"y\"" >> plotter.gnu
  echo "stats \"$input\" using 2 name \"y_all\"" >> plotter.gnu
  echo "stats \"$input\" using 1 name \"x\"" >> plotter.gnu
  echo "set title 'N=$N, p=$p'" >> plotter.gnu
  echo "set xlabel 'time'" >> plotter.gnu
  echo "set ylabel 'density'" >> plotter.gnu
  echo "min_y=y_all_min-3*y_stddev" >> plotter.gnu
  echo "max_y=y_all_max+3*y_stddev" >> plotter.gnu
  echo "set yrange [min_y-(max_y-min_y)/50:max_y+(max_y-min_y)/50]" >> plotter.gnu
  echo "set xrange [x_min-(x_max-x_min)/50:x_max+(x_max-x_min)/50]" >> plotter.gnu
  echo "plot sample [$start_step:x_max] y_mean+3*y_stddev with filledcurve above y=y_mean-3*y_stddev title sprintf('3{/Symbol s}=%f', 3*y_stddev), \
        [$start_step:x_max] y_mean+2*y_stddev with filledcurve above y=y_mean-2*y_stddev title sprintf('2{/Symbol s}=%f', 2*y_stddev),\
        [$start_step:x_max] y_mean+y_stddev with filledcurve above y=y_mean-y_stddev title sprintf('{/Symbol s}=%f', y_stddev),\
        [$start_step:x_max] y_mean with lines lc rgb \"red\" title sprintf('mean=%f', y_mean),\
        \"$input\" with lines lc rgb \"black\" title \"{/Symbol m}=$mu\n{/Symbol l}=$lambda\"" >> plotter.gnu
  echo "set nokey" >> plotter.gnu
  echo "set title" >> plotter.gnu
  echo "set xlabel" >> plotter.gnu
  echo "set ylabel" >> plotter.gnu
  echo "set origin 0.42, 0.54" >> plotter.gnu
  echo "set size 0.5, 0.4" >> plotter.gnu
  echo "set yrange [y_min-3*y_stddev:y_max+3*y_stddev]" >> plotter.gnu
  echo "set xrange [$start_step:x_max]" >> plotter.gnu
  echo "clear" >> plotter.gnu
  echo "plot sample [$start_step:x_max] y_mean+3*y_stddev with filledcurve above y=y_mean-3*y_stddev, \
        [$start_step:x_max] y_mean+2*y_stddev with filledcurve above y=y_mean-2*y_stddev,\
        [$start_step:x_max] y_mean+y_stddev with filledcurve above y=y_mean-y_stddev,\
        [$start_step:x_max] y_mean with lines lc rgb \"red\",\
        \"$input\" with lines lc rgb \"black\"" >> plotter.gnu
  echo "unset multiplot" >> plotter.gnu
}

function setup_plotter()
{
  > plotter.gnu
  echo "set terminal pngcairo enhanced size 4096, 2160 font \"Tahoma, 15\"" >> plotter.gnu
  echo "set output \"$output\"" >> plotter.gnu
  echo "set key top right outside spacing 5" >> plotter.gnu
  echo "set style fill solid" >> plotter.gnu
  echo "set grid xtics mxtics ytics mytics front lw 1.5, lw 0.5" >> plotter.gnu
  echo "set mytics 5" >> plotter.gnu
  echo "set mxtics 10" >> plotter.gnu
  echo "stats \"$input\" every ::$start_step using 2 name \"y\"" >> plotter.gnu
  echo "stats \"$input\" using 2 name \"y_all\"" >> plotter.gnu
  echo "stats \"$input\" using 1 name \"x\"" >> plotter.gnu
  echo "set title 'N=$N, p=$p'" >> plotter.gnu
  echo "set xlabel 'time'" >> plotter.gnu
  echo "set ylabel 'density'" >> plotter.gnu
  echo "min_y=y_all_min-3*y_stddev" >> plotter.gnu
  echo "max_y=y_all_max+3*y_stddev" >> plotter.gnu
  echo "set yrange [min_y-(max_y-min_y)/50:max_y+(max_y-min_y)/50]" >> plotter.gnu
  echo "set xrange [x_min-(x_max-x_min)/50:x_max+(x_max-x_min)/50]" >> plotter.gnu
  echo "plot sample [$start_step:x_max] y_mean+3*y_stddev with filledcurve above y=y_mean-3*y_stddev title sprintf('3{/Symbol s}=%f', 3*y_stddev), \
        [$start_step:x_max] y_mean+2*y_stddev with filledcurve above y=y_mean-2*y_stddev title sprintf('2{/Symbol s}=%f', 2*y_stddev),\
        [$start_step:x_max] y_mean+y_stddev with filledcurve above y=y_mean-y_stddev title sprintf('{/Symbol s}=%f', y_stddev),\
        [$start_step:x_max] y_mean with lines lc rgb \"red\" title sprintf('mean=%f', y_mean),\
        \"$input\" with lines lc rgb \"black\" title \"{/Symbol m}=$mu\n{/Symbol l}=$lambda\"" >> plotter.gnu
}

if [ -z "$input" -o -z "$lambda" -o -z "$mu" -o -z "$N" -o -z "$p" ]; then
  echo "Wrong input arguments."
  exit
fi

touch plotter.gnu
if $with_inset; then
  setup_plotter_with_inset
else
  setup_plotter
fi
gnuplot plotter.gnu
rm -f plotter.gnu