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
#include <iomanip>
#include <ostream>
#include <sstream>
#include <map>
#include <string>
#include <chrono>

#include "nsapi_types.h"
#include "EthernetInterface.h"

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
    return (gs_ErrorCodesMap.at(key));
}
    
template <typename T, typename U>
struct TrueTypesEquivalent : std::is_same<typename std::decay_t<T>, U>::type
{};

template <typename E>
constexpr auto ToUnderlyingType(E e) -> typename std::underlying_type<E>::type
{
    return static_cast<typename std::underlying_type<E>::type>(e);
}

template <typename E, typename V = unsigned long>
constexpr auto ToEnum(V value) -> E
{
    return static_cast<E>(value);
}
    
namespace Utilities
{
    // TBD Nuertey Odzeyem; remove these clock functions if not needed in
    // the LightControl application. Likewise any other superfluous functions
    // such as IntegerToHex(). Note that Domain... functions might be needed.
    
    // This custom clock type obtains the time from RTC too whilst noting the Processor speed.
    struct NucleoF767ZIClock_t
    {
        using rep        = std::int64_t;
        using period     = std::ratio<1, 216'000'000>; // Processor speed, 1 tick == 4.62962963ns
        using duration   = std::chrono::duration<rep, period>;
        using time_point = std::chrono::time_point<NucleoF767ZIClock_t>;
        static constexpr bool is_steady = true;

        // This method/approach has been proven to work. Yay!
        static time_point now() noexcept
        {
            return from_time_t(time(NULL));
        }

        // This method/approach has been proven to work. Yay!
        static std::time_t to_time_t(const time_point& __t) noexcept
        {
            return std::time_t(std::chrono::duration_cast<Seconds_t>(__t.time_since_epoch()).count());
        }

        // This method/approach has been proven to work. Yay!
        static time_point from_time_t(std::time_t __t) noexcept
        {
            typedef std::chrono::time_point<NucleoF767ZIClock_t, Seconds_t> __from;
            return std::chrono::time_point_cast<NucleoF767ZIClock_t::duration>(__from(Seconds_t(__t)));
        }
    };

    // Now you can say things like:
    //
    // auto t = Utility::NucleoF767ZIClock_t::now();  // a chrono::time_point
    //
    // and
    //
    // auto d = Utility::NucleoF767ZIClock_t::now() - t;  // a chrono::duration

    // This custom clock type obtains the time from the 'chip-external' Real Time Clock (RTC).
    template <typename Clock_t = SystemClock_t>
    const auto Now = []()
    {
        time_t seconds = time(NULL);
        typename Clock_t::time_point tempTimepoint = Clock_t::from_time_t(seconds);

        return tempTimepoint;
    };

    const auto WhatTimeNow = []()
    {
        char buffer[32];
        time_t seconds = time(NULL);
        //printf("\r\nTime as seconds since January 1, 1970 = %lld", seconds);
        //printf("\r\nTime as a basic string = %s", ctime(&seconds));
        std::size_t len = std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&seconds));
        std::string timeStr(buffer, len);

        //printf("\r\nDebug len :-> [%d]", len);
        //printf("\r\nDebug timeStr :-> [%s]", timeStr.c_str());

        return timeStr;
    };

    const auto SecondsToString = [](const time_t& seconds)
    {
        char buffer[32];
        std::size_t len = std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&seconds));
        std::string timeStr(buffer, len);

        return timeStr;
    };
    
    template <typename T>
    constexpr auto TruncateAndToString = [](const T& x, const int& decimalDigits = 2)
    {
        std::ostringstream oss;  
        oss << std::fixed;
        oss.precision(decimalDigits);
        oss << x;
        return oss.str();
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
                                             , NetworkInterface * pInterface)
    {
        std::optional<std::string> ipAddress(std::nullopt);

        if (!address.empty())
        {
            if (IsDomainNameAddress(address))
            {
                // Note that the MBED_ASSERT macro is only available in the Debug 
                // and Development build profiles and not in the Release build profile. 
                MBED_ASSERT(pInterface);
                
                SocketAddress serverSocketAddress;

                printf("\r\nPerforming DNS lookup for : \"%s\" ...", address.c_str());
                nsapi_error_t retVal = pInterface->gethostbyname(address.c_str(), &serverSocketAddress);
                if (retVal < 0)
                {
                    printf("\r\nError! On DNS lookup, Network returned: [%d] -> %s", retVal, ToString(retVal).c_str());
                }

                // No need to do the explicit construction of std::string
                // as we are emplace'ing:
                ipAddress.emplace(serverSocketAddress.get_ip_address());
            }
        }

        return ipAddress;
    };

    template <typename T>
    std::string IntegerToHex(const T& i)
    {
        std::stringstream stream;
        stream << std::showbase << std::setfill('0') 
               << std::setw(sizeof(T)*2) 
               << std::hex 
               << std::uppercase 
               << i;
        return stream.str();
    }

    template <typename T>
    std::string IntegerToDec(const T& i)
    {
        std::stringstream stream;
        stream << std::dec << std::to_string(i);
        return stream.str();
    }
} //end of namespace

