all:
	g++ -o dtm2joybus dtm2joybus.c

.PHONY: clean
clean:
	@rm -v dtm2joybus
