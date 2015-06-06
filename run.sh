main() {
  while true; do
    clear
    echo ''
    echo ''
    echo ''
    echo ''
    echo ''
    echo ''
    echo ''
    echo ''
    echo ''
    ./build/main &
    pid=$!
    cat reload
    kill $pid
  done
}

main
