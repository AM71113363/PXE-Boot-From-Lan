# Project: AM71113363
# Makefile created by Dev-C++ 4.9.9.2

CC   = gcc.exe -s
WINDRES = windres.exe
RES  = main.res
OBJ  = main.o dhcp.o tftp.o $(RES)
LINKOBJ  = main.o dhcp.o tftp.o $(RES)
LIBS =  -L"C:/Dev-Cpp/lib" -mwindows -lws2_32 -lcomctl32  
INCS =  -I"C:/Dev-Cpp/include" 
BIN  = BOOT_LAN.exe
CFLAGS = $(INCS)  
RM = rm -f

.PHONY: all all-before all-after clean clean-custom

all: all-before BOOT_LAN.exe all-after


clean: clean-custom
	${RM} $(OBJ) $(BIN)

$(BIN): $(OBJ)
	$(CC) $(LINKOBJ) -o "BOOT_LAN.exe" $(LIBS)

main.o: main.c
	$(CC) -c main.c -o main.o $(CFLAGS)

dhcp.o: dhcp.c
	$(CC) -c dhcp.c -o dhcp.o $(CFLAGS)

tftp.o: tftp.c
	$(CC) -c tftp.c -o tftp.o $(CFLAGS)

main.res: main.rc 
	$(WINDRES) -i main.rc --input-format=rc -o main.res -O coff 
