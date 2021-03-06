#ifndef UUID_3260390C_7569_4E66_B6BA_CC5CC7E58F9A
#define UUID_3260390C_7569_4E66_B6BA_CC5CC7E58F9A
#pragma once

#include "../item.hpp"
#include "../../source.hpp"
#include "../../fixed_string.hpp"
#include <boost/endian/arithmetic.hpp>

namespace Neptools
{
namespace Stcm
{

class HeaderItem;
class ExportsItem final : public Item
{
public:
    enum Type : uint32_t
    {
        CODE = 0,
        DATA = 1,
    };

    struct Entry
    {
        boost::endian::little_uint32_t type;
        FixedString<0x20> name;
        boost::endian::little_uint32_t offset;

        void Validate(FilePosition file_size) const;
    };
    NEPTOOLS_STATIC_ASSERT(sizeof(Entry) == 0x28);

    ExportsItem(Key k, Context* ctx, Source src, uint32_t export_count);
    static ExportsItem& CreateAndInsert(ItemPointer ptr, uint32_t export_count);

    FilePosition GetSize() const noexcept override
    { return sizeof(Entry) * entries.size(); }

    struct EntryType
    {
        Type type;
        FixedString<0x20> name;
        const Label* lbl;
    };
    std::vector<EntryType> entries;

    void Dispose() noexcept override;

private:
    void Dump_(Sink& sink) const override;
    void Inspect_(std::ostream& os) const override;
    void Parse_(Source& src, uint32_t export_count);
};

}
}
#endif
