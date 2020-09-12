obj = server.o server_running.o server_database.o server_log.o main.o

yzmond:$(obj)
	g++ -o yzmond $(obj) -I/usr/include/mysql -L/usr/lib64/mysql -lmysqlclient
	
server.o:server.cpp server.h encrpty.h
	g++ -c server.cpp -I/usr/include/mysql -L/usr/lib64/mysql -lmysqlclient
	
server_running.o:server_running.cpp server.h
	g++ -c server_running.cpp -I/usr/include/mysql -L/usr/lib64/mysql -lmysqlclient
	
server_database.o:server_database.cpp server.h
	g++ -c server_database.cpp -I/usr/include/mysql -L/usr/lib64/mysql -lmysqlclient
	
server_log.o:server_log.cpp server.h
	g++ -c server_log.cpp -I/usr/include/mysql -L/usr/lib64/mysql -lmysqlclient
	
main.o:main.cpp server.h
	g++ -c main.cpp -I/usr/include/mysql -L/usr/lib64/mysql -lmysqlclient

clean:
	rm -rf yzmond $(obj) *.log