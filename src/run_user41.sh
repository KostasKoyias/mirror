# run user 41 wait until he syncs with 12 and has completed at least one monitoring cycle
./mirror -n 41 -c common_dir -i input_2 -m mirror_dir_2 -b 256 -l log_2 &
client=$(ps|grep mirror|head -1|cut -d " " -f 1)
if [ "$client" == "" ]
then 
    client=$(ps|grep mirror|head -1|cut -d " " -f 2)
fi 
echo pid = $client
sleep 40

# send SIGINT signal to 41 so that he leaves the system
kill -s SIGINT $client
