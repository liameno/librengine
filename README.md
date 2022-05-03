# ![](https://raw.githubusercontent.com/liameno/librengine/master/logo.png)
#### Privacy Web Search Engine
## Website
[![https://raw.githubusercontent.com/liameno/librengine/master/preview.gif](https://raw.githubusercontent.com/liameno/librengine/master/demo.png)]()
## Donate to web-hosting
| Сurrency | Address |
| --- | --- |
| Bitcoin (BTC) | bc1qxpu9vfzah3vw5pzanny0zmfsgd64klcj24pa8x |
| Dogecoin (DOGE) | DM8cqzbrW2rrmGk4K6UCD7rfeoqnKjJTum |
| Ethereum (ETH)| 0x1857A1A7a543ED123151ACCAbBF4EB058741e614 |
| Litecoin (LTC) | LLQMiWpF1cxET7p7UMYoWjJ26JuTp14u8K |
| Monero (XMR) | 4AkPUBr4uoFV1K4fSitpGJjRHo4dfSzZ257YR9HxiQi3DvmgLW1rteRQfRRCFYytKugcygfHAvvJu3Tt96mSoVUE6JKJDZL |

## Features
-  Crawler
	-  Proxy
	-  Http To Https
	-  Robots Txt...
-  Website
	-  Encryption RSA (if js is enabled)
	-  API...

## TODO
- [x] Encryption (assymetric)
- [ ] Robots Rules from headers && html, crawl-delay
- [ ] Images Crawler
- [ ] Adaptive Website

## Dependencies
- libcurl 	(https://github.com/curl/curl)
- lexbor	(https://github.com/lexbor/lexbor)
- typesense	(https://typesense.org)
- openssl 	(https://www.openssl.org)

Arch: 
```shell
yay -S curl lexbor openssl &&
wget https://dl.typesense.org/releases/0.22.2/typesense-server-0.22.2-linux-amd64.tar.gz &&
tar -zxf typesense-server-0.22.2-linux-amd64.tar.gz
```
Debian: 
```shell
sudo apt install libcurl4-openssl-dev &&
wget https://dl.typesense.org/releases/0.22.2/typesense-server-0.22.2-linux-amd64.tar.gz &&
tar -zxf typesense-server-0.22.2-linux-amd64.tar.gz &&
git clone https://github.com/lexbor/lexbor && 
cd lexbor &&
cmake . && make && sudo make install &&
sudo apt install libssl-dev
```
## Build
```shell
git clone https://github.com/liameno/librengine &&
cd librengine &&
sh scripts/build_all.sh
```
## Run
```shell
./typesense-server --data-dir=/tmp/typesense-data --api-key=xyz --enable-cors &&
sh scripts/init_db.sh
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
  "crawler": {
    "user_agent": "librengine",
    "proxy": "socks5://127.0.0.1:9050",
    "load_page_timeout_s": 20,
    "update_time_site_info_s_after": 86400, //10 days
    "delay_time_s": 3, 
    "max_recursive_deep": 2,
    "max_pages_site": 1,
    "max_page_symbols": 50000000, //50mb
    "max_robots_txt_symbols": 3000,
    "is_one_site": false,
    "is_http_to_https": true,
    "is_check_robots_txt": true
  },
  "cli": {
    //local=0 | nodes=1(website/backend)
    "mode": 0
  },
  "website": {
    "port": 8080,
    "proxy": "socks5://127.0.0.1:9050",
    "nodes": [ {
        "name": "This",
        "url": "http://127.0.0.1:8080"
      }
    ]
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
