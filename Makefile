CXX := $(shell which clang)

# just a clean way to distinguish the two deployment environments
DEVELOPMENT_FLAGS := -g -Weverything -gdwarf-4 -fPIC
PRODUCTION_FLAGS := -O3 -fPIC

ifeq "$(DEPLOYMENT)" "production"
    CXXFLAGS:= $(PRODUCTION_FLAGS)
else
    CXXFLAGS:= $(DEVELOPMENT_FLAGS)
endif

# language specific
SOURCE_FILES_SUFFIX := .c
HEADER_SUFFIX := .h
OBJECT_SUFFIX := .o

# source
NAME_OF_THE_LIBRARY := libmod.so
NAME_OF_SOURCE_CODE_FOR_THE_LIBRARY := src
RELATIVE_SOURCE_FOLDER_PATH := ./$(NAME_OF_SOURCE_CODE_FOR_THE_LIBRARY)/
NAME_OF_THE_OBJECTS_OF_THE_LIBRARY := temp_obj_directory
OBJECT_FILES_OF_SOURCE_WITHOUT_PREFIX := $(patsubst %$(SOURCE_FILES_SUFFIX),%$(OBJECT_SUFFIX),$(patsubst %$(HEADER_SUFFIX),%$(OBJECT_SUFFIX),$(shell find $(RELATIVE_SOURCE_FOLDER_PATH) -type f -name "*$(HEADER_SUFFIX)")))
OBJECT_FILES_OF_SOURCE_WITH_PREFIX := $(foreach F,$(OBJECT_FILES_OF_SOURCE_WITHOUT_PREFIX),$(lastword $(subst $(RELATIVE_SOURCE_FOLDER_PATH), ,$F)))
LIBRARY_OBJECTS := $(addprefix $(NAME_OF_THE_OBJECTS_OF_THE_LIBRARY)/, $(OBJECT_FILES_OF_SOURCE_WITH_PREFIX))
SHARED := -shared

# tests
NAME_OF_TEST_EXECUTABLE := executable
NAME_OF_SOURCE_CODE_FOR_TESTS:= tests
RELATIVE_TESTS_FOLDER_PATH := ./$(NAME_OF_SOURCE_CODE_FOR_TESTS)/
NAME_OF_THE_OBJECTS_OF_THE_TESTS := temp_test_obj_directory
OBJECT_FILES_OF_TESTS_WITHOUT_PREFIX := $(patsubst %$(SOURCE_FILES_SUFFIX),%$(OBJECT_SUFFIX),$(patsubst %$(HEADER_SUFFIX),%$(OBJECT_SUFFIX),$(shell find $(RELATIVE_TESTS_FOLDER_PATH) -type f -name "*$(HEADER_SUFFIX)")))
OBJECT_FILES_OF_TESTS_WITH_PREFIX := $(foreach F,$(OBJECT_FILES_OF_TESTS_WITHOUT_PREFIX),$(lastword $(subst $(RELATIVE_TESTS_FOLDER_PATH), ,$F)))
TEST_OBJECTS :=$(addprefix $(NAME_OF_THE_OBJECTS_OF_THE_TESTS)/, $(OBJECT_FILES_OF_TESTS_WITH_PREFIX))
LIBS := -lsubunit -lm -lcheck $(NAME_OF_THE_LIBRARY)

MEM_CHECK_FILE := valgrind_results.txt

# build the library
build: clean $(NAME_OF_THE_LIBRARY)

# compile everything needed for the library
$(NAME_OF_THE_LIBRARY): lib_obj_dirs $(LIBRARY_OBJECTS)
	$(CXX) $(CXXFLAGS) $(SHARED) $(LIBRARY_OBJECTS) -o $(NAME_OF_THE_LIBRARY)

# compile command for each source file
$(NAME_OF_THE_OBJECTS_OF_THE_LIBRARY)/%$(OBJECT_SUFFIX): $(NAME_OF_SOURCE_CODE_FOR_THE_LIBRARY)/%$(SOURCE_FILES_SUFFIX) $(NAME_OF_SOURCE_CODE_FOR_THE_LIBRARY)/%$(HEADER_SUFFIX)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# make temporary directory for the library objects
lib_obj_dirs:
	cp -R --attributes-only ./$(NAME_OF_SOURCE_CODE_FOR_THE_LIBRARY)/ ./$(NAME_OF_THE_OBJECTS_OF_THE_LIBRARY)
	find ./$(NAME_OF_THE_OBJECTS_OF_THE_LIBRARY) -type f -exec rm {} \;
	
# build the tests
test: clean $(NAME_OF_THE_LIBRARY) $(NAME_OF_TEST_EXECUTABLE)

# build an executable that tests the library
$(NAME_OF_TEST_EXECUTABLE): test_lib_dirs $(TEST_OBJECTS)
	$(CXX) $(CXXFLAGS) $(TEST_OBJECTS) -o $(NAME_OF_TEST_EXECUTABLE) -L. $(LIBS) 

# compile command for each test file
$(NAME_OF_THE_OBJECTS_OF_THE_TESTS)/%$(OBJECT_SUFFIX): $(NAME_OF_SOURCE_CODE_FOR_TESTS)/%$(SOURCE_FILES_SUFFIX) $(NAME_OF_SOURCE_CODE_FOR_TESTS)/%$(HEADER_SUFFIX)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# make temporary directory for the test objects
test_lib_dirs:
	cp -R --attributes-only ./$(NAME_OF_SOURCE_CODE_FOR_TESTS)/ ./$(NAME_OF_THE_OBJECTS_OF_THE_TESTS)
	find ./$(NAME_OF_THE_OBJECTS_OF_THE_TESTS) -type f -exec rm {} \;

clean:
	rm -f *~ *$(OBJECT_SUFFIX) $(NAME_OF_THE_LIBRARY) $(NAME_OF_TEST_EXECUTABLE) $(MEM_CHECK_FILE)
	rm -rf $(NAME_OF_THE_OBJECTS_OF_THE_LIBRARY) $(NAME_OF_THE_OBJECTS_OF_THE_TESTS)

memory_check: test
	valgrind -s --leak-check=full --show-leak-kinds=all --leak-resolution=med --track-origins=yes --verbose --log-file=$(MEM_CHECK_FILE) ./$(NAME_OF_TEST_EXECUTABLE)