CXX=icpx -fsycl
CXXFLAGS=-I.
LIBS=-L/usr/lib/gcc/x86_64-linux-gnu/11

%.o: %.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

exec: vector_add.o
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS) 

clean:
	rm -f exec *.o
