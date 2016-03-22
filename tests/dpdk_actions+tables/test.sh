P4DPDK_OPTS=${P4DPDK_OPTS--c 0x3 -n 3 -- -p 0x3 --config \"\"}
MAKE_CMD=${MAKE_CMD-make}

${MAKE_CMD} clean && ${MAKE_CMD} -j16 && sudo ./build/switch ${P4DPDK_OPTS}
