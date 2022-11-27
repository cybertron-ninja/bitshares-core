#!/bin/bash
###############################################################################
# This script deploys the Cybertron Ninja REST API.
SCRIPTS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"

function output_usage () {
	echo ""
	echo "Usage: $0 mainnet|testnet"
	echo ""
}

if [[ -z $1 ]]; then
	output_usage
	exit 1
fi

if [[ "$1" != "mainnet" && "$1" != "testnet" ]]; then
	output_usage
	exit 1
fi

ENVIRONMENT="$1"
docker stack rm "bts-$1"
docker-compose -f "stack.$1.yaml" build
sleep 15
docker stack deploy -c "stack.$1.yaml" "bts-$1"

if [[ ! -z $2 ]]; then
	docker service logs -f -t "bts-$1_fullnode"
fi


