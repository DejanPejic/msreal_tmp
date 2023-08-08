sources=$(wildcard *.c)
objs=$(sources:.c=.o)
PROJECT=hough

all: hough 
hough: $(objs)
	@echo "Building output binary..."
	$(CC) -o $@ $(objs) -lm
%.o: %.c
	@echo "Compiling sources..."
	$(CC) -c $(sources) -lm
%.d: %.c
	@echo -n  "Creating dependancies..."
	@echo $@
	@set -e; rm -f $@; \
	$(CC) -MM $(CPPFLAGS) $< > $@.$$$$; \
	sed 's,    \($*\)\.o[ :]*     ,    \1.o $@ :   ,   g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
run:
	./$(PROJECT) 0.5 ../../data/logo.png 0.6 ../../data/djokovic.png
clean:
	@rm -f hough *.o *.d* result_img*.jpg
	@echo "Clean done..."

-include $(sources:.c=.d)
