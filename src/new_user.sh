# compile and remove common and mirror directories
make
make reset

# run client 12 and run "new_user41.sh" 
# in another window of the terminal first, he detects that there is a new user 41 and mirrors his data
# then, while monitoring realizes that is synced with 41, so he does nothing
# now, press ^C in the other window, 41 will leave the system and 12 will delete mirror_dir_1/41
./mirror -n 12 -c common_dir -i input_1 -m mirror_dir_1 -b 128 -l log_1 &
client=$! 

sleep 60  #monitor_cycle = 5, max_wait_on_read = 10

# send a SIGINT signal to 12 to terminate
kill -s SIGINT $client

# checkout what we mirrored, mirror_dir_2 should be identical to input_1 and mirror_dir_1 must have been deleted because 41 left the system
tree input_*
tree mirror_dir_*
diff -sr mirror_dir_2/12/ input_1

# concatenate all logfiles and get their statistics running the script 'get_stash.sh'
cat log_*
cat log_1 log_2 | ../scripts/get_stash.sh
make clean