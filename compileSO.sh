docker start $1
docker exec -it $1 make clean
docker exec -it $1 make -C /root/Toolchain
docker exec -it $1 make -C /root/
docker stop $1
sudo ./run.sh $2
