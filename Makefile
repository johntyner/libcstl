QUIET	:= @

CC	:= gcc
CFLAGS	:= -MMD -Wall -Wextra -Wno-unused-function

MODULES	:= bintree rbtree

all: $(addprefix libcstl,.so .a) testexe

b build: $(addprefix libcstl,.so .a)

libcstl.so: $(addsuffix .o,$(MODULES))
	$(QUIET)echo "  LD  $(@)"
	$(QUIET)$(CC) -fPIC -rdynamic -shared -o $(@) $(^)

libcstl.a: $(addsuffix .o,$(MODULES))
	$(QUIET)echo "  AR  $(@)"
	$(QUIET)$(AR) -rc $(@) $(^)

%.o: %.c
	$(QUIET)echo "  CC  $(@)"
	$(QUIET)$(CC) $(CFLAGS) -O2 -fPIC -DNDEBUG -o $(@) -c $(<)

%_test.o: %.c
	$(QUIET)echo "  CC  $(@)"
	$(QUIET)$(CC) $(CFLAGS) -g -D__cfg_test__ -o $(@) -c $(<)

testexe: $(addsuffix _test.o,$(MODULES)) check_test.o
	$(QUIET)echo "  CC  $(@)"
	$(QUIET)$(CC) $(CFLAGS) -g -o $(@) $(^) -lcheck -lsubunit -lm

t test: testexe
	$(QUIET)CK_VERBOSITY=normal ./$(<)

testv: testexe
	$(QUIET)CK_VERBOSITY=verbose ./$(<)

valgrind: testexe
	$(QUIET)CK_VERBOSITY=silent ./$(<)
	$(QUIET)CK_VERBOSITY=silent CK_FORK=no $(@) --leak-check=full -s ./$(<)

devclean:
	$(QUIET)rm -f *~

clean: devclean
	$(QUIET)rm -f testexe check_test.o check_test.d
	$(QUIET)rm -f $(addsuffix .o,$(MODULES)) $(addsuffix _test.o,$(MODULES))
	$(QUIET)rm -f $(addsuffix .d,$(MODULES)) $(addsuffix _test.d,$(MODULES))
	$(QUIET)rm -f $(addprefix libcstl,.a .so)

sinclude *.d
