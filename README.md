### New version: https://github.com/sightnet
This project will not be maintained.

### ==========
### ==========

<br>
<br>

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


## Usage (Docker)

Please run the build every time to change the arguments. <br>
The site is launched by default on port 8080 AND with tor proxy (<b>!!!</b>), to edit it you need to change config.json and rebuild website. <br>
The api key for the database must be changed in the config and when the database is started(--api-key).

#### DB - please run before using other
```shell
sudo docker pull typesense/typesense:0.24.0.rcn6
mkdir /tmp/typesense-data
sudo docker run -p 8108:8108 -v/tmp/data:/data typesense/typesense:0.24.0.rcn6 --data-dir /data --api-key=xyz
```

#### Crawler
```shell
sudo docker-compose build crawler --build-arg SITES="$(cat sites.txt)"  --build-arg THREADS=1 --build-arg CONFIG="$(cat config.json)"
sudo docker-compose up crawler
```

#### Website
```shell
sudo docker-compose build website --build-arg CONFIG="$(cat config.json)"
sudo docker-compose up website
```

## Usage (Manual)

## Deps
```shell
cd scripts && sh install_deps.sh
```

## Build
```shell
cd scripts && sh build_all.sh
```

## Run

The site is launched by default on port 8080 AND with tor proxy (<b>!!!</b>), to edit it you need to change config.json. <br>
The api key for the database must be changed in the config and when the database is started(--api-key).

#### DB - please run before using other
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

## Instances
¯\\_(ツ)_/¯

## TODO
- [x] Docker
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
- https://github.com/liameno/librengine
- https://codeberg.org/liameno/librengine

## License
GNU Affero General Public License v3.0
