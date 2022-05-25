# ![](https://raw.githubusercontent.com/liameno/librengine/master/logo.png)
#### Privacy Web Search Engine
## Website
[![https://raw.githubusercontent.com/liameno/librengine/master/preview.gif](https://raw.githubusercontent.com/liameno/librengine/master/demo.png)]()

## Features
#### Crawler
- Cache
- Robots.txt
- Update info after time
- Proxy
- Queue (BFS)
- Detect trackers
- Http to https
- Normalize url (remove #fragment, ?query)

#### Website / CLI
- Encryption (rsa)
- API
- Proxy
- Node Info
- Nodes
- Rating (min=0, max=200, def=100)

## TODO
- [x] Encryption (assymetric)
- [ ] Robots Rules (from headers & html) & crawl-delay
- [ ] Images Crawler
- [ ] Adaptive Website

## Dependencies
- libcurl   ([source](https://github.com/curl/curl))
- lexbor    ([source](https://github.com/lexbor/lexbor))
- typesense ([source](https://github.com/typesense/typesense))
- openssl   ([source](https://github.com/openssl/openssl))

Arch: 
```shell
yay -S curl lexbor openssl &&
curl -O https://dl.typesense.org/releases/0.23.0.rc20/typesense-server-0.23.0.rc20-linux-amd64.tar.gz &&
tar -xzf typesense-server-0.23.0.rc20-linux-amd64.tar.gz
```
Debian: 
```shell
sudo apt install libcurl4-openssl-dev &&
curl -O https://dl.typesense.org/releases/0.23.0.rc20/typesense-server-0.23.0.rc20-linux-amd64.tar.gz &&
tar -xzf typesense-server-0.23.0.rc20-linux-amd64.tar.gz &&
git clone https://github.com/lexbor/lexbor && 
cd lexbor &&
cmake . && make && sudo make install &&
sudo apt install libssl-dev
```
## Build
```shell
#git clone ...
cd librengine &&
sh scripts/build_all.sh
```
## Run
```shell
mkdir /tmp/typesense-data &&
./typesense-server --data-dir=/tmp/typesense-data --api-key=xyz --enable-cors &&
sh scripts/init_db.sh
```
#### CLI
```shell
./cli gnu 1 ../../config.json
#[query] [page] [config path]
```
#### Crawler
```shell
./crawler https://www.gnu.org ../../config.json
#[start_site] [config path]
```
#### Website
```shell
./website ../../config.json
#[config path]
```
## Config 
```json
//proxy: type://ip:port OR empty ("")
//socks5://127.0.0.1:9050

//_s - seconds

{
  "global": {
    //edit also website/frontend/js/search_encrypt.js
    "rsa_key_length": 1024, //1024|2048|4096
    "max_title_show_size": 55,
    "max_desc_show_size": 350,
    "nodes": [
      {
        "name": "This",
        "url": "http://127.0.0.1:8080"
      }
    ]
  },
  "crawler": {
    "user_agent": "librengine",
    "proxy": "socks5://127.0.0.1:9050",
    "load_page_timeout_s": 10,
    "update_time_site_info_s_after": 864000, //10 days
    "delay_time_s": 3, 
    "max_pages_site": 5,
    "max_page_symbols": 50000000, //50mb
    "max_robots_txt_symbols": 3000,
    "max_lru_cache_size_host": 512,
    "max_lru_cache_size_url": 512,
    "is_http_to_https": true,
    "is_check_robots_txt": true
  },
  "cli": {
    "proxy": "socks5://127.0.0.1:9050"
  },
  "website": {
    "port": 8080,
    "proxy": "socks5://127.0.0.1:9050"
  },
  //edit also init_db.sh
  "db": {
    "url": "http://localhost:8108",
    "api_key": "xyz"
  }
}

```

## License
GNU AFFERO GENERAL PUBLIC LICENSE v3.0
