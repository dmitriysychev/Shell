TEST_EXTS = .ok .img
UTCS_ID ?= $(shell pwd | sed -e 's/.*_//')

MY_TESTS = ${addprefix ${UTCS_ID},${TEST_EXTS}}
SIG = ${firstword ${shell (ls ${MY_TESTS} > /dev/null 2>&1  && (cat ${MY_TESTS} | md5sum)) || echo "?" }}

POSSIBLE_TESTS = ${basename ${wildcard *${firstword ${TEST_EXTS}}}}
TESTS = ${sort ${POSSIBLE_TESTS}}
TEST_OKS = ${addsuffix .ok,${TESTS}}
TEST_RESULTS = ${addsuffix .result,${TESTS}}
TEST_TARGETS = ${addsuffix .test,${TESTS}}
TEST_OUTS = ${addsuffix .out,${TESTS}}
TEST_RAWS = ${addsuffix .raw,${TESTS}}
TEST_DIFFS = ${addsuffix .diff,${TESTS}}

KERNELS = ${addsuffix .kernel,${TESTS}}

all : the_kernel;

the_kernel :
	-(cd kernel; make)

clean:
	rm -rf *.diff *.raw *.out *.result
	(make -C kernel clean)

${TEST_RAWS} : %.raw : Makefile the_kernel %.img
	rm -f $*.raw
	@echo "*** failed to run" > $*.raw
	timeout 10 qemu-system-i386 \
	    -nographic \
	    -smp 4 \
	    --monitor none \
	    --serial file:$*.raw \
	    -drive file=kernel/kernel.img,index=0,media=disk,format=raw \
	    -drive file=$*.img,index=3,media=disk,format=raw \
	    -device isa-debug-exit,iobase=0xf4,iosize=0x04 2> /dev/null || true

${TEST_OUTS} : %.out : Makefile %.raw
	-egrep '^\*\*\*' $*.raw > $*.out 2> /dev/null

${TEST_DIFFS} : %.diff : Makefile %.out %.ok
	-(diff -wBb $*.out $*.ok > $*.diff 2> /dev/null || true)

${TEST_RESULTS} : %.result : Makefile %.diff
	(test -z "`cat $*.diff`" && echo "pass" > $*.result) || echo "fail" > $*.result

${TEST_TARGETS} : %.test : Makefile %.result
	@echo "$* ... `cat $*.result`"

test: Makefile ${TEST_RESULTS} ${TEST_TARGETS} ;

% :
	(make -C kernel $@)

###################### Test #################


sig:
	@echo ${SIG}

tests :
	@echo "$(TESTS)"

copy_student_tests:
ifeq ($(SIG),?)
	@echo "no tests"
else
ifeq ($(TEST_DIR),)
	@echo "no TEST_DIR"
else
	@for i in $(TEST_EXTS); do \
	    cp $(UTCS_ID)$$i $(TEST_DIR)/$(SIG)$$i; \
	done
endif
endif

copy_given_tests:
ifeq ($(TEST_DIR),)
	@echo "no TEST_DIR"
else
	@for i in $(TEST_EXTS); do \
	    cp t?$$i $(TEST_DIR); \
	done
endif

remove_tests:
	@for i in $(TEST_EXTS); do \
	    rm -rf *$$i; \
	done

copy_results:
ifeq ($(RESULTS_DIR),)
	@echo "no RESULTS_DIR"
else
	-cp *.result $(RESULTS_DIR)
	-cp *.ok $(RESULTS_DIR)
	-cp *.out $(RESULTS_DIR)
	-cp *.raw $(RESULTS_DIR)
endif

