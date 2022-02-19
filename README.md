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
- [ ]Robots in headers && html, crawl-delay
- [ ]Encryption (assymetric)
- [x]Site Rating
- [ ]CLI Search
- [x]API
- [ ]Export DB
- [ ]Images Crawler
- [ ]Admin Panel
- [ ]Adaptive Website

## Dependencies
Arch: `yay -S curl lexbor opensearch`
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

