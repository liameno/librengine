## Librengine
![GitHub top language](https://img.shields.io/github/languages/top/liameno/librengine) ![GitHub](https://img.shields.io/github/license/liameno/librengine)

Private Opensource Web Search Engine
## Website
[![https://raw.githubusercontent.com/liameno/librengine/master/preview.gif](https://raw.githubusercontent.com/liameno/librengine/master/preview.gif)]()
## Features
-  Crawler
	-  Http/Socks proxy (optional)
	-  DeepMax
	-  Timeout
	-  Delay
	-  UserAgent
	-  Limits
	-  Http To Https (optional)
	-  Robots Txt (optional)
-  Website
	-  Search
	-  Site Rating
	-  Without Js
	-  API
	-  Node Info

## TODO
- [ ] Robots in headers && html, crawl-delay
- [ ] Encryption (assymetric)
- [x] Site Rating
- [ ] CLI Search
- [x] API
- [ ] Export DB
- [ ] Images Crawler
- [ ] Admin Panel
- [ ] Adaptive Website

## Dependencies
- libcurl 		(https://github.com/curl/curl)
- lexbor		(https://github.com/lexbor/lexbor)
- opensearch	(https://www.opensearch.org/)

Arch: 
```shell
yay -S curl lexbor opensearch
```
Debian: 
```shell
sudo apt install libcurl4-openssl-dev &&
wget https://artifacts.opensearch.org/releases/bundle/opensearch/1.2.4/opensearch-1.2.4-linux-	x64.tar.gz &&
tar -zxf opensearch-1.2.4-linux-x64.tar.gz && cd opensearch-1.2.4 && 
./opensearch-tar-install.sh &&
git clone https://github.com/lexbor/lexbor && 
cd lexbor &&
cmake . && make && sudo make install
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
cd crawler/build
./crawler https://www.gnu.org ../../config.json
#[start_site] [config path]
```
#### Backend
```shell
cd website/backend/build
./backend ../../config.json
#[config path]
```
## Config 
```json
{
  "crawler": {
    "user_agent": "librengine",
    "opensearch_url": "http://localhost:9200",
    //type://ip:port
    "proxy": "socks5://127.0.0.1:9050",
    "load_page_timeout_s": 10,
    "update_time_site_info_s_after": 86400,
    "delay_time_s": 3, 
    "max_recursive_deep": 3,
    "max_pages_site": 3,
    "max_page_symbols": 50000000,
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
    //type://ip:port
    "proxy": "",
    "nodes": [ {
        "name": "This",
        "url": "http://127.0.0.1:8080"
      }, {
        "name": "2",
        "url": "http://127.0.0.1:1356"
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

