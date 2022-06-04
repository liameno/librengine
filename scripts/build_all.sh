cd lib
mkdir build && cd build && cmake .. && sudo make install

cd ../../crawler
mkdir build && cd build && cmake .. && make

cd ../../website
mkdir build && cd build && cmake .. && make