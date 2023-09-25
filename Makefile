QUIET	:= @

CC	:= gcc
CFLAGS	:= -Wall -Wextra -Wno-unused-functions

MODULES	:= bintree

all: $(addprefix libcstl,.so .a)

libcstl.so: $(addsuffix .o,$(MODULES))
	$(CC) -fPIC -rdynamic -shared -o $(@) $(^)

libcstl.a: $(addsuffix .o,$(MODULES))
	$(AR) -r $(@) $(^)

%.o: %.c
	$(CC) $(CFLAGS) -O2 -fPIC -o $(@) -c $(<)

%_test.o: %.c
	$(CC) $(CFLAGS) -g -D__test__ -o $(@) -c $(<)

testexe: $(addsuffix _test.o,$(MODULES))
	$(CC) $(CFLAGS) -g -D__test__ -o test.o -c test.c
	$(CC) $(CFLAGS) -g -o $(@) $(filter %.o,$(^)) test.o \
	  -lcheck -lsubunit -lm

test: testexe
	./$(<)

valgrind: test
	CK_FORK=no $(@) --leak-check=full -s ./testexe

devclean:
	rm -f *~

clean: devclean
	rm -f test.o testexe
	rm -f $(addsuffix .o,$(MODULES)) $(addsuffix _test.o,$(MODULES))
	rm -f $(addprefix libcstl,.a .so)
