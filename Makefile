QUIET	:= @

CC	:= gcc
CFLAGS	:= -Wall -Wextra -O2

libcstl.a: bintree.o
	$(AR) -r $(@) $(^)

%.o: %.c
	$(CC) $(CFLAGS) -o $(@) -c $(<)

devclean:
	rm -f *~

clean: devclean
	rm -f bintree.o
	rm -f libcstl.a
