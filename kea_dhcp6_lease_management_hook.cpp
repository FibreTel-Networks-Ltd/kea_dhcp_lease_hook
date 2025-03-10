#include <dhcpsrv/lease.h>
#include <hooks/hooks.h>
#include <dhcp/pkt6.h>
#include <dhcp/dhcp6.h>
#include <curl/curl.h>
#include <fstream>

using namespace isc::hooks;
using namespace isc::dhcp;

const std::string API_URL = "https://api.fibretel.ca/webhook/dhcp/ipv6";

extern "C" int lease4_select(CalloutHandle &handle);

extern "C"
{

  int version()
  {
    return KEA_HOOKS_VERSION;
  }

  bool multi_threading_compatible()
  {
    return true;
  }

  int load(isc::hooks::LibraryHandle &handle)
  {
    std::cout << "Loading Sonar API hook (UPDATED)" << std::endl;
    return 0;
  }

  int unload()
  {
    std::cout << "Unloading Sonar API hook" << std::endl;
    return 0;
  }

  std::string extract_link_layer_address(const std::vector<uint8_t> &duid)
  {
    // Ensure DUID is long enough (at least 8 bytes for DUID-LL, 10+ for DUID-LLT)
    if (duid.size() < 8)
    {
      return "invalid_duid";
    }

    // Check if it's a DUID-LL (Type 3) or DUID-LLT (Type 1)
    uint16_t duid_type = (duid[0] << 8) | duid[1]; // First two bytes = DUID type

    if (duid_type != 0x0003 && duid_type != 0x0001)
    {
      return "not_duid_ll_or_llt";
    }
    size_t mac_offset = (duid_type == 0x0003) ? 4 : 8; // DUID-LL starts at byte 4, DUID-LLT starts at 8

    if (duid.size() < mac_offset + 6) // Ensure at least 6 bytes for MAC
    {
      return "invalid_duid_length";
    }
    std::ostringstream mac;
    for (size_t i = mac_offset; i < mac_offset + 6; ++i)
    {
      if (i > mac_offset)
      {
        mac << ":";
      }
      mac << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(duid[i]);
    }

    return mac.str();
  }

  void make_api_call(const std::string &url, const std::string &leaseActIP, const std::string &leaseActMAC, int expired)
  {
    CURL *curl;
    CURLcode res;
    int attempts = 0;
    bool success = false;
    curl = curl_easy_init();
    if (curl)
    {
      std::ostringstream full_url;
      full_url << url << "?ip_address=" << leaseActIP
               << "&mac_address=" << leaseActMAC
               << "&expired=" << expired;

      curl_easy_setopt(curl, CURLOPT_URL, full_url.str().c_str());
      curl_easy_setopt(curl, CURLOPT_POST, 1L);
      res = curl_easy_perform(curl);

      if (res == CURLE_OK)
      {
        success = true;
        std::cout << "Successfully sent lease update to Sonar API." << std::endl;
      }

      curl_easy_cleanup(curl);
    }

    if (!success)
    {
      std::cerr << "Failed to send lease update, expired=" << expired << std::endl;
    }
  }

  int lease6_select(CalloutHandle &handle)
  {
    Lease6Ptr lease;
    handle.getArgument("lease6", lease); // Retrieve lease object

    Pkt6Ptr query;
    handle.getArgument("query6", query);

    if (query)
    {
      uint8_t message_type = query->getType();
      int messageType = static_cast<int>(message_type);
      if (messageType == DHCPV6_SOLICIT)
      {
        return 0;
      }
    }
    std::string leaseActIP = lease->addr_.toText();
    std::string leaseActMAC = extract_link_layer_address(lease->duid_->getDuid());
    std::cout << "KEA DHCPv6 lease select: " << leaseActIP << ", " << leaseActMAC << std::endl;
    make_api_call(API_URL, leaseActIP, leaseActMAC, 0);

    return 0;
  }

  int lease6_expire(CalloutHandle &handle)
  {
    Lease6Ptr lease;
    handle.getArgument("lease6", lease);
    std::string leaseActIP = lease->addr_.toText();
    std::string leaseActMAC = extract_link_layer_address(lease->duid_->getDuid());
    std::cout << "KEA DHCPv6 lease expire: " << leaseActIP << ", " << leaseActMAC << std::endl;
    make_api_call(API_URL, leaseActIP, leaseActMAC, 1);
    return 0;
  }
}