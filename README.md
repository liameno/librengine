Librengine

![GitHub top language](https://img.shields.io/github/languages/top/liameno/librengine) ![GitHub](https://img.shields.io/github/license/liameno/librengine)

Privacy Opensource Web Search Engine
## Website
[![https://raw.githubusercontent.com/liameno/librengine/master/preview.gif](https://raw.githubusercontent.com/liameno/librengine/master/preview.gif)]()
## Donate to web-hosting
| Cryptocurrency | Address |
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
- opensearch	(https://www.opensearch.org/)
- openssl 	(https://www.openssl.org/)

Arch: 
```shell
yay -S curl lexbor opensearch openssl
```
Debian: 
```shell
sudo apt install libcurl4-openssl-dev &&
wget https://artifacts.opensearch.org/releases/bundle/opensearch/1.2.4/opensearch-1.2.4-linux-x64.tar.gz &&
tar -zxf opensearch-1.2.4-linux-x64.tar.gz && cd opensearch-1.2.4 && 
./opensearch-tar-install.sh &&
git clone https://github.com/lexbor/lexbor && 
cd lexbor &&
cmake . && make && sudo make install &&
sudo apt install libssl-dev
```
## Build
```shell
git clone https://github.com/liameno/librengine
cd librengine
sh build_all.sh
```
## Run
```shell
opensearch
sh set_opensearch.sh
```
#### Crawler
```shell
./crawler https://www.gnu.org ../../config.json
#[start_site] [config path]
```
#### Backend
```shell
./backend ../../config.json
#[config path]
```
## Config 
```json
//proxy: type://ip:port
//socks5://127.0.0.1:9050

{
  "crawler": {
    "user_agent": "librengine",
    "opensearch_url": "http://localhost:9200",
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
  }
}
```

#### OpenSearch: Permissions Denied

```shell
sudo chmod -R 777 /usr/share/opensearch/config
sudo chmod -R 777 /usr/share/opensearch/logs
```

## License
GNU General Public License v3.0
