sudo apt install libcurl4-openssl-dev &&
curl -O https://dl.typesense.org/releases/0.23.0.rc20/typesense-server-0.23.0.rc20-linux-amd64.tar.gz &&
tar -xzf typesense-server-0.23.0.rc20-linux-amd64.tar.gz &&
git clone https://github.com/lexbor/lexbor && 
cd lexbor &&
cmake . && make && sudo make install &&
sudo apt install libssl-dev