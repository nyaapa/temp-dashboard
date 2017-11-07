all:	smonitor

CC     = g++
CFLAGS = -Wall -W -Werror -O3 -std=c++1z -lcurl
EXEC   = smonitor

SRC    = smonitor.cpp
OBJ    = $(SRC:.cpp=.o)
DEP    = $(SRC:.cpp=.d)

%.o: %.cpp
	$(CC) $(CFLAGS) -c -MMD -MP $<

$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ)

clean:		
	rm -f $(EXEC) $(OBJ) $(DEP)

rules-install:
	sudo cp smonitor.{service,timer} /etc/systemd/system/
	sudo systemctl enable smonitor.timer

-include $(DEP)