#!/bin/bash
set -e
SCRIPTS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"
echo "-------------------------------------------------------------------------"

if [[ ! -z "$BTS_CHAIN_NAME" && ! -z "$BTS_NODE_MODE" ]]; then
	# The following two ENVs are outputted to aid in trouble shooting, but
	# are not necessary, strictly speaking.
	echo "BTS_CHAIN_NAME=${BTS_CHAIN_NAME}"
	echo "BTS_NODE_MODE=${BTS_NODE_MODE}"
	SECRET_NAME="BTS_${BTS_CHAIN_NAME^^}_${BTS_NODE_MODE^^}_KEY"
	echo "setting BITSHARESD_PRIVATE_KEY from Docker secret: ${SECRET_NAME}"
	export BITSHARESD_PRIVATE_KEY=$(cat "/run/secrets/${SECRET_NAME}")
else
	echo "We will not use BITSHARESD_PRIVATE_KEY from Docker secret"
	echo "but, will get the private key from /etc/bitshares/config.ini"
	unset BITSHARESD_PRIVATE_KEY
fi

echo "Starting up container with the following command: "
echo "${@}"
echo "-------------------------------------------------------------------------"
#exec sleep 9999999999
exec "$@"