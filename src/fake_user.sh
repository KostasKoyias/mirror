# compile and remove common and mirror directories
make
make reset

# run client 12 
./mirror -n 12 -c common_dir -i input_1 -m mirror_dir_1 -b 128 -l log_1 &
client=$(ps|grep mirror|head -1|cut -d " " -f 1)
if [ "$client" == "" ]
then 
    client=$(ps|grep mirror|head -1|cut -d " " -f 2)
fi 
echo pid = $client  

# insert a new <<fake>> user 93 in common_dir, receiving process should fail three times to retrieve his data
touch common_dir/93.id
sleep 70  #monitor_cycle = 5, max_wait_on_read = 10

# send a SIGINT signal to 12 to terminate
kill -s SIGINT $client

# clean things up
make clean
