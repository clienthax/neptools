#ifndef UUID_3260390C_7569_4E66_B6BA_CC5CC7E58F9A
#define UUID_3260390C_7569_4E66_B6BA_CC5CC7E58F9A
#pragma once

#include "../item.hpp"
#include "../../source.hpp"
#include "../../fixed_string.hpp"
#include <boost/endian/arithmetic.hpp>

namespace Stcm
{

class HeaderItem;
class ExportsItem final : public Item
{
public:
    struct Entry
    {
        boost::endian::little_uint32_t field_0;
        FixedString<0x20> name;
        boost::endian::little_uint32_t offset;

        void Validate(FilePosition file_size) const;
    };
    STATIC_ASSERT(sizeof(Entry) == 0x28);

    ExportsItem(Key k, Context* ctx, Source src, uint32_t export_count);
    static ExportsItem* CreateAndInsert(ItemPointer ptr, uint32_t export_count);

    void Dump(std::ostream& os) const override;
    void PrettyPrint(std::ostream& os) const override;
    FilePosition GetSize() const noexcept override
    { return sizeof(Entry) * entries.size(); }

    using EntryType = std::pair<FixedString<0x20>, const Label*>;
    std::vector<EntryType> entries;

private:
    void Parse_(Source& src, uint32_t export_count);
};

}

#endif
