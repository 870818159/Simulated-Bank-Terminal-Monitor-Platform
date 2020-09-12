obj = client_main.o

ts:$(obj)
	g++ -o ts $(obj)
	
client_main.o:client_main.cpp client_ReadConf.h client_PrepareMsg.h client_Log.h
	g++ -c client_main.cpp
	
clean:	
	rm -rf ts $(obj) *.log *.xls