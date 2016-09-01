
Q1= gcc tcp_server.c thread_pool.c -o tcp_server -lpthread
Q2= gcc a_tcp_client.c -o a_tcp_client -lpthread
Q3= gcc b_tcp_client.c -o b_tcp_client -lpthread

echo "$Q1 $Q2 $Q3"
echo "success!"
