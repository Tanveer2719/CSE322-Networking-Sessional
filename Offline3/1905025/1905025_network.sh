#!/usr/bin/bash

run() {
    local arg="$1"
    shift
    local inputs=("$@")

    for val in "${inputs[@]}"
    do
        for i in {1..3}
        do
            echo "task = ${i} ${arg} = ${val}"
            ./ns3 run "scratch/1905025_networkSimulation.cc --task=${i} --${arg}=${val}"
        done
    done

}

dataRate=(50 100 150 200 250 300)
exponent=(2 3 4 5 6)
run "datarate" "${dataRate[@]}"
run "exp" "${exponent[@]}"


# #################### GNUPLOT ################################

locations=('datarate' 'exp')
task=('taskA' 'taskB1' 'taskB2')
for x in "${locations[@]}"
do
    for y in "${task[@]}"
    do
        if [ ${y} == 'taskA' ]
        then 
            title1='TcpWestWoodPlus' 
            title2='TcpNewReno'
        elif [ ${y} == 'taskB1' ]
        then
            title1='TcpAdaptiveReno' 
            title2='TcpHighSpeed' 
        else
            title1='TcpAdaptiveReno'
            title2='TcpNewReno'
        fi

        gnuplot <<EOM
        set terminal pngcairo enhanced font "arial,10" size 800,600
        set title "Throughput vs ${x}"
        set xlabel "${x}"
        set ylabel "Throughput(Kbps)"
        set grid
        set autoscale
        set style data linespoints
        set output "scratch/files/Offline3/${x}/${y}/output.png"
        plot "scratch/files/Offline3/${x}/${y}/${x}.txt" using 1:2 with linespoints title "${title1}" linewidth 2,\
            "scratch/files/Offline3/${x}/${y}/${x}.txt" using 1:3 with linespoints title "${title2}" linewidth 2

EOM

    done
done

# for congestion
for y in "${task[@]}"
do
    if [ ${y} == 'taskA' ]
    then 
        title1='TcpWestWoodPlus' 
        title2='TcpNewReno'
    elif [ ${y} == 'taskB1' ]
    then
        title1='TcpAdaptiveReno' 
        title2='TcpHighSpeed' 
    else
        title1='TcpAdaptiveReno'
        title2='TcpNewReno'
    fi
    gnuplot <<EOM
    set terminal pngcairo enhanced font "arial,10" size 800,600
    set title "Congestion vs Time"
    set xlabel "Time"
    set ylabel "Congestion"
    set grid
    set autoscale
    set style data linespoints
    set output "scratch/files/Offline3/congestion/${y}/output.png"
    plot "scratch/files/Offline3/congestion/${y}/1.txt" using 1:2 with lines title "${title1}" linewidth 2,\
         "scratch/files/Offline3/congestion/${y}/2.txt" using 1:2 with lines title "${title2}" linewidth 2
EOM


done


# delete files
# rm -r "scratch/files"
