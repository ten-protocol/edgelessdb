#!/bin/sh
set -e

mkdir -p /dev/sgx
if [ ! -L /dev/sgx/enclave ]; then
	ln -s /dev/sgx_enclave /dev/sgx/enclave
fi

PCCS_URL=https://global.acccache.azure.net/sgx/certification/v4/
echo "PCCS_URL: ${PCCS_URL}"

apt-get install -qq libsgx-dcap-default-qpl

echo "PCCS_URL=${PCCS_URL}\nUSE_SECURE_CERT=FALSE" > /etc/sgx_default_qcnl.conf

./edb "$@"
