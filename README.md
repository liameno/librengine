![](images/logo.png)
#### Privacy Web Search Engine
## Website
![](images/demo.png)

## Features

#### Crawler
- Multithreading
- Cache
- Robots.txt
- Proxy
- Queue (BFS)
- Detect Trackers
- Http -> Https

#### Website / CLI
- Encryption (rsa)
- API
- Proxy
- Nodes
- Rating

```shell
cd scripts && sh install_deps.sh
```

## Build
```shell
cd scripts && sh build_all.sh
```

## Run

#### DB
```shell
mkdir /tmp/typesense-data &&
./typesense-server --data-dir=/tmp/typesense-data --api-key=xyz --enable-cors &&
sh scripts/init_db.sh
```

#### Crawler
```shell
./crawler ../../sites.txt 5 ../../config.json
#[sites_path] [threads_count] [config path]
```

#### Website
```shell
./website ../../config.json
#[config path]
```

#### CLI
###### Run website before!
```shell
./cli gnu 1 ../../config.json
#[query] [page] [config path]
```

## Instances
¯\\_(ツ)_/¯

## TODO
- [x] Encryption (assymetric)
- [x] Multithreading crawler
- [ ] Robots Rules (from headers & html) & crawl-delay
- [x] Responsive web design
- [ ] Own FTS ([...](https://github.com/liameno/kissearch))
- [ ] Images Crawler

## Dependencies
- libcurl   ([source](https://github.com/curl/curl))
- lexbor    ([source](https://github.com/lexbor/lexbor))
- typesense ([source](https://github.com/typesense/typesense))
- openssl   ([source](https://github.com/openssl/openssl))

## Config
./config.json

## Mirrors
https://github.com/liameno/librengine
https://codeberg.org/liameno/librengine

## License
GNU Affero General Public License v3.0
