#gcc -I ../includes/ get_hosts.c -o a.out
gcc -I ../includes/ client.c -o client
gcc -I ../includes/ server.c -o server 
gcc -I ../includes/ udp_client.c -o udp_client
gcc -I ../includes/ udp_server.c -o udp_server
