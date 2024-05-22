/*-----------------------------------------------------------/
/ FlashParam.h
/------------------------------------------------------------/
/ Copyright (c) 2024, Elehobica
/ Released under the BSD-2-Clause
/ refer to https://opensource.org/licenses/BSD-2-Clause
/-----------------------------------------------------------*/

#pragma once

#include <map>
#include <string>
#include <cinttypes>  // this must be located at later than <string>
#include <variant>

#include "UserFlash.h"

namespace FlashParamNs {
//=================================
// Interface of Parameter class
//=================================
template <class T>
class Parameter {
    using valueType = T;
public:
    Parameter(const uint32_t& id, const char* name, const uint32_t& flashAddr, const valueType& defaultValue, size_t& size);
    Parameter(const uint32_t& id, const char* name, const uint32_t& flashAddr, const valueType& defaultValue);
    void set(valueType value_) { value = value_; }
    valueType get() const { return value; }
private:
    Parameter(const Parameter&) = delete;
    Parameter& operator=(const Parameter&) = delete;  // don't permit copy
    void loadDefault() { value = defaultValue; }
    const uint32_t id;
    const char* name;
    const uint32_t flashAddr;
    const valueType defaultValue;
    const size_t size;
    valueType value = defaultValue;
    friend class Params;
    friend class FlashParam;
    friend class ReadFromFlashVisitor;
    friend class WriteReserveVisitor;
    friend class PrintInfoVisitor;
};

using variant_t = std::variant<
    Parameter<bool>*,
    Parameter<uint8_t>*, Parameter<uint16_t>*, Parameter<uint32_t>*, Parameter<uint64_t>*,
    Parameter<int8_t>*, Parameter<int16_t>*, Parameter<int32_t>*, Parameter<int64_t>*,
    Parameter<float>*, Parameter<double>*,
    Parameter<char*>*>;

//=================================
// Interface of Visitors
//=================================
struct FlashVisitor {
    UserFlash& userFlash = UserFlash::instance();
};

struct ReadFromFlashVisitor : FlashVisitor {
    template <typename T>
    void operator()(const T& param) const {
        userFlash.read(param->flashAddr, param->size, param->value);
    }
};

struct WriteReserveVisitor : FlashVisitor {
    template <typename T>
    void operator()(const T& param) const {
        userFlash.writeReserve(param->flashAddr, param->size, param->value);
    }
};

struct PrintInfoVisitor {
    template <typename T>
    void operator()(T& param) const {
        const variant_t item = param;
        const auto& format = printFormat.at(item.index());
        printf(format.c_str(), param->flashAddr, param->name, param->value, param->value);
    }
    const std::array<std::string, 12> printFormat = {  // this must be matched with variant order due to being referred by index()
        "0x%04x %s: %" PRIi32 "d (0x%" PRIx32 ")\n",  // bool
        "0x%04x %s: %" PRIu8 "d (0x%" PRIx8 ")\n",    // uint8_t
        "0x%04x %s: %" PRIu16 "d (0x%" PRIx16 ")\n",  // uint16_t
        "0x%04x %s: %" PRIu32 "d (0x%" PRIx32 ")\n",  // uint32_t
        "0x%04x %s: %" PRIu64 "d (0x%" PRIx64 ")\n",  // uint64_t
        "0x%04x %s: %" PRIi8 "d (0x%" PRIx8 ")\n",    // int8_t
        "0x%04x %s: %" PRIi16 "d (0x%" PRIx16 ")\n",  // int16_t
        "0x%04x %s: %" PRIi32 "d (0x%" PRIx32 ")\n",  // int32_t
        "0x%04x %s: %" PRIi64 "d (0x%" PRIx64 ")\n",  // int64_t
        "0x%04x %s: %7.4f (%7.4e)\n",                 // float
        "0x%04x %s: %7.4f (%7.4e)\n",                 // double
        "0x%04x %s: %s\n",                            // char*
    };
};

//=================================
// Interface of Params class
//=================================
class Params
{
// all private except for friend classes
    static Params& instance(); // Singleton
    void printInfo() const;
    void loadDefault();
    void loadFromFlash();
    void storeToFlash() const;
    template <typename T>
    void add(const uint32_t& id, T* paramPtr) { paramMap[id] = paramPtr; }
    template <typename T>
    T& getParam(const uint32_t& id) {
        auto& item = paramMap.at(id);
        auto& paramPtr = std::get<T*>(item);
        return *paramPtr;
    }
    std::map<const uint32_t, variant_t> paramMap;
    template<typename> friend class Parameter;  // for all Parameter<> classes
    friend class FlashParam;
};

//=================================
// Interface of FlashParam class
//=================================
class FlashParam
{
public:
    static FlashParam& instance(); // Singleton
    void printInfo() const;
    void finalize() const;
    /*
    // accessor by Parameter<> instance on template T = Parameter<>  --> use directly .set(), .get()
    template <typename T>
    decltype(auto) getValue(const T& param) const { return param.get(); }
    template <typename T>
    void setValue(T& param, const typename T::valueType value) { param.set(value); }
    */
    // accessor by id on template T = primitive type
    template <typename T>
    decltype(auto) getValue(const uint32_t& id) const { return _getValue<Parameter<T>>(id); }
    template <typename T>
    void setValue(const uint32_t& id, const T& value) { _setValue<Parameter<T>>(id, value); }

protected:
    FlashParam() = default;
    ~FlashParam() = default;
    FlashParam(const FlashParam&) = delete;
    FlashParam& operator=(const FlashParam&) = delete;
    void loadDefault();
    void loadFromFlash();
    // accessor by uint32_t on template T = Patameter<>
    template <typename T>
    void _setValue(const uint32_t& id, const typename T::valueType& value) {
        auto& param = Params::instance().getParam<T>(id);
        return param.set(value);
    }
    template <typename T>
    decltype(auto) _getValue(const uint32_t& id) const {
        const auto& param = Params::instance().getParam<T>(id);
        return param.get();
    }
};
}
