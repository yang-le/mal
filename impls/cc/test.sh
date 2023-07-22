#! /bin/bash

cd ../..

make "test^cc"
make MAL_IMPL=cc "test^mal"

make DOCKERIZE=1 "test^cc"
make DOCKERIZE=1 MAL_IMPL=cc "test^mal"

./ci.sh build cc
./ci.sh test cc

cd impls/tests

tests=`ls lib`
tests=${tests[*]/memoize.mal}
tests=${tests[*]/protocols.mal}

for t in $tests; do
    python ../../runtest.py lib/$t ../cc/run
    if [ $? -ne 0 ]; then
        exit
    fi
done

python ../../runtest.py --test-timeout 100 lib/memoize.mal ../cc/run
