# System76 ACPI Driver (DKMS)

This provides the system76_acpi in-tree driver for systems missing it.

## Building .deb from source
```bash
sudo dpkg-buildpackage  -b -uc -us
```

## Installing from built .deb
```bash
sudo dpkg -i ../system76-acpi-dkms_1.0.2_amd64.deb
```

## build, install and reload kernel module (for developing)
```bash
sudo dpkg-buildpackage  -b -uc -us && sudo dpkg -i ../system76-acpi-dkms_1.0.2_amd64.deb && sudo modprobe -r system76_acpi && sleep 5 && sudo modprobe system76_acpi
```
