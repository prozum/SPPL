#pragma once

#include "Location.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace common {

enum class TypeId {
    UNKNOWN,
    INT,
    FLOAT,
    CHAR,
    BOOL,
    STRING,
    LIST,
    TUPLE,
    SIGNATURE,
    GENERIC,
    EMPTYLIST,
    CUSTOM
};

class Type {
  public:
    TypeId Id;
    std::vector<Type> Subtypes;
    std::string Name;
    Location Loc;

    Type();
    ~Type();
    Type(TypeId Id);
    Type(TypeId Id, size_t SubtypeCount);
    Type(TypeId Id, Location Loc);
    Type(TypeId Id, std::vector<Type> Subtypes);
    Type(TypeId Id, std::vector<Type> Subtypes, Location Loc);
    Type(TypeId Id, std::vector<Type> Subtypes, std::string Name);
    Type(TypeId Id, std::vector<Type> Subtypes, std::string Name, Location Loc);

    bool operator==(const Type &Other) const;
    bool operator!=(const Type &Other) const;
    Type operator[](const size_t Index);

    std::string str(bool isTop = false);
    std::string strJoin(std::string JoinStr);

    size_t subtypeCount();
private:
    size_t SubtypeCount = 0;
};
}

namespace std {
template <> struct hash<common::Type> {
    std::size_t operator()(const common::Type &Ty) const {
        size_t Res = hash<int>()(static_cast<int>(Ty.Id));
        Res ^= hash<size_t>()(Ty.Subtypes.size()) << 1;

        for (auto &Subtype : Ty.Subtypes) {
            Res ^= hash<common::Type>()(Subtype) << 1;
        }

        return Res;
    }
};
}
