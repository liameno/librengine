# ![](https://raw.githubusercontent.com/liameno/librengine/master/logo.png)
#### Privacy Web Search Engine
## Website
[![https://raw.githubusercontent.com/liameno/librengine/master/preview.gif](https://raw.githubusercontent.com/liameno/librengine/master/demo.png)]()

## Features
-  Crawler
	-  Proxy...
	-  Robots...
-  Website
	-  Encryption RSA (if js is enabled)
	-  API...

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
//proxy: type://ip:port
//socks5://127.0.0.1:9050

//_s - seconds

{
  "global": {
    "nodes": [
      {
        "name": "This",
        "url": "http://127.0.0.1:8080"
      }
    ]
  },
  "crawler": {
    "user_agent": "librengine", //user agent for robots.txt
    "proxy": "socks5://127.0.0.1:9050",
    "load_page_timeout_s": 20, //http timeout
    "update_time_site_info_s_after": 86400, //10 days
    "delay_time_s": 3, //delay between http requests
    "max_recursive_deep": 2,
    "max_pages_site": 1, //max urls for host
    "max_page_symbols": 50000000, //50mb
    "max_robots_txt_symbols": 3000,
    "is_one_site": false, //only one host
    "is_http_to_https": true, //http -> https
    "is_check_robots_txt": true //check robots.txt
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
