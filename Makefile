QUIET	:= @

CC	:= gcc
CFLAGS	:= -MMD -Wall -Wextra -Wno-unused-function

MODULES	:= bintree rbtree

all: $(addprefix libcstl,.so .a) testexe

b build: $(addprefix libcstl,.so .a)

libcstl.so: $(MODULES:=.o)
	@echo "  LD  $(@)"
	$(QUIET)$(CC) -fPIC -rdynamic -shared -o $(@) $(^)

libcstl.a: $(MODULES:=.o)
	@echo "  AR  $(@)"
	$(QUIET)$(AR) -rc $(@) $(^)

%.o: %.c
	@echo "  CC  $(@)"
	$(QUIET)$(CC) $(CFLAGS) -O2 -fPIC -DNDEBUG -o $(@) -c $(<)

%_test.o: %.c
	@echo "  CC  $(@)"
	$(QUIET)$(CC) $(CFLAGS) -g -D__cfg_test__ -o $(@) -c $(<)

testexe: $(addsuffix _test.o,$(MODULES) check)
	@echo "  CC  $(@)"
	$(QUIET)$(CC) $(CFLAGS) -g -o $(@) $(^) -lcheck -lsubunit -lm

t test: testexe
	$(QUIET)CK_VERBOSITY=normal ./$(<)

tv testv: testexe
	$(QUIET)CK_VERBOSITY=verbose ./$(<)

valgrind: testexe
	$(QUIET)CK_VERBOSITY=silent ./$(<)
	$(QUIET)CK_VERBOSITY=silent CK_FORK=no $(@) --leak-check=full -s ./$(<)

devclean:
	$(QUIET)rm -f *~

clean: devclean
	$(QUIET)rm -f testexe $(addprefix check_test,.o .d)
	$(QUIET)rm -f $(MODULES:=.o) $(MODULES:=_test.o)
	$(QUIET)rm -f $(MODULES:=.d) $(MODULES:=_test.d)
	$(QUIET)rm -f $(addprefix libcstl,.a .so)

sinclude *.d
