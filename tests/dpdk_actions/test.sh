P4DPDK_OPTS=${P4DPDK_OPTS--c 0x1 -n 1 -- -p 0x1 --config \"\"}
MAKE_CMD=${MAKE_CMD-make}

${MAKE_CMD} clean && ${MAKE_CMD} -j16 && sudo ./build/actions ${P4DPDK_OPTS}
