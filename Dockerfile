# The image for building
FROM phusion/baseimage:focal-1.2.0 as build
ENV LANG=en_US.UTF-8
ARG MAKE_JOBS=${MAKE_JOBS:-1}

# Install dependencies
RUN \
    apt-get update && \
    apt-get upgrade -y -o Dpkg::Options::="--force-confold" && \
    apt-get update && \
    apt-get install -y \
      g++ \
      autoconf \
      cmake \
      git \
      libbz2-dev \
      libcurl4-openssl-dev \
      libssl-dev \
      libncurses-dev \
      libboost-thread-dev \
      libboost-iostreams-dev \
      libboost-date-time-dev \
      libboost-system-dev \
      libboost-filesystem-dev \
      libboost-program-options-dev \
      libboost-chrono-dev \
      libboost-test-dev \
      libboost-context-dev \
      libboost-regex-dev \
      libboost-coroutine-dev \
      libtool \
      doxygen \
      ca-certificates \
    && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

ADD . /bitshares-core
WORKDIR /bitshares-core

# Compile
RUN \
    ( git submodule sync --recursive || \
		find `pwd`  -type f -name .git | \
	while read f; do \
		rel="$(echo "${f#$PWD/}" | sed 's=[^/]*/=../=g')"; \
		sed -i "s=: .*/.git/=: $rel/=" "$f"; \
	done && \
		git submodule sync --recursive ) && \
    git submodule update --init --recursive && \
    cmake \
        -DCMAKE_BUILD_TYPE=Release \
		-DGRAPHENE_DISABLE_UNITY_BUILD=ON \
        . && \
    make witness_node cli_wallet get_dev_key && \
    install -s programs/witness_node/witness_node \
        programs/genesis_util/get_dev_key \
        programs/cli_wallet/cli_wallet \
        /usr/local/bin && \
    #
    # Obtain version
    mkdir -p /etc/bitshares && \
    git rev-parse --short HEAD > /etc/bitshares/version && \
    cd / && \
    rm -rf /bitshares-core

################################################################################
# The final image
FROM phusion/baseimage:focal-1.2.0
LABEL maintainer="The bitshares decentralized organisation"
ENV LANG=en_US.UTF-8

ARG BTS_USER=${BTS_USER:-btsnode}
ENV BTS_USER=${BTS_USER}
ARG BTS_USER_ID=${BTS_USER_ID:-10000}
ARG BTS_GROUP_ID=${BTS_GROUP_ID:-10001}
ARG HOME=${HOME:-/var/lib/bitshares}
ENV HOME=${HOME}
ARG SCRIPTS_DIR=${SCRIPTS_DIR:-/usr/local/bin}
ENV SCRIPTS_DIR=${SCRIPTS_DIR}

# Install required libraries
RUN \
    apt-get update && \
    apt-get upgrade -y -o Dpkg::Options::="--force-confold" && \
    apt-get update && \
    apt-get install --no-install-recommends -y \
      libcurl4 \
      ca-certificates \
    && \
    mkdir -p /etc/bitshares && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

COPY --from=build /usr/local/bin/* /usr/local/bin/
COPY --from=build /etc/bitshares/version /etc/bitshares/

WORKDIR /
RUN groupadd -f -g ${BTS_GROUP_ID} ${BTS_USER}
RUN	useradd -r \
	-g ${BTS_USER} \
	--uid $BTS_USER_ID \
	--home-dir "${HOME}" \
	--no-create-home \
#	-s /bin/bash \
	-s /usr/sbin/nologin \
	--no-log-init \
	${BTS_USER}
#RUN useradd -u 10000 -g bitshares -s /bin/bash -m -d /var/lib/bitshares --no-log-init bitshares
#RUN chown bitshares:bitshares -R /var/lib/bitshares
RUN chown -R ${BTS_USER}:${BTS_USER} ${HOME}

# default exec/config files
COPY docker/configs/default_config.ini /etc/bitshares/config.ini
COPY docker/configs/default_logging.ini /etc/bitshares/logging.ini
COPY docker/scripts/bitsharesentry.sh /usr/local/bin/bitsharesentry.sh
RUN chmod a+x /usr/local/bin/bitsharesentry.sh

# copy the new docker-entrypoint script and make it executable
COPY docker/scripts/docker-entrypoint.sh /usr/local/bin/docker-entrypoint.sh
RUN chmod a+x /usr/local/bin/docker-entrypoint.sh

# Volume
VOLUME ["/var/lib/bitshares", "/etc/bitshares"]

# rpc service:
EXPOSE 8091
# p2p service:
EXPOSE 9091

# Make Docker send SIGINT instead of SIGTERM to the daemon
STOPSIGNAL SIGINT

# Temporarily commented out due to permission issues caused by older versions, to be restored in a future version
#USER bitshares:bitshares
USER ${BTS_USER}

# If we use a ENTRYPOINT + CMD it allows for more flexibility in how we
# bring up the instance using either ENV variables or Docker secrets.
# For more info on using ENTRYPOINT + CMD together
# see: https://docs.docker.com/engine/reference/builder/#understand-how-cmd-and-entrypoint-interact
CMD ["/usr/local/bin/bitsharesentry.sh"]
ENTRYPOINT ["/usr/local/bin/docker-entrypoint.sh"]
