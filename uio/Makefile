EXEC = ip_reg
EXEC2 = ip_status
EXEC3 = ip_tests
EXEC4 = modem_test
EXEC5 = ip_setup_defaults
CFLAGS = -Wall -Wextra -O0 -lncurses -liio

all: $(EXEC) $(EXEC2) $(EXEC3) $(EXEC4) $(EXEC5)

$(EXEC): ip_reg.c
	$(CC) $+ $(CFLAGS) -o $(EXEC)

$(EXEC2): ip_status.c
	$(CC) $+ $(CFLAGS) -o $(EXEC2)

$(EXEC3): ip_tests.c
	$(CC) $+ $(CFLAGS) -o $(EXEC3)

$(EXEC4): modem_test.c
	$(CC) $+ $(CFLAGS) -o $(EXEC4)

$(EXEC5): ip_setup_defaults.c
	$(CC) $+ $(CFLAGS) -o $(EXEC5)

clean:
	rm -rf $(EXEC) $(EXEC2) $(EXEC3) $(EXEC4) $(EXEC5)
