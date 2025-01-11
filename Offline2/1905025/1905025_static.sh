#!/bin/bash

run() {
    local arg="$1"
    shift
    local inputs=("$@")

    for val in "${inputs[@]}"
    do
        echo "${arg} = ${val}"
        ./ns3 run "scratch/1905025_static_wifi.cc --${arg}=${val}"
    done

}

./ns3 configure

nNodes=(20 40 60 80 100)
coverage=(1 2 3 4 5)
nFlow=(10 20 30 40 50)
nPackets=(100 200 300 400 500)

run "nodes" "${nNodes[@]}"
run "flows" "${nFlow[@]}"
run "packets" "${nPackets[@]}"
run "coverageArea" "${coverage[@]}"


#################### GNUPLOT ################################


locations=('nodes' 'packets' 'flows' 'coverageArea' )

# for throughput 
for x in "${locations[@]}"
do
    gnuplot <<EOM
    set title "Throughput vs ${x}"
    set xlabel "${x}"
    set ylabel "Throughput(Kbps)"
    set grid
    set terminal png
    set output "scratch/files/static/${x}/throughput/output.png"
    plot "scratch/files/static/${x}/throughput/sThroughput.txt" using 1:2 with lines title "Throughput"
EOM
done

# for packet delivery ratio
for x in "${locations[@]}"
do
    gnuplot <<EOM
    set title "PacketDeliveryRatio vs ${x}"
    set xlabel "${x}"
    set ylabel "ratio"
    set grid
    set terminal png
    set output "scratch/files/static/${x}/pktRatio/output.png"
    plot "scratch/files/static/${x}/pktRatio/sPktRatio.txt" using 1:2 with lines title "Throughput"
EOM
done 

# # delete the .txt .png files
# for x in "${locations[@]}"
# do
#     rm "scratch/files/static/${x}/pktRatio/sPktRatio.txt"
#     rm "scratch/files/static/${x}/throughput/sThroughput.txt"
#     rm "scratch/files/static/${x}/pktRatio/output.png"
#     rm "scratch/files/static/${x}/throughput/output.png"
# done
