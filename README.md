# Kea DHCP Lease Hook

This repository contains a **custom Kea DHCPv4/v6 hook** for processing DHCP lease assignments and events.
It is designed to integrate with [Kea DHCP](https://kea.isc.org/) and trigger API calls when leases are assigned, renewed, expired, or released.

---

## 📋 Prerequisites

Before installing this hook, ensure you have:

* **Kea DHCP** installed (v4 and/or v6 server as needed)
* **g++** (GNU C++ compiler)
* **libcurl** development libraries
* Kea development headers (`kea-dev` package on some systems)

---

## 🛠 Installation

### 1. Clone the Repository

```bash
git clone https://github.com/FibreTel-Networks-Ltd/kea_dhcp_lease_hook.git
cd kea_dhcp_lease_hook
```

---

### 2. Build and Install the Hook

#### **DHCPv4 Hook**

```bash
sudo g++ -o /usr/local/lib/kea/hooks/sonar_api/sonar_api_hook4.so -shared -fPIC kea_dhcp4_lease_management_hook.cpp -I/usr/local/include/kea/ -lcurl -lpthread
```

#### **DHCPv6 Hook**

```bash
sudo g++ -o /usr/local/lib/kea/hooks/sonar_api/sonar_api_hook6.so -shared -fPIC kea_dhcp6_lease_management_hook.cpp -I/usr/local/include/kea/ -lcurl -lpthread
```

---

### 3. Configure Kea to Use the Hook

Edit your Kea configuration file (e.g., `/etc/kea/kea-dhcp4.conf` or `/etc/kea/kea-dhcp6.conf`) and add the hook library path under the `hooks-libraries` section:

```json
"hooks-libraries": [
    {
        "library": "/usr/local/lib/kea/hooks/sonar_api/sonar_api_hook4.so"
    }
]
```

For DHCPv6, replace with:

```json
"hooks-libraries": [
    {
        "library": "/usr/local/lib/kea/hooks/sonar_api/sonar_api_hook6.so"
    }
]
```

---

### 4. Restart Kea Service

```bash
# Restart DHCPv4
sudo systemctl restart isc-kea-dhcp4-server

# Restart DHCPv6
sudo systemctl restart isc-kea-dhcp6-server
```


## 🧪 Testing

After restarting Kea, monitor logs to confirm the hook is loading correctly:

```bash
sudo journalctl -u isc-kea-dhcp4-server -f
```

or

```bash
sudo journalctl -u isc-kea-dhcp6-server -f
```