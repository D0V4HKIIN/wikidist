g++ -O3 -pg -o main.o -I/usr/local/include -lzim main.cpp && \
  ./main.o $@ && \
  gprof ./main.o gmon.out | \
  gprof2dot | \
  dot -Tpng -o call_graph.png && \
  firefox call_graph.png
