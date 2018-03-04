GCC=gcc
CXXFLAGS=-O3 -std=c++14 -I. -fopenmp
TAG=opt
LFLAGS=-lstdc++ -lm -fopenmp -L/usr/lib/x86_64-linux-gnu -lboost_program_options -lboost_filesystem -fopenmp -pthread
DIR=objs
BIN=bin
SOURCES=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp, $(DIR)/%.o, $(SOURCES))
TARGET_NAME=autocorrelation.exe
TARGET=$(BIN)/$(TARGET_NAME)

all: $(TARGET)

.PHONY: clean cleandep all

clean:
	rm -rf $(TARGET) $(DIR)/*.o

cleandep:
	rm -rf $(DIR)/*.d $(DIR)/*.P

$(DIR)/%.o : %.cpp
	@mkdir -p $(DIR)
	$(GCC) $(CXXFLAGS) -MD -c -o $@ $<
	@cp $(DIR)/$*.d $(DIR)/$*.P; \
    sed -e 's/#.*//' -e 's/^[^:]*: *//' -e 's/ *\\$$//' \
        -e '/^$$/ d' -e 's/$$/ :/' < $(DIR)/$*.d >> $(DIR)/$*.P; \
   rm -f $(DIR)/$*.d

$(TARGET): $(OBJS)
	@mkdir -p $(BIN)
	$(GCC) $(OBJS) $(CXXFLAGS) $(LFLAGS) -o $@

-include $(DIR)/*.P