find ./include -iname "*.h" | xargs clang-format -style=file -i
find ./src -iname "*.cc" | xargs clang-format -style=file -i
find ./test -iname "*.cc" | xargs clang-format -style=file -i