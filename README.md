# Kea DHCP Lease Hook
This repository contains a custom Kea DHCPv4 hook for processing DHCP lease assignments and events.

**Prerequisites**: Before installing the Kea DHCP lease hook, ensure you have Kea DHCP installed.


## 🛠 Installation

### **1. Clone This Repository**
```sh
git clone https://github.com/FibreTel-Networks-Ltd/kea_dhcp_lease_hook.git
cd kea_dhcp_lease_hook
```

### **2. Build and Install the Hook**
```sh
sudo g++ -fPIC -shared -o /usr/lib/fibretel/webhook.so kea_dhcp4_lease_management_hook.cpp  -lcurl -lkea-hooks -lkea-dhcpsrv -I/usr/include/kea
```