## Librengine
Private Web Search engine
## Website
[![https://raw.githubusercontent.com/liameno/librengine/master/search_page.png](https://raw.githubusercontent.com/liameno/librengine/master/search_page.png)]()
## Features
-  Crawler
-  Website
	-  Search
	-  Node Info

## TODO
-  Encryption (public key)
-  Site Rating
-  CLI Search
-  API
-  Export DB
-  Images Crawler

## Dependencies
Arch: `yay -S curl lexbor opensearch`
## Build
```shell
git clone https://github.com/liameno/librengine
cd ~/librengine/lib
mkdir build && cd build && cmake .. && sudo make install

cd ~/librengine/crawler
mkdir build && cd build && cmake .. && make

cd ~/librengine/website/backend
mkdir build && cd build && cmake .. && make
```
## Run
```shell
opensearch
cd ~/librengine
sh set_opensearch.sh
```
#### Crawler
```shell
cd crawler/build
./crawler https://www.gnu.org socks5://127.0.0.1:9050
#[start_site] [proxy = null] [recursive_deep_max = 3] [load_page_timeout_s = 10] [delay_time_s = 5] [limit_pages_site = 300]
```
#### Backend
```shell
cd website/backend/build
./backend 8080
#[port]
```
#### OpenSearch: Permissions Denied

```shell
sudo chmod -R 777 /usr/share/opensearch/config
sudo chmod -R 777 /usr/share/opensearch/logs
```

## License
GNU General Public License v3.0

