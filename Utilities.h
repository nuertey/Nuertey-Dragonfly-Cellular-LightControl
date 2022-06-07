/***********************************************************************
* @file
*
* A miscellany of utilities for programming ARM Mbed-enabled targets.
*       
* @note     Quiet Thought is the Mother of Innovation. 
* 
* @warning  
* 
*  Created: May 7, 2022
*   Author: Nuertey Odzeyem        
************************************************************************/
#pragma once

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <ctime>
#include <cctype>
#include <climits>
#include <cstdio>
#include <cstring>
#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <cassert>
#include <memory>
#include <utility> 
#include <type_traits>
#include <algorithm>
#include <functional>
#include <optional>
#include <map>
#include <string>

#include "nsapi_types.h"

using ErrorCodesMap_t = std::map<nsapi_error_t, std::string>;
using IndexElement_t  = ErrorCodesMap_t::value_type;

inline static auto make_error_codes_map()
{
    ErrorCodesMap_t eMap;
    
    eMap.insert(IndexElement_t(NSAPI_ERROR_OK, std::string("\"no error\"")));
    eMap.insert(IndexElement_t(NSAPI_ERROR_WOULD_BLOCK, std::string("\"no data is not available but call is non-blocking\"")));
    eMap.insert(IndexElement_t(NSAPI_ERROR_UNSUPPORTED, std::string("\"unsupported functionality\"")));
    eMap.insert(IndexElement_t(NSAPI_ERROR_PARAMETER, std::string("\"invalid configuration\"")));
    eMap.insert(IndexElement_t(NSAPI_ERROR_NO_CONNECTION, std::string("\"not connected to a network\"")));
    eMap.insert(IndexElement_t(NSAPI_ERROR_NO_SOCKET, std::string("\"socket not available for use\"")));
    eMap.insert(IndexElement_t(NSAPI_ERROR_NO_ADDRESS, std::string("\"IP address is not known\"")));
    eMap.insert(IndexElement_t(NSAPI_ERROR_NO_MEMORY, std::string("\"memory resource not available\"")));
    eMap.insert(IndexElement_t(NSAPI_ERROR_NO_SSID, std::string("\"ssid not found\"")));
    eMap.insert(IndexElement_t(NSAPI_ERROR_DNS_FAILURE, std::string("\"DNS failed to complete successfully\"")));
    eMap.insert(IndexElement_t(NSAPI_ERROR_DHCP_FAILURE, std::string("\"DHCP failed to complete successfully\"")));
    eMap.insert(IndexElement_t(NSAPI_ERROR_AUTH_FAILURE, std::string("\"connection to access point failed\"")));
    eMap.insert(IndexElement_t(NSAPI_ERROR_DEVICE_ERROR, std::string("\"failure interfacing with the network processor\"")));
    eMap.insert(IndexElement_t(NSAPI_ERROR_IN_PROGRESS, std::string("\"operation (eg connect) in progress\"")));
    eMap.insert(IndexElement_t(NSAPI_ERROR_ALREADY, std::string("\"operation (eg connect) already in progress\"")));
    eMap.insert(IndexElement_t(NSAPI_ERROR_IS_CONNECTED, std::string("\"socket is already connected\"")));
    eMap.insert(IndexElement_t(NSAPI_ERROR_CONNECTION_LOST, std::string("\"connection lost\"")));
    eMap.insert(IndexElement_t(NSAPI_ERROR_CONNECTION_TIMEOUT, std::string("\"connection timed out\"")));
    eMap.insert(IndexElement_t(NSAPI_ERROR_ADDRESS_IN_USE, std::string("\"Address already in use\"")));
    eMap.insert(IndexElement_t(NSAPI_ERROR_TIMEOUT, std::string("\"operation timed out\"")));    
    return eMap;
}

static ErrorCodesMap_t gs_ErrorCodesMap = make_error_codes_map();

inline std::string ToString(const nsapi_error_t & key)
{
    std::string result;

    // Prevent the possibility of std::out_of_range exception if the container
    // does not have an error element with the specified key.
    auto iter = gs_ErrorCodesMap.find(key);     
    if (iter != gs_ErrorCodesMap.end())
    {
        result = iter->second;
    }
    else
    {
        result = std::string("\"Warning! Code does not indicate an error and consequently does not exist in gs_ErrorCodesMap!\"");
    }
    
    return result;
}
    
namespace Utilities
{
    const auto GetNetworkInterfaceProfile = [](NetworkInterface * pInterface)
    {
        //printf("Running GetNetworkInterfaceProfile() lambda... \r\n");
     
        std::optional<const char *> ip(std::nullopt);
        std::optional<const char *> netmask(std::nullopt);
        std::optional<const char *> gateway(std::nullopt);
        std::optional<const char *> mac(std::nullopt);
        
        // Retrieve the network address(es):
        //printf("Running GetNetworkInterfaceProfile()::ip ... \r\n");        
        SocketAddress socketAddress;
        pInterface->get_ip_address(&socketAddress);
        ip = socketAddress.get_ip_address();

        //printf("Running GetNetworkInterfaceProfile()::netmask ... \r\n");                
        SocketAddress socketAddress1;
        pInterface->get_netmask(&socketAddress1);        
        netmask = socketAddress1.get_ip_address();
        
        //printf("Running GetNetworkInterfaceProfile()::gateway ... \r\n");
        SocketAddress socketAddress2;
        pInterface->get_gateway(&socketAddress2);
        gateway = socketAddress2.get_ip_address();
        
        // "Provided MAC address is intended for info or debug purposes
        // and may be not provided if the underlying network interface
        // does not provide a MAC address."
        mac = pInterface->get_mac_address();
        //printf("Ending GetNetworkInterfaceProfile() lambda... \r\n");
        
        return std::make_tuple(ip, netmask, gateway, mac);
    };
    
    const auto IsDomainNameAddress = [](const std::string & address)
    {
        bool result = false;

        if (!address.empty())
        {
            if (std::count_if(address.begin(), address.end(), [](unsigned char c){ return std::isalpha(c); } ) > 0)
            {
                result = true;
            }
        }
        return result;
    };

    const auto ResolveAddressIfDomainName = [](const std::string & address
                                             , NetworkInterface * pInterface
                                             , SocketAddress * pTheSocketAddress)
    {
        std::optional<std::string> ipAddress(std::nullopt);

        if (!address.empty())
        {
            if (IsDomainNameAddress(address))
            {
                // Note that the MBED_ASSERT macro is only available in the Debug 
                // and Development build profiles and not in the Release build profile. 
                MBED_ASSERT(pInterface);

                printf("Performing DNS lookup for : \"%s\" ...\n", address.c_str());
                nsapi_error_t retVal = pInterface->gethostbyname(address.c_str(), pTheSocketAddress);
                if (retVal < 0)
                {
                    printf("Error! On DNS lookup, Network returned: [%d] -> %s\n", retVal, ToString(retVal).c_str());
                }

                ipAddress = pTheSocketAddress->get_ip_address();
            }
            else
            {
                // Assume that we are already dealing with an IP address. i.e.,
                // as the designer of this application, I am mandating that the
                // user can choose to specify IP addresses directly in the mbed_app.json
                // echo-server-hostname field. This will facilitate testing with
                // locally hosted Echo Servers, which by necessity, do not have 
                // DNS names. 
                
                // In which case the following is how we can ensure that the requisite 
                // SocketAddress object that is normally created during FQDN resolution,
                // is manually constructed and propagated back to the User:
                SocketAddress tempSockAddress(address.c_str(), 0);
                *pTheSocketAddress = tempSockAddress;

                ipAddress = address.c_str();                
            }
        }

        return ipAddress;
    };
} //end of namespace

