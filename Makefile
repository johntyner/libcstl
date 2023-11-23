QUIET	:= @

CC	:= gcc
CFLAGS	:= -MMD \
	-Wall -Wextra -Werror=vla -Werror=declaration-after-statement \
	-std=c99 -pedantic \
	-D_POSIX_C_SOURCE=199309L

LIBSRCS	:= $(filter-out src/check.c src/_string.c,$(wildcard src/*.c))
LIBOBJS	:= $(patsubst src/%.c,build/%.o, $(LIBSRCS))

CHKSRCS	:= $(filter-out src/_string.c,$(wildcard src/*.c))
CHKOBJS	:= $(patsubst src/%.c,build/test/%.o,$(CHKSRCS))

.PHONY: all
all: $(addprefix build/,$(addprefix libcstl,.so .a) test/check) doc

.PHONY: b build
b build: $(addprefix build/libcstl,.so .a)

build/libcstl.so: $(LIBOBJS)
	@echo "  LD\t$(@)"
	$(QUIET)$(CC) -fPIC -rdynamic -shared -o $(@) $(^) -lm

build/libcstl.a: $(LIBOBJS)
	@echo "  AR\t$(@)"
	$(QUIET)$(AR) -rc $(@) $(^)

build/%.o: src/%.c Makefile
	@echo "  CC\t$(@)"
	$(QUIET)$(CC) $(CFLAGS) -O2 -Wno-unused-function -fPIC \
		-DNDEBUG -Iinclude -o $(@) -c $(<)

build/test/%.o: src/%.c Makefile
	@echo "  CC\t$(@)"
	$(QUIET)$(CC) $(CFLAGS) -g -fprofile-arcs -ftest-coverage \
		-D__cfg_test__ -Iinclude -o $(@) -c $(<)
	$(QUIET)rm -f $(@:.o=.gcda)

build/test/check: $(CHKOBJS)
	@echo "  LD\t$(@)"
	$(QUIET)$(CC) $(CFLAGS) -g -o $(@) $(^) -lcheck -lsubunit -lm -lgcov

.PHONY: t test
t test: build/test/check
	$(QUIET)CK_VERBOSITY=normal ./$(<)

.PHONY: gcov
gcov: build/test/check
	$(QUIET)CK_VERBOSITY=silent ./$(<)
	$(QUIET)gcov $(LIBSRCS) --object-directory build/test
	$(QUIET)mv *.gcov build/test

.PHONY: gcovr
gcovr: gcov
	$(QUIET)gcovr build/test

.PHONY: tv testv
tv testv: build/test/check
	$(QUIET)CK_VERBOSITY=verbose ./$(<)

.PHONY: vg valgrind
vg valgrind: build/test/check
	$(QUIET)CK_FORK=no \
		valgrind --leak-check=full -s --error-exitcode=1 ./$(<)

.PHONY: gdb
gdb: build/test/check
	$(QUIET)CK_FORK=no gdb ./$(<)

.PHONY: fmt
fmt: $(wildcard src/*.c include/internal/*.h include/cstl/*.h)
	$(QUIET)astyle --options=.astylerc --suffix=none -Q $(^)

.PHONY: doc
doc: build/doc/html/index.html
build/doc/html/index.html: doxygen.conf \
		$(wildcard include/cstl/*.h) $(wildcard src/*.c)
	$(QUIET)echo "  DOC\t$(@D)"
	$(QUIET)doxygen $(<) >/dev/null

.PHONY: docclean
docclean:
	$(QUIET)rm -rf build/doc

.PHONY: devclean
devclean:
	$(QUIET)find . -type f -name "*~" -exec rm -f {} \;

.PHONY: clean
clean: devclean docclean
	$(QUIET)rm -f $(addprefix build/libcstl,.a .so)
	$(QUIET)rm -f $(LIBOBJS) $(LIBOBJS:.o=.d)
	$(QUIET)rm -f  build/test/check
	$(QUIET)rm -f $(CHKOBJS) $(CHKOBJS:.o=.d)
	$(QUIET)rm -f $(CHKOBJS:.o=.gcno) $(CHKOBJS:.o=.gcda)
	$(QUIET)rm -f build/test/*.gcov

sinclude build/*.d
sinclude build/test/*.d
