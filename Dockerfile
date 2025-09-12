FROM testnetobscuronet.azurecr.io/obscuronet/ten-edb-build-base:v0.3.4 AS build

COPY . /edgelessdb

# download 3rd party dependencies (mariadb & rocksdb)
RUN cd edgelessdb &&  git submodule update --init --recursive

# comment out some deprecated RND initialisation
RUN cd edgelessdb/3rdparty/edgeless-rocksdb && ls -ll
RUN sed -i '37,49s/^/\/\/ /' /edgelessdb/3rdparty/edgeless-rocksdb/libedgeless/src/crypto.cc

# build edb
RUN cd edgelessdb && export SOURCE_DATE_EPOCH=$(git log -1 --pretty=%ct)  \
  && go mod vendor \
  && go mod tidy \
  && cd /edbbuild \
  && . /opt/edgelessrt/share/openenclave/openenclaverc \
  && cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF \
    -DCMAKE_C_FLAGS="-Wno-deprecated-declarations" \
    -DCMAKE_CXX_FLAGS="-Wno-deprecated-declarations -Wno-other" \
    /edgelessdb \
  && make -j1 VERBOSE=1  edb-enclave

# Run the complete Azure HSM setup (builds signer tool, signs binary, or skips if not needed)
RUN /edgelessdb/tools/enclavesigner/AzureHSMSignatureScript.sh /edbbuild/edb-enclave.signed /edgelessdb/tools/enclavesigner/main

ARG HEAP_SIZE
ARG MAX_THREADS
ARG PRODUCTION

RUN --mount=type=secret,id=signingkey,dst=/edbbuild/private.pem,required=true \
  cd edbbuild \
  && . /opt/edgelessrt/share/openenclave/openenclaverc \
  && cmake -DHEAPSIZE=$HEAP_SIZE -DNUMTCS=$MAX_THREADS -DPRODUCTION=$PRODUCTION \
    -DCMAKE_C_FLAGS="-Wno-deprecated-declarations" \
    -DCMAKE_CXX_FLAGS="-Wno-deprecated-declarations" \
    /edgelessdb \
  && make sign-edb \
  && cat edgelessdb-sgx.json

# deploy
FROM --platform=linux/amd64 ubuntu:jammy-20250819
RUN apt-get update && apt-get install -y --no-install-recommends \
  ca-certificates \
  gnupg \
  libcurl4 \
  wget \
  && wget -qO- https://download.01.org/intel-sgx/sgx_repo/ubuntu/intel-sgx-deb.key | apt-key add \
  && echo 'deb [arch=amd64] https://download.01.org/intel-sgx/sgx_repo/ubuntu jammy main' >> /etc/apt/sources.list \
  && apt-get update \
  && apt-get install -y --no-install-recommends \
    libsgx-dcap-default-qpl \
    libsgx-enclave-common \
    libsgx-urts \
    libsgx-dcap-ql \
  && rm -rf /var/lib/apt/lists/*

COPY --from=build /edbbuild/edb /edbbuild/edb-enclave.signed /edbbuild/edgelessdb-sgx.json /edgelessdb/src/entry.sh /
COPY --from=build /opt/edgelessrt/bin/erthost /opt/edgelessrt/bin/
ENV PATH=${PATH}:/opt/edgelessrt/bin
ENTRYPOINT ["/entry.sh"]
EXPOSE 3306 8080
