
ARCH=pcap

rm -rf build

make -f Makefile_pcap.mk && \
(cd build; mkdir -p output; ./example_pcap1 ../examples/table_config_$ARCH.cfg ../examples/captures/ipv4_cipso_option.pcap 1000 1001 1002) && \
echo "Test execution finished."
