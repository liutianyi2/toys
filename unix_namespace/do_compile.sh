gcc -I ../includes/ test.c
gcc -I ../includes/ server.c -o server
gcc -I ../includes/ client.c -o client
gcc -I ../includes/ server_udp.c -o server_udp
gcc -I ../includes/ client_udp.c -o client_udp
