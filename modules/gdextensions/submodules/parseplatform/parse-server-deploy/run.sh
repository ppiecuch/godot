#!/bin/bash

set -e

env="local"

if [ ! "$(docker ps -q -f name=db-mongo)" ]; then
	docker run --name db-mongo -d mongo
fi

if [ ! "$(docker ps -q -f name=${env}-parse-server)" ]; then
	if [ "$(docker ps -aq -f status=exited -f name=${env}-parse-server)" ]; then
		docker rm ${env}-parse-server
	fi
	docker run --name ${env}-parse-server -v ${PWD}/${env}/config:/parse-server/config -v ${PWD}/${env}/cloud:/parse-server/cloud -p 1337:1337 --link db-mongo:mongo -d parse-server --appId $(echo $env | tr '[:lower:]' '[:upper:]')_APP_ID --masterKey MasterAccessKey --databaseURI mongodb://mongo/test --cloud /parse-server/cloud/main.js
else
	echo "${env}-parse-server already running."
	exit 1
fi
