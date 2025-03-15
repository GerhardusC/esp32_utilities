#!/bin/bash

cd ./api && cargo build --release && cd ../

cd ./data_collection && cargo build --release && cd ../

mkdir out;

cp ./api/target/release/local_data_server ./out/server;
cp ./data_collection/target/release/mqttinrust ./out/collector;