echo "** idf.py build is not run automatically, ensure files are up to date **"

espsecure.py encrypt_flash_data --keyfile flash_encryption_key.bin --address 0x10000 --output build/partition_table/partition-table-encrypted.bin build/partition_table/partition-table.bin
espsecure.py encrypt_flash_data --keyfile flash_encryption_key.bin --address 0x20000 --output build/nsec-badge-controller-screen-encrypted.bin build/nsec-badge-controller-screen.bin

/home/einstein/.espressif/python_env/idf5.0_py3.10_env/bin/python ../esp-idf/components/esptool_py/esptool/esptool.py -p $1 -b 460800 --before default_reset --after no_reset --chip esp32  write_flash --force --flash_mode dio --flash_size keep --flash_freq 40m 0x10000 build/partition_table/partition-table-encrypted.bin 0x20000 build/nsec-badge-controller-screen-encrypted.bin
