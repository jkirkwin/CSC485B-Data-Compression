#!/bin/bash
CATCH_URL=https://github.com/catchorg/Catch2/releases/download/v2.12.2/catch.hpp

mkdir -p catch

if [ ! -f "catch/catch.hpp" ]; then
    cd catch && wget $CATCH_URL
fi

