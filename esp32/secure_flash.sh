echo "** idf.py build is not run automatically, ensure files are up to date **"


echo "BURN" | espefuse.py --port $1 burn_key_digest secure_boot_2_signing_key.pem
echo "BURN" | espefuse.py --port $1 burn_key flash_encryption flash_encryption_key.bin


/home/einstein/.espressif/python_env/idf5.0_py3.10_env/bin/python  /home/einstein/src/nsec/nsec-badge-controller-screen/esp-idf/components/esptool_py/esptool/esptool.py --chip esp32 --port=$1 --before=default_reset --after=no_reset write_flash --flash_mode dio --flash_freq 40m --flash_size keep 0x1000 /home/einstein/src/nsec/nsec-badge-controller-screen/esp32/build/bootloader/bootloader.bin
#idf.py encrypted-flash monitor
/home/einstein/.espressif/python_env/idf5.0_py3.10_env/bin/python ../esp-idf/components/esptool_py/esptool/esptool.py -p $1 -b 460800 --before default_reset --after no_reset --chip esp32  write_flash --flash_mode dio --flash_size keep --flash_freq 40m 0x10000 build/partition_table/partition-table.bin 0x20000 build/nsec-badge-controller-screen.bin


idf.py monitor
