MODE= 0

SRC=  main.cpp Graph.cpp gen-graph.cpp fixage.cpp test-properties.cpp compute_arrays_compat.cpp compare_with_cleophee.cpp problemArray.cpp misc.cpp gzstream/gzstream.cpp
OBJ= ${SRC:.cpp=.o}
d_OBJ= ${SRC:.cpp=_d.o}
p_OBJ= ${SRC:.cpp=_p.o}
CXX	 = g++
LFLAGS   = -lboost_thread -lboost_system -lm -lz #-L gurobi/linux64/lib/ -lgurobi_g++5.2 -lgurobi75 #lgurobi_c++ lgurobi_g++5.2
IMPLFLAGS = -pthread #-DBOOST_DYNAMIC_BITSET_DONT_USE_FRIENDS -DMODE=$(MODE) $(LIBPATH)
CXXFLAGS = $(IMPLFLAGS) -std=c++17 -Wall -s -O3 -Wno-unused-but-set-variable -Wno-unused-variable -Wno-unused-parameter -Wno-sign-compare -Wno-alloc-size-larger-than#-march=native #-Winline#-D_GLIBCXX_DEBUG
CXXDEBUGFLAGS = $(IMPLFLAGS) -std=c++17 -Wall -Wextra -g -Wno-sign-compare -D_GLIBCXX_DEBUG -DDEBUG -O0
CXXPROFILEFLAGS = $(IMPLFLAGS) -std=c++17 -Wall -Wextra -O3 -g -fno-inline


all : release
      
release: $(OBJ) $(HDR) 
	${CXX} $(CXXFLAGS) -o a.out $(OBJ) $(LFLAGS)  $(LIB);

debug: $(d_OBJ) $(HDR) 
	${CXX} $(CXXDEBUGFLAGS) -o $@ $(d_OBJ) $(LFLAGS)  $(LIB);

profile: $(p_OBJ) $(HDR) 
	${CXX} $(CXXPROFILEFLAGS) -o $@ $(p_OBJ) $(LFLAGS)  $(LIB);

clean: 
	rm -f $(OBJ) $(d_OBJ) $(p_OBJ)

destroy: clean
	rm -f release debug profile

asm: $(OBJ) $(HDR)
	$(CXX) $(CXXFLAGS) -S $(SRC) $(LFLAGS) $(LIB);

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
     
%_p.o: %.cpp
	$(CXX) $(CXXPROFILEFLAGS) -c $< -o $@

%_d.o: %.cpp
	$(CXX) $(CXXDEBUGFLAGS) -c $< -o $@
