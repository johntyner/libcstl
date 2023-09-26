QUIET	:= @

CC	:= gcc
CFLAGS	:= -MMD -Wall -Wextra -Wno-unused-function

MODULES	:= bintree rbtree

all: $(addprefix libcstl,.so .a)

libcstl.so: $(addsuffix .o,$(MODULES))
	$(CC) -fPIC -rdynamic -shared -o $(@) $(^)

libcstl.a: $(addsuffix .o,$(MODULES))
	$(AR) -r $(@) $(^)

%.o: %.c
	$(CC) $(CFLAGS) -O2 -fPIC -DNDEBUG -o $(@) -c $(<)

%_test.o: %.c
	$(CC) $(CFLAGS) -g -D__test__ -o $(@) -c $(<)

testexe: $(addsuffix _test.o,$(MODULES)) check_test.o
	$(CC) $(CFLAGS) -g -o $(@) $(^) -lcheck -lsubunit -lm

test: testexe
	CK_VERBOSITY=normal ./$(<)

testv: testexe
	CK_VERBOSITY=verbose ./$(<)

valgrind: testexe
	CK_VERBOSITY=silent ./$(<)
	CK_VERBOSITY=silent CK_FORK=no $(@) --leak-check=full -s ./$(<)

devclean:
	rm -f *~

clean: devclean
	rm -f testexe check_test.o check_test.d
	rm -f $(addsuffix .o,$(MODULES)) $(addsuffix _test.o,$(MODULES))
	rm -f $(addsuffix .d,$(MODULES)) $(addsuffix _test.d,$(MODULES))
	rm -f $(addprefix libcstl,.a .so)

sinclude *.d
