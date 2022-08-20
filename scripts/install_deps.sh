source /etc/os-release

case "$ID" in
*arch* | *artix*)
  yay -Sy curl lexbor openssl &&
  curl -O https://dl.typesense.org/releases/0.23.0.rc20/typesense-server-0.23.0.rc20-linux-amd64.tar.gz &&
  tar -xzf typesense-server-0.23.0.rc20-linux-amd64.tar.gz
  echo "Done"
  ;;
*debian* | *ubuntu*)
  sudo apt install libcurl4-openssl-dev &&
  curl -O https://dl.typesense.org/releases/0.23.0.rc20/typesense-server-0.23.0.rc20-linux-amd64.tar.gz &&
  tar -xzf typesense-server-0.23.0.rc20-linux-amd64.tar.gz &&
  git clone https://github.com/lexbor/lexbor &&
  cd lexbor &&
  cmake . && make && sudo make install &&
  sudo apt install libssl-dev
  echo "Done"
  ;;
*)
  echo "Not supported;
  Install:
    curl,
    lexbor(https://github.com/lexbor/lexbor),
    openssl,
    libcurl,
    typesense-server(https://dl.typesense.org/releases/0.23.0.rc20/typesense-server-0.23.0.rc20-linux-amd64.tar.gz)"
  exit 1
esac
