COMPILER = gcc
COMPILER_FLAGS = -Wall

EXECUTABLE_DIRECTORY = executable

.PHONY: clean

run_test: test
	./$(EXECUTABLE_DIRECTORY)/test

test: $(EXECUTABLE_DIRECTORY)/test

TEST_SOURCE = \
test/teststringplus.c\
source/stringplus.c

$(EXECUTABLE_DIRECTORY)/test: $(TEST_SOURCE) | $(EXECUTABLE_DIRECTORY)
	$(COMPILER) $(COMPILER_FLAGS) $(TEST_SOURCE) -o $@

$(EXECUTABLE_DIRECTORY):
	mkdir -p $@

clean:
	rm -rf $(EXECUTABLE_DIRECTORY)
