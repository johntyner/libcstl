QUIET	:= @

CC	:= gcc
CFLAGS	:= -MMD -Wall -Wextra -Werror=vla -std=c99 -pedantic

MODULES	:= common memory \
	bintree rbtree heap dlist slist hash vector \
	string map array

all: $(addprefix build/,$(addprefix libcstl,.so .a) test/check) doc

b build: $(addprefix build/libcstl,.so .a)

build/libcstl.so: $(addprefix build/,$(MODULES:=.o))
	@echo "  LD\t$(@)"
	$(QUIET)$(CC) -fPIC -rdynamic -shared -o $(@) $(^)

build/libcstl.a: $(addprefix build/,$(MODULES:=.o))
	@echo "  AR\t$(@)"
	$(QUIET)$(AR) -rc $(@) $(^)

build/%.o: src/%.c
	@echo "  CC\t$(@)"
	$(QUIET)$(CC) $(CFLAGS) -O2 -Wno-unused-function -fPIC -DNDEBUG -Iinclude -o $(@) -c $(<)

build/test/%.o: src/%.c
	@echo "  CC\t$(@)"
	$(QUIET)$(CC) $(CFLAGS) -g -fprofile-arcs -ftest-coverage -D__cfg_test__ -Iinclude -o $(@) -c $(<)

build/test/check: $(addprefix build/test/,$(addsuffix .o,$(MODULES) check))
	@echo "  LD\t$(@)"
	$(QUIET)$(CC) $(CFLAGS) -g -o $(@) $(^) -lcheck -lsubunit -lm -lgcov

t test: build/test/check
	$(QUIET)CK_VERBOSITY=normal ./$(<)

gcov: build/test/check
	$(QUIET)CK_VERBOSITY=silent ./$(<)
	$(QUIET)gcov $(addprefix src/,$(MODULES:=.c)) --object-directory build/test
	$(QUIET)mv *.gcov build/test

gcovr: gcov
	$(QUIET)gcovr build/test

tv testv: build/test/check
	$(QUIET)CK_VERBOSITY=verbose ./$(<)

vg valgrind: build/test/check
	$(QUIET)CK_VERBOSITY=silent ./$(<)
	$(QUIET)CK_VERBOSITY=silent CK_FORK=no CK_EXCLUDE_TAGS=abort valgrind --leak-check=full -s ./$(<)

gdb: build/test/check
	$(QUIET)CK_FORK=no CK_EXCLUDE_TAGS=abort gdb ./$(<)

doc: build/doc/html/index.html
build/doc/html/index.html: doxygen.conf \
		$(wildcard include/cstl/*.h) $(wildcard src/*.c)
	$(QUIET)echo "  DOC\t$(@D)"
	$(QUIET)doxygen $(<) >/dev/null

docclean:
	$(QUIET)rm -rf build/doc

devclean:
	$(QUIET)find . -type f -name "*~" -exec rm -f {} \;

clean: devclean docclean
	$(QUIET)rm -f $(addprefix build/,$(MODULES:=.o) $(MODULES:=.d))
	$(QUIET)rm -f $(addprefix build/libcstl,.a .so)
	$(QUIET)rm -f $(addprefix build/test/,$(MODULES:=.o) $(MODULES:=.d))
	$(QUIET)rm -f $(addprefix build/test/,check $(addprefix check.,o d gcno gcda))
	$(QUIET)rm -f $(addprefix build/test/,$(MODULES:=.gcno) $(MODULES:=.gcda))
	$(QUIET)rm -f build/test/*.gcov

sinclude build/*.d
sinclude build/test/*.d

.PHONY: doc docclean devclean clean
