#include <dhcpsrv/lease.h>
#include <hooks/hooks.h>
#include <dhcp/pkt4.h>
#include <dhcp/dhcp4.h>
#include <curl/curl.h>
#include <fstream>

using namespace isc::hooks;
using namespace isc::dhcp;

const std::string API_URL = "https://api.fibretel.ca/webhook/dhcp";

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

  int lease4_select(CalloutHandle &handle)
  {
    Lease4Ptr lease;
    handle.getArgument("lease4", lease); // Retrieve lease object

    Pkt4Ptr query;
    handle.getArgument("query4", query);

    if (query)
    {
      uint8_t message_type = query->getType();
      int messageType = static_cast<int>(message_type);
      if (messageType == 1)
      {
        return 0;
      }
    }
    std::string leaseActIP = lease->addr_.toText();
    std::string leaseActMAC = lease->hwaddr_->toText(false);

    make_api_call(API_URL, leaseActIP, leaseActMAC, 0);
    std::cout << "lease4_select: " << leaseActIP << ", " << leaseActMAC << "\n";
    return 0;
  }

  int lease4_expire(CalloutHandle &handle)
  {
    Lease4Ptr lease;
    handle.getArgument("lease4", lease);

    std::string leaseActIP = lease->addr_.toText();
    std::string leaseActMAC = lease->hwaddr_->toText(false);

    std::cout << "lease4_expire: " << leaseActIP << ", " << leaseActMAC << std::endl;
    make_api_call(API_URL, leaseActIP, leaseActMAC, 1);

    return 0;
  }
}