#cd lib
#mkdir build && cd build && cmake .. && sudo make install

function build() {
  cd "$1" || exit
  mkdir build
  cd build
  cmake ..
  make
  pwd
}

current_directory=$(pwd)

case "$current_directory" in
  *scripts*)
    echo ""
    ;;
  *)
    echo "You must run it from scripts directory"
    exit 1
esac

build "../crawler"
build "../../website"
build "../../cli"